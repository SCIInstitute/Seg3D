/*
 For more information, please see: http://software.sci.utah.edu
 ArithmeticFilter
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

#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ArithmeticFilter.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Filters/Actions/ActionArithmeticFilter.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ArithmeticFilter )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ArithmeticFilterPrivate
//////////////////////////////////////////////////////////////////////////

class ArithmeticFilterPrivate
{
public:
  void handle_groups_changed();
  void handle_group_layers_changed( LayerGroupHandle group );
  void handle_active_layer_changed( LayerHandle layer );
  void handle_layer_name_changed( std::string layer_id );

  void handle_use_active_group_changed( bool use_active_group );
  void handle_target_group_changed( std::string group_id );
  void handle_input_layer_changed( int index );
  void handle_replace_changed( bool replace );
  void handle_output_type_changed( std::string type );

  void update_input_options( std::vector< LayerIDNamePair >& layer_names, int start_index = 0 );

  ArithmeticFilter* tool_;
  size_t signal_block_count_;
};

void ArithmeticFilterPrivate::handle_groups_changed()
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

void ArithmeticFilterPrivate::handle_group_layers_changed( LayerGroupHandle group )
{
  if ( group->get_group_id() != this->tool_->target_group_state_->get() )
  {
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  std::vector< LayerIDNamePair > layer_names;
  group->get_layer_names( layer_names, Core::VolumeType::ALL_E );
  this->update_input_options( layer_names, 0 );
}

void ArithmeticFilterPrivate::handle_active_layer_changed( LayerHandle layer )
{
  if ( !this->tool_->use_active_group_state_->get() || !layer )
  {
    return;
  }

  LayerGroupHandle active_group = layer->get_layer_group();
  this->tool_->target_group_state_->set( active_group->get_group_id() );
  this->tool_->input_layers_state_[ 0 ]->set( layer->get_layer_id() );
}

void ArithmeticFilterPrivate::handle_use_active_group_changed( bool use_active_group )
{
  if ( use_active_group )
  {
    LayerHandle layer = LayerManager::Instance()->get_active_layer();
    if ( layer )
    {
      LayerGroupHandle active_group = layer->get_layer_group();
      this->tool_->target_group_state_->set( active_group->get_group_id() );
      this->tool_->input_layers_state_[ 0 ]->set( layer->get_layer_id() );
    }
  }
}

void ArithmeticFilterPrivate::handle_target_group_changed( std::string group_id )
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

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  std::vector< LayerIDNamePair > layer_names;
  if ( group_id != "" && group_id != Tool::NONE_OPTION_C )
  {
    LayerGroupHandle group = LayerManager::Instance()->get_layer_group( group_id );
    group->get_layer_names( layer_names, Core::VolumeType::ALL_E );
  }
  this->update_input_options( layer_names, 0 );
}

void ArithmeticFilterPrivate::handle_input_layer_changed( int index )
{
  if ( this->signal_block_count_ > 0 ||
    index == ArithmeticFilter::NUMBER_OF_INPUTS_C - 1 )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );

  LayerGroupHandle group = LayerManager::Instance()->get_layer_group(
    this->tool_->target_group_state_->get() );
  std::vector< LayerIDNamePair > layer_names;
  group->get_layer_names( layer_names, Core::VolumeType::ALL_E );
  this->update_input_options( layer_names, index + 1 );
}

void ArithmeticFilterPrivate::update_input_options( 
  std::vector< LayerIDNamePair >& layer_names, 
  int start_index /*= 0 */ )
{
  // Remove all the layers selected by previous inputs
  for ( int i = 0; i < start_index && layer_names.size() > 0; ++i )
  {
    const std::string& input_i = this->tool_->input_layers_state_[ i ]->get();
    for ( size_t j = 0; j < layer_names.size(); ++j )
    {
      if ( input_i == layer_names[ j ].first )
      {
        layer_names.erase( layer_names.begin() + j );
        break;;
      }
    }
  }
  
  // Set the option list for all the remaining inputs
  for ( int i = start_index; i < ArithmeticFilter::NUMBER_OF_INPUTS_C; ++i )
  {
    this->tool_->input_layers_state_[ i ]->set_option_list( layer_names );
    if ( i == 0 && this->tool_->use_active_group_state_->get() && layer_names.size() >0 )
    {
      LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
      this->tool_->input_layers_state_[ i ]->set( active_layer->get_layer_id() );
    }

    if ( layer_names.size() > 0 )
    {
      const std::string& input_i = this->tool_->input_layers_state_[ i ]->get();
      for ( size_t j = 0; j < layer_names.size(); ++j )
      {
        if ( input_i == layer_names[ j ].first )
        {
          layer_names.erase( layer_names.begin() + j );
          break;;
        }
      }
    } 
  }
}

