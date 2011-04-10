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
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tool/GroupTargetTool.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class GroupTargetToolPrivate
//////////////////////////////////////////////////////////////////////////
  
class GroupTargetToolPrivate
{

public:
  // The type of layer that can be used with this filter
  Core::VolumeType target_type_;
  
  // Pointer back to the tool.
  // NOTE: This can be a pointer, as the callbacks are deleted when the tool is deleted and all
  // the tool components run on the same thread.
  GroupTargetTool* tool_;
  
  // -- handle updates from layermanager --
  void handle_groups_changed();
  void handle_layers_changed( LayerGroupHandle group );
  void handle_active_layer_changed( LayerHandle layer );
  void handle_layer_name_changed( std::string layer_id );
  
  // -- handle updates from state variables --
  void handle_use_active_group_changed( bool use_active_group );
  void handle_target_group_changed( std::string group_id );
  void handle_target_layers_changed();
  
  GroupTargetToolPrivate() :
    target_type_( Core::VolumeType::DATA_E ),
    tool_( 0 )
  {}
};

void GroupTargetToolPrivate::handle_groups_changed()
{
  std::vector< LayerGroupHandle > groups;
  LayerManager::Instance()->get_groups( groups );
  std::vector< Core::OptionLabelPair > group_names;
  for ( size_t i = 0; i < groups.size(); ++i )
  {
    const Core::GridTransform& grid_trans = groups[ i ]->get_grid_transform();
    std::string group_name = Core::ExportToString( grid_trans.get_nx() ) + " x " +
      Core::ExportToString( grid_trans.get_ny() ) + " x " + 
      Core::ExportToString( grid_trans.get_nz() );
    group_names.push_back( std::make_pair( groups[ i ]->get_group_id(), group_name ) );
  }
  this->tool_->target_group_state_->set_option_list( group_names );
}

void GroupTargetToolPrivate::handle_layers_changed( LayerGroupHandle group )
{
  if ( group->get_group_id() != this->tool_->target_group_state_->get() )
  {
    return;
  }

  std::vector< LayerIDNamePair > layer_names;
  group->get_layer_names( layer_names, this->target_type_ );
  this->tool_->target_layers_state_->set_option_list( layer_names );
}

void GroupTargetToolPrivate::handle_active_layer_changed( LayerHandle layer )
{
  if ( !this->tool_->use_active_group_state_->get() || !layer )
  {
    return;
  }

  LayerGroupHandle active_group = layer->get_layer_group();
  this->tool_->target_group_state_->set( active_group->get_group_id() );
}

void GroupTargetToolPrivate::handle_use_active_group_changed( bool use_active_group )
{
  if ( use_active_group )
  {
    LayerHandle layer = LayerManager::Instance()->get_active_layer();
    if ( layer )
    {
      LayerGroupHandle active_group = layer->get_layer_group();
      this->tool_->target_group_state_->set( active_group->get_group_id() );
    }
  }
}

void GroupTargetToolPrivate::handle_target_group_changed( std::string group_id )
{
  // If use_active_group_state_ is set to true, make sure that group_id is the same
  // as the current active group
  if ( this->tool_->use_active_group_state_->get() )
  {
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
    if ( active_layer )
    {
      LayerGroupHandle active_group = active_layer->get_layer_group();
      if ( active_group->get_group_id() != group_id )
      {
        this->tool_->target_group_state_->set( active_group->get_group_id() );
        return;
      }
    }
  }

  std::vector< LayerIDNamePair > layer_names;
  std::vector< std::string > selected_layers;
  if ( group_id != "" && group_id != Tool::NONE_OPTION_C )
  {
    LayerGroupHandle group = LayerManager::Instance()->get_group_by_id( group_id );
    group->get_layer_names( layer_names, this->target_type_ );
    if ( this->tool_->use_active_group_state_->get() )
    {
      LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
      if ( active_layer )
      {
        selected_layers.push_back( active_layer->get_layer_id() );
      }   
    } 
  }
  this->tool_->target_layers_state_->set_option_list( layer_names );
  if ( selected_layers.size() > 0 )
  {
    this->tool_->target_layers_state_->set( selected_layers );
  }
}

void GroupTargetToolPrivate::handle_target_layers_changed()
{
  this->tool_->valid_target_state_->set( this->tool_->target_layers_state_->get().size() > 0 );
}

void GroupTargetToolPrivate::handle_layer_name_changed( std::string layer_id )
{
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( layer_id );
  LayerGroupHandle layer_group = layer->get_layer_group();
  if ( layer_group->get_group_id() == this->tool_->target_group_state_->get() &&
    ( layer->get_type() & this->target_type_ ) )
  {
    this->handle_layers_changed( layer_group );
  } 
}


//////////////////////////////////////////////////////////////////////////
// Class GroupTargetTool
//////////////////////////////////////////////////////////////////////////

GroupTargetTool::GroupTargetTool(  Core::VolumeType target_type, const std::string& tool_type ) :
  Tool( tool_type ),
  private_( new GroupTargetToolPrivate )
{
  // The target type that we allow for the input
  this->private_->target_type_ = target_type;
  this->private_->tool_ = this;
  
  std::vector< std::string > empty_option;
      
  // Add the states of this class to the StateEngine    
  this->add_state( "target_group", this->target_group_state_, "", "" );
  this->add_state( "target_layers", this->target_layers_state_, empty_option, "" );
  this->add_state( "use_active_group", this->use_active_group_state_, true ); 
  this->add_state( "valid_layer", this->valid_target_state_, false );

  // Adding connections to handle updates
  this->add_connection( LayerManager::Instance()->groups_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_groups_changed, this->private_ ) ) );
  this->add_connection( LayerManager::Instance()->group_internals_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_layers_changed, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_active_layer_changed, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layer_name_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_layer_name_changed, this->private_, _1 ) ) );

  this->add_connection( this->use_active_group_state_->value_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_use_active_group_changed, 
    this->private_, _1 ) ) );
  this->add_connection( this->target_group_state_->value_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_target_group_changed, this->private_, _2 ) ) );
  this->add_connection( this->target_layers_state_->state_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_target_layers_changed, this->private_ ) ) );

  // Handle the updates to the StateEngine
  // As tools are created on the application thread, the state engine should not change underneath
  // it, as every change to the state engine has to go through the same thread.
  this->private_->handle_groups_changed();
}

GroupTargetTool::~GroupTargetTool()
{
}

} // end namespace Seg3D
