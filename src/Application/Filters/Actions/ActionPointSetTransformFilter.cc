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

//ITK Includes
#include <itkResampleImageFilter.h>

// Core includes
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Math/MathFunctions.h>
#include <Core/Geometry/Point.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionPointSetTransformFilter.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/PointSetFilter.h>

#include <boost/lexical_cast.hpp>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, PointSetTransformFilter )

namespace Seg3D
{

  //////////////////////////////////////////////////////////////////////////
  // Class ActionTransformPrivate
  //////////////////////////////////////////////////////////////////////////

class ActionPointSetTransformPrivate
{
public:
  std::vector< std::string > layer_ids_;
  Core::Point origin_;
  Core::Vector spacing_;
  SandboxID sandbox_;

  std::string target_layer_;
  std::vector< double > matrix_params_;
  //std::string pointset_tool_id_;
};

typedef itk::Euler3DTransform< double > transform_type;

class PointSetTransformAlgo : public  ITKFilter
{

public:
  std::vector< LayerHandle > src_layers_;
  std::vector< LayerHandle > dst_layers_;
  LayerHandle target_layer_;
  std::vector< double > matrix_params_;
  //std::string pointset_tool_id_;

public:

  //void transform_data_layer( DataLayerHandle input, DataLayerHandle output );
  void transform_mask_layer( MaskLayerHandle fixed_layer,
    MaskLayerHandle moving_layer,
    MaskLayerHandle dst_layer );

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.

  SCI_BEGIN_TYPED_ITK_RUN(this->target_layer_->get_data_type())
  {
	  MaskLayerHandle input_fixed_layer =
		  boost::dynamic_pointer_cast<MaskLayer>(this->target_layer_);

	  for (size_t i = 0; i < this->src_layers_.size(); ++i)
	  {

		  MaskLayerHandle fixed_layer = input_fixed_layer;
		  MaskLayerHandle moving_layer =
			  boost::dynamic_pointer_cast<MaskLayer>(this->src_layers_[i]);

		  MaskLayerHandle dst_layer =
			  boost::dynamic_pointer_cast<MaskLayer>(this->dst_layers_[i]);

		  transform_type::Pointer transform = transform_type::New();

		  //PointSetFilterHandle pointset_tool = 
		  //  boost::dynamic_pointer_cast<PointSetFilter> (ToolManager::Instance()->get_tool(this->pointset_tool_id_));

		  //std::vector<double> matrix_params;
		  //{
		  //  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
		  //  matrix_params = pointset_tool->transform_matrix_state_->get();
		  //}


		  transform_type::ParametersType params =
			  transform_type::ParametersType(this->matrix_params_.size());

		  for (unsigned int i = 0; i < this->matrix_params_.size(); ++i)
		  {
			  params.SetElement(i, this->matrix_params_[i]);
		  }

		  transform->SetParameters(params);

		  typedef itk::LinearInterpolateImageFunction<
			  TYPED_IMAGE_TYPE,
			  double          >    InterpolatorType;

		  typedef itk::ResampleImageFilter<
			  TYPED_IMAGE_TYPE,
			  TYPED_IMAGE_TYPE > ResampleFilterType;

		  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
		  typename Core::ITKImageDataT<VALUE_TYPE>::Handle moving_image;
		  this->get_itk_image_from_layer<VALUE_TYPE>(moving_layer, moving_image);
		  resampler->SetInput(moving_image->get_image());
		  transform->GetInverse(transform);
		  resampler->SetTransform(transform);

		  typename Core::ITKImageDataT<VALUE_TYPE>::Handle fixed_image;
		  this->get_itk_image_from_layer<VALUE_TYPE>(fixed_layer, fixed_image);
		  resampler->SetSize(fixed_image->get_image()->GetLargestPossibleRegion().GetSize());
		  resampler->SetOutputOrigin(fixed_image->get_image()->GetOrigin());
		  resampler->SetOutputSpacing(fixed_image->get_image()->GetSpacing());
		  resampler->SetOutputDirection(fixed_image->get_image()->GetDirection());
		  resampler->SetDefaultPixelValue(0);

		  resampler->Update();

		  if (resampler->GetOutput() != NULL)
		  {
			  this->insert_itk_image_into_layer(dst_layer, resampler->GetOutput());
		  }
	  }
  }

