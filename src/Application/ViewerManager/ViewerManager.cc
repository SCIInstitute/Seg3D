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
#include <Application/Interface/Interface.h>

// Utils includes
#include <Utils/Core/ScopedCounter.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ViewerManager );

ViewerManager::ViewerManager() :
  StateHandler( "view" ),
  active_axial_viewer_( -1 ),
  active_coronal_viewer_( -1 ),
  active_sagittal_viewer_( -1 ),
  signal_block_count_( 0 )
{
  // Step (1)
  // Set the default state of this element
  this->add_state( "layout", this->layout_state_, "1and3", 
    "single|1and1|1and2|1and3|2and2|2and3|3and3" );
  this->add_state( "active_viewer", this->active_viewer_state_, 0 );

  // Step (2)
  // Create the viewers that are part of the application
  // Currently a maximum of 6 viewers can be created
  this->viewers_.resize( 6 );
  for ( size_t j = 0; j < viewers_.size(); j++ )
  {
    this->viewers_[ j ] = ViewerHandle( new Viewer( j ) );

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
    this->add_connection( this->viewers_[ j ]->viewer_lock_state_->value_changed_signal_.
      connect( boost::bind( &ViewerManager::viewer_lock_state_changed, this, j ), 
      boost::signals2::at_front ) );
  }
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
    if ( this->viewers_[ i ]->get_stateid() == viewer_name )
    {
      handle = this->viewers_[ i ];
      break;
    }
  }
  return handle;
}

void ViewerManager::get_2d_viewers_info( ViewerInfoList viewers[ 3 ] )
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );
  if ( !LayerManager::Instance()->get_active_layer() )
  {
    return;
  }
  for ( size_t i = 0; i < 6; i++ )
  {
    ViewerHandle viewer = this->viewers_[ i ];
    if ( viewer->viewer_visible_state_->get() && !viewer->is_volume_view() )
    {
      StateView2D* view2d = static_cast< StateView2D* >( viewer->get_active_view_state().get() );
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
  Utils::ScopedCounter signal_block_counter( this->signal_block_count_ );

  if ( this->active_axial_viewer_ == static_cast< int >( viewer_id ) )
  {
    this->active_axial_viewer_ = -1;
  }
  else if ( this->active_coronal_viewer_ == static_cast< int >( viewer_id ) )
  {
    this->active_coronal_viewer_ = -1;
  }
  else if ( this->active_sagittal_viewer_ == static_cast< int >( viewer_id ) )
  {
    this->active_sagittal_viewer_ = -1;
  }
  
  this->viewers_[ viewer_id ]->is_picking_target_state_->set( false );
  this->update_picking_targets();
}

void ViewerManager::viewer_visibility_changed( size_t viewer_id )
{
  Utils::ScopedCounter signal_block_counter( this->signal_block_count_ );

  if ( !this->viewers_[ viewer_id ]->viewer_visible_state_->get() )
  {
    if ( this->active_axial_viewer_ == static_cast< int >( viewer_id ) )
    {
      this->active_axial_viewer_ = -1;
    }
    else if ( this->active_coronal_viewer_ == static_cast< int >( viewer_id ) )
    {
      this->active_coronal_viewer_ = -1;
    }
    else if ( this->active_sagittal_viewer_ == static_cast< int >( viewer_id ) )
    {
      this->active_sagittal_viewer_ = -1;
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
    Utils::ScopedCounter signal_block_counter( this->signal_block_count_ );

    if ( viewer->view_mode_state_->get() == Viewer::AXIAL_C )
    {
      if ( this->active_axial_viewer_ >= 0 )
      {
        assert( this->active_axial_viewer_ != viewer_id );
        this->viewers_[ this->active_axial_viewer_ ]->is_picking_target_state_->set( false );
      }
      this->active_axial_viewer_ = static_cast< int >( viewer_id );
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      if ( this->active_coronal_viewer_ >= 0 )
      {
        assert( this->active_coronal_viewer_ != viewer_id );
        this->viewers_[ this->active_coronal_viewer_ ]->is_picking_target_state_->set( false );
      }
      this->active_coronal_viewer_ = static_cast< int >( viewer_id );
    }
    else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      if ( this->active_sagittal_viewer_ >= 0 )
      {
        assert( this->active_sagittal_viewer_ != viewer_id );
        this->viewers_[ this->active_sagittal_viewer_ ]->is_picking_target_state_->set( false );
      }
      this->active_sagittal_viewer_ = static_cast< int >( viewer_id );
    }
    else
    {
      assert( false );
    }
  }

  this->picking_target_changed_signal_( viewer_id );
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
      if ( this->active_axial_viewer_ < 0 )
      {
        this->active_axial_viewer_ = static_cast< int >( i );
        viewer->is_picking_target_state_->set( true );
      }
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      if ( this->active_coronal_viewer_ < 0 )
      {
        this->active_coronal_viewer_ = static_cast< int >( i );
        viewer->is_picking_target_state_->set( true );
      }
    }
    else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      if ( this->active_sagittal_viewer_ < 0 )
      {
        this->active_sagittal_viewer_ = static_cast< int >( i );
        viewer->is_picking_target_state_->set( true );
      }
    }
  }
}

void ViewerManager::pick_point( size_t source_viewer, const Utils::Point& pt )
{
  ViewerHandle src_viewer = this->viewers_[ source_viewer ];
  if ( this->active_axial_viewer_ >= 0 && 
    this->active_axial_viewer_ != static_cast< int >( source_viewer ) )
  {
    ViewerHandle viewer = this->viewers_[ this->active_axial_viewer_ ];
    if ( viewer->view_mode_state_->get() != src_viewer->view_mode_state_->get() )
    {
      viewer->move_slice_to( pt );
    }
  }
  if ( this->active_coronal_viewer_ >= 0 && 
    this->active_coronal_viewer_ != static_cast< int >( source_viewer ) )
  {
    ViewerHandle viewer = this->viewers_[ this->active_coronal_viewer_ ];
    if ( viewer->view_mode_state_->get() != src_viewer->view_mode_state_->get() )
    {
      viewer->move_slice_to( pt );
    }
  }
  if ( this->active_sagittal_viewer_ >= 0 && 
    this->active_sagittal_viewer_ != static_cast< int >( source_viewer ) )
  {
    ViewerHandle viewer = this->viewers_[ this->active_sagittal_viewer_ ];
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
  bool locked = viewer->viewer_lock_state_->get();
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

} // end namespace Seg3D

