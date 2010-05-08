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
#include <Core/Utils/ScopedCounter.h>
#include <Core/Interface/StatusBar.h>
#include <Core/State/Actions/ActionSet.h>

// Application includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/Viewer/ViewManipulator.h>
#include <Application/ViewerManager/Actions/ActionPickPoint.h>
#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{

const std::string Viewer::AXIAL_C( "axial" );
const std::string Viewer::SAGITTAL_C( "sagittal" );
const std::string Viewer::CORONAL_C( "coronal" );
const std::string Viewer::VOLUME_C( "volume" );

Viewer::Viewer( size_t viewer_id ) :
  StateHandler( std::string( "viewer" ) + Core::ToString( viewer_id ) ),
  adjusting_contrast_brightness_( false ),
  viewer_id_( viewer_id ),
  signals_block_count_( 0 ),
  slice_lock_count_( 0 )
{
  add_state( "view_mode", view_mode_state_, AXIAL_C, AXIAL_C + Core::StateOption::SPLITTER_C
      + CORONAL_C + Core::StateOption::SPLITTER_C + SAGITTAL_C + Core::StateOption::SPLITTER_C + VOLUME_C );

  add_state( "axial_view", axial_view_state_ );
  add_state( "coronal_view", coronal_view_state_ );
  add_state( "sagittal_view", sagittal_view_state_ );
  add_state( "volume_view", volume_view_state_ );

  this->view_states_[ 0 ] = this->axial_view_state_;
  this->view_states_[ 1 ] = this->coronal_view_state_;
  this->view_states_[ 2 ] = this->sagittal_view_state_;
  this->view_states_[ 3 ] = this->volume_view_state_;

  add_state( "slice_number", this->slice_number_state_, 0, 0, 0, 1 );

  add_state( "slice_lock", viewer_lock_state_, false );
  add_state( "slice_grid", slice_grid_state_, true );
  add_state( "slice_visible", slice_visible_state_, true );

  add_state( "volume_slices_visible", volume_slices_visible_state_, true );
  add_state( "volume_isosurfaces_visible", volume_isosurfaces_visible_state_, true );
  add_state( "volume_volume_rendering_visible", volume_volume_rendering_visible_state_, false );

  add_state( "viewer_visible", this->viewer_visible_state_, false );
  add_state( "is_picking_target", this->is_picking_target_state_, false );

  this->view_manipulator_ = ViewManipulatorHandle( new ViewManipulator( this ) );
  this->add_connection( LayerManager::Instance()->layer_inserted_signal_.connect( 
    boost::bind( &Viewer::insert_layer, this, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layers_deleted_signal_.connect(
    boost::bind( &Viewer::delete_layers, this, _1 ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &Viewer::set_active_layer, this, _1 ) ) );
  this->add_connection( this->view_mode_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::change_view_mode, this, _1, _2 ) ) );
  this->add_connection( this->slice_number_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::set_slice_number, this, _1, _2 ) ) );
  this->add_connection( this->viewer_visible_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::change_visibility, this, _1, _2 ) ) );
  this->add_connection( this->viewer_lock_state_->value_changed_signal_.connect(
    boost::bind( &Viewer::viewer_lock_state_changed, this, _1 ) ) );

  // Connect state variables that should trigger redraw.
  // For those state variables that will trigger both redraw and redraw_overlay, 
  // "delay_update" is set to true for redraw.
  this->add_connection( this->view_mode_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw, this, true ) ) );
  this->add_connection( this->axial_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw, this, true ) ) );
  this->add_connection( this->coronal_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw, this, true ) ) );
  this->add_connection( this->sagittal_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw, this, true ) ) );
  this->add_connection( this->volume_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw, this, false ) ) );
  this->add_connection( this->slice_number_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw, this, false ) ) );

  // Connect state variables that should trigger redraw_overlay
  this->add_connection( this->view_mode_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw_overlay, this, false ) ) );
  this->add_connection( this->axial_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw_overlay, this, false ) ) );
  this->add_connection( this->coronal_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw_overlay, this, false ) ) );
  this->add_connection( this->sagittal_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw_overlay, this, false ) ) );
  this->add_connection( this->slice_grid_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::trigger_redraw_overlay, this, false ) ) );
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

  if ( this->adjusting_contrast_brightness_ )
  {
    this->adjust_contrast_brightness( mouse_history.current.x - mouse_history.previous.x,
      mouse_history.previous.y - mouse_history.current.y );
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

  if ( button == MouseButton::RIGHT_BUTTON_E &&
    modifiers == KeyModifier::NO_MODIFIER_E &&
    !this->is_volume_view() )
  {
    this->pick_point( mouse_history.current.x, mouse_history.current.y );
    return;
  }

  if ( button == MouseButton::LEFT_BUTTON_E &&
    modifiers == KeyModifier::NO_MODIFIER_E &&
    !this->is_volume_view() )
  {
    this->adjusting_contrast_brightness_ = true;
    return;
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

  if ( button == MouseButton::LEFT_BUTTON_E )
  {
    this->adjusting_contrast_brightness_ = false;
  }
  
  // default handling here
  this->view_manipulator_->mouse_release( mouse_history, button, buttons, modifiers );
}

bool Viewer::wheel_event( int delta, int x, int y, int buttons, int modifiers )
{
  this->offset_slice( -delta );

  // Update the status bar display.
  // 'update_status_bar' reposts itself to the application thread, so it's guaranteed that 
  // by the time it actually runs, the slice number has been updated.
  this->update_status_bar( x, y );
  
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
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Viewer::update_status_bar, this, x, y ) );
    return;
  }

  if ( !this->is_volume_view() && 
    this->active_layer_slice_ &&
    !this->active_layer_slice_->out_of_boundary() )
  {
    Core::VolumeSlice* volume_slice = this->active_layer_slice_.get();
    // Scale the mouse position to [-1, 1]
    double xpos = x * 2.0 / ( this->width_ - 1 ) - 1.0;
    double ypos = ( this->height_ - 1 - y ) * 2.0 / ( this->height_ - 1 ) - 1.0;
    double left, right, bottom, top;
    Core::StateView2D* view_2d = dynamic_cast<Core::StateView2D*>( this->get_active_view_state().get() );
    view_2d->get().compute_clipping_planes( this->width_ * 1.0 / this->height_, left, right, bottom, top );
    Core::Matrix proj, inv_proj;
    Core::Transform::BuildOrtho2DMatrix( proj, left, right, bottom, top );
    Core::Invert( proj, inv_proj );
    Core::Point pos( xpos, ypos, 0 );
    pos = inv_proj * pos;
    int i, j;
    volume_slice->world_to_index( pos.x(), pos.y(), i, j );
    Core::Point index;
    if ( i >= 0 && static_cast<size_t>( i ) < volume_slice->nx() && 
       j >= 0 && static_cast<size_t>( j ) < volume_slice->ny() )
    {
      volume_slice->to_index( static_cast<size_t>( i ), static_cast<size_t>( j ), index );
      Core::Point world_pos = volume_slice->apply_grid_transform( index );
      double value = 0.0;
      if ( volume_slice->volume_type() == Core::VolumeType::DATA_E )
      {
        Core::DataVolumeSlice* data_slice = dynamic_cast< Core::DataVolumeSlice* >( volume_slice );
        value = data_slice->get_data_at( static_cast<size_t>( i ), static_cast<size_t>( j ) );
      }
      else
      {
        Core::MaskVolumeSlice* mask_slice = dynamic_cast< Core::MaskVolumeSlice* >( volume_slice );
        value = mask_slice->get_mask_at( static_cast<size_t>( i ), static_cast<size_t>( j ) );
      }
      Core::DataPointInfoHandle data_point( new Core::DataPointInfo( index, world_pos, value ) );
      Core::StatusBar::Instance()->set_data_point_info( data_point );
    }
    else
    {
      Core::DataPointInfoHandle data_point( new Core::DataPointInfo );
      Core::StatusBar::Instance()->set_data_point_info( data_point );
    }
  }
}

