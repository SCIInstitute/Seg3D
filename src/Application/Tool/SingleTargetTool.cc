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
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/State/State.h>
#include <Core/State/StateEngine.h>
#include <Core/Volume/Volume.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

class SingleTargetToolPrivate
{

public:
  // The type of layer that can be used with this filter
  int target_type_;
  
  // Pointer back to the tool.
  // NOTE: This can be a pointer, as the callbacks are deleted when the tool is deleted and all
  // the tool components run on the same thread.
  SingleTargetTool* tool_;
  
  // -- handle updates from layermanager --
  void handle_layers_changed();
  void handle_use_active_layer_changed( bool use_active_layer );
  
  // -- handle updates from state variables --
  void handle_active_layer_changed( LayerHandle layer );
  void handle_target_layer_changed( std::string layer_id );   
};


void SingleTargetToolPrivate::handle_active_layer_changed( LayerHandle layer )
{
  if ( ! this->tool_->use_active_layer_state_->get() )
  {
    return;
  }

  this->tool_->target_layer_state_->set( layer->type() & this->target_type_ ? 
    layer->get_layer_id() : Tool::NONE_OPTION_C );
  this->tool_->valid_target_state_->set( layer->type() & this->target_type_ );

}

void SingleTargetToolPrivate::handle_use_active_layer_changed( bool use_active_layer )
{
  if ( use_active_layer )
  {
    LayerHandle layer = LayerManager::Instance()->get_active_layer();
    this->tool_->target_layer_state_->set( ( layer && ( layer->type() & this->target_type_ ) ) ? 
      layer->get_layer_id() : Tool::NONE_OPTION_C );
    this->tool_->valid_target_state_->set( layer && ( layer->type() & this->target_type_ ) );
  }
}

void SingleTargetToolPrivate::handle_target_layer_changed( std::string layer_id )
{
  if ( this->tool_->use_active_layer_state_->get() )
  {
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
    if ( active_layer && ( active_layer->type() & this->target_type_ ) && 
      layer_id != active_layer->get_layer_id() )
    {
      this->tool_->target_layer_state_->set( active_layer->get_layer_id() );
      this->tool_->valid_target_state_->set( true );
      return;
    }

    if ( !active_layer || ( active_layer && !(active_layer->type() & this->target_type_) 
      && layer_id != Tool::NONE_OPTION_C ) )
    {
      this->tool_->target_layer_state_->set( Tool::NONE_OPTION_C );
      this->tool_->valid_target_state_->set( false );
      return;
    }
  }
  else
  {
    this->tool_->valid_target_state_->set( layer_id != Tool::NONE_OPTION_C );
  }
}

void SingleTargetToolPrivate::handle_layers_changed()
{
  std::vector< LayerIDNamePair > layer_names;
  layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  LayerManager::Instance()->get_layer_names( layer_names, Core::VolumeType::DATA_E );
  this->tool_->target_layer_state_->set_option_list( layer_names );
}


SingleTargetTool::SingleTargetTool(  Core::VolumeType target_type, 
  const std::string& tool_type, size_t version_number, bool auto_number ) :
  Tool( tool_type, version_number, auto_number ),
  private_( new SingleTargetToolPrivate )
{
  // The target type that we allow for the input
  this->private_->target_type_ = target_type;
  this->private_->tool_ = this;
  
  // Create a list of empty labels
  std::vector< LayerIDNamePair > empty_names( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
      
  // Add the states of this class to the StateEngine    
  this->add_state( "target", this->target_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_state( "use_active_layer", this->use_active_layer_state_, true ); 
  this->add_state( "valid_layer", this->valid_target_state_, false ); 

  // Handle the updates to the StateEngine
  // As tools are created on the application thread, the state engine should not change underneath
  // it, as every change to the state engine has to go through the same thread.
  this->private_->handle_layers_changed();
  
  // Adding connections to handle updates
  this->add_connection( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_layers_changed, this->private_.get() ) ) );
    
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_active_layer_changed, 
    this->private_.get(), _1 ) ) );
    
  this->add_connection( this->use_active_layer_state_->value_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_use_active_layer_changed, 
    this->private_.get(), _1 ) ) );
    
  this->add_connection( this->target_layer_state_->value_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_target_layer_changed, this->private_.get(), _2 ) ) );   
}

SingleTargetTool::~SingleTargetTool()
{
}

} // end namespace Seg3D
