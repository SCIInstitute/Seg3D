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


// boost includes
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core includes
#include <Core/State/StateEngine.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Utils/Exception.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Layer/Actions/ActionComputeIsosurface.h>


namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerGroupPrivate
//////////////////////////////////////////////////////////////////////////

class LayerGroupPrivate : public Core::ConnectionHandler
{
public:
  void update_layers_visible_state();
  void update_grid_information();
  
  void update_layers_iso_visible_state();

  void handle_layers_visible_state_changed( std::string state );
  void handle_layers_iso_visible_state_changed( std::string state );

  // INITIALIZE_STATES:
  // This function sets the initial states of the layer group.
  void initialize_states();

public:
  // The transformation that describes the grid dimensions and the spacing
  Core::GridTransform grid_transform_;

  // An exclusive group of boolean states that control the visibility of different parts
  Core::BooleanStateGroupHandle gui_state_group_;

  // The list that contains the layers that are stored in this class
  LayerList layer_list_;

  // Pointer to the LayerGroup object
  LayerGroup* layer_group_;

  // An internal counter for temporarily blocking state signals
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

  LayerList::const_iterator it = this->layer_list_.begin();
  while ( it != this->layer_list_.end() )
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

void LayerGroupPrivate::update_layers_iso_visible_state()
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  size_t total_effective_layers = 0;
  size_t total_visible_layers = 0;

