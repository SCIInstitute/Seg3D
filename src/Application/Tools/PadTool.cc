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

#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PadTool.h>

#include <Application/Filters/Utils/PadValues.h>
#include <Application/Filters/Actions/ActionPadFilter.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, PadTool )

namespace Seg3D
{

using namespace Filter;

class PadToolPrivate
{
public:

  // UPDATE_DST_GROUP_LIST:
  // Update list of destination groups that can be chosen by the user.
  void update_dst_group_list();

  void handle_output_dimension_changed( int index );
  void handle_constraint_aspect_changed( bool constraint );
  void handle_layer_groups_changed( bool changed );

  size_t signal_block_count_;
  PadTool* tool_;
};

void PadToolPrivate::update_dst_group_list()
{
  const std::string& group_id = this->tool_->target_group_state_->get();
  std::vector< Core::OptionLabelPair > dst_groups;
  if ( group_id.empty() || group_id == Tool::NONE_OPTION_C )
  {
    this->tool_->dst_group_state_->set_option_list( dst_groups );
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  LayerGroupHandle layer_group = LayerManager::Instance()->find_group( group_id );

  std::vector< LayerGroupHandle > layer_groups;
  LayerManager::Instance()->get_groups( layer_groups );
  for ( size_t i = 0; i < layer_groups.size(); ++i )
  {
    if ( layer_groups[ i ] == layer_group )
    {
      continue;
    }
    const Core::GridTransform& grid_trans = layer_groups[ i ]->get_grid_transform();
    std::string group_name = Core::ExportToString( grid_trans.get_nx() ) + " x " +
                             Core::ExportToString( grid_trans.get_ny() ) + " x " +
                             Core::ExportToString( grid_trans.get_nz() );
    dst_groups.push_back( std::make_pair( layer_groups[ i ]->get_group_id(), group_name ) );
  }
  this->tool_->dst_group_state_->set_option_list( dst_groups );
}

void PadToolPrivate::handle_output_dimension_changed( int index )
{
  if ( this->signal_block_count_ > 0 || this->tool_->target_group_state_->get() == "" )
  {
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  if ( this->tool_->constraint_aspect_state_->get() )
  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );
    this->tool_->pad_level_state_[ ( index + 1 ) % 3 ]->set( this->tool_->pad_level_state_[ index ]->get() );
    this->tool_->pad_level_state_[ ( index + 2 ) % 3 ]->set( this->tool_->pad_level_state_[ index ]->get() );
  }
}

void PadToolPrivate::handle_constraint_aspect_changed( bool constraint )
{
  if ( constraint ) this->handle_output_dimension_changed( 0 );
}

void PadToolPrivate::handle_layer_groups_changed( bool changed )
{
  if ( changed )
  {
    this->update_dst_group_list();
  }
}

//////////////////////////////////////////////////////////////////////////
// Class PadTool
//////////////////////////////////////////////////////////////////////////

PadTool::PadTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_REGULAR_E, toolid ),
  SCALE_MAX_C(2.0),
  private_( new PadToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->signal_block_count_ = 0;

  this->add_state( "input_x", this->input_dimensions_state_[ 0 ], 0 );
  this->add_state( "input_y", this->input_dimensions_state_[ 1 ], 0 );
  this->add_state( "input_z", this->input_dimensions_state_[ 2 ], 0 );

  this->add_state( "dst_group", this->dst_group_state_, "", "" );

  std::vector< Core::OptionLabelPair > padding_values;
  padding_values.push_back( std::make_pair( PadValues::ZERO_C, "0" ) );
  padding_values.push_back( std::make_pair( PadValues::MIN_C, "Minimum Value" ) );
  padding_values.push_back( std::make_pair( PadValues::MAX_C, "Maximum Value" ) );
  this->add_state( "pad_value", this->padding_value_state_, PadValues::ZERO_C, padding_values );

  this->add_state( "output_x", this->pad_level_state_[ 0 ], 1, 1, 50, 1 );
  this->add_state( "output_y", this->pad_level_state_[ 1 ], 1, 1, 50, 1 );
  this->add_state( "output_z", this->pad_level_state_[ 2 ], 1, 1, 50, 1 );
  this->add_state( "constraint_aspect", this->constraint_aspect_state_, true );
  this->add_state( "replace", this->replace_state_, false );

  // Need to load dimensions after constraint_aspect_state_ is loaded so that scale isn't
  // incorrectly used to overwrite output dimensions
  for ( size_t i = 0; i < 3; ++i )
  {
    this->pad_level_state_[ i ]->set_session_priority( Core::StateBase::LOAD_LAST_E );
    this->add_connection( this->pad_level_state_[ i ]->value_changed_signal_.connect(
      boost::bind( &PadToolPrivate::handle_output_dimension_changed, this->private_, i ) ) );
  }

  this->add_connection( this->constraint_aspect_state_->value_changed_signal_.connect(
    boost::bind( &PadToolPrivate::handle_constraint_aspect_changed, this->private_, _1 ) ) );
  // If groups were added or removed, update the destination group list in the tool interface
  this->add_connection( LayerManager::Instance()->layer_inserted_signal_.connect(
    boost::bind( &PadToolPrivate::handle_layer_groups_changed, this->private_, _2 ) ) );
  this->add_connection( LayerManager::Instance()->layers_deleted_signal_.connect(
    boost::bind( &PadToolPrivate::handle_layer_groups_changed, this->private_, _3 ) ) );
}

PadTool::~PadTool()
{
  this->disconnect_all();
}

void PadTool::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Reverse the order of target layers
  std::vector< std::string > target_layers = this->target_layers_state_->get();
  std::reverse( target_layers.begin(), target_layers.end() );

  Core::Vector pad_level( this->pad_level_state_[ 0 ]->get(),
                          this->pad_level_state_[ 1 ]->get(),
                          this->pad_level_state_[ 2 ]->get() );

  ActionPadFilter::Dispatch( context,
                             target_layers,
                             pad_level,
                             this->padding_value_state_->get(),
                             this->replace_state_->get()
                           );
}

} // end namespace Seg3D
