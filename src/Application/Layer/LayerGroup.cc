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
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core includes
#include <Core/State/StateEngine.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/ViewerManager/ViewerManager.h>


namespace Seg3D
{

  //////////////////////////////////////////////////////////////////////////
  // Class LayerGroupPrivate
  //////////////////////////////////////////////////////////////////////////

class LayerGroupPrivate
{
public:
  void update_layers_visible_state();
  void update_grid_information();

  void handle_layers_visible_state_changed( std::string state );

  LayerGroup* layer_group_;
  size_t signal_block_count_;
};

void LayerGroupPrivate::update_layers_visible_state()
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  size_t total_effective_layers = 0;
  size_t total_visible_layers = 0;

  layer_list_type::iterator it = this->layer_group_->layer_list_.begin();
  while ( it != this->layer_group_->layer_list_.end() )
  {
    LayerHandle layer = *it;
    bool layer_visible = false;

    // Check the visibility of the layer in all the current visible viewers
    // and make sure it's visible in at least one
    for ( size_t i = 0; i < num_of_viewers; ++i )
    {
      ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
      if ( viewer->viewer_visible_state_->get() &&
        layer->visible_state_[ i ]->get() )
      {
        layer_visible = true;
        break;
      }
    }

    if ( layer_visible )
    {
      ++total_effective_layers;
      if ( layer->master_visible_state_->get() )
      {
        ++total_visible_layers;
      }
    }

    ++it;
  }

  std::string state;
  if ( total_visible_layers == 0 )
  {
    state = "none";
  }
  else if ( total_visible_layers == total_effective_layers )
  {
    state = "all";
  }
  else
  {
    state = "some";
  }

  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );
    this->layer_group_->layers_visible_state_->set( state );
  }
}

void LayerGroupPrivate::handle_layers_visible_state_changed( std::string state )
{
  if ( this->signal_block_count_ > 0 || state == "some" )
  {
    return;
  }

  bool visible = state ==  "all";

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  layer_list_type::iterator it = this->layer_group_->layer_list_.begin();
  while ( it != this->layer_group_->layer_list_.end() )
  {
    LayerHandle layer = *it;
    bool layer_visible = false;

    // Check the visibility of the layer in all the current visible viewers
    // and make sure it's visible in at least one
    for ( size_t i = 0; i < num_of_viewers; ++i )
    {
      ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
      if ( viewer->viewer_visible_state_->get() &&
        layer->visible_state_[ i ]->get() )
      {
        layer_visible = true;
        break;
      }
    }

    if ( layer_visible )
    {
      layer->master_visible_state_->set( visible );
    }

    ++it;
  }
}

void LayerGroupPrivate::update_grid_information()
{
  Core::Point dimensions( static_cast< double>( this->layer_group_->grid_transform_.get_nx() ), 
    static_cast< double>( this->layer_group_->grid_transform_.get_ny() ), 
    static_cast< double>( this->layer_group_->grid_transform_.get_nz() ) );
  this->layer_group_->dimensions_state_->set( dimensions );

  this->layer_group_->origin_state_->set( this->layer_group_->grid_transform_ * 
    Core::Point( 0, 0, 0 ) );

  Core::Vector spacing( 1, 1, 1 );
  spacing = this->layer_group_->grid_transform_ * spacing;
  this->layer_group_->spacing_state_->set( Core::Point( spacing ) );
}

//////////////////////////////////////////////////////////////////////////
// Class LayerGroup
//////////////////////////////////////////////////////////////////////////

LayerGroup::LayerGroup( Core::GridTransform grid_transform ) :
  StateHandler( "group", true ),
  private_( new LayerGroupPrivate )
{
  this->private_->layer_group_ = this;
  this->private_->signal_block_count_ = 0;
  this->grid_transform_ = grid_transform;
  this->initialize_states();
  
}

LayerGroup::LayerGroup( const std::string& state_id ) :
  StateHandler( state_id, true ),
  private_( new LayerGroupPrivate )
{
  this->private_->layer_group_ = this;
  this->private_->signal_block_count_ = 0;
  this->grid_transform_ = Core::GridTransform( 1, 1, 1 );
  this->initialize_states();
}