  LayerList::const_iterator it = this->layer_list_.begin();
  while ( it != this->layer_list_.end() )
  {
    LayerHandle layer = *it;

    if( ( layer->get_type() == Core::VolumeType::MASK_E ) )
    {
      MaskLayer* mask_layer = static_cast< MaskLayer * >( layer.get() );
      if ( mask_layer->iso_generated_state_->get() )
      {
        ++total_effective_layers;
        if ( mask_layer->show_isosurface_state_->get() )
        {
          ++total_visible_layers;
        }
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
    this->layer_group_->layers_iso_visible_state_->set( state );
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
  LayerList::const_iterator it = this->layer_list_.begin();
  while ( it != this->layer_list_.end() )
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

void LayerGroupPrivate::handle_layers_iso_visible_state_changed( std::string state )
{
  if ( this->signal_block_count_ > 0 || state == "some" )
  {
    return;
  }

  bool visible = ( state ==  "all" );

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  LayerList::const_iterator it = this->layer_list_.begin();
  while ( it != this->layer_list_.end() )
  {
    LayerHandle layer = *it;

    if( ( layer->get_type() == Core::VolumeType::MASK_E ) )
    {
      MaskLayer* mask_layer = static_cast< MaskLayer * >( layer.get() );
      if ( mask_layer->iso_generated_state_->get() )
      {
        mask_layer->show_isosurface_state_->set( visible );
      }
    }

    ++it;
  }
}

void LayerGroupPrivate::update_grid_information()
{
  Core::Point dimensions( static_cast< double>( this->grid_transform_.get_nx() ), 
    static_cast< double>( this->grid_transform_.get_ny() ), 
    static_cast< double>( this->grid_transform_.get_nz() ) );
  this->layer_group_->dimensions_state_->set( dimensions );

  this->layer_group_->origin_state_->set( this->grid_transform_ * 
    Core::Point( 0, 0, 0 ) );

  Core::Vector spacing( 1, 1, 1 );
  spacing = this->grid_transform_ * spacing;
  this->layer_group_->spacing_state_->set( Core::Point( spacing ) );
}

void LayerGroupPrivate::initialize_states()
{
  this->layer_group_->add_state( "isosurface_quality", 
    this->layer_group_->isosurface_quality_state_, "1.0", "1.0|0.5|0.25|0.125" );
  this->layer_group_->add_state( "isosurface_capping_enabled", 
    this->layer_group_->isosurface_capping_enabled_state_, true ); 
  this->layer_group_->add_state( "layers_visible", 
    this->layer_group_->layers_visible_state_, "all", "none|some|all" );
  this->layer_group_->add_state( "layers_iso_visible", 
    this->layer_group_->layers_iso_visible_state_, "all", "none|some|all" );

  this->layer_group_->add_state( "provenance_id", this->layer_group_->provenance_id_state_, -1 );
  this->layer_group_->add_state( "meta_data", this->layer_group_->meta_data_state_, "" ); 
  this->layer_group_->add_state( "meta_data_info", this->layer_group_->meta_data_info_state_, "" ); 

  Core::Point dimensions( static_cast< double>( this->grid_transform_.get_nx() ), 
    static_cast< double>( this->grid_transform_.get_ny() ), 
    static_cast< double>( this->grid_transform_.get_nz() ) );
  this->layer_group_->add_state( "dimensions", this->layer_group_->dimensions_state_, dimensions );
  this->layer_group_->dimensions_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->layer_group_->add_state( "origin", this->layer_group_->origin_state_, 
    this->grid_transform_ * Core::Point( 0, 0, 0 ) );
  this->layer_group_->origin_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  Core::Vector spacing( 1, 1, 1 );
  spacing = this->grid_transform_ * spacing;
  this->layer_group_->add_state( "spacing", this->layer_group_->spacing_state_, Core::Point( spacing ) );
  this->layer_group_->spacing_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->layer_group_->add_state( "group_widget_expanded", 
    this->layer_group_->group_widget_expanded_state_, true );
  this->layer_group_->add_state( "show_iso_menu", 
    this->layer_group_->show_iso_menu_state_, false );
  this->layer_group_->add_state( "show_delete_menu", 
    this->layer_group_->show_delete_menu_state_, false );
  this->layer_group_->add_state( "show_duplicate_menu", 
    this->layer_group_->show_duplicate_menu_state_, false );

  this->gui_state_group_.reset( new Core::BooleanStateGroup );
  this->gui_state_group_->add_boolean_state( this->layer_group_->show_delete_menu_state_ );
  this->gui_state_group_->add_boolean_state( this->layer_group_->show_iso_menu_state_ );
  this->gui_state_group_->add_boolean_state( this->layer_group_->show_duplicate_menu_state_ );

  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; ++i )
  {
    this->add_connection( ViewerManager::Instance()->get_viewer( i )->
      viewer_visible_state_->state_changed_signal_.connect( boost::bind( 
      &LayerGroupPrivate::update_layers_visible_state, this ) ) );
  }
  this->add_connection( this->layer_group_->layers_visible_state_->value_changed_signal_.connect( 
    boost::bind( &LayerGroupPrivate::handle_layers_visible_state_changed, this, _1 ) ) );

  this->add_connection( this->layer_group_->layers_iso_visible_state_->value_changed_signal_.connect(
    boost::bind( &LayerGroupPrivate::handle_layers_iso_visible_state_changed, this, _1 ) ) );
}

//////////////////////////////////////////////////////////////////////////
// Class LayerGroup
//////////////////////////////////////////////////////////////////////////

LayerGroup::LayerGroup( Core::GridTransform grid_transform, 
  ProvenanceID provenance_id, const LayerMetaData& meta_data ) :
  StateHandler( "group", true ),
  private_( new LayerGroupPrivate )
{
  this->private_->layer_group_ = this;
  this->private_->signal_block_count_ = 0;
  this->private_->grid_transform_ = grid_transform;
  this->private_->initialize_states();
  
  // This is the layer that generated this group. Hence that is the dependent layer
  // for new mask or other layers that are created in the group.
  this->provenance_id_state_->set( provenance_id );
  this->set_meta_data( meta_data );
}

LayerGroup::LayerGroup( const std::string& state_id ) :
  StateHandler( state_id, true ),
  private_( new LayerGroupPrivate )
{
  this->private_->layer_group_ = this;
  this->private_->signal_block_count_ = 0;
  this->private_->grid_transform_ = Core::GridTransform( 1, 1, 1 );
  this->private_->initialize_states();
}


LayerGroup::~LayerGroup()
{
  // Disconnect all current connections
  this->disconnect_all();
  this->private_->disconnect_all();
}


LayerMetaData LayerGroup::get_meta_data() const
{
  LayerMetaData meta_data;
  meta_data.meta_data_ = this->meta_data_state_->get();
  meta_data.meta_data_info_ = this->meta_data_info_state_->get();
  return meta_data;
}

void LayerGroup::set_meta_data( const LayerMetaData& meta_data )
{
  this->meta_data_state_->set( meta_data.meta_data_ );
  this->meta_data_info_state_->set( meta_data.meta_data_info_ );
}


void LayerGroup::insert_layer( LayerHandle new_layer )
{ 
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  new_layer->set_layer_group( this->shared_from_this() );

  if( new_layer->get_type() == Core::VolumeType::MASK_E )
  { 
    this->private_->layer_list_.push_front( new_layer );
    boost::dynamic_pointer_cast< MaskLayer >( new_layer )->show_isosurface_state_->
      state_changed_signal_.connect( boost::bind(
      &LayerGroupPrivate::update_layers_iso_visible_state, this->private_ ) );
  }
  else
  {
    LayerList::iterator it = std::find_if( this->private_->layer_list_.begin(), 
      this->private_->layer_list_.end(), boost::lambda::bind( &Layer::get_type, 
      boost::lambda::bind( &LayerHandle::get, boost::lambda::_1 ) ) 
      == Core::VolumeType::DATA_E || boost::lambda::bind( &Layer::get_type, 
      boost::lambda::bind( &LayerHandle::get, boost::lambda::_1 ) ) 
      == Core::VolumeType::LARGE_DATA_E );
    this->private_->layer_list_.insert( it, new_layer );
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
  this->private_->update_layers_iso_visible_state();
}

void LayerGroup::insert_layer( LayerHandle new_layer, size_t pos )
{
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // If the layer already exists, do nothing
  if ( std::find( this->private_->layer_list_.begin(), this->private_->layer_list_.end(), new_layer ) !=
    this->private_->layer_list_.end() )
  {
    return;
  }
  
  LayerList::iterator layer_it;
  if ( this->private_->layer_list_.size() > pos )
  {
    layer_it = this->private_->layer_list_.begin();
    std::advance( layer_it, pos );
  }
  else
  {
    layer_it = this->private_->layer_list_.end();
  }

  this->private_->layer_list_.insert( layer_it, new_layer );
  this->private_->update_layers_visible_state();
  this->private_->update_layers_iso_visible_state();
}

bool LayerGroup::move_layer( LayerHandle src_layer, LayerHandle dst_layer )
{
  ASSERT_IS_APPLICATION_THREAD();

  // Make sure that src_layer and dst_layer belong to this group
  assert( src_layer->get_layer_group().get() == this );
  assert( !dst_layer || dst_layer->get_layer_group().get() == this );

  // src_layer is the only layer in the group, no need to move
  if ( this->private_->layer_list_.size() == 1 )
  {
    return false;
  }

  // Find the src_layer in the list
  LayerList::iterator src_it = std::find( this->private_->layer_list_.begin(), 
    this->private_->layer_list_.end(), src_layer );
  assert( src_it != this->private_->layer_list_.end() );

  // Record the layer right below the src_layer, so later we can check
  // if the new position is actually different.
  LayerList::iterator tmp_it = src_it;
  ++tmp_it;
  LayerHandle old_neighbor;
  if ( tmp_it != this->private_->layer_list_.end() )
  {
    old_neighbor = *tmp_it;
  }

  LayerList::iterator dst_it = this->private_->layer_list_.end();
  if ( dst_layer )
  {
    dst_it = std::find( this->private_->layer_list_.begin(),
      this->private_->layer_list_.end(), dst_layer );
  }
  
  // If src_layer is a data layer, it must be below all mask layers
  if ( src_layer->get_type() == Core::VolumeType::DATA_E || src_layer->get_type() == Core::VolumeType::LARGE_DATA_E )
  {
    if ( dst_layer && dst_layer->get_type() == Core::VolumeType::MASK_E )
    {
      do 
      {
        ++dst_it;
      } while ( dst_it != this->private_->layer_list_.end() && 
        ( *dst_it )->get_type() == Core::VolumeType::MASK_E );
    }
  }
  // Else, src_layer is a mask layer and it must be above all data layers
  else
  {
    tmp_it = dst_it;
    if ( tmp_it == this->private_->layer_list_.end() ) --tmp_it;

    while ( tmp_it != this->private_->layer_list_.begin() &&
      ( ( *tmp_it )->get_type() == Core::VolumeType::DATA_E || 
      ( *tmp_it )->get_type() == Core::VolumeType::LARGE_DATA_E ) )
    {
      dst_it = tmp_it--;
    }
  }

  // If destination position is the same as 'old_neighbor' position, no need to move
  if ( ( dst_it == this->private_->layer_list_.end() && !old_neighbor ) ||
    ( dst_it != this->private_->layer_list_.end() && ( *dst_it ) == old_neighbor ) ||
    src_it == dst_it )
  {
    return false;
  }

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    // Remove the src_layer from the list
    this->private_->layer_list_.erase( src_it );
    // Insert the src_layer back to the new position
    // NOTE: dst_it remains valid after erasing src_it
    this->private_->layer_list_.insert( dst_it, src_layer );
  }

  return true;
}

void LayerGroup::delete_layer( LayerHandle layer )
{
  ASSERT_IS_APPLICATION_THREAD();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if( layer->get_type() ==  Core::VolumeType::MASK_E )
  {
    MaskLayerHandle temp_mask_handle = boost::dynamic_pointer_cast< MaskLayer >( layer );
    if( temp_mask_handle->iso_generated_state_->get() )
    {
      temp_mask_handle->delete_isosurface();
    }
  }
  this->private_->layer_list_.remove( layer );
  if ( !this->private_->layer_list_.empty() )
  {
    this->private_->gui_state_group_->clear_selection();
    this->private_->update_layers_visible_state();
    this->private_->update_layers_iso_visible_state();
  }
}

void LayerGroup::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
  Core::VolumeType type, LayerHandle excluded_layer ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  LayerList::const_iterator it = this->private_->layer_list_.begin();
  for ( ; it != this->private_->layer_list_.end(); it++ )
  {
    // Only include layers of the right type and with valid data
    if ( *it != excluded_layer && ( ( *it )->get_type() & type ) &&
      ( *it )->has_valid_data() )
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
  
  bool succeeded = true;
  LayerList::reverse_iterator it = this->private_->layer_list_.rbegin();
  for ( ; it != this->private_->layer_list_.rend(); it++ )
  {
    if ( ( *it )->has_valid_data() )
    {
      succeeded = succeeded && ( *it )->save_states( state_io );
    }
  }
  
  state_io.pop_current_element();
  return succeeded;
}
  
bool LayerGroup::has_a_valid_layer() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  LayerList::const_iterator it = this->private_->layer_list_.begin();
  for ( ; it != this->private_->layer_list_.end(); it++ )
  {
    if ( ( *it )->has_valid_data() )
    {
      return true;
    }
  }
  
  return false;
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
    else if ( layer_type == "large" )
    {
      layer.reset( new LargeVolumeLayer( layer_id ) );
    }   
    else
    {
      CORE_LOG_ERROR( "Unsupported layer type" );
    }

    if ( layer && layer->load_states( state_io ) )
    {
      if( this->private_->layer_list_.empty() )
      {
        // Use the first grid transform as the grid transform for the entire group
        this->private_->grid_transform_ = layer->get_grid_transform();
      }
      else
      {
        // In order to provide backward compatibility, we need to force all layers in a 
        // group to have the same grid transform (space origin and space direction).  
        // Otherwise it is possible for a session file to have a layer group with a mix of 
        // cell-centered and node-centered layers that, when correctly processed, have 
        // slightly different grid transforms.

        // We are supposed to preserve the original centering (node vs. cell) when exporting 
        // nrrds, so we only change the matrix portion of the grid transform and leave the
        // original centering alone.
        bool preserve_centering = true;
        layer->set_grid_transform( this->private_->grid_transform_, preserve_centering );
      }
      this->insert_layer( layer );
      
      // Here we do any post loading processing that requires both the group and layer info
      
      // Now, if the mask had its isosurface generated we dispatch an action to do it again
      if( layer_type == "mask" ) 
      {
        MaskLayerHandle temp_mask_handle = boost::dynamic_pointer_cast< MaskLayer >( layer );
        if( temp_mask_handle->iso_generated_state_->get() ) 
        {
          double quality = 1.0;
          Core::ImportFromString( this->isosurface_quality_state_->get(), quality );
          bool capping_enabled = this->isosurface_capping_enabled_state_->get();
          ActionComputeIsosurface::Dispatch( Core::Interface::GetWidgetActionContext(), 
            temp_mask_handle, quality, capping_enabled );
        }
      }
    }
    else
    {
      success = false;
    }

    layer_element = layer_element->NextSiblingElement();
  }

  state_io.pop_current_element();

  // If the layer group didn't have a provenance ID, assign it that of its first layer
  if ( success && this->provenance_id_state_->get() < 0 )
  {
    this->provenance_id_state_->set( this->private_->layer_list_.front()->provenance_id_state_->get() );
  }
  
  this->private_->update_grid_information();
  return success;
}

void LayerGroup::clear()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  BOOST_FOREACH( LayerHandle layer, this->private_->layer_list_ )
  {
    layer->invalidate();
  }
  this->private_->layer_list_.clear();
}

