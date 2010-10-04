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
#include <Core/State/StateEngine.h>

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/ViewerManager/ViewerManager.h>


namespace Seg3D
{

LayerGroup::LayerGroup( Core::GridTransform grid_transform ) :
  StateHandler( "group", true )
{
  this->grid_transform_ = grid_transform;

  this->visibility_state_.resize( ViewerManager::Instance()->number_of_viewers() );
  for ( size_t i = 0; i < this->visibility_state_.size(); ++i )
  {
    this->add_state( "visible" + Core::ExportToString( i ), this->visibility_state_[ i ], true );
  }
  
  this->add_state( "isosurface_quality", this->isosurface_quality_state_, 
    "1.0", "1.0|0.5|0.25|0.125" );

  Core::Point dimensions( static_cast< double>( grid_transform.get_nx() ), 
    static_cast< double>( grid_transform.get_ny() ), 
    static_cast< double>( grid_transform.get_nz() ) );
  this->add_state( "dimensions", this->dimensions_state_, dimensions );

  this->add_state( "origin", this->origin_state_, grid_transform * Core::Point( 0, 0, 0 ) );

  Core::Vector spacing( 1, 1, 1 );
  spacing = grid_transform * spacing;
  this->add_state( "spacing", this->spacing_state_, Core::Point( spacing ) );

  this->add_state( "show_iso_menu", this->show_iso_menu_state_, false );
  this->add_state( "show_delete_menu", this->show_delete_menu_state_, false );

  this->gui_state_group_.reset( new Core::BooleanStateGroup );
  this->gui_state_group_->add_boolean_state( this->show_delete_menu_state_ );
  this->gui_state_group_->add_boolean_state( this->show_iso_menu_state_ );
}

LayerGroup::~LayerGroup()
{
  // Disconnect all current connections
  this->disconnect_all();
}

void LayerGroup::insert_layer( LayerHandle new_layer )
{
  if( new_layer->type() == Core::VolumeType::MASK_E )
  { 
    this->layer_list_.push_front( new_layer );
    return;
  }
    
  for( layer_list_type::iterator i = this->layer_list_.begin(); 
    i != this->layer_list_.end(); ++i )
  {
    if( ( *i )->type() == Core::VolumeType::DATA_E )
    {
      this->layer_list_.insert( ( i ), new_layer );
      return;
    }
  }
  this->layer_list_.push_back( new_layer );
}


void LayerGroup::move_layer_above( LayerHandle layer_above, LayerHandle layer_below )
{
  for( layer_list_type::iterator i = this->layer_list_.begin(); 
    i != this->layer_list_.end(); ++i )
  {
    if( ( *i ) == layer_below )
    { 
      this->layer_list_.insert( i, layer_above );
    }
  }
}

void LayerGroup::delete_layer( LayerHandle layer )
{
  layer_list_.remove( layer );
}

void LayerGroup::get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
  Core::VolumeType type ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  layer_list_type::const_iterator it = this->layer_list_.begin();
  for ( ; it != this->layer_list_.end(); it++ )
  {
    if ( ( *it )->type() & type )
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
    if ( *it != excluded_layer && ( ( *it )->type() & type ) )
    {
      layer_names.push_back( std::make_pair( ( *it )->get_layer_id(),
        ( *it )->get_layer_name() ) );
    }
  }
}

} // end namespace Seg3D