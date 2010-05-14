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

#include <algorithm>

#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PaintTool.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

// Register the tool into the tool factory
SCI_REGISTER_TOOL(PaintTool)

PaintTool::PaintTool( const std::string& toolid ) :
  Tool( toolid ),
  signal_block_count_( 0 )
{
  // Need to set ranges and default values for all parameters

  this->add_state( "target_", this->target_layer_state_, Tool::NONE_OPTION_C );
  this->add_state( "mask_", this->mask_layer_state_, Tool::NONE_OPTION_C );

  this->add_state( "target", this->target_layer_name_state_, Tool::NONE_OPTION_C );
  this->add_state( "data_constraint", this->data_constraint_layer_name_state_, 
    Tool::NONE_OPTION_C );
  this->add_state( "mask_constraint", this->mask_constraint_layer_name_state_,
    Tool::NONE_OPTION_C );
  this->add_state( "brush_radius", this->brush_radius_state_, 23, 1, 250, 1 );
  this->add_state( "upper_threshold", this->upper_threshold_state_, 1.0, 00.0, 1.0, 0.01 );
  this->add_state( "lower_threshold", this->lower_threshold_state_, 0.0, 00.0, 1.0, 0.01 );
  this->add_state( "erase", this->erase_state_, false );
  
  this->handle_layers_changed();

  // Add constaints, so that when the state changes the right ranges of
  // parameters are selected
  //this->add_connection ( this->target_layer_state_->value_changed_signal_.connect( 
  //  boost::bind( &PaintTool::target_constraint, this, _1 ) ) );
  //this->add_connection ( this->mask_layer_state_->value_changed_signal_.connect( boost::bind( &PaintTool::mask_constraint,
  //    this, _1 ) ) );

  this->add_connection( this->target_layer_name_state_->value_changed_signal_.connect(
    boost::bind( &PaintTool::update_constraint_options, this ) ) );
  
  this->add_connection ( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &PaintTool::handle_layers_changed, this ) ) );

}

PaintTool::~PaintTool()
{
  this->disconnect_all();
}

void PaintTool::handle_layers_changed()
{
  this->update_target_options();
  this->update_constraint_options();
}

void PaintTool::update_target_options()
{
  this->target_layer_options_.clear();
  Tool::CreateLayerNameList( this->target_layer_options_, Core::VolumeType::MASK_E );

  this->target_layer_options_.push_back( Tool::NONE_OPTION_C );

  if ( std::find( this->target_layer_options_.begin(), this->target_layer_options_.end(), 
    this->target_layer_name_state_->get() ) == this->target_layer_options_.end() )
  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->target_layer_name_state_->set( this->target_layer_options_[ 0 ] );
  }
}

void PaintTool::update_constraint_options()
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  this->data_constraint_options_.clear();
  this->mask_constraint_options_.clear();

  if ( this->target_layer_name_state_->get() != Tool::NONE_OPTION_C )
  {
    LayerHandle target_layer = LayerManager::Instance()->
      get_layer_by_name( this->target_layer_name_state_->get() );
    if ( !target_layer )
    {
      CORE_THROW_LOGICERROR( std::string( "Layer named " ) + 
        this->target_layer_name_state_->get() + " does not exist" );
    }

    LayerGroupHandle layer_group = target_layer->get_layer_group();
    Tool::CreateLayerNameList( this->data_constraint_options_, 
      Core::VolumeType::DATA_E, layer_group );
    Tool::CreateLayerNameList( this->mask_constraint_options_, 
      Core::VolumeType::MASK_E, layer_group, this->target_layer_name_state_->get() );
  }

  this->data_constraint_options_.push_back( Tool::NONE_OPTION_C );
  this->mask_constraint_options_.push_back( Tool::NONE_OPTION_C );

  if ( std::find( this->data_constraint_options_.begin(), this->data_constraint_options_.end(), 
    this->data_constraint_layer_name_state_->get() ) == this->data_constraint_options_.end() )
  {
    this->data_constraint_layer_name_state_->set( this->data_constraint_options_.back() );
  }

  if ( std::find( this->mask_constraint_options_.begin(), this->mask_constraint_options_.end(), 
    this->mask_constraint_layer_name_state_->get() ) == this->mask_constraint_options_.end() )
  {
    this->mask_constraint_layer_name_state_->set( this->mask_constraint_options_.back() );
  }
}

void PaintTool::target_constraint( std::string layerid )
{
}

void PaintTool::mask_constraint( std::string layerid )
{
}

void PaintTool::activate()
{
}

void PaintTool::deactivate()
{
}

} // end namespace Seg3D