void Viewer::pick_point( int x, int y )
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Viewer::pick_point, this, x, y ) );
    return;
  }
  
  if ( !this->is_volume_view() && this->active_layer_slice_ )
  {
    Core::VolumeSlice* volume_slice = this->active_layer_slice_.get();
    // Scale the mouse position to [-1, 1]
    double xpos = x * 2.0 / ( this->width_ - 1 ) - 1.0;
    double ypos = ( this->height_ - 1 - y ) * 2.0 / ( this->height_ - 1 ) - 1.0;
    double left, right, bottom, top;
    Core::StateView2D* view_2d = dynamic_cast<Core::StateView2D*>( this->get_active_view_state().get() );
    view_2d->get().compute_clipping_planes( this->width_ * 1.0 / this->height_, left, right, bottom, top );
    Core::Matrix proj, inv_proj;
    Core::Transform::BuildOrtho2DMatrix( proj, left, right, bottom, top );
    Core::Invert( proj, inv_proj );
    Core::Point pos( xpos, ypos, 0 );
    pos = inv_proj * pos;
    volume_slice->get_world_coord( pos.x(), pos.y(), pos );

    ActionPickPoint::Dispatch( this->viewer_id_, pos );
  }
}

void Viewer::state_changed()
{
}

bool Viewer::is_volume_view() const
{
  return this->view_mode_state_->get() == VOLUME_C;
}