size_t LayerGroup::get_layer_position( LayerHandle layer ) const
{
  ASSERT_IS_APPLICATION_THREAD();

  LayerList::const_iterator it =  this->private_->layer_list_.begin();
  LayerList::const_iterator it_end = this->private_->layer_list_.end();
  size_t position = 0;
  while ( it != it_end && ( *it ) != layer )
  {
    ++it;
    ++position;
  }

  if ( it == it_end )
  {
    assert( false );
    CORE_THROW_LOGICERROR( "Layer no longer exists in LayerManager" );
  }

  return position;
}

const Core::GridTransform& LayerGroup::get_grid_transform() const
{
  return this->private_->grid_transform_;
}

const std::string& LayerGroup::get_group_id() const
{
  return this->get_statehandler_id();
}

std::list< LayerHandle >& LayerGroup::get_layer_list()
{
  return this->private_->layer_list_;
}

void LayerGroup::get_layers( std::vector< LayerHandle >& layers ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layers.resize( this->private_->layer_list_.size() );
  std::copy( this->private_->layer_list_.begin(), this->private_->layer_list_.end(), layers.begin() );
}

LayerHandle LayerGroup::top_layer() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->layer_list_.front();
}

LayerHandle LayerGroup::bottom_layer() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->layer_list_.back();
}

size_t LayerGroup::number_of_layers() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->layer_list_.size();
}

bool LayerGroup::is_empty() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->layer_list_.empty();
}


} // end namespace Seg3D