void ArithmeticFilterPrivate::handle_replace_changed( bool replace )
{
}

void ArithmeticFilterPrivate::handle_output_type_changed( std::string type )
{
}

void ArithmeticFilterPrivate::handle_layer_name_changed( std::string layer_id )
{
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( layer_id );
  LayerGroupHandle layer_group = layer->get_layer_group();
  this->handle_group_layers_changed( layer_group );
}


//////////////////////////////////////////////////////////////////////////
// Class ArithmeticFilter
//////////////////////////////////////////////////////////////////////////

const int ArithmeticFilter::NUMBER_OF_INPUTS_C = 4;

// Constructor, set default values
ArithmeticFilter::ArithmeticFilter( const std::string& toolid ) :
  Tool( toolid ),
  private_( new ArithmeticFilterPrivate )
{
  this->private_->tool_ = this;
  this->private_->signal_block_count_ = 0;

  this->add_state( "target_group", this->target_group_state_, "", "" );
  this->add_state( "use_active_group", this->use_active_group_state_, true );
  std::string input_name( "input_a" );
  for ( int i = 0; i < NUMBER_OF_INPUTS_C; ++i )
  {
    input_name[ 6 ] = 'a' +  i;
    this->add_state( input_name, this->input_layers_state_[ i ], "", "" );
  }
  
  this->add_state( "expressions", this->expressions_state_, "" );

  std::vector< std::string > predefined_expressions( 1, "A&&B" );
  predefined_expressions.push_back( "A||B" );
  this->add_state( "predefined_expressions", this->predefined_expressions_state_, 
    predefined_expressions );

  this->add_state( "output_type", this->output_type_state_, ActionArithmeticFilter::DATA_C, 
    ActionArithmeticFilter::DATA_C + "=Data Layer|" + ActionArithmeticFilter::MASK_C + 
    "=Mask Layer" );

  this->add_state( "replace", this->replace_state_, false );
  this->add_state( "preserve_data_format", this->preserve_data_format_state_, false );

  // Adding connections to handle updates
  this->add_connection( LayerManager::Instance()->groups_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_groups_changed, this->private_ ) ) );
  this->add_connection( LayerManager::Instance()->group_internals_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_group_layers_changed, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_active_layer_changed, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layer_name_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_layer_name_changed, this->private_, _1 ) ) );

  this->add_connection( this->use_active_group_state_->value_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_use_active_group_changed, 
    this->private_, _1 ) ) );
  this->add_connection( this->target_group_state_->value_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_target_group_changed, this->private_, _2 ) ) );
  for ( int i = 0; i < NUMBER_OF_INPUTS_C; ++i )
  {
    this->add_connection( this->input_layers_state_[ i ]->state_changed_signal_.connect(
      boost::bind( &ArithmeticFilterPrivate::handle_input_layer_changed, this->private_, i ) ) );
  }

  this->add_connection( this->replace_state_->value_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_replace_changed, this->private_, _1 ) ) );
  this->add_connection( this->output_type_state_->value_changed_signal_.connect(
    boost::bind( &ArithmeticFilterPrivate::handle_output_type_changed, this->private_, _2 ) ) );

  this->private_->handle_groups_changed();
} 

ArithmeticFilter::~ArithmeticFilter()
{
  this->disconnect_all();
}

void ArithmeticFilter::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Get action inputs from state engine
  std::vector< std::string > layer_ids;
  for( int i = 0; i < 4; i++ )
  {
    std::string layer_id = this->input_layers_state_[ i ]->get();
    if( layer_id != "" )
    {
      layer_ids.push_back( layer_id );
    }
  }

  ActionArithmeticFilter::Dispatch( context, layer_ids, this->expressions_state_->get(),
    this->output_type_state_->get(), this->replace_state_->get(), 
    this->preserve_data_format_state_->get() );
}


} // end namespace Seg3D