Core::StateViewBaseHandle Viewer::get_active_view_state()
{
  return this->view_states_[ this->view_mode_state_->index() ];
}

void Viewer::insert_layer( LayerHandle layer )
{
  lock_type lock( this->get_mutex() );

  Core::VolumeSliceHandle volume_slice;

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  if ( this->view_mode_state_->get() == CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( this->view_mode_state_->get() == SAGITTAL_C )
  {
    slice_type = Core::VolumeSliceType::SAGITTAL_E;
  }

  this->layer_connection_map_.insert( connection_map_type::value_type( layer->get_layer_id(),
    layer->opacity_state_->state_changed_signal_.connect( 
    boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );
  this->layer_connection_map_.insert( connection_map_type::value_type( layer->get_layer_id(),
    layer->visible_state_[ this->viewer_id_ ]->state_changed_signal_.connect(
    boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );

  switch( layer->type() )
  {
  case Core::VolumeType::DATA_E:
    {
      DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
      Core::DataVolumeHandle data_volume = data_layer->get_data_volume();
      Core::DataVolumeSliceHandle data_volume_slice( 
        new Core::DataVolumeSlice( data_volume, slice_type ) );
      this->data_slices_[ layer->get_layer_id() ] = data_volume_slice;
      volume_slice = data_volume_slice;
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        data_layer->contrast_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        data_layer->brightness_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        data_layer->volume_rendered_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, true ) ) ) );
    }
    break;
  case Core::VolumeType::MASK_E:
    {
      MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
      Core::MaskVolumeHandle mask_volume = mask_layer->get_mask_volume();
      Core::MaskVolumeSliceHandle mask_volume_slice(
        new Core::MaskVolumeSlice( mask_volume, slice_type ) );
      this->mask_slices_[ layer->get_layer_id() ] = mask_volume_slice;
      volume_slice = mask_volume_slice;
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        mask_layer->color_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        mask_layer->border_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        mask_layer->fill_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, false ) ) ) );
      this->layer_connection_map_.insert( 
        connection_map_type::value_type( layer->get_layer_id(),
        mask_layer->show_isosurface_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::layer_state_changed, this, true ) ) ) );
    }
    break;
  default:
    // Should never reach here.
    assert( false );
  }

  lock.unlock();

  // Auto adjust the view and depth if it is the first layer inserted
  if ( !this->active_layer_slice_ )
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->auto_orient( volume_slice );
    this->adjust_view( volume_slice );
    this->adjust_depth( volume_slice );
    this->set_active_layer( layer );
  }
  else if ( !this->is_volume_view() )
  { 
    volume_slice->move_slice_to( this->active_layer_slice_->depth() );
  }

  if ( !this->is_volume_view() )
  {
    this->trigger_redraw( false );
  }
}

void Viewer::delete_layers( std::vector< LayerHandle > layers )
{
  lock_type lock( this->get_mutex() );

  for ( size_t i = 0; i < layers.size(); i++ )
  {
    LayerHandle layer = layers[ i ];
    
    // Disconnect from the signals of the layer states
    std::pair< connection_map_type::iterator, connection_map_type::iterator > range = 
      this->layer_connection_map_.equal_range( layer->get_layer_id() );
    for ( connection_map_type::iterator it = range.first; it != range.second; it++ )
    {
      ( *it ).second.disconnect();
    }
    this->layer_connection_map_.erase( layer->get_layer_id() );

    switch( layer->type() )
    {
    case Core::VolumeType::DATA_E:
      {
        data_slices_map_type::iterator it = this->data_slices_.find( layer->get_layer_id() );
        assert( it != this->data_slices_.end() );
        if ( this->active_layer_slice_ == ( *it ).second )
        {
          this->active_layer_slice_.reset();
        }
        this->data_slices_.erase( it );
      }
      break;
    case Core::VolumeType::MASK_E:
      {
        mask_slices_map_type::iterator it = this->mask_slices_.find( layer->get_layer_id() );
        assert( it != this->mask_slices_.end() );
        if ( this->active_layer_slice_ == ( *it ).second )
        {
          this->active_layer_slice_.reset();
        }
        this->mask_slices_.erase( it );
      }
      break;
    default:
      // Should never reach here.
      assert( false );
    }
  }

  lock.unlock();
  
  if ( !LayerManager::Instance()->get_active_layer() )
  {
    this->trigger_redraw( true );
    this->trigger_redraw_overlay( false );
  }
  else
  {
    this->trigger_redraw( false );
  }
}

