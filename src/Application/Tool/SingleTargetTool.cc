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

// boost includes
#include <boost/tuple/tuple.hpp>

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
#include <Application/Tool/SingleTargetTool.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class SingleTargetToolPrivate
//////////////////////////////////////////////////////////////////////////

class SingleTargetToolPrivate
{

public:
  // The type of layer that can be used with this filter
  int target_type_;
  
  // Pointer back to the tool.
  // NOTE: This can be a pointer, as the callbacks are deleted when the tool is deleted and all
  // the tool components run on the same thread.
  SingleTargetTool* tool_;
  
//  std::vector<Core::StateLabeledOptionHandle> dependent_layer_states_;
//  std::vector<int> dependent_layer_types_;

  // The tuple contains the following types:
  // boost::tuple< Core::StateLabeledOptionHandle, int dependent_layer_type, bool required >
  std::vector< option_list_tuple_type_ > dependent_option_lists_;
  
  // -- handle updates from layermanager --
  void handle_layers_changed();
  void handle_use_active_layer_changed( bool use_active_layer );
  void handle_layer_name_changed( std::string layer_id );
  
  void update_dependent_layers();
  
  void check_dependent_layers();
  
  // -- handle updates from state variables --
  void handle_active_layer_changed( LayerHandle layer );
  void handle_target_layer_changed( std::string layer_id );   
  
  SingleTargetToolPrivate() :
    target_type_( Core::VolumeType::DATA_E ),
    tool_( 0 )
  {}
};

void SingleTargetToolPrivate::update_dependent_layers()
{
  if ( this->tool_->valid_primary_target_state_->get()  )
  {
    // Fill in the new list and reset the option to the first one, if none of the options
    // match the current one 
    // The tuple contains the following types:
    // boost::tuple< Core::StateLabeledOptionHandle, int dependent_layer_type, bool required >
    for ( size_t j = 0;  j < this->dependent_option_lists_.size(); j++ )
    {
      // Create a list with the new options
      std::vector< LayerIDNamePair > layer_names( 1, 
        std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
      LayerHandle layer = LayerManager::Instance()->find_layer_by_id( 
        this->tool_->target_layer_state_->get() );
      //if ( layer )
      //{
      //  LayerManager::Instance()->get_layer_names_from_group( layer->get_layer_group(),
      //    layer_names, this->dependent_option_lists_[ j ].get< 1 >() );
      //}

      if ( layer )
      {
        bool independent_layer = this->dependent_option_lists_[j].get<3>();
        if ( independent_layer )
        {
          LayerManager::Instance()->get_layer_names( layer_names );
        }
        else
        {
          layer->get_layer_group()->get_layer_names( layer_names, 
            static_cast< Core::VolumeType::enum_type >( this->dependent_option_lists_[ j ].get< 1 >() ) );
        }
        
      }

      // Insert this list
      this->dependent_option_lists_[ j ].get< 0 >()->set_option_list( layer_names );
      //this->dependent_layer_states_[ j ]->set_option_list( layer_names );
    }
  }
  else
  {
    // Reset the dependent layers to <none> as there is no valid option to choose
    std::vector< LayerIDNamePair > layer_names( 1, 
      std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
    for ( size_t j = 0;  j < this->dependent_option_lists_.size(); j++ )
    {
      this->dependent_option_lists_[ j ].get< 0 >()->set_option_list( layer_names );
      //this->dependent_layer_states_[ j ]->set_option_list( layer_names );
    }   
  }
  
  this->check_dependent_layers();
}

void SingleTargetToolPrivate::handle_active_layer_changed( LayerHandle layer )
{
  if ( ! this->tool_->use_active_layer_state_->get() )
  {
    return;
  }

  if ( ! layer )
  {
    // No active layer was returned: set the target layer to <none>
    this->tool_->valid_primary_target_state_->set( false );
    this->tool_->target_layer_state_->set( Tool::NONE_OPTION_C );   
    this->update_dependent_layers();
  }
  else
  {
    this->tool_->valid_primary_target_state_->set( ( layer->get_type() & this->target_type_ ) != 0 && 
      layer->has_valid_data() );
    this->tool_->target_layer_state_->set( this->tool_->valid_primary_target_state_->get() ? 
      layer->get_layer_id() : Tool::NONE_OPTION_C );
      
    this->update_dependent_layers();
  }
}

void SingleTargetToolPrivate::handle_use_active_layer_changed( bool use_active_layer )
{
  if ( use_active_layer )
  {
    LayerHandle layer = LayerManager::Instance()->get_active_layer();
    this->tool_->valid_primary_target_state_->set( layer && layer->has_valid_data() &&
      ( layer->get_type() & this->target_type_ ) );
    this->tool_->target_layer_state_->set( this->tool_->valid_primary_target_state_->get() ? 
      layer->get_layer_id() : Tool::NONE_OPTION_C );
    this->update_dependent_layers();
  }
}

void SingleTargetToolPrivate::handle_target_layer_changed( std::string layer_id )
{
  if ( this->tool_->use_active_layer_state_->get() )
  {
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
    if ( active_layer && ( active_layer->get_type() & this->target_type_ ) && 
      layer_id != active_layer->get_layer_id() )
    {
      this->tool_->target_layer_state_->set( active_layer->get_layer_id() );
      this->tool_->valid_primary_target_state_->set( true );
//      this->update_dependent_layers();
//      return;
    }

    if ( !active_layer || ( active_layer && !(active_layer->get_type() & this->target_type_ ) 
      && layer_id != Tool::NONE_OPTION_C ) )
    {
      this->tool_->target_layer_state_->set( Tool::NONE_OPTION_C );
      this->tool_->valid_primary_target_state_->set( false );
    }
  }
  else
  {
    this->tool_->valid_primary_target_state_->set( layer_id != Tool::NONE_OPTION_C );
//    this->tool_->valid_target_state_->set( layer_id != Tool::NONE_OPTION_C );
//    this->update_dependent_layers();
  }
  
  this->update_dependent_layers();
}

void SingleTargetToolPrivate::handle_layers_changed()
{
  std::vector< LayerIDNamePair > layer_names;
  layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  LayerManager::Instance()->get_layer_names( layer_names, this->target_type_ );
  this->tool_->target_layer_state_->set_option_list( layer_names );
  this->update_dependent_layers();
}

void SingleTargetToolPrivate::handle_layer_name_changed( std::string layer_id )
{
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id );
  if ( !layer ) return;
  
  if( layer->get_type() & this->target_type_ )
  {
    this->handle_layers_changed();
    return;
  }
  
  for( size_t j = 0; j < this->dependent_option_lists_.size(); j++ )
  {
    // The tuple contains the following types:
    // boost::tuple< Core::StateLabeledOptionHandle, int dependent_layer_type, bool required >
    if ( layer->get_type() & this->dependent_option_lists_[ j ].get< 1 >() )
    {
      this->handle_layers_changed();
      return;
    }   
  }
}

void SingleTargetToolPrivate::check_dependent_layers()
{
  for( size_t j = 0; j < this->dependent_option_lists_.size(); j++ )
  {
    // the third member of the tuple contains whether or not the option list is required,
    // if it isn't, then we don't check it
    
    // The tuple contains the following types:
    // boost::tuple< Core::StateLabeledOptionHandle, int dependent_layer_type, bool required >
    if( !this->dependent_option_lists_[ j ].get< 2 >() ) continue;
    
    if( this->dependent_option_lists_[ j ].get< 0 >()->get() == Tool::NONE_OPTION_C )
    {
      this->tool_->valid_target_state_->set( false );
      return;
    }   
  }
  this->tool_->valid_target_state_->set( this->tool_->valid_primary_target_state_->get() );
}


//////////////////////////////////////////////////////////////////////////
// Class SingleTargetTool
//////////////////////////////////////////////////////////////////////////

SingleTargetTool::SingleTargetTool(  int target_type, const std::string& tool_type ) :
  Tool( tool_type ),
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
  this->add_state( "valid_primary_target", this->valid_primary_target_state_, false );  

  this->target_layer_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 10 );
  this->use_active_layer_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 20 );

  // Handle the updates to the StateEngine
  // As tools are created on the application thread, the state engine should not change underneath
  // it, as every change to the state engine has to go through the same thread.
  this->private_->handle_layers_changed();
  
  // Adding connections to handle updates
  this->add_connection( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_layers_changed, this->private_.get() ) ) );
    
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_active_layer_changed, 
    this->private_.get(), _1 ), boost::signals2::at_front ) );

  this->add_connection( LayerManager::Instance()->layer_name_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_layer_name_changed, 
    this->private_.get(), _1 ) ) );
    
  this->add_connection( this->use_active_layer_state_->value_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_use_active_layer_changed, 
    this->private_.get(), _1 ) ) );
    
  this->add_connection( this->use_active_layer_state_->state_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::check_dependent_layers, 
    this->private_.get() ) ) );
    
  this->add_connection( this->target_layer_state_->value_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::handle_target_layer_changed, this->private_.get(), _2 ) ) );

  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( active_layer )
  {
    this->private_->handle_active_layer_changed( active_layer );
  }
}

