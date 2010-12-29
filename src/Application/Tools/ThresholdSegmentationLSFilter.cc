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
#include <Application/Filters/Actions/ActionThresholdSegmentationLSFilter.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ThresholdSegmentationLSFilter.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ThresholdSegmentationLSFilter )

namespace Seg3D
{

class ThresholdSegmentationLSFilterPrivate
{
public:
  void handle_target_layer_changed();

public:
  ThresholdSegmentationLSFilter* filter_;
};

void ThresholdSegmentationLSFilterPrivate::handle_target_layer_changed()
{
  std::string target_layer_id = this->filter_->target_layer_state_->get();

  if ( target_layer_id != Tool::NONE_OPTION_C )
  {
    DataLayerHandle data_layer = boost::dynamic_pointer_cast< DataLayer >(
      LayerManager::Instance()->get_layer_by_id( target_layer_id ) );

    double min_val = data_layer->get_data_volume()->get_data_block()->get_min();
    double max_val = data_layer->get_data_volume()->get_data_block()->get_max();

    this->filter_->lower_threshold_state_->set_range( min_val, max_val );
    this->filter_->upper_threshold_state_->set_range( min_val, max_val );
  }
}


ThresholdSegmentationLSFilter::ThresholdSegmentationLSFilter( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::DATA_E, toolid ),
  private_( new ThresholdSegmentationLSFilterPrivate )
{
  this->private_->filter_ = this;

  // Create an empty list of label options
  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  // add default values for the the states
  add_state( "seed_mask", this->seed_mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_dependent_layer_input( this->seed_mask_state_, Core::VolumeType::MASK_E, true );

  // add number of iterations
  add_state( "iterations", this->iterations_state_, 20, 1, 60000, 1 );

  // thresholds for the data layer
  double inf = std::numeric_limits< double >::infinity();    
  add_state( "upper_threshold", this->upper_threshold_state_, inf, -inf, inf, .01 );
  add_state( "lower_threshold", this->lower_threshold_state_, inf, -inf, inf, .01 );
  
  add_state( "curvature", this->curvature_state_, 1.0, 0.0, 10.0, 0.1 );
  add_state( "propagation", this->propagation_state_, 1.0, 0.0, 10.0, 0.1 );
  add_state( "edge", this->edge_state_, 0.0, 0.0, 10.0, 0.1 );
  
  this->add_connection( this->target_layer_state_->state_changed_signal_.connect(
    boost::bind( &ThresholdSegmentationLSFilterPrivate::handle_target_layer_changed, 
    this->private_ ) ) );
  
  this->private_->handle_target_layer_changed();
}

ThresholdSegmentationLSFilter::~ThresholdSegmentationLSFilter()
{
  this->disconnect_all();
}

void ThresholdSegmentationLSFilter::execute( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionThresholdSegmentationLSFilter::Dispatch( context,
    this->target_layer_state_->get(),
    this->seed_mask_state_->get(),
    this->iterations_state_->get(),
    this->upper_threshold_state_->get(),
    this->lower_threshold_state_->get(),
    this->curvature_state_->get(),
    this->propagation_state_->get(),
    this->edge_state_->get() );
}

} // end namespace Seg3D