void Viewer::set_active_layer( LayerHandle layer )
{
  Core::VolumeSliceHandle new_active_slice;

  data_slices_map_type::iterator data_slice_it = this->data_slices_.find( layer->get_layer_id() );
  if ( data_slice_it != this->data_slices_.end() )
  {
    new_active_slice = ( *data_slice_it ).second;
  }
  else
  {
    mask_slices_map_type::iterator mask_slice_it = this->mask_slices_.find( layer->get_layer_id() );
    if ( mask_slice_it != this->mask_slices_.end() )
    {
      new_active_slice = ( *mask_slice_it ).second;
    }
    else
    {
      SCI_THROW_LOGICERROR( std::string("Active layer '") 
        + layer->get_layer_id() + "' could not be found" );
    }
  }

  if ( this->active_layer_slice_ == new_active_slice )
  {
    if ( !this->is_volume_view() )
    {
      this->trigger_redraw_overlay( false );
      if ( this->viewer_visible_state_->get() )
      {
        this->slice_changed_signal_( this->viewer_id_ );
      }
    }
    return;
  }
  this->active_layer_slice_ = new_active_slice;

  // Update slice number ranges
  if ( !this->is_volume_view() )
  {
    // Only needs redraw when the new active slice was out of boundary
    bool needs_redraw = this->active_layer_slice_->out_of_boundary();

    {
      // Disable redraws triggered by StateBase::state_changed_signal_
      Core::ScopedCounter block_counter( this->signals_block_count_ );

      // NOTE: The following state changes are due to internal program logic, 
      // so they should not go through the action mechanism.

      {
        Core::ScopedCounter slice_lock_counter( this->slice_lock_count_ );
        this->slice_number_state_->set_range(
          0, static_cast< int >( this->active_layer_slice_->number_of_slices() - 1 ) );
      }
      Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
        this->get_active_view_state().get() );
      this->active_layer_slice_->move_slice_to( view2d_state->get().center().z(), true );
      if ( needs_redraw && this->slice_number_state_->get() == 
        static_cast< int >( this->active_layer_slice_->get_slice_number() ) )
      {
        this->set_slice_number( static_cast< int >( this->active_layer_slice_->get_slice_number() ) );
      }
      else
      {
        this->slice_number_state_->set( static_cast< int >( 
          this->active_layer_slice_->get_slice_number() ) );
      }
    }

    if ( needs_redraw )
    {
      this->trigger_redraw( false );
      if ( this->viewer_visible_state_->get() )
      {
        this->slice_changed_signal_( this->viewer_id_ );
      }
    }
  }
}

Core::MaskVolumeSliceHandle Viewer::get_mask_volume_slice( const std::string& layer_id )
{
  lock_type lock( this->get_mutex() );

  mask_slices_map_type::iterator it = this->mask_slices_.find( layer_id );
  if ( it != this->mask_slices_.end() )
  {
    return ( *it ).second;
  }

  return Core::MaskVolumeSliceHandle();
}

Core::DataVolumeSliceHandle Viewer::get_data_volume_slice( const std::string& layer_id )
{
  lock_type lock( this->get_mutex() );

  data_slices_map_type::iterator it = this->data_slices_.find( layer_id );
  if ( it != this->data_slices_.end() )
  {
    return ( *it ).second;
  }

  return Core::DataVolumeSliceHandle();
}

void Viewer::change_view_mode( std::string mode, Core::ActionSource source )
{
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->viewer_lock_state_->set( false );
  }

  if ( mode == VOLUME_C )
  {   
    return;
  }

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  if ( mode == CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( mode == SAGITTAL_C )
  {
    slice_type =  Core::VolumeSliceType::SAGITTAL_E;
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

    Core::ScopedCounter block_counter( this->signals_block_count_ );

    {
      Core::ScopedCounter slice_lock_counter( this->slice_lock_count_ );
      this->slice_number_state_->set_range(
        0, static_cast< int >( this->active_layer_slice_->number_of_slices() - 1 ) );
    }

    Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
      this->get_active_view_state().get() );
    this->active_layer_slice_->move_slice_to( view2d_state->get().center().z(), true );
    // Force an update even if the slice number is the same
    if ( this->slice_number_state_->get() == 
      static_cast< int >( this->active_layer_slice_->get_slice_number() ) )
    {
      this->set_slice_number( static_cast< int >( this->active_layer_slice_->get_slice_number() ) );
    }
    else
    {
      this->slice_number_state_->set( static_cast< int >( 
        this->active_layer_slice_->get_slice_number() ) );
    }
  } // end if ( this->active_layer_slice_ )
}

