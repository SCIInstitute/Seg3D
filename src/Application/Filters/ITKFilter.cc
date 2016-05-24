/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */
 
// ITK includes 
#include <itkCommand.h>
 
// Core includes
#include <Core/Utils/Exception.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/ITKDataBlock.h>
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Filters/ITKFilter.h>

 
namespace Seg3D
{

// CLASS ITKPROGRESSREPORTER:
//
// This class keeps track of the progress made in a filter


class ITKProgressReporter;
typedef boost::shared_ptr<ITKProgressReporter> ITKProgressReporterHandle;

class ITKProgressReporter
{
public:
  ITKProgressReporter( LayerHandle layer, float progress_start, float progress_amount ) :
    layer_( layer ),
    progress_( 0.0 ),
    progress_start_( progress_start ),
    progress_amount_( progress_amount )
  {}

  // Destination layer for progress
  LayerHandle layer_;

  // Keep track of current progress, itk is sometimes inconsistent and will not uniformly increase
  // progress. Hence we keep track of the furthest progress reported so far
  float progress_;
  float progress_start_;
  float progress_amount_;
  
public:
  static void Report( ITKProgressReporterHandle reporter, const itk::Object* itk_object );
};

void ITKProgressReporter::Report( ITKProgressReporterHandle reporter, const itk::Object* itk_object )
{
  const itk::ProcessObject* obj = dynamic_cast<const itk::ProcessObject* >( itk_object );
  if ( obj )
  {
    float progress = ( obj->GetProgress() * reporter->progress_amount_ ) + 
      reporter->progress_start_;
    if ( progress > ( reporter->progress_ + 0.01f ) )
    {
      reporter->progress_ = progress;
      reporter->layer_->update_progress_signal_( reporter->progress_ );
    }
  }
}

// CLASS ITKOBSERVER:
//
// This object is installed in an itk filter so it can call functor objects when events
// happen such as completion of an iteration or reporting of progress

class ITKObserver : public itk::Command
{
  // -- constructor /destructor --
public:
  ITKObserver( boost::function< void ( itk::Object* ) > function ) :
    function_( function )
  {}

  // NOTE: Virtual destructor is needed for ITK
  virtual ~ITKObserver()
  {}
  
  // Overloaded ITK function that is called when progress is reported
  virtual void Execute(itk::Object *caller, const itk::EventObject& event )
  {
    function_( caller );
  }

  // Overloaded ITK function that is called when progress is reported
  virtual void Execute(const itk::Object *caller, const itk::EventObject& event )
  {
  }
  
private:
  // Handle to the layer that is displaying progress
  boost::function< void ( itk::Object* ) > function_;
};


class ITKConstObserver : public itk::Command
{
  // -- constructor /destructor --
public:
  ITKConstObserver( boost::function< void ( const itk::Object* ) > function ) :
    function_( function )
  {}

  // NOTE: Virtual destructor is needed for ITK
  virtual ~ITKConstObserver()
  {}
  
  // Overloaded ITK function that is called when progress is reported
  virtual void Execute(itk::Object *caller, const itk::EventObject& event )
  {
    function_( caller );
  }

  // Overloaded ITK function that is called when progress is reported
  virtual void Execute(const itk::Object *caller, const itk::EventObject& event )
  {
    function_( caller );
  }

private:
  // Handle to the layer that is displaying progress
  boost::function< void ( const itk::Object* ) > function_;
};

class ITKFilterPrivate : public Core::Lockable, public Core::ConnectionHandler
{
public:
  // Pointer to the itk filter class
  itk::ProcessObject::Pointer filter_;
};

ITKFilter::ITKFilter() :
  private_( new ITKFilterPrivate )
{
  this->private_->filter_ = 0;
}

ITKFilter::~ITKFilter()
{
  ITKFilterPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->disconnect_all();
  this->private_->filter_ = 0;
}


void ITKFilter::observe_itk_progress_internal( itk::ProcessObject::Pointer filter, 
  const LayerHandle& layer, float progress_start, float progress_amount )
{
  // Setup a new reporter
  ITKProgressReporterHandle reporter( new ITKProgressReporter( layer, progress_start, 
    progress_amount ) );

  // Setup progress measuring, by forwarding progress to the filter
  filter->AddObserver( itk::ProgressEvent(), new ITKConstObserver( boost::bind( 
    &ITKProgressReporter::Report, reporter, _1 ) ) );
}

//void ITKFilter::observe_itk_iterations_internal( itk::ProcessObject::Pointer filter, 
//  boost::function< void( itk::Object* ) > iteration_fcn )
//{
//  // Setup progress measuring, by forwarding progress to the filter
//  filter->AddObserver( itk::IterationEvent(), new ITKObserver( iteration_fcn ) ); 
//}

void ITKFilter::observe_itk_iterations_internal( itk::Object::Pointer filter, 
                        boost::function< void( itk::Object* ) > iteration_fcn )
{
  // Setup progress measuring, by forwarding progress to the filter
  filter->AddObserver( itk::IterationEvent(), new ITKObserver( iteration_fcn ) ); 
}

void ITKFilter::forward_abort_to_filter_internal( itk::ProcessObject::Pointer filter, 
  LayerHandle layer )
{   
  // TODO: Most of this can be handled by the layer above --JGS

  // Setup forwarding of the abort signal to the itk filter
  ITKFilterPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->disconnect_all();
  this->private_->filter_ = filter;
  
  // NOTE: The following logic is already done by LayerFilter.
  //this->private_->add_connection( layer->abort_signal_.connect( boost::bind(
  //  &ITKFilter::raise_abort, this ) ) );
  //this->private_->add_connection( layer->stop_signal_.connect( boost::bind(
  //  &ITKFilter::raise_stop, this ) ) );
}

void ITKFilter::handle_abort()
{
  ITKFilterPrivate::lock_type lock( this->private_->get_mutex() );
  if ( this->private_->filter_.GetPointer() )
  {
    this->private_->filter_->SetAbortGenerateData( true );
  }
}

void ITKFilter::handle_stop()
{
  ITKFilterPrivate::lock_type lock( this->private_->get_mutex() );
  if ( this->private_->filter_.GetPointer() )
  {
    this->private_->filter_->SetAbortGenerateData( true );
  }
}

void ITKFilter::limit_number_of_itk_threads_internal( itk::ProcessObject::Pointer filter )
{
  // Assume we will have a minimum of 2 threads. As we subtract one this will ensure that
  // there is at least one thread doing the computation.
  unsigned int max_threads = boost::thread::hardware_concurrency();
  if ( max_threads < 2 ) max_threads = 2;
  
  filter->GetMultiThreader()->SetNumberOfThreads( max_threads - 1 );
}

} // end namespace Core
