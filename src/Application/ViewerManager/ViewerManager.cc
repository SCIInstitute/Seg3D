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

// STL includes

// Boost includes 

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Viewer/Viewer.h> 
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// Core includes
#include <Core/Utils/ScopedCounter.h>
#include <Core/Interface/Interface.h>

namespace Seg3D
{

const size_t ViewerManager::VERSION_NUMBER_C = 1;

CORE_SINGLETON_IMPLEMENTATION( ViewerManager );

ViewerManager::ViewerManager() :
  StateHandler( "view", VERSION_NUMBER_C, false, 1 ),
  signal_block_count_( 0 )
{
  // Step (1)
  // Set the default state of this element
  this->add_state( "layout", this->layout_state_, PreferencesManager::Instance()->
    default_viewer_mode_state_->export_to_string(), PreferencesManager::Instance()->
    default_viewer_mode_state_->export_list_to_string() );
  this->add_state( "active_viewer", this->active_viewer_state_, 0 );

  // No viewer will be the active viewer for picking
  // NOTE: The interface will set this up
  this->add_state( "active_axial_viewer", active_axial_viewer_, -1 );
  this->add_state( "active_coronal_viewer", active_coronal_viewer_, -1 );
  this->add_state( "active_sagittal_viewer", active_sagittal_viewer_, -1 );

  // NOTE: Privately held state variable for  recording which viewers are used in the program
  // Currently there are six default viewers
  std::vector< std::string> viewers;
  this->add_state( "viewers", this->viewers_state_, viewers );

  // Step (2)
  // Create the viewers that are part of the application
  // Currently a maximum of 6 viewers can be created
  this->viewers_.resize( 6 );
  
  for ( size_t j = 0; j < viewers_.size(); j++ )
  {
    this->viewers_[ j ] = ViewerHandle( new Viewer( j ) );
  }

  // Step set defaults for viewers
  this->viewers_[ 0 ]->view_mode_state_->set( Viewer::VOLUME_C );
  this->viewers_[ 0 ]->slice_visible_state_->set( false );
  this->viewers_[ 1 ]->view_mode_state_->set( Viewer::AXIAL_C );
  this->viewers_[ 1 ]->slice_visible_state_->set( false );
  this->viewers_[ 2 ]->view_mode_state_->set( Viewer::AXIAL_C );
  this->viewers_[ 2 ]->slice_visible_state_->set( false );
  this->viewers_[ 3 ]->view_mode_state_->set( Viewer::AXIAL_C );
  this->viewers_[ 3 ]->slice_visible_state_->set( true );
  this->viewers_[ 4 ]->view_mode_state_->set( Viewer::SAGITTAL_C );
  this->viewers_[ 4 ]->slice_visible_state_->set( true );
  this->viewers_[ 5 ]->view_mode_state_->set( Viewer::CORONAL_C );
  this->viewers_[ 5 ]->slice_visible_state_->set( true );

  for ( size_t j = 0; j < viewers_.size(); j++ )
  {
    // NOTE: ViewerManager needs to process these signals first
    this->add_connection( this->viewers_[ j ]->view_mode_state_->value_changed_signal_.
      connect( boost::bind( &ViewerManager::viewer_mode_changed, this, j ), 
      boost::signals2::at_front ) );
    this->add_connection( this->viewers_[ j ]->viewer_visible_state_->value_changed_signal_.
      connect( boost::bind( &ViewerManager::viewer_visibility_changed, this, j ), 
      boost::signals2::at_front ) );
    this->add_connection( this->viewers_[ j ]->is_picking_target_state_->value_changed_signal_.
      connect( boost::bind( &ViewerManager::viewer_became_picking_target, this, j ), 
      boost::signals2::at_front ) );
    this->add_connection( this->viewers_[ j ]->lock_state_->value_changed_signal_.
      connect( boost::bind( &ViewerManager::viewer_lock_state_changed, this, j ), 
      boost::signals2::at_front ) );
      
    // NOTE: For these signals order does not matter  
    this->add_connection( this->viewers_[ j ]->slice_visible_state_->state_changed_signal_.
      connect( boost::bind( &ViewerManager::update_volume_viewers, this ) ) );
  }

  // Finally we will put the viewers into state variables for loading to/from file
  // NOTE: This state variable is for internal use only
  std::vector< std::string > viewers_vector;
  for( size_t i = 0; i < this->viewers_.size(); ++i )
  {
    viewers_vector.push_back( this->viewers_[ i ]->get_statehandler_id() );
  }
  this->viewers_state_->set( viewers_vector );
}

ViewerManager::~ViewerManager()
{
  this->disconnect_all();
}

ViewerHandle ViewerManager::get_viewer( size_t idx )
{
  ViewerHandle handle;
  if ( idx < this->viewers_.size() ) handle = this->viewers_[ idx ];
  return handle;
}

ViewerHandle ViewerManager::get_viewer( const std::string viewer_name )
{
  ViewerHandle handle;
  for ( size_t i = 0; i < this->viewers_.size(); i++ )
  {
    if ( this->viewers_[ i ]->get_statehandler_id() == viewer_name )
    {
      handle = this->viewers_[ i ];
      break;
    }
  }
  return handle;
}

void ViewerManager::get_2d_viewers_info( ViewerInfoList viewers[ 3 ] )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if ( !LayerManager::Instance()->get_active_layer() )
  {
    return;
  }
  for ( size_t i = 0; i < 6; i++ )
  {
    ViewerHandle viewer = this->viewers_[ i ];
    if ( viewer->viewer_visible_state_->get() && !viewer->is_volume_view() )
    {
      Core::StateView2D* view2d = static_cast< Core::StateView2D* >( 
        viewer->get_active_view_state().get() );
      ViewerInfoHandle viewer_info( new ViewerInfo );
      viewer_info->viewer_id_ = i;
      viewer_info->view_mode_ = viewer->view_mode_state_->index();
      viewer_info->depth_ = view2d->get().center().z();
      viewer_info->is_picking_target_ = viewer->is_picking_target_state_->get();
      viewers[ viewer_info->view_mode_ ].push_back( viewer_info );
    }
  }
}