void Viewer::set_slice_number( int num, Core::ActionSource source )
{
  const std::string& view_mode = this->view_mode_state_->get();
  if ( this->slice_lock_count_ > 0 || 
     view_mode == VOLUME_C || 
     !this->active_layer_slice_ )
  {
    return;
  }

  this->active_layer_slice_->set_slice_number( num );
  double depth_offset = 0;

  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );

    // Update the depth info
    Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
      this->get_active_view_state().get() );
    Core::View2D view2d( view2d_state->get() );
    depth_offset = this->active_layer_slice_->depth() - view2d.center().z();
    view2d.center().z( this->active_layer_slice_->depth() );
    view2d_state->set( view2d ) ;

    // Move other layer slices to the new position
    mask_slices_map_type::iterator mask_slice_it = this->mask_slices_.begin();
    for ( ; mask_slice_it != this->mask_slices_.end(); mask_slice_it++ )
    {
      if ( ( *mask_slice_it ).second == this->active_layer_slice_ )
        continue;
      ( *mask_slice_it ).second->move_slice_to( this->active_layer_slice_->depth() );
    }
    data_slices_map_type::iterator data_slice_it = this->data_slices_.begin();
    for ( ; data_slice_it != this->data_slices_.end(); data_slice_it++ )
    {
      if ( ( *data_slice_it ).second == this->active_layer_slice_ )
        continue;
      ( *data_slice_it ).second->move_slice_to( this->active_layer_slice_->depth() );
    }
  }

  if ( this->viewer_lock_state_->get() )
  {
    std::vector< size_t > locked_viewers = ViewerManager::Instance()->
      get_locked_viewers( this->view_mode_state_->index() );
    for ( size_t i = 0; i < locked_viewers.size(); i++ )
    {
      size_t viewer_id = locked_viewers[ i ];
      if ( this->viewer_id_ != viewer_id )
      {
        ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
        viewer->move_slice_by( depth_offset );
      }
    }
  }

  if ( this->viewer_visible_state_->get() && this->signals_block_count_ == 0 )
  {
    this->slice_changed_signal_( this->viewer_id_ );
  }
}

void Viewer::change_visibility( bool visible, Core::ActionSource /*source*/ )
{
  if ( !visible && this->viewer_lock_state_->get() )
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->viewer_lock_state_->set( false );
  }

  if ( visible)
  {
    this->reset_active_slice();
  }
  
  this->slice_changed_signal_( this->viewer_id_ );
}

void Viewer::viewer_lock_state_changed( bool locked )
{
  if ( this->signals_block_count_ > 0 || locked )
  {
    return;
  }
  
  this->reset_active_slice();
}

void Viewer::trigger_redraw( bool delay_update )
{
  if ( this->signals_block_count_ == 0 )
  {
    this->redraw_signal_( delay_update );
  }
}

void Viewer::trigger_redraw_overlay( bool delay_update )
{
  if ( this->signals_block_count_ == 0 )
  {
    this->redraw_overlay_signal_( delay_update );
  }
}

