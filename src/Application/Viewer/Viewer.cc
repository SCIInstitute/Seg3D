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
  StateHandler( key ), 
  ignore_state_changes_( false )
{
  add_state( "view_mode", view_mode_state_, AXIAL_C, AXIAL_C + StateOption::SPLITTER_C
      + CORONAL_C + StateOption::SPLITTER_C + SAGITTAL_C + StateOption::SPLITTER_C + VOLUME_C );

  add_state( "axial_view", axial_view_state_ );
  add_state( "coronal_view", coronal_view_state_ );
  add_state( "sagittal_view", sagittal_view_state_ );
  add_state( "volume_view", volume_view_state_ );

  add_state( "slice_number", this->slice_number_state_, 0, 0, 0, 1 );
  this->slice_numbers_[ 0 ] = this->slice_numbers_[ 1 ] = this->slice_numbers_[ 2 ] = 0;

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
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &Viewer::set_active_layer, this, _1 ) ) );
  this->add_connection( this->view_mode_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::change_view_mode, this, _1, _2 ) ) );
  this->add_connection( this->slice_number_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::set_slice_number, this, _1, _2 ) ) );
}

Viewer::~Viewer()
{
  this->disconnect_all();
}

void Viewer::resize( int width, int height )
{
  this->width_ = width;
  this->height_ = height;
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

  if ( buttons == MouseButton::NO_BUTTON_E )
  {
    this->update_status_bar( mouse_history.current.x, mouse_history.current.y );
    return;
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

bool Viewer::wheel_event( int delta, int x, int y, int buttons, int modifiers )
{
  if ( !this->is_volume_view() && delta != 0 && this->active_layer_slice_ )
  {
    int slice_num = static_cast<int>( this->active_layer_slice_->get_slice_number() );
    int new_slice = static_cast<int>( slice_num - delta );
    if ( new_slice >= 0 && 
       static_cast< size_t >( new_slice ) < this->active_layer_slice_->number_of_slices() )
    {
      ActionSet::Dispatch( this->slice_number_state_, new_slice );
    }
  }

  return true;
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

void Viewer::update_status_bar( int x, int y )
{
  // TODO: update status bar here
  if ( !this->is_volume_view() && this->active_layer_slice_ &&
     this->active_layer_slice_->volume_type() == Utils::VolumeType::DATA_E )
  {
    Utils::DataVolumeSlice* data_slice = dynamic_cast< Utils::DataVolumeSlice* >(
      this->active_layer_slice_.get() );
    // Scale the mouse position to [-1, 1]
    double xpos = x * 2.0 / ( this->width_ - 1 ) - 1.0;
    double ypos = ( this->height_ - 1 - y ) * 2.0 / ( this->height_ - 1 ) - 1.0;
    double left, right, bottom, top;
    StateView2D* view_2d = dynamic_cast<StateView2D*>( this->get_active_view_state().get() );
    view_2d->get().compute_clipping_planes( this->width_ * 1.0 / this->height_, left, right, bottom, top );
    Utils::Matrix proj, inv_proj;
    Utils::Transform::BuildOrtho2DMatrix( proj, left, right, bottom, top );
    Utils::Invert( proj, inv_proj );
    Utils::Point pos( xpos, ypos, 0 );
    pos = inv_proj * pos;
    int i, j;
    data_slice->world_to_index( pos.x(), pos.y(), i, j );
    Utils::Point index;
    if ( i >= 0 && static_cast<size_t>( i ) < data_slice->nx() && 
       j >= 0 && static_cast<size_t>( j ) < data_slice->ny() )
    {
      data_slice->to_index( static_cast<size_t>( i ), static_cast<size_t>( j ), index );
      Utils::Point world_pos = data_slice->apply_grid_transform( index );
      double value = data_slice->get_data_at( static_cast<size_t>( i ), static_cast<size_t>( j ) );
      SCI_LOG_DEBUG( std::string( "Index: " ) + Utils::to_string( index ) 
        + " World Pos: " + Utils::to_string( world_pos ) 
        + " Value: " + Utils::to_string( value ) );
    }
  }
}

void Viewer::state_changed()
{
  if ( !this->ignore_state_changes_ )
  {
    this->redraw_signal_();
  }
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
  size_t slice_number = static_cast< size_t >( this->slice_numbers_[ 0 ] );
  if ( this->view_mode_state_->get() == CORONAL_C )
  {
    slice_type = Utils::VolumeSliceType::CORONAL_E;
    slice_number = static_cast< size_t >( this->slice_numbers_[ 1 ] );
  }
  else if ( this->view_mode_state_->get() == SAGITTAL_C )
  {
    slice_type = Utils::VolumeSliceType::SAGITTAL_E;
    slice_number = static_cast< size_t >( this->slice_numbers_[ 2 ] );
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

  lock.unlock();

  if ( first_layer )
  {
    this->ignore_state_changes_ = true;
    this->adjust_view();
    // TODO: remove the following line. Hardcoded for testing.
    this->set_active_layer( layer );
    this->ignore_state_changes_ = false;
  }

  this->redraw_signal_();
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

  lock.unlock();
  this->redraw_signal_();
}

void Viewer::set_active_layer( LayerHandle layer )
{
  data_slices_map_type::iterator data_slice_it = this->data_slices_.find( layer->get_layer_id() );
  if ( data_slice_it != this->data_slices_.end() )
  {
    this->active_layer_slice_ = ( *data_slice_it ).second;
  }
  else
  {
    mask_slices_map_type::iterator mask_slice_it = this->mask_slices_.find( layer->get_layer_id() );
    if ( mask_slice_it != this->mask_slices_.end() )
    {
      this->active_layer_slice_ = ( *mask_slice_it ).second;
    }
    else
    {
      assert( false );
    }
  }

  // Update slice number ranges
  if ( this->active_layer_slice_ && !this->is_volume_view() )
  {
    // Disable redraws triggered by StateBase::state_changed_signal_
    this->ignore_state_changes_ = true;

    // NOTE: The following state changes are due to internal program logic, 
    // so they should not go through the action mechanism.

    this->updating_states_ = true;
    this->slice_number_state_->set_range(
      0, static_cast< int >( this->active_layer_slice_->number_of_slices() - 1 ) );
    this->updating_states_ = false;
    const std::string& view_mode = this->view_mode_state_->get();
    if ( view_mode == AXIAL_C )
    {
      this->slice_number_state_->set( this->slice_numbers_[ 0 ] );
    }
    else if ( view_mode == CORONAL_C )
    {
      this->slice_number_state_->set( this->slice_numbers_[ 1 ] );
    }
    else
    {
      this->slice_number_state_->set( this->slice_numbers_[ 2 ] );
    }

    // Enable redraws
    this->ignore_state_changes_ = false;
    this->redraw_signal_();
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

  if ( this->active_layer_slice_ )
  {
    // NOTE: The following state changes are due to internal program logic, 
    // so they should not go through the action mechanism.

    this->ignore_state_changes_ = true;
    this->updating_states_ = true;
    this->slice_number_state_->set_range(
      0, static_cast< int >( this->active_layer_slice_->number_of_slices() - 1 ) );
    this->updating_states_ = false;
    this->slice_number_state_->set( this->slice_numbers_[ slice_type ] );
    this->ignore_state_changes_ = false;
  }
}

void Viewer::set_slice_number( int num, ActionSource source )
{
  const std::string& view_mode = this->view_mode_state_->get();
  if ( this->updating_states_ || view_mode == VOLUME_C )
  {
    return;
  }

  // Update the cached slice number
  if ( view_mode == AXIAL_C )
  {
    this->slice_numbers_[ 0 ] = num;
  }
  else if ( view_mode == CORONAL_C )
  {
    this->slice_numbers_[ 1 ] = num;
  }
  else
  {
    this->slice_numbers_[ 2 ] = num;
  }

  this->active_layer_slice_->set_slice_number( num );
  // Get the world coordinate of the slice in the active layer
  Utils::Point slice_pos;
  this->active_layer_slice_->to_index( 0, 0, slice_pos );
  slice_pos = this->active_layer_slice_->apply_grid_transform( slice_pos );

  // Move other layer slices to the new position
  mask_slices_map_type::iterator mask_slice_it = this->mask_slices_.begin();
  for ( ; mask_slice_it != this->mask_slices_.end(); mask_slice_it++ )
  {
    if ( ( *mask_slice_it ).second == this->active_layer_slice_ )
      continue;
    ( *mask_slice_it ).second->move_slice( slice_pos );
  }
  data_slices_map_type::iterator data_slice_it = this->data_slices_.begin();
  for ( ; data_slice_it != this->data_slices_.end(); data_slice_it++ )
  {
    if ( ( *data_slice_it ).second == this->active_layer_slice_ )
      continue;
    ( *data_slice_it ).second->move_slice( slice_pos );
  }
}

void Viewer::adjust_view()
{
  Utils::DataVolumeSliceHandle volume_slice = Utils::DataVolumeSliceHandle( 
    new Utils::DataVolumeSlice( *( *this->data_slices_.begin() ).second ) );

  double scale;
  Utils::Point center;

  volume_slice->set_slice_type( Utils::VolumeSliceType::AXIAL_E );
  center = Utils::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
    ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 0.0 );
  scale = 1.0 / Utils::Abs( volume_slice->top() - volume_slice->bottom() );
  this->axial_view_state_->set( Utils::View2D( center, scale ) );
  this->slice_numbers_[ 0 ] = static_cast< int >( volume_slice->number_of_slices() / 2 );

  volume_slice->set_slice_type( Utils::VolumeSliceType::CORONAL_E );
  center = Utils::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
    ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 0.0 );
  scale = 1.0 / Utils::Abs( volume_slice->top() - volume_slice->bottom() );
  this->coronal_view_state_->set( Utils::View2D( center, scale ) );
  this->slice_numbers_[ 1 ] = static_cast< int >( volume_slice->number_of_slices() / 2 );

  volume_slice->set_slice_type( Utils::VolumeSliceType::SAGITTAL_E );
  center = Utils::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
    ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 0.0 );
  scale = 1.0 / Utils::Abs( volume_slice->top() - volume_slice->bottom() );
  this->sagittal_view_state_->set( Utils::View2D( center, scale ) );
  this->slice_numbers_[ 2 ] = static_cast< int >( volume_slice->number_of_slices() / 2 );
}

} // end namespace Seg3D

