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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/State/State.h>
#include <Core/State/StateEngine.h>
#include <Core/Volume/Volume.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Tool/GroupTargetTool.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class GroupTargetToolPrivate
//////////////////////////////////////////////////////////////////////////
  
class GroupTargetToolPrivate
{

public:
  GroupTargetToolPrivate() :
    target_type_( Core::VolumeType::DATA_E ),
      tool_( 0 )
    {}

  // -- handle updates from layermanager --
  void handle_layer_inserted( LayerHandle layer, bool new_group );
  void handle_layers_deleted( std::vector< std::string > group_ids, bool groups_deleted );
  void handle_active_layer_changed( LayerHandle layer );
  void handle_layer_name_changed( std::string layer_id );
  void handle_layer_data_changed( LayerHandle layer );

  // -- handle updates from state variables --
  void handle_use_active_group_changed( bool use_active_group );
  void handle_target_group_changed( std::string group_id );
  void handle_target_layers_changed();

  // -- Helper functions --
public:
  // UPDATE_GROUP_LIST:
  // Update the target group option list.
  void update_group_list();

  // UPDATE_LAYER_LIST:
  // Update the target layers option list.
  void update_layer_list();
  
public:
  // The type of layer that can be used with this filter
  Core::VolumeType target_type_;
  
  // Pointer back to the tool.
  // NOTE: This can be a pointer, as the callbacks are deleted when the tool is deleted and all
  // the tool components run on the same thread.
  GroupTargetTool* tool_;
};

void GroupTargetToolPrivate::handle_layer_inserted( LayerHandle layer, bool new_group )
{
  // Store the current target group, so later we can check if the target group
  // has changed.
  std::string old_target_group = this->tool_->target_group_state_->get();
  bool target_changed = false;

  // If a new group has been added, update the group list first
  if ( new_group )
  {
    this->update_group_list();
    target_changed = ( old_target_group != this->tool_->target_group_state_->get() );
  }
  
  // If the layer belongs to the current target group, update the layer list as well
  if ( !target_changed && layer->get_layer_group()->get_group_id() == 
    this->tool_->target_group_state_->get() )
  {
    this->update_layer_list();
  }
}

void GroupTargetToolPrivate::handle_layers_deleted( std::vector< std::string > group_ids,
                           bool groups_deleted )
{
  // Store the current target group, so later we can check if the target group
  // has changed.
  std::string old_target_group = this->tool_->target_group_state_->get();
  bool target_changed = false;

  // If groups have been deleted, update the group list
  if ( groups_deleted )
  {
    this->update_group_list();
    target_changed = ( old_target_group != this->tool_->target_group_state_->get() );
  }
  
  // Update the layer list only if the target hasn't been changed. Otherwise, the layer
  // list should have already been updated by handle_target_group_changed.
  if ( !target_changed )
  {
    // If the target group is affected
    if ( std::find( group_ids.begin(), group_ids.end(), old_target_group ) != group_ids.end() )
    {
      this->update_layer_list();
    }
  }
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

  this->update_layer_list();

  // Select the active layer
  if ( this->tool_->use_active_group_state_->get() )
  {
    std::vector< std::string > selected_layers;
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
    if ( active_layer )
    {
      selected_layers.push_back( active_layer->get_layer_id() );
      this->tool_->target_layers_state_->set( selected_layers );
    }   
  }
}

void GroupTargetToolPrivate::handle_target_layers_changed()
{
  this->tool_->valid_target_state_->set( this->tool_->target_layers_state_->get().size() > 0 );
}

void GroupTargetToolPrivate::handle_layer_name_changed( std::string layer_id )
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id );
  if ( !layer ) return;

  LayerGroupHandle layer_group = layer->get_layer_group();
  if ( layer_group->get_group_id() == this->tool_->target_group_state_->get() &&
    ( layer->get_type() & this->target_type_ ) )
  {
    this->update_layer_list();
  } 
}

void GroupTargetToolPrivate::handle_layer_data_changed( LayerHandle layer )
{
  LayerGroupHandle layer_group = layer->get_layer_group();
  if ( layer_group && 
    layer_group->get_group_id() == this->tool_->target_group_state_->get() &&
    ( layer->get_type() & this->target_type_ ) &&
    layer->data_state_->get() != Layer::CREATING_C )
  {
    this->update_layer_list();
  }
}

void GroupTargetToolPrivate::update_group_list()
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

void GroupTargetToolPrivate::update_layer_list()
{
  std::string group_id = this->tool_->target_group_state_->get();
  std::vector< LayerIDNamePair > layer_names;
  if ( group_id != "" && group_id != Tool::NONE_OPTION_C )
  {
    LayerGroupHandle group = LayerManager::Instance()->find_group( group_id );
    group->get_layer_names( layer_names, this->target_type_ );
  }
  this->tool_->target_layers_state_->set_option_list( layer_names );
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

  this->target_group_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 10 );
  this->use_active_group_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 20 );

  // Adding connections to handle updates
  this->add_connection( LayerManager::Instance()->layer_inserted_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_layer_inserted, this->private_, _1, _2 ) ) );
  this->add_connection( LayerManager::Instance()->layers_deleted_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_layers_deleted, this->private_, _2, _3 ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_active_layer_changed, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layer_name_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_layer_name_changed, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layer_data_changed_signal_.connect(
    boost::bind( &GroupTargetToolPrivate::handle_layer_data_changed, this->private_, _1 ) ) );

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
  this->private_->update_group_list();
}

GroupTargetTool::~GroupTargetTool()
{
}

} // end namespace Seg3D