void Viewer::adjust_view( Core::VolumeSliceHandle target_slice )
{
  if ( !target_slice )
  {
    SCI_LOG_ERROR( "Invalid volume slice handle" );
    return;
  }

  Core::VolumeSliceHandle volume_slice;
  if ( target_slice->volume_type() == Core::VolumeType::DATA_E )
  {
    volume_slice = Core::VolumeSliceHandle( new Core::DataVolumeSlice( 
      *static_cast< Core::DataVolumeSlice* >( target_slice.get() ) ) );
  }
  else
  {
    volume_slice = Core::VolumeSliceHandle( new Core::MaskVolumeSlice( 
      *static_cast< Core::MaskVolumeSlice* >( target_slice.get() ) ) );
  }

  double aspect = 1.0;
  if ( this->width_ != 0 && this->height_ != 0 ) 
  {
    aspect = this->width_ * 1.0 / this->height_;
  }
  double scale, scalex, scaley;
  Core::Point center;

  volume_slice->set_slice_type( Core::VolumeSliceType::AXIAL_E );
  center = Core::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
              ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 
              this->axial_view_state_->get().center().z() );
  scale = 1.0 / Core::Max( Core::Abs( volume_slice->top() - volume_slice->bottom() ),
    Core::Abs( volume_slice->right() - volume_slice->left() ) / aspect );
  scalex = scale * Core::Sign( this->axial_view_state_->get().scalex() );
  scaley = scale * Core::Sign( this->axial_view_state_->get().scaley() );
  this->axial_view_state_->set( Core::View2D( center, scalex, scaley ) );

  volume_slice->set_slice_type( Core::VolumeSliceType::CORONAL_E );
  center = Core::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
              ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 
              this->coronal_view_state_->get().center().z() );
  scale = 1.0 / Core::Max( Core::Abs( volume_slice->top() - volume_slice->bottom() ),
    Core::Abs( volume_slice->right() - volume_slice->left() ) / aspect );
  scalex = scale * Core::Sign( this->coronal_view_state_->get().scalex() );
  scaley = scale * Core::Sign( this->coronal_view_state_->get().scaley() );
  this->coronal_view_state_->set( Core::View2D( center, scalex, scaley ) );

  volume_slice->set_slice_type( Core::VolumeSliceType::SAGITTAL_E );
  center = Core::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
              ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 
              this->sagittal_view_state_->get().center().z() );
  scale = 1.0 / Core::Max( Core::Abs( volume_slice->top() - volume_slice->bottom() ),
    Core::Abs( volume_slice->right() - volume_slice->left() ) / aspect );
  scalex = scale * Core::Sign( this->sagittal_view_state_->get().scalex() );
  scaley = scale * Core::Sign( this->sagittal_view_state_->get().scaley() );
  this->sagittal_view_state_->set( Core::View2D( center, scalex, scaley ) );

  Core::VolumeHandle volume = volume_slice->get_volume();
  Core::Point corner1 = volume->apply_grid_transform( Core::Point( 0, 0, 0 ) );
  Core::Point corner2 = volume->apply_grid_transform(
    Core::Point( static_cast< double >( volume->get_nx() - 1 ), 
    static_cast< double >( volume->get_ny() - 1 ), static_cast< double >( volume->get_nz() - 1 ) ) );
  Core::View3D view3d( this->volume_view_state_->get() );
  center = Core::Point( ( corner1 + corner2 ) * 0.5 );
  view3d.translate( view3d.lookat() - center );
  Core::Matrix mat;
  Core::Transform::BuildViewMatrix( mat, view3d.eyep(), view3d.lookat(), view3d.up() );
  double ctan_hfov = 1.0 / Core::Tan( Core::DegreeToRadian( view3d.fov() * 0.5 ) );
  // For each vertex of the bounding box, compute its coordinates in eye space
  Core::Point pt = mat * corner1;
  double eye_offset = Core::Max( Core::Abs( pt.y() ), Core::Abs( pt.x() ) / aspect ) * 
    ctan_hfov + pt.z();
  pt = mat * corner2;
  eye_offset = Core::Max( eye_offset, Core::Max( Core::Abs( pt.y() ), 
    Core::Abs( pt.x() ) / aspect ) * ctan_hfov + pt.z() );  
  for ( int i = 0; i < 3; i++ )
  {
    pt = corner1;
    pt[ i ] = corner2[ i ];
    pt = mat * pt;
    eye_offset = Core::Max( eye_offset, Core::Max( Core::Abs( pt.y() ), 
      Core::Abs( pt.x() ) / aspect ) * ctan_hfov + pt.z() );  
    
    pt = corner2;
    pt[ i ] = corner1[ i ];
    pt = mat * pt;
    eye_offset = Core::Max( eye_offset, Core::Max( Core::Abs( pt.y() ), 
      Core::Abs( pt.x() ) / aspect ) * ctan_hfov + pt.z() );  
  }

  Core::Vector eye_vec = view3d.eyep() - view3d.lookat();
  eye_vec.normalize();
  //double eye_dist = eye_vec.normalize() + depth * 0.99;
  //height = Core::Max( height, width / aspect ) * 1.01;
  //eye_dist = Core::Max( height / Core::Tan( 
  //  Core::DegreeToRadian( view3d.fov() * 0.5 ) ), eye_dist );

  //view3d.eyep( view3d.lookat() + eye_vec * eye_dist );
  view3d.eyep( view3d.eyep() + eye_vec * eye_offset );
  this->volume_view_state_->set( view3d );
}