  SCI_END_TYPED_ITK_RUN()

  // GET_FITLER_NAME:
  // This functions returns the name of the filter that is used in the error report.
  virtual std::string get_filter_name() const
  {
    return "PointSet Transform Tool";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "PointSetTransform";
  }

};

  bool ActionPointSetTransformFilter::validate(Core::ActionContextHandle& context)
  {
	  // Make sure that the sandbox exists
	  if (!LayerManager::CheckSandboxExistence(this->private_->sandbox_, context)) return false;

	  // Check for layer existence and type information
	  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
	  if (layer_ids.size() == 0)
	  {
		  context->report_error("No input layers specified");
		  return false;
	  }

	  Core::GridTransform grid_trans;
	  for (size_t i = 0; i < layer_ids.size(); ++i)
	  {
		  // Check for layer existence
		  LayerHandle layer = LayerManager::FindLayer(layer_ids[i], this->private_->sandbox_);
		  if (!layer)
		  {
			  context->report_error("Layer '" + layer_ids[i] + "' doesn't exist");
			  return false;
		  }

		  // Make sure that all the layers are in the same group
		  if (i == 0)
		  {
			  grid_trans = layer->get_grid_transform();
		  }
		  else if (grid_trans != layer->get_grid_transform())
		  {
			  context->report_error("Input layers do not belong to the same group");
			  return false;
		  }

		  // Check for layer availability 
		  if (!LayerManager::CheckLayerAvailability(layer_ids[i],
			  false, context, this->private_->sandbox_)) return false;
	  }

	  // Validation successful
	  return true;
  }


// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

ActionPointSetTransformFilter::ActionPointSetTransformFilter() :
private_( new ActionPointSetTransformPrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_layer_id( this->private_->target_layer_ );
  this->add_parameter( this->private_->sandbox_ );
}


bool ActionPointSetTransformFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<PointSetTransformAlgo> algo( new PointSetTransformAlgo );
  algo->set_sandbox( this->private_->sandbox_ );

  // Find the handle to the layer
  algo->target_layer_ = LayerManager::FindLayer( this->private_->target_layer_, this->private_->sandbox_ );

  //algo->pointset_tool_id_ = this->private_->pointset_tool_id_;
  algo->matrix_params_ = this->private_->matrix_params_;

  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
  algo->dst_layers_.resize( num_of_layers );
  std::vector< std::string > dst_layer_ids( num_of_layers );
  
  // Check whether the source layer was found
  if ( !algo->target_layer_ ) return false;

  for ( size_t j = 0; j < num_of_layers; ++j )
  {
    size_t i = num_of_layers - 1 - j;
    algo->find_layer( layer_ids[ i ], algo->src_layers_[ i ] );
    if ( !algo->src_layers_[i] ) return false;

      // Lock the src layer, so it cannot be used else where
    algo->lock_for_use( algo->src_layers_[ i ] );

    // Create the destination layer, which will show progress
    switch ( algo->target_layer_->get_type() )
    { 
    case Core::VolumeType::DATA_E:
      algo->create_and_lock_data_layer_from_layer( 
        algo->target_layer_, algo->dst_layers_[i] );
      break;
    case Core::VolumeType::MASK_E:
      algo->create_and_lock_mask_layer_from_layer( 
        algo->target_layer_, algo->dst_layers_[i], algo->src_layers_[ i ]->get_layer_name() );
      break;
    default:
      assert( false );
    }
  }

  // Return the id of the destination layer.

  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record for this action
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;

}

void ActionPointSetTransformFilter::Dispatch( Core::ActionContextHandle context,
  std::string target_layer, const std::vector< std::string >& layer_ids,
  const std::vector< double >& matrix_params )
{ 
  // Create a new action
  ActionPointSetTransformFilter* action = new ActionPointSetTransformFilter;

  // Setup the parameters
  action->private_->layer_ids_ = layer_ids;
  action->private_->target_layer_ = target_layer;
  //action->private_->pointset_tool_id_ = toolid;
  action->private_->matrix_params_ = matrix_params;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