LayerGroup::~LayerGroup()
{
  // Disconnect all current connections
  this->disconnect_all();
}

void LayerGroup::initialize_states()
{
  this->add_state( "isosurface_quality", this->isosurface_quality_state_, 
    "1.0", "1.0|0.5|0.25|0.125" );

  this->add_state( "layers_visible", this->layers_visible_state_, "all", "none|some|all" );

  Core::Point dimensions( static_cast< double>( this->grid_transform_.get_nx() ), 
    static_cast< double>( this->grid_transform_.get_ny() ), 
    static_cast< double>( this->grid_transform_.get_nz() ) );
  this->add_state( "dimensions", this->dimensions_state_, dimensions );
  this->dimensions_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->add_state( "origin", this->origin_state_, this->grid_transform_ * Core::Point( 0, 0, 0 ) );
  this->origin_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  Core::Vector spacing( 1, 1, 1 );
  spacing = this->grid_transform_ * spacing;
  this->add_state( "spacing", this->spacing_state_, Core::Point( spacing ) );
  this->spacing_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->add_state( "show_iso_menu", this->show_iso_menu_state_, false );
  this->add_state( "show_delete_menu", this->show_delete_menu_state_, false );
  this->add_state( "show_duplicate_menu", this->show_duplicate_menu_state_, false );

  this->gui_state_group_.reset( new Core::BooleanStateGroup );
  this->gui_state_group_->add_boolean_state( this->show_delete_menu_state_ );
  this->gui_state_group_->add_boolean_state( this->show_iso_menu_state_ );
  this->gui_state_group_->add_boolean_state( this->show_duplicate_menu_state_ );

  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; ++i )
  {
    this->add_connection( ViewerManager::Instance()->get_viewer( i )->viewer_visible_state_->
      state_changed_signal_.connect( boost::bind( 
      &LayerGroupPrivate::update_layers_visible_state, this->private_ ) ) );
  }
  this->add_connection( this->layers_visible_state_->value_changed_signal_.connect(
    boost::bind( &LayerGroupPrivate::handle_layers_visible_state_changed, this->private_, _1 ) ) );
}


void LayerGroup::insert_layer( LayerHandle new_layer )
{ 
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  new_layer->set_layer_group( this->shared_from_this() );

  if( new_layer->get_type() == Core::VolumeType::MASK_E )
  { 
    this->layer_list_.push_front( new_layer );
  }
  else
  {
    layer_list_type::iterator it = std::find_if( this->layer_list_.begin(), 
      this->layer_list_.end(), boost::lambda::bind( &Layer::get_type, 
      boost::lambda::bind( &LayerHandle::get, boost::lambda::_1 ) ) 
      == Core::VolumeType::DATA_E );
    this->layer_list_.insert( it, new_layer );
  }

  // NOTE: LayerGroup will always out live layers, so it's safe to not keep track
  // of the following connections.
  new_layer->master_visible_state_->state_changed_signal_.connect( boost::bind(
    &LayerGroupPrivate::update_layers_visible_state, this->private_ ) );
  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; ++i )
  {
    new_layer->visible_state_[ i ]->state_changed_signal_.connect( boost::bind(
      &LayerGroupPrivate::update_layers_visible_state, this->private_ ) );
  }

  this->private_->update_layers_visible_state();
}


void LayerGroup::move_layer_above( LayerHandle layer_above, LayerHandle layer_below )
{
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if( ( layer_above->get_type() == Core::VolumeType::DATA_E ) 
    && ( layer_below->get_type() == Core::VolumeType::MASK_E ) )
  {
    for( layer_list_type::iterator i = this->layer_list_.begin(); 
      i != this->layer_list_.end(); ++i )
    {
      if( ( *i )->get_type() == Core::VolumeType::DATA_E )
      { 
        this->layer_list_.insert( i, layer_above );
        return;
      }
    }
  }

  if( layer_above->get_type() != layer_below->get_type() )
  {
    this->move_layer_below( layer_above );
    return;
  }

  for( layer_list_type::iterator i = this->layer_list_.begin(); 
    i != this->layer_list_.end(); ++i )
  {
    if( ( *i ) == layer_below )
    { 
      this->layer_list_.insert( i, layer_above );
    }
  }
}