void Viewer::adjust_depth( Core::VolumeSliceHandle target_slice )
{
  if ( !target_slice )
  {
    SCI_LOG_ERROR( "Invalid volume slice handle" );
    return;
  }

  Core::VolumeSliceHandle volume_slice;
  if ( target_slice->volume_type() == Core::VolumeType::DATA_E )
  {
    volume_slice = Core::VolumeSliceHandle( new Core::DataVolumeSlice( 
      *dynamic_cast< Core::DataVolumeSlice* >( target_slice.get() ) ) );
  }
  else
  {
    volume_slice = Core::VolumeSliceHandle( new Core::MaskVolumeSlice( 
      *dynamic_cast< Core::MaskVolumeSlice* >( target_slice.get() ) ) );
  }

  volume_slice->set_slice_type( Core::VolumeSliceType::AXIAL_E );
  volume_slice->set_slice_number( volume_slice->number_of_slices() / 2 );
  Core::View2D view2d( this->axial_view_state_->get() );
  view2d.center().z( volume_slice->depth() );
  this->axial_view_state_->set( view2d );

  volume_slice->set_slice_type( Core::VolumeSliceType::CORONAL_E );
  volume_slice->set_slice_number( volume_slice->number_of_slices() / 2 );
  view2d = this->coronal_view_state_->get();
  view2d.center().z( volume_slice->depth() );
  this->coronal_view_state_->set( view2d );

  volume_slice->set_slice_type( Core::VolumeSliceType::SAGITTAL_E );
  volume_slice->set_slice_number( volume_slice->number_of_slices() / 2 );
  view2d = this->sagittal_view_state_->get();
  view2d.center().z( volume_slice->depth() );
  this->sagittal_view_state_->set( view2d );
}

void Viewer::auto_orient( Core::VolumeSliceHandle target_slice )
{
  Core::VolumeHandle volume = target_slice->get_volume();
  Core::Point corner1 = volume->apply_grid_transform( Core::Point( 0, 0, 0 ) );
  Core::Point corner2 = volume->apply_grid_transform(
    Core::Point( static_cast< double >( volume->get_nx() - 1 ), 
    static_cast< double >( volume->get_ny() - 1 ), static_cast< double >( volume->get_nz() - 1 ) ) );
  Core::View3D view3d;
  view3d.lookat( Core::Point( ( corner1 + corner2 ) * 0.5 ) );
  view3d.eyep( view3d.lookat() + Core::Vector( 0, 0, 1 ) );
  view3d.up( Core::Vector( 0, 1, 0 ) );
  view3d.rotate( Core::Vector( 1, 0, 0 ), -30 );
  view3d.rotate( Core::Vector( 0, 1, 0 ), 135 );
  this->volume_view_state_->set( view3d );
}

void Viewer::auto_view()
{
  if ( !this->active_layer_slice_ )
  {
    return;
  }
  
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->adjust_view( this->active_layer_slice_ );
  }
  this->trigger_redraw( true );
  this->trigger_redraw_overlay( false );
}

void Viewer::layer_state_changed( bool volume_view )
{
  if ( volume_view == this->is_volume_view() )
  {
    this->trigger_redraw( false );
  }
}

void Viewer::move_slice_to( const Core::Point& pt )
{
  if ( !this->is_volume_view() && this->active_layer_slice_ )
  {
    this->active_layer_slice_->move_slice_to( pt, true );
    this->slice_number_state_->set( static_cast< int >( 
      this->active_layer_slice_->get_slice_number() ) );
  }
}

void Viewer::offset_slice( int delta )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  if ( !this->is_volume_view() && delta != 0 && this->active_layer_slice_ )
  {
    if ( this->active_layer_slice_->out_of_boundary() )
    {
      this->reset_active_slice();
      return;
    }

    int slice_num = static_cast<int>( this->active_layer_slice_->get_slice_number() );
    int new_slice = static_cast<int>( slice_num + delta );
    if ( new_slice >= 0 && 
      static_cast< size_t >( new_slice ) < this->active_layer_slice_->number_of_slices() )
    {
      Core::ActionSet::Dispatch( this->slice_number_state_, new_slice );
    }
  }
}

void Viewer::move_slice_by( double depth_offset )
{
  if ( depth_offset == 0.0 || 
    this->is_volume_view() || 
    !this->active_layer_slice_ )
  {
    return;
  }

  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );

    Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
      this->get_active_view_state().get() );
    Core::View2D view2d( view2d_state->get() );
    double depth = view2d.center().z();
    depth += depth_offset;
    view2d.center().z( depth );
    view2d_state->set( view2d ) ;

    // Move all the slices to the new position
    mask_slices_map_type::iterator mask_slice_it = this->mask_slices_.begin();
    for ( ; mask_slice_it != this->mask_slices_.end(); mask_slice_it++ )
    {
      ( *mask_slice_it ).second->move_slice_to( depth );
    }
    data_slices_map_type::iterator data_slice_it = this->data_slices_.begin();
    for ( ; data_slice_it != this->data_slices_.end(); data_slice_it++ )
    {
      ( *data_slice_it ).second->move_slice_to( depth );
    }

    if ( !this->active_layer_slice_->out_of_boundary() )
    {
      Core::ScopedCounter slice_lock_counter( this->slice_lock_count_ );
      this->slice_number_state_->set( static_cast< int >( 
        this->active_layer_slice_->get_slice_number() ) );
    }
  }
  
  this->trigger_redraw( false );

  // NOTE: No need to trigger slice_changed_signal_ here because the viewer that caused
  // the slice change will trigger it.

}

void Viewer::reset_active_slice()
{
  if ( !this->is_volume_view() && 
    this->active_layer_slice_ &&
    this->active_layer_slice_->out_of_boundary() )
  {
    Core::StateView2D* view2d_state = 
      dynamic_cast< Core::StateView2D* >( this->get_active_view_state().get() );
    this->active_layer_slice_->move_slice_to( view2d_state->get().center().z(), true );
    if ( this->active_layer_slice_->get_slice_number() ==
      static_cast< size_t >( this->slice_number_state_->get() ) )
    {
      this->set_slice_number( this->slice_number_state_->get() );
      this->trigger_redraw( false );
    }
    else
    {
      this->slice_number_state_->set( static_cast< int >( 
        this->active_layer_slice_->get_slice_number() ) );
    }
  }
}

void Viewer::adjust_contrast_brightness( int dx, int dy )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( !active_layer || active_layer->type() != Core::VolumeType::DATA_E )
  {
    return;
  }
  
  DataLayer* data_layer = static_cast< DataLayer* >( active_layer.get() );
  const double old_contrast = data_layer->contrast_state_->get();
  double contrast_min, contrast_max, contrast_step;
  data_layer->contrast_state_->get_range( contrast_min, contrast_max );
  data_layer->contrast_state_->get_step( contrast_step );
  double contrast = old_contrast + dy * contrast_step;
  contrast = Core::Max( contrast_min, contrast );
  contrast = Core::Min( contrast_max, contrast );
  
  const double old_brightness = data_layer->brightness_state_->get();
  double brightness_min, brightness_max, brightness_step;
  data_layer->brightness_state_->get_range( brightness_min, brightness_max );
  data_layer->brightness_state_->get_step( brightness_step );
  double brightness = old_brightness + dx * brightness_step;
  brightness = Core::Max( brightness_min, brightness );
  brightness = Core::Min( brightness_max, brightness );

  lock.unlock();

  if ( contrast != old_contrast )
  {
    Core::ActionSet::Dispatch( data_layer->contrast_state_, contrast );
  }
  if ( brightness != old_brightness )
  {
    Core::ActionSet::Dispatch( data_layer->brightness_state_, brightness );
  }
}

void Viewer::install_renderer( Core::AbstractRendererHandle renderer )
{
  this->add_connection( renderer->redraw_completed_signal_.connect(
    boost::bind( &Viewer::set_texture, this, _1, _2 ) ) );
  this->add_connection( renderer->redraw_overlay_completed_signal_.connect(
    boost::bind( &Viewer::set_overlay_texture, this, _1, _2 ) ) );
}

Core::Texture2DHandle Viewer::get_texture()
{
  lock_type lock( this->get_mutex() );
  return this->texture_;
}

Core::Texture2DHandle Viewer::get_overlay_texture()
{
  lock_type lock( this->get_mutex() );
  return this->overlay_texture_;
}

void Viewer::set_texture( Core::Texture2DHandle texture, bool delay_update )
{
  {
    lock_type lock( this->get_mutex() );
    this->texture_ = texture;
  }

  if ( !delay_update )
  {
    this->update_display_signal_();
  }
}

void Viewer::set_overlay_texture( Core::Texture2DHandle texture, bool delay_update )
{
  {
    lock_type lock( this->get_mutex() );
    this->overlay_texture_ = texture;
  }

  if ( !delay_update )
  {
    this->update_display_signal_();
  }
}

} // end namespace Seg3D

