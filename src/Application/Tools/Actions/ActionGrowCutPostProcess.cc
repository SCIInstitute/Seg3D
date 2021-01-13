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
#include <itkConnectedComponentImageFilter.h>
#include <itkSliceBySliceImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <Application/Layer/Actions/ActionComputeIsosurface.h>

// Core includes
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Utils/Log.h>
#include <Core/Log/LogbookManager.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/ITKFilter.h>
#include <Applicatoin/Tools/ActionGrowCutPostProcess.h>
#include <Application/Tools/Algorithm/IslandRemoval.h>

//Remove later if not used
//#include <Corview/Utils/CrvUtils.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, GrowCutPostProcess )

namespace Seg3D
{

ActionGrowCutPostProcess::ActionGrowCutPostProcess()
{
  this->add_layer_id( this->target_layer_ );
  this->add_parameter( this->replace_ );
  this->add_parameter( this->sandbox_ );
}

bool ActionGrowCutPostProcess::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) { return false; }

  // Check for MRA layer existence and type information
  if ( !LayerManager::CheckLayerExistenceAndType( this->target_layer_,
                                                  Core::VolumeType::MASK_E,
                                                  context, this->sandbox_ ) )
  { return false; }

  // Check for layer availability
  if ( !LayerManager::CheckLayerAvailability( this->target_layer_, this->replace_, context ) ) { return false; }

  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class GrowCutPostProcessFilterAlgo : public ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;
  Core::ActionContextHandle context_;

public:

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.

  virtual void run_filter()
  {

    Core::ITKUCharImageDataHandle src_image;
    this->get_itk_image_from_layer<unsigned char>( this->src_layer_, src_image );

    this->dst_layer_->update_progress_signal_( 0.1 );
    if ( this->check_abort() )
    {
      return;
    }

    try {

      // step 1 : smoothing
      typedef itk::BinaryMedianImageFilter< UCHAR_IMAGE_TYPE, UCHAR_IMAGE_TYPE > SmoothType;
      SmoothType::Pointer smooth_filter = SmoothType::New();
      smooth_filter->SetInput( src_image->get_image() );
      UCHAR_IMAGE_TYPE::SizeType radius;
      radius[0] = 4;
      radius[1] = 4;
      radius[2] = 0;
      smooth_filter->SetRadius( radius );
      smooth_filter->SetForegroundValue( 1 );
      smooth_filter->SetBackgroundValue( 0 );
      smooth_filter->Update();
      this->dst_layer_->update_progress_signal_( 0.33 );

      // step 2 : island removal
      typedef Seg3D::IslandRemovalFilter< UCHAR_IMAGE_TYPE_2D, UCHAR_IMAGE_TYPE_2D > IslandRemovalFilterType;
      typedef itk::SliceBySliceImageFilter< UCHAR_IMAGE_TYPE, UCHAR_IMAGE_TYPE > UcharSliceBySliceType;

      IslandRemovalFilterType::Pointer island_filter = IslandRemovalFilterType::New();
      UcharSliceBySliceType::Pointer slice_filter = UcharSliceBySliceType::New();
      island_filter->SetIslandSize( 35 );
      slice_filter->SetDimension( 2 );
      slice_filter->SetFilter( island_filter );
      slice_filter->SetInput( smooth_filter->GetOutput() );
      slice_filter->Update();
      this->dst_layer_->update_progress_signal_( 0.66 );

      // step 3 : hole filling
      typedef itk::InvertIntensityImageFilter< UCHAR_IMAGE_TYPE > InvertType;
      InvertType::Pointer invert = InvertType::New();
      invert->SetInput( slice_filter->GetOutput() );
      invert->SetMaximum( 1 );
      invert->Update();

      IslandRemovalFilterType::Pointer island_filter2 = IslandRemovalFilterType::New();
      UcharSliceBySliceType::Pointer slice_filter2 = UcharSliceBySliceType::New();
      island_filter2->SetIslandSize( 35 );
      slice_filter2->SetDimension( 2 );
      slice_filter2->SetFilter( island_filter2 );
      slice_filter2->SetInput( invert->GetOutput() );
      slice_filter2->Update();
      this->dst_layer_->update_progress_signal_( 0.99 );

      InvertType::Pointer invert2 = InvertType::New();
      invert2->SetInput( slice_filter2->GetOutput() );
      invert2->SetMaximum( 1 );
      invert2->Update();

      this->dst_layer_->update_progress_signal_( 1.0 );

      this->insert_itk_image_into_layer( this->dst_layer_, invert2->GetOutput() );

      this->dispatch_unlock_layer( this->dst_layer_ );

      // re-create the isosurface for the result
      LayerGroupHandle group = this->dst_layer_->get_layer_group();
      double quality = 1.0;
      Core::ImportFromString( group->isosurface_quality_state_->get(), quality );
      bool capping_enabled = group->isosurface_capping_enabled_state_->get();

      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( this->dst_layer_ );

      ActionComputeIsosurface::Dispatch( this->context_, mask_layer, quality, capping_enabled, true );
    }
    catch ( std::exception& e )
    {
      std::cerr << "Uncaught exception during Grow Cut Post Processing: " << e.what() << std::endl;
      return;
    }

    if ( this->check_abort() )
    {
      return;
    }

    this->dst_layer_->update_progress_signal_( 1.0 );
  }

  // GET_FILTER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "GrowCutPostProcess Tool";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const
  {
    return std::string( "GrowCutPostProcess" );
  }
};

bool ActionGrowCutPostProcess::run( Core::ActionContextHandle& context,
                                    Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< GrowCutPostProcessFilterAlgo > algo( new GrowCutPostProcessFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );

  // Find the handle to the MRA layer
  LayerHandle src_layer;
  algo->find_layer( this->target_layer_, src_layer );
  algo->src_layer_ = boost::dynamic_pointer_cast< MaskLayer >( src_layer );
  algo->context_ = context;

  if ( this->replace_ )
  {
    algo->dst_layer_ = algo->src_layer_;
    // Lock the layer, so it cannot be used else where
    algo->lock_for_processing( src_layer );
  }
  else
  {
    // Lock the src layer, so it cannot be used else where
    algo->lock_for_use( algo->src_layer_ );

    // Create the destination layer, which will show progress
    algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
  }

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
       context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}

void ActionGrowCutPostProcess::Dispatch( Core::ActionContextHandle context, std::string layer, bool replace /*= false*/ )
{
  // Create a new action
  ActionGrowCutPostProcess* action = new ActionGrowCutPostProcess;

  // Setup the parameters
  action->target_layer_ = layer;
  action->replace_ = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
} // end namespace Seg3D