void LayerGroup::move_layer_below( LayerHandle layer )
{
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // if we are inserting a mask layer then we put it at the b
  if( layer->get_type() == Core::VolumeType::MASK_E )
  {
    for( layer_list_type::iterator i = this->layer_list_.begin(); 
      i != this->layer_list_.end(); ++i )
    {
      if( ( *i )->get_type() == Core::VolumeType::DATA_E )
      { 
        this->layer_list_.insert( i, layer);
        return;
      }
    }
  }

  this->layer_list_.insert( this->layer_list_.end(), layer );
}

void LayerGroup::delete_layer( LayerHandle layer )
{
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layer_list_.remove( layer );
  this->private_->update_layers_visible_state();
}

void LayerGroup::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
  Core::VolumeType type ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layer_list_type::const_iterator it = this->layer_list_.begin();
  for ( ; it != this->layer_list_.end(); it++ )
  {
    if ( ( *it )->get_type() & type )
    {
      layer_names.push_back( std::make_pair( ( *it )->get_layer_id(),
        ( *it )->get_layer_name() ) );
    }
  }
}

void LayerGroup::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
  Core::VolumeType type, LayerHandle excluded_layer ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layer_list_type::const_iterator it = this->layer_list_.begin();
  for ( ; it != this->layer_list_.end(); it++ )
  {
    if ( *it != excluded_layer && ( ( *it )->get_type() & type ) )
    {
      layer_names.push_back( std::make_pair( ( *it )->get_layer_id(),
        ( *it )->get_layer_name() ) );
    }
  }
}

bool LayerGroup::post_save_states( Core::StateIO& state_io )
{
  TiXmlElement* lm_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == lm_element->Value() );

  TiXmlElement* layers_element = new TiXmlElement( "layers" );
  lm_element->LinkEndChild( layers_element );

  state_io.push_current_element();
  state_io.set_current_element( layers_element );
  
  layer_list_type::reverse_iterator it = this->layer_list_.rbegin();
  for ( ; it != this->layer_list_.rend(); it++ )
  {
    if ( ( *it )->has_valid_data() )
    {
      ( *it )->save_states( state_io );
    }
  }
  
  state_io.pop_current_element();
  return true;
}

bool LayerGroup::post_load_states( const Core::StateIO& state_io )
{
  const TiXmlElement* layers_element = state_io.get_current_element()->
    FirstChildElement( "layers" );
  if ( layers_element == 0 )
  {
    return false;
  }

  state_io.push_current_element();
  state_io.set_current_element( layers_element );

  bool success = true;
  const TiXmlElement* layer_element = layers_element->FirstChildElement();
  while ( layer_element != 0 )
  {
    std::string layer_id( layer_element->Value() );
    std::string layer_type( layer_element->Attribute( "type" ) );
    LayerHandle layer;
    if ( layer_type == "data" )
    {
      layer.reset( new DataLayer( layer_id ) );
    }
    else if ( layer_type == "mask" )
    {
      layer.reset( new MaskLayer( layer_id ) );
    }
    else
    {
      CORE_LOG_ERROR( "Unsupported layer type" );
    }

    if ( layer && layer->load_states( state_io ) )
    {
      if( this->layer_list_.empty() )
      {
        this->grid_transform_ = layer->get_grid_transform();
      }
      this->insert_layer( layer );
    }
    else
    {
      success = false;
    }

    layer_element = layer_element->NextSiblingElement();
  }

  state_io.pop_current_element();
  
  this->private_->update_grid_information();
  return success;
}

void LayerGroup::clear()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  std::for_each( this->layer_list_.begin(), this->layer_list_.end(), boost::lambda::bind( 
    &Layer::invalidate, boost::lambda::bind( &LayerHandle::get, boost::lambda::_1 ) ) );
  this->layer_list_.clear();
}

} // end namespace Seg3D