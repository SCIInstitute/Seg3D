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

// Core includes
#include <Core/Math/MathFunctions.h>
#include <Core/Utils/TimeSince.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Layer/Actions/ActionComputeIsosurface.h>

#include <Application/Tools/Actions/ActionGrowCut.h>
#include <Application/Tools/Actions/ActionGrowCutPostProcess.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, GrowCut )

namespace Seg3D
{

bool ActionGrowCut::validate( Core::ActionContextHandle& context )
{
  // Check for layer existence and type information
  if ( !LayerManager::CheckLayerExistenceAndType( this->data_layer_,
                                                  Core::VolumeType::DATA_E, context ) ) { return false; }

  // Check for layer availability
  if ( !LayerManager::CheckLayerAvailability( this->data_layer_,
                                              false, context ) ) { return false; }

  // Check for layer existence and type information foreground layer
  if ( !LayerManager::CheckLayerExistenceAndType( this->foreground_layer_,
                                                  Core::VolumeType::MASK_E, context ) ) { return false; }

  // Check for layer existence and type information background layer
  if ( !LayerManager::CheckLayerExistenceAndType( this->background_layer_,
                                                  Core::VolumeType::MASK_E, context ) ) { return false; }

  // Check for layer existence and type information output layer
  if ( !LayerManager::CheckLayerExistenceAndType( this->output_layer_,
                                                  Core::VolumeType::MASK_E, context ) ) { return false; }

  // Check whether foreground and data have the same size
  if ( !LayerManager::CheckLayerSize( this->foreground_layer_, this->data_layer_,
                                      context ) ) { return false; }

  // Check whether background and data have the same size
  if ( !LayerManager::CheckLayerSize( this->background_layer_, this->data_layer_,
                                      context ) ) { return false; }

  // Check whether output and data have the same size
  if ( !LayerManager::CheckLayerSize( this->output_layer_, this->data_layer_,
                                      context ) ) { return false; }

  // Check for layer availability foreground layer
  if ( !LayerManager::CheckLayerAvailability( this->foreground_layer_,
                                              false, context ) ) { return false; }

  // Check for layer availability background layer
  if ( !LayerManager::CheckLayerAvailability( this->background_layer_,
                                              false, context ) ) { return false; }

  // Check for layer availability output layer
  if ( !LayerManager::CheckLayerAvailability( this->output_layer_,
                                              false, context ) ) { return false; }

  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class GrowCutAlgo : public ITKFilter
{

public:
  LayerHandle data_layer_;
  LayerHandle foreground_layer_;
  LayerHandle background_layer_;
  LayerHandle output_layer_;

  GrowCutterHandle grow_cutter_;

  bool run_smoothing_;

  Core::ActionContextHandle context_;

public:
  // RUN:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.

  virtual void run_filter()
  {
    Core::TimeSince::start_timer( "growcut" );
    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.

    Core::ITKImageDataT<short>::Handle data_image;
    this->get_itk_image_from_layer<short>( this->data_layer_, data_image );

    Core::ITKImageDataT<unsigned char>::Handle foreground_image;
    this->get_itk_image_from_layer<unsigned char>( this->foreground_layer_, foreground_image );

    Core::ITKImageDataT<unsigned char>::Handle background_image;
    this->get_itk_image_from_layer<unsigned char>( this->background_layer_, background_image );

    if ( !this->grow_cutter_ )
    {
      this->grow_cutter_ = GrowCutterHandle( new GrowCutter() );
      std::cerr << " New grow cutter handle" << std::endl;
    }

    this->output_layer_->update_progress_signal_( 0.1 );

    this->grow_cutter_->set_foreground_image( foreground_image->get_image() );
    this->grow_cutter_->set_background_image( background_image->get_image() );
    this->grow_cutter_->set_data_image( data_image->get_image() );

    this->output_layer_->update_progress_signal_( 0.2 );

    this->grow_cutter_->execute();

    this->output_layer_->update_progress_signal_( 1.0 );

    this->insert_itk_image_into_layer( this->output_layer_, this->grow_cutter_->get_output().GetPointer() );

    CORE_LOG_SUCCESS( "GrowCut duration: " +
                      Core::TimeSince::format_double( Core::TimeSince::get_time_since( "growcut" ) / 1000, 2 ) + "s" );

    this->dispatch_unlock_layer( this->output_layer_ );

    if ( this->run_smoothing_ )
    {
      ActionGrowCutPostProcess::Dispatch( this->context_, this->output_layer_->get_layer_id(), true );
    }
    else
    {
      // re-create the isosurface for the result
      LayerGroupHandle group = this->output_layer_->get_layer_group();
      double quality = 1.0;
      Core::ImportFromString( group->isosurface_quality_state_->get(), quality );
      bool capping_enabled = group->isosurface_capping_enabled_state_->get();

      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( this->output_layer_ );
      ActionComputeIsosurface::Dispatch( this->context_, mask_layer, quality, capping_enabled, true );
    }
  }

  // GET_FILTER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "GrowCut";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const
  {
    return "GrowCut";
  }
};

bool ActionGrowCut::run( Core::ActionContextHandle& context,
                         Core::ActionResultHandle& result )
{

  // Create algorithm
  boost::shared_ptr<GrowCutAlgo> algo( new GrowCutAlgo );

  // Find the handle to the layers
  if ( !( algo->find_layer( this->data_layer_, algo->data_layer_ ) ) )
  {
    return false;
  }
  if ( !( algo->find_layer( this->foreground_layer_, algo->foreground_layer_ ) ) )
  {
    return false;
  }
  if ( !( algo->find_layer( this->background_layer_, algo->background_layer_ ) ) )
  {
    return false;
  }

  if ( !( algo->find_layer( this->output_layer_, algo->output_layer_ ) ) )
  {
    return false;
  }

  // Lock the layers, so they cannot be used else where
  algo->lock_for_use( algo->data_layer_ );
  algo->lock_for_use( algo->foreground_layer_ );
  algo->lock_for_use( algo->background_layer_ );
  algo->lock_for_processing( algo->output_layer_ );

  algo->grow_cutter_ = this->grow_cutter_;
  algo->run_smoothing_ = this->run_smoothing_;
  algo->context_ = context;

  result = Core::ActionResultHandle( new Core::ActionResult( algo->output_layer_->get_layer_id() ) );

  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
       context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionGrowCut::Dispatch( Core::ActionContextHandle context, std::string data_layer,
                              std::string foreground_layer, std::string background_layer,
                              std::string output_layer, bool run_smoothing, GrowCutterHandle grow_cutter )
{
  // Create a new action
  ActionGrowCut* action = new ActionGrowCut;

  // Setup the parameters
  action->data_layer_ = data_layer;
  action->foreground_layer_ = foreground_layer;
  action->background_layer_ = background_layer;
  action->output_layer_ = output_layer;
  action->grow_cutter_ = grow_cutter;
  action->run_smoothing_ = run_smoothing;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionGrowCut::DispatchAndWait( Core::ActionContextHandle context, std::string data_layer,
                                     std::string foreground_layer, std::string background_layer,
                                     std::string output_layer, bool run_smoothing,
                                     GrowCutterHandle grow_cutter )
{
  // Create a new action
  ActionGrowCut* action = new ActionGrowCut;

  // Setup the parameters
  action->data_layer_ = data_layer;
  action->foreground_layer_ = foreground_layer;
  action->background_layer_ = background_layer;
  action->output_layer_ = output_layer;
  action->grow_cutter_ = grow_cutter;
  action->run_smoothing_ = run_smoothing;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAndWaitAction( Core::ActionHandle( action ), context );
}
} // end namespace Seg3D
