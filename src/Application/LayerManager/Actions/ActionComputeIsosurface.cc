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

// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionComputeIsosurface.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ComputeIsosurface )

namespace Seg3D
{

bool ActionComputeIsosurface::validate( Core::ActionContextHandle& context )
{
  // Check whether the layer exists and is of the right type and return an
  // error if not
  std::string error;
  if ( !( LayerManager::CheckLayerExistanceAndType( this->layer_id_.value(), 
    Core::VolumeType::MASK_E, error ) ) )
  {
    context->report_error( error );
    return false;
  }
  
  // Check whether the layer is not locked for processing or creating, in which case the
  // data is not data yet, hence we cannot compute an isosurface on it. The function returns
  // a notifier when the action can be completed.
  Core::NotifierHandle notifier;
  if ( !( LayerManager::CheckLayerAvailabilityForUse( this->layer_id_.value(), 
     notifier ) ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  // Check for valid quality factor
  double quality_factor = this->quality_factor_.value();
  if( !( quality_factor == 1.0 || quality_factor == 0.5 || 
    quality_factor == 0.25 || quality_factor == 0.125 ) )
  {
    return false;
  }

  return true; // validated
}

bool ActionComputeIsosurface::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  MaskLayerHandle mask_layer = LayerManager::FindMaskLayer( this->layer_id_.value() );
  mask_layer->compute_isosurface( this->quality_factor_.value(), this->capping_enabled_.value() );

  /*
  Hide the abort message (if aborted).  This is a workaround for the fact that this action is
  not run on a separate algorithm thread, so the LayerWidget::trigger_abort() posts an action to
  show the abort message and this event is put on the event queue and not processed until this
  run function returns.  The problem is that the last event processed is the one showing the 
  abort message, so the abort message never goes away.  We work around this by posting our own
  hide abort message event (via the Action dispatch mechanism) that will get placed in the queue
  after the show message.  

  Normally for other filters the abort message is hidden when the layer's data_state_ changes, 
  e.g. from PROCESSING_C to AVAILABLE_C.  That state change does happen in 
  MaskLayer::compute_isosurface(), but then the action to show the abort message 
  gets processed afterward due to the scenario described above.

  Put another way, normally the "Waiting for process to abort message" is displayed while we wait
  for the filter algorithm thread to abort.  In this case there is no separate algorithm thread
  to wait for because the isosurface computation is done directly on the application thread.
  */
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    mask_layer->show_abort_message_state_, false );

  return true;
}

Core::ActionHandle ActionComputeIsosurface::Create( MaskLayerHandle mask_layer, 
  double quality_factor, bool capping_enabled )
{
  ActionComputeIsosurface* action = new ActionComputeIsosurface;

  action->layer_id_.value() = mask_layer->get_layer_id();
  action->quality_factor_.value() = quality_factor;
  action->capping_enabled_.value() = capping_enabled;

  return Core::ActionHandle( action );
}

void ActionComputeIsosurface::Dispatch( Core::ActionContextHandle context, 
  MaskLayerHandle mask_layer, double quality_factor, bool capping_enabled )
{
  Core::ActionDispatcher::PostAction( Create( mask_layer, quality_factor, capping_enabled ), 
    context );
}

void ActionComputeIsosurface::Dispatch( Core::ActionContextHandle context )
{
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( !active_layer || active_layer->get_type() != Core::VolumeType::MASK_E )
  {
    return;
  }
  
  double quality;
  bool capping_enabled;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    Core::ImportFromString( active_layer->get_layer_group()->isosurface_quality_state_->get(), quality );
    capping_enabled = active_layer->get_layer_group()->isosurface_capping_enabled_state_->get();
  }
  Dispatch( context, boost::dynamic_pointer_cast< MaskLayer >( active_layer ), quality, 
    capping_enabled );
}

} // end namespace Seg3D