SingleTargetTool::~SingleTargetTool()
{
  this->disconnect_all();
}

//void SingleTargetTool::add_dependent_layer_input( 
//  Core::StateLabeledOptionHandle dependent_layer_state, 
//  int dependent_layer_type, bool required )
//{
////  this->private_->dependent_layer_states_.push_back( dependent_layer_state );
////  this->private_->dependent_layer_types_.push_back( dependent_layer_type );
//
//  this->add_connection( dependent_layer_state->state_changed_signal_.connect(
//    boost::bind( &SingleTargetToolPrivate::check_dependent_layers, 
//    this->private_.get() ) ) );
//  
//  // The tuple contains the following types:
//  // boost::tuple< Core::StateLabeledOptionHandle, int dependent_layer_type, bool required >
//  this->private_->dependent_option_lists_.push_back( 
//    option_list_tuple_type_( dependent_layer_state, dependent_layer_type, required ) );
//  this->private_->update_dependent_layers();
//}


void SingleTargetTool::add_dependent_layer_input( 
  Core::StateLabeledOptionHandle dependent_layer_state, 
  int dependent_layer_type, bool required, bool independent_layer )
{
  //  this->private_->dependent_layer_states_.push_back( dependent_layer_state );
  //  this->private_->dependent_layer_types_.push_back( dependent_layer_type );

  this->add_connection( dependent_layer_state->state_changed_signal_.connect(
    boost::bind( &SingleTargetToolPrivate::check_dependent_layers, 
    this->private_.get() ) ) );

  // The tuple contains the following types:
  // boost::tuple< Core::StateLabeledOptionHandle, int dependent_layer_type, bool required >
  this->private_->dependent_option_lists_.push_back( 
    option_list_tuple_type_( dependent_layer_state, dependent_layer_type, required, independent_layer ) );
  this->private_->update_dependent_layers();
}


} // end namespace Seg3D
