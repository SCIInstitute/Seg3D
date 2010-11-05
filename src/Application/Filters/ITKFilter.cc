/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
 
// Boost includes
#include <boost/signals.hpp> 
 
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

class ITKFilterPrivate : public Core::Lockable, public Core::ConnectionHandler
{
public:
  // Pointer to the filter base class
  ITKFilter* base_;

  // Pointer to the itk filter class
  itk::ProcessObject::Pointer filter_;
  
public:
  // Function for handling abort
  void handle_abort();
};

void ITKFilterPrivate::handle_abort()
{
  lock_type lock( this->get_mutex() );
  
  if ( this->base_ )
  {
    this->filter_->SetAbortGenerateData( true );
    this->base_->raise_abort();
  }
}

ITKFilter::ITKFilter() :
  private_( new ITKFilterPrivate )
{
  this->private_->base_ = this;
  this->private_->filter_ = 0;
}

ITKFilter::~ITKFilter()
{
  ITKFilterPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->disconnect_all();
  this->private_->filter_ = 0;
  this->private_->base_ = 0;
}

class ITKProgressObserver : public itk::Command
{
  // -- constructor /destructor --
public:
  ITKProgressObserver( LayerHandle layer, float progress_start, float progress_amount ) :
    layer_( layer ),
    progress_( 0.0f ),
    progress_start_ ( progress_start ),
    progress_amount_ ( progress_amount )
  {}

  // NOTE: Virtual destructor is needed for ITK
  virtual ~ITKProgressObserver()
  {}
  
  // Overloaded ITK function that is called when progress is reported
  virtual void Execute(itk::Object *caller, const itk::EventObject & event )
  {
    const itk::ProcessObject* obj = dynamic_cast< itk::ProcessObject* >( caller );
    if ( obj )
    {
      std::string event_name = event.GetEventName(); 
      if ( event_name == "ProgressEvent" )
      {
        float progress = ( obj->GetProgress() * this->progress_amount_ ) + 
          this->progress_start_;
        if ( progress > ( this->progress_ + 0.01f ) )
        {
          this->progress_ = progress;
          this->layer_->update_progress_signal_( this->progress_ );
        }
      }
    }
  }

  // Overloaded ITK function that is called when progress is reported
  virtual void Execute(const itk::Object *caller, const itk::EventObject & event )
  {
    const itk::ProcessObject* obj = dynamic_cast<const itk::ProcessObject* >( caller );
    if ( obj )
    {
      std::string event_name = event.GetEventName(); 
      if ( event_name == "ProgressEvent" )
      {
        float progress = obj->GetProgress();
        if ( progress > ( this->progress_ + 0.01f ) )
        {
          this->progress_ = progress;
          this->layer_->update_progress_signal_( this->progress_ );
        }
      }
    }   
  }

private:
  // Handle to the layer that is displaying progress
  LayerHandle layer_;
  
  float progress_;
  float progress_start_;
  float progress_amount_;

};

void ITKFilter::observe_itk_filter_internal( itk::ProcessObject::Pointer filter, 
  const LayerHandle& layer, float progress_start, float progress_amount )
{
  // Setup progress measuring, by forwarding progress to the filter
  filter->AddObserver( itk::ProgressEvent() , new ITKProgressObserver( layer, progress_start,
    progress_amount ) );

  ITKFilterPrivate::lock_type lock( this->private_->get_mutex() );

  this->private_->disconnect_all();
  this->private_->filter_ = filter;
  this->private_->add_connection( layer->abort_signal_.connect( boost::bind(
    &ITKFilterPrivate::handle_abort, this->private_ ) ) );
} 

void ITKFilter::limit_number_of_itk_threads_internal( itk::ProcessObject::Pointer filter )
{
  // Assume we will have a minimum of 2 threads. As we subtract one this will ensure that
  // there is at least one thread doing the computation.
  unsigned int max_threads = boost::thread::hardware_concurrency();
  if ( max_threads < 2 ) max_threads = 2;
  
  filter->GetMultiThreader()->SetGlobalMaximumNumberOfThreads( max_threads - 1 );
}


} // end namespace Core
