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

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/State/Actions/ActionSet.h>
#include <Application/Viewer/Viewer.h>
#include <Application/Viewer/ViewManipulator.h>

namespace Seg3D
{

const std::string Viewer::AXIAL_C( "axial" );
const std::string Viewer::SAGITTAL_C( "sagittal" );
const std::string Viewer::CORONAL_C( "coronal" );
const std::string Viewer::VOLUME_C( "volume" );

Viewer::Viewer( const std::string& key ) :
  StateHandler( key )
{
  add_state( "view_mode", view_mode_state_, AXIAL_C, AXIAL_C + StateOption::SPLITTER_C
      + CORONAL_C + StateOption::SPLITTER_C + SAGITTAL_C + StateOption::SPLITTER_C + VOLUME_C );

  add_state( "axial_view", axial_view_state_ );
  add_state( "coronal_view", coronal_view_state_ );
  add_state( "sagittal_view", sagittal_view_state_ );
  add_state( "volume_view", volume_view_state_ );

  add_state( "axial_slice_number", this->axial_slice_number_state_, 0, 0, 0, 1 );
  add_state( "coronal_slice_number", this->coronal_slice_number_state_, 0, 0, 0, 1 );
  add_state( "sagittal_slice_number", this->sagittal_slice_number_state_, 0, 0, 0, 1 );

  add_state( "slice_lock", slice_lock_state_, true );
  add_state( "slice_grid", slice_grid_state_, true );
  add_state( "slice_visible", slice_visible_state_, true );

  add_state( "volume_lock", volume_lock_state_, true );
  add_state( "volume_slices_visible", volume_slices_visible_state_, true );
  add_state( "volume_isosurfaces_visible", volume_isosurfaces_visible_state_, true );
  add_state( "volume_volume_rendering_visible", volume_volume_rendering_visible_state_, false );

  this->view_manipulator_ = ViewManipulatorHandle( new ViewManipulator( this ) );
  this->add_connection( LayerManager::Instance()->layer_inserted_signal_.connect( 
    boost::bind( &Viewer::insert_layer, this, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layer_deleted_signal_.connect(
    boost::bind( &Viewer::delete_layer, this, _1 ) ) );
  this->add_connection( this->view_mode_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::change_view_mode, this, _1, _2 ) ) );
}

Viewer::~Viewer()
{
  this->disconnect_all();
}

void Viewer::resize( int width, int height )
{
  this->view_manipulator_->resize( width, height );
}

void Viewer::mouse_move_event( const MouseHistory& mouse_history, int button, int buttons,
    int modifiers )
{
  if ( !mouse_move_handler_.empty() )
  {
    // if the registered handler handled the event, no further process needed
    if ( mouse_move_handler_( mouse_history, button, buttons, modifiers ) )
    {
      return;
    }
  }

  // default handling here
  this->view_manipulator_->mouse_move( mouse_history, button, buttons, modifiers );
}

void Viewer::mouse_press_event( const MouseHistory& mouse_history, int button, int buttons,
    int modifiers )
{
  if ( !mouse_press_handler_.empty() )
  {
    // if the registered handler handled the event, no further process needed
    if ( mouse_press_handler_( mouse_history, button, buttons, modifiers ) )
    {
      return;
    }
  }

  // default handling here
  this->view_manipulator_->mouse_press( mouse_history, button, buttons, modifiers );
}

void Viewer::mouse_release_event( const MouseHistory& mouse_history, int button, int buttons,
    int modifiers )
{
  if ( !mouse_release_handler_.empty() )
  {
    // if the registered handler handled the event, no further process needed
    if ( mouse_release_handler_( mouse_history, button, buttons, modifiers ) )
    {
      return;
    }
  }

  // default handling here
  this->view_manipulator_->mouse_release( mouse_history, button, buttons, modifiers );
}

void Viewer::set_mouse_move_handler( mouse_event_handler_type func )
{
  this->mouse_move_handler_ = func;
}

void Viewer::set_mouse_press_handler( mouse_event_handler_type func )
{
  this->mouse_press_handler_ = func;
}

void Viewer::set_mouse_release_handler( mouse_event_handler_type func )
{
  this->mouse_release_handler_ = func;
}

void Viewer::reset_mouse_handlers()
{
  this->mouse_move_handler_ = 0;
  this->mouse_press_handler_ = 0;
  this->mouse_release_handler_ = 0;
}

void Viewer::state_changed()
{
  redraw_signal_();
}

bool Viewer::is_volume_view() const
{
  return this->view_mode_state_->get() == VOLUME_C;
}

StateViewBaseHandle Viewer::get_active_view_state()
{
  std::string view_mode = this->view_mode_state_->get();
  if ( view_mode == VOLUME_C )
  {
    return this->volume_view_state_;
  }
  else if ( view_mode == AXIAL_C )
  {
    return this->axial_view_state_;
  }
  else if ( view_mode == CORONAL_C )
  {
    return this->coronal_view_state_;
  }
  else
  {
    return this->sagittal_view_state_;
  }
}

void Viewer::insert_layer( LayerHandle layer )
{
  lock_type lock( this->layer_map_mutex_ );

  bool first_layer = false;

  Utils::VolumeSliceType slice_type( Utils::VolumeSliceType::AXIAL_E );
  size_t slice_number = static_cast< size_t >( this->axial_slice_number_state_->get() );
  if ( this->view_mode_state_->get() == CORONAL_C )
  {
    slice_type = Utils::VolumeSliceType::CORONAL_E;
    slice_number = static_cast< size_t >( this->coronal_slice_number_state_->get() );
  }
  else if ( this->view_mode_state_->get() == SAGITTAL_C )
  {
    slice_type = Utils::VolumeSliceType::SAGITTAL_E;
    slice_number = static_cast< size_t >( this->sagittal_slice_number_state_->get() );
  }

  switch( layer->type() )
  {
  case Utils::VolumeType::DATA_E:
    {
      if ( this->data_slices_.empty() )
      {
        first_layer = true;
      }
      DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
      Utils::DataVolumeHandle data_volume = data_layer->get_data_volume();
      Utils::DataVolumeSliceHandle data_volume_slice( 
        new Utils::DataVolumeSlice( data_volume, slice_type, slice_number ) );
      this->data_slices_[ layer->get_layer_id() ] = data_volume_slice;
    }
    break;
  case Utils::VolumeType::MASK_E:
    {
      MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
      Utils::MaskVolumeHandle mask_volume = mask_layer->get_mask_volume();
      Utils::MaskVolumeSliceHandle mask_volume_slice(
        new Utils::MaskVolumeSlice( mask_volume, slice_type, slice_number ) );
      this->mask_slices_[ layer->get_layer_id() ] = mask_volume_slice;
    }
    break;
  default:
    // Should never reach here.
    assert( false );
  }

  if ( first_layer )
  {
    this->adjust_view();
  }
}

void Viewer::delete_layer( LayerHandle layer )
{
  lock_type lock( this->layer_map_mutex_ );

  switch( layer->type() )
  {
  case Utils::VolumeType::DATA_E:
    {
      data_slices_map_type::iterator it = this->data_slices_.find( layer->get_layer_id() );
      assert( it != this->data_slices_.end() );
      this->data_slices_.erase( it );
    }
    break;
  case Utils::VolumeType::MASK_E:
    {
      mask_slices_map_type::iterator it = this->mask_slices_.find( layer->get_layer_id() );
      assert( it != this->mask_slices_.end() );
      this->mask_slices_.erase( it );
    }
    break;
  default:
    // Should never reach here.
    assert( false );
  }
}

Utils::MaskVolumeSliceHandle Viewer::get_mask_volume_slice( const std::string& layer_id )
{
  mask_slices_map_type::iterator it = this->mask_slices_.find( layer_id );
  if ( it != this->mask_slices_.end() )
  {
    return ( *it ).second;
  }
  else
  {
    // Should never reach here
    assert ( false );
    return Utils::MaskVolumeSliceHandle();
  }
}

Utils::DataVolumeSliceHandle Viewer::get_data_volume_slice( const std::string& layer_id )
{
  data_slices_map_type::iterator it = this->data_slices_.find( layer_id );
  if ( it != this->data_slices_.end() )
  {
    return ( *it ).second;
  }
  else
  {
    // Should never reach here
    assert ( false );
    return Utils::DataVolumeSliceHandle();
  }
}

void Viewer::change_view_mode( std::string mode, ActionSource source )
{
  if ( mode == VOLUME_C )
  {
    return;
  }

  Utils::VolumeSliceType slice_type( Utils::VolumeSliceType::AXIAL_E );
  if ( mode == CORONAL_C )
  {
    slice_type = Utils::VolumeSliceType::CORONAL_E;
  }
  else if ( mode == SAGITTAL_C )
  {
    slice_type =  Utils::VolumeSliceType::SAGITTAL_E;
  }

  mask_slices_map_type::iterator mask_slices_it = this->mask_slices_.begin();
  for ( ; mask_slices_it != this->mask_slices_.end(); mask_slices_it++ )
  {
    ( *mask_slices_it ).second->set_slice_type( slice_type );
  }

  data_slices_map_type::iterator data_slices_it = this->data_slices_.begin();
  for ( ; data_slices_it != this->data_slices_.end(); data_slices_it++ )
  {
    ( *data_slices_it ).second->set_slice_type( slice_type );
  }
}

void Viewer::adjust_view()
{
  Utils::DataVolumeSliceHandle data_volume_slice = Utils::DataVolumeSliceHandle( 
    new Utils::DataVolumeSlice( *( *this->data_slices_.begin() ).second ) );

  double left, right, bottom, top;
  double scale;
  Utils::Point center;

  data_volume_slice->set_slice_type( Utils::VolumeSliceType::AXIAL_E );
  data_volume_slice->get_world_space_boundary_2d( left, right, bottom, top );
  center = Utils::Point( ( left + right ) * 0.5, ( bottom + top ) * 0.5, 0.0 );
  scale = 1.0 / Utils::Max( Utils::Abs( right - left ), Utils::Abs( top - bottom ) );
  ActionSet::Dispatch( this->axial_view_state_, Utils::View2D( center, scale ) );

  data_volume_slice->set_slice_type( Utils::VolumeSliceType::CORONAL_E );
  data_volume_slice->get_world_space_boundary_2d( left, right, bottom, top );
  center = Utils::Point( ( left + right ) * 0.5, ( bottom + top ) * 0.5, 0.0 );
  scale = 1.0 / Utils::Max( Utils::Abs( right - left ), Utils::Abs( top - bottom ) );
  ActionSet::Dispatch( this->coronal_view_state_, Utils::View2D( center, scale ) );

  data_volume_slice->set_slice_type( Utils::VolumeSliceType::SAGITTAL_E );
  data_volume_slice->get_world_space_boundary_2d( left, right, bottom, top );
  center = Utils::Point( ( left + right ) * 0.5, ( bottom + top ) * 0.5, 0.0 );
  scale = 1.0 / Utils::Max( Utils::Abs( right - left ), Utils::Abs( top - bottom ) );
  ActionSet::Dispatch( this->sagittal_view_state_, Utils::View2D( center, scale ) );
}

} // end namespace Seg3D

