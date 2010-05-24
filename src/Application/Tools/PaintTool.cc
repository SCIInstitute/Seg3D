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
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PaintTool.h>

namespace Seg3D
{

// Register the tool into the tool factory
SCI_REGISTER_TOOL(PaintTool)

PaintTool::PaintTool( const std::string& toolid ) :
  Tool( toolid ),
  signal_block_count_( 0 )
{
  // Need to set ranges and default values for all parameters

  std::vector< LayerIDNamePair > empty_names( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  this->add_state( "target", this->target_layer_state_, 
    Tool::NONE_OPTION_C, empty_names );
  this->add_state( "data_constraint", this->data_constraint_layer_state_, 
    Tool::NONE_OPTION_C, empty_names );
  this->add_state( "mask_constraint", this->mask_constraint_layer_state_,
    Tool::NONE_OPTION_C, empty_names );

  this->add_state( "brush_radius", this->brush_radius_state_, 23, 1, 250, 1 );
  this->add_state( "upper_threshold", this->upper_threshold_state_, 1.0, 00.0, 1.0, 0.01 );
  this->add_state( "lower_threshold", this->lower_threshold_state_, 0.0, 00.0, 1.0, 0.01 );
  this->add_state( "erase", this->erase_state_, true );
  
  this->handle_layers_changed();

  this->add_connection( this->target_layer_state_->value_changed_signal_.connect(
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
  std::vector< LayerIDNamePair > mask_layer_names;
  mask_layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  LayerManager::Instance()->get_layer_names( mask_layer_names, Core::VolumeType::MASK_E );

  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->target_layer_state_->set_option_list( mask_layer_names );
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
    if ( active_layer && active_layer->type() == Core::VolumeType::MASK_E )
    {
      this->target_layer_state_->set( active_layer->get_layer_id() );
    }
  }
}

void PaintTool::update_constraint_options()
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  std::vector< LayerIDNamePair > mask_layer_names;
  mask_layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  std::vector< LayerIDNamePair > data_layer_names;
  data_layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  if ( this->target_layer_state_->get() != Tool::NONE_OPTION_C )
  {
    LayerHandle target_layer = LayerManager::Instance()->
      get_layer_by_id( this->target_layer_state_->get() );
    if ( !target_layer )
    {
      CORE_THROW_LOGICERROR( std::string( "Layer " ) + 
        this->target_layer_state_->get() + " does not exist" );
    }

    LayerGroupHandle layer_group = target_layer->get_layer_group();
    layer_group->get_layer_names( mask_layer_names, Core::VolumeType::MASK_E,
      target_layer );
    layer_group->get_layer_names( data_layer_names, Core::VolumeType::DATA_E );
  }

  this->data_constraint_layer_state_->set_option_list( data_layer_names );
  this->mask_constraint_layer_state_->set_option_list( mask_layer_names );
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
