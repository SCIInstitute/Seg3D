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
#include <Application/Tool/ITKFilterAlgo.h>

 
namespace Seg3D
{

ITKFilterAlgo::ITKFilterAlgo()
{
}

ITKFilterAlgo::~ITKFilterAlgo()
{
}


class ITKProgress : public itk::Command
{

  // -- constructor /destructor --
public:
  ITKProgress( LayerHandle layer ) :
    layer_( layer )
  {}

  // NOTE: Virtual destructor is needed for ITK
  virtual ~ITKProgress()
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
        layer_->update_progress_signal_( obj->GetProgress() );
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
        layer_->update_progress_signal_( obj->GetProgress() );
      }
    }   
  }

private:
  // Handle to the layer that is displaying progress
  LayerHandle layer_;

};
    
void ITKFilterAlgo::forward_progress_internal( itk::ProcessObject::Pointer filter, LayerHandle layer )
{
  itk::Command::Pointer observer = new ITKProgress( layer );
  filter->AddObserver( itk::ProgressEvent() , observer );
} 

} // end namespace Core