void ViewerManager::viewer_mode_changed( size_t viewer_id )
{
  Core::ScopedCounter signal_block_counter( this->signal_block_count_ );

  if ( this->active_axial_viewer_->get() == static_cast< int >( viewer_id ) )
  {
    this->active_axial_viewer_->set( -1 );
  }
  else if ( this->active_coronal_viewer_->get() == static_cast< int >( viewer_id ) )
  {
    this->active_coronal_viewer_->set( -1 );
  }
  else if ( this->active_sagittal_viewer_->get() == static_cast< int >( viewer_id ) )
  {
    this->active_sagittal_viewer_->set( -1 );
  }
  
  this->viewers_[ viewer_id ]->is_picking_target_state_->set( false );
  this->update_picking_targets();
}

void ViewerManager::viewer_visibility_changed( size_t viewer_id )
{
  Core::ScopedCounter signal_block_counter( this->signal_block_count_ );

  if ( !this->viewers_[ viewer_id ]->viewer_visible_state_->get() )
  {
    if ( this->active_axial_viewer_->get() == static_cast< int >( viewer_id ) )
    {
      this->active_axial_viewer_->set( -1 );
    }
    else if ( this->active_coronal_viewer_->get() == static_cast< int >( viewer_id ) )
    {
      this->active_coronal_viewer_->set( -1 );
    }
    else if ( this->active_sagittal_viewer_->get() == static_cast< int >( viewer_id ) )
    {
      this->active_sagittal_viewer_->set( -1 );
    }

    this->viewers_[ viewer_id ]->is_picking_target_state_->set( false );
  }

  this->update_picking_targets();
}

void ViewerManager::viewer_became_picking_target( size_t viewer_id )
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }
  
  ViewerHandle viewer = this->viewers_[ viewer_id ];
  if ( !viewer->is_picking_target_state_->get() )
  {
    return;
  }

  {
    Core::ScopedCounter signal_block_counter( this->signal_block_count_ );

    if ( viewer->view_mode_state_->get() == Viewer::AXIAL_C )
    {
      if ( this->active_axial_viewer_->get() != static_cast< int >( viewer_id ) )
      {
        if ( this->active_axial_viewer_->get() >= 0 )
        {
          this->viewers_[ this->active_axial_viewer_->get() ]->is_picking_target_state_->set( false );
        }
        this->active_axial_viewer_->set( static_cast< int >( viewer_id ) );
      }
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      if ( this->active_coronal_viewer_->get() != static_cast< int >( viewer_id ) )
      {
        if ( this->active_coronal_viewer_->get() >= 0 )
        {
          this->viewers_[ this->active_coronal_viewer_->get() ]->is_picking_target_state_->set( false );
        }
        this->active_coronal_viewer_->set( static_cast< int >( viewer_id ) );
      }
    }
    else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      if ( this->active_sagittal_viewer_->get() != static_cast< int >( viewer_id ) )
      {
        if ( this->active_sagittal_viewer_->get() >= 0 )
        {
          this->viewers_[ this->active_sagittal_viewer_->get() ]->is_picking_target_state_->set( false );
        }
        this->active_sagittal_viewer_->set( static_cast< int >( viewer_id ) );
      }
    }
    else
    {
      //assert( false );
    }
  }

  this->picking_target_changed_signal_( viewer_id );
}

void ViewerManager::update_volume_viewers()
{

  for ( size_t i = 0; i < 6; i++ )
  {
    ViewerHandle viewer = this->viewers_[ i ];
    if ( !viewer->viewer_visible_state_->get() )
    {
      continue;
    }

    if ( viewer->view_mode_state_->get() == Viewer::VOLUME_C )
    {
      viewer->redraw();
    }
  }
}

void ViewerManager::update_picking_targets()
{
  for ( size_t i = 0; i < 6; i++ )
  {
    ViewerHandle viewer = this->viewers_[ i ];
    if ( !viewer->viewer_visible_state_->get() )
    {
      continue;
    }
    
    if ( viewer->view_mode_state_->get() == Viewer::AXIAL_C )
    {
      if ( this->active_axial_viewer_->get() < 0 )
      {
        this->active_axial_viewer_->set( static_cast< int >( i ) );
        viewer->is_picking_target_state_->set( true );
      }
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      if ( this->active_coronal_viewer_->get() < 0 )
      {
        this->active_coronal_viewer_->set( static_cast< int >( i ) );
        viewer->is_picking_target_state_->set( true );
      }
    }
    else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      if ( this->active_sagittal_viewer_->get() < 0 )
      {
        this->active_sagittal_viewer_->set( static_cast< int >( i ) );
        viewer->is_picking_target_state_->set( true );
      }
    }
  }
}

void ViewerManager::pick_point( size_t source_viewer, const Core::Point& pt )
{
  ViewerHandle src_viewer = this->viewers_[ source_viewer ];
  if ( this->active_axial_viewer_->get() >= 0 && 
    this->active_axial_viewer_->get() != static_cast< int >( source_viewer ) )
  {
    ViewerHandle viewer = this->viewers_[ this->active_axial_viewer_->get() ];
    if ( viewer->view_mode_state_->get() != src_viewer->view_mode_state_->get() )
    {
      viewer->move_slice_to( pt );
    }
  }
  if ( this->active_coronal_viewer_->get() >= 0 && 
    this->active_coronal_viewer_->get() != static_cast< int >( source_viewer ) )
  {
    ViewerHandle viewer = this->viewers_[ this->active_coronal_viewer_->get() ];
    if ( viewer->view_mode_state_->get() != src_viewer->view_mode_state_->get() )
    {
      viewer->move_slice_to( pt );
    }
  }
  if ( this->active_sagittal_viewer_->get() >= 0 && 
    this->active_sagittal_viewer_->get() != static_cast< int >( source_viewer ) )
  {
    ViewerHandle viewer = this->viewers_[ this->active_sagittal_viewer_->get() ];
    if ( viewer->view_mode_state_->get() != src_viewer->view_mode_state_->get() )
    {
      viewer->move_slice_to( pt );
    }
  }
}

std::vector< size_t > ViewerManager::get_locked_viewers( int mode_index )
{
  return this->locked_viewers_[ mode_index ];
}

void ViewerManager::viewer_lock_state_changed( size_t viewer_id )
{
  ViewerHandle viewer = this->viewers_[ viewer_id ];
  bool locked = viewer->lock_state_->get();
  if ( locked )
  {
    this->locked_viewers_[ viewer->view_mode_state_->index() ].push_back( viewer_id );
  }
  else
  {
    // We need to go over the locked viewer list for all modes because a viewer can become
    // unlocked due to change of mode.
    bool found = false;
    for ( int i = 0; i < 4; i++ )
    {
      for ( size_t j = 0; j < this->locked_viewers_[ i ].size(); j++ )
      {
        if ( this->locked_viewers_[ i ][ j ] == viewer_id )
        {
          this->locked_viewers_[ i ].erase( this->locked_viewers_[ i ].begin() + j );
          found =  true;
          break;
        }
      }
      if ( found )
      {
        break;
      }
    }
    assert( found );
  }
}

bool ViewerManager::post_save_states()
{
  std::vector< std::string > viewers_vector = this->viewers_state_->get();

  if( viewers_vector.size() != viewers_.size() )
    return false;

  for( size_t i = 0; i < viewers_vector.size(); ++i )
  {
    if( ( viewers_vector[ i ] != "]" ) && ( viewers_vector[ i ] != "\0" ) )
    {
      if( !( viewers_[ i ] )->populate_session_states() )
      {
        return false;
      }
    }
  }
  return true;
}

bool ViewerManager::post_load_states()
{
  std::vector< std::string > viewers_vector = this->viewers_state_->get();
  for( size_t i = 0; i < viewers_vector.size(); ++i )
  {
    if( ( viewers_vector[ i ] != "]" ) && ( viewers_vector[ i ] != "\0" ) )
    {
      std::vector< std::string > state_values;
      Core::StateEngine::Instance()->get_session_states( state_values );
      // TODO: Need to implement signal blocking before we can load the viewers
      if( !( viewers_[ i ] )->load_states( state_values ) )
      {
        return false;
      }
    }
  }
  return true;
}

} // end namespace Seg3D

