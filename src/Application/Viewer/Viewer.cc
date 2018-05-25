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

// Core includes
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Exception.h>
#include <Core/State/Actions/ActionOffset.h>
#include <Core/State/Actions/ActionToggle.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/Utils/AtomicCounter.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/LargeVolumeSlice.h>
#include <Core/LargeVolume/LargeVolumeCache.h>
#include <Core/Renderer/RendererBase.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionShiftActiveLayer.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Viewer/Viewer.h>
#include <Application/Viewer/ViewManipulator.h>
#include <Application/Viewer/Actions/ActionOffsetSlice.h>
#include <Application/Viewer/Actions/ActionAutoView.h>
#include <Application/ViewerManager/Actions/ActionPickPoint.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Layer/Actions/ActionComputeIsosurface.h>

namespace Seg3D
{

CORE_ENUM_CLASS
(
  ViewModeType,
  AXIAL_E = 0x1,
  CORONAL_E = 0x2,
  SAGITTAL_E = 0x4,
  NON_VOLUME_E = AXIAL_E | CORONAL_E | SAGITTAL_E,
  VOLUME_E = 0x8,
  ALL_E = NON_VOLUME_E | VOLUME_E
)

//////////////////////////////////////////////////////////////////////////
// Implementation of class ViewerPrivate
//////////////////////////////////////////////////////////////////////////

class ViewerPrivate
{
  // -- Signals handling --
public:
  void change_view_mode( std::string mode, Core::ActionSource source );
  void set_slice_number( int num, Core::ActionSource source = Core::ActionSource::NONE_E );
  void change_visibility( bool visible );
  void viewer_lock_state_changed( bool locked );
  void layer_state_changed( int affected_view_modes );
  void insert_layer( LayerHandle layer );
  void delete_layers( std::vector< std::string > layers );
  void set_active_layer( LayerHandle layer );
  void set_viewer_labels();
  void handle_flip_horizontal_changed( bool flip );
  void handle_flip_vertical_changed( bool flip );
  // Called by pick_point() for volume view, or called with empty handle for normal redraw.
  void redraw_scene( Core::PickPointHandle pick_point );
  void reset();

  // -- Helper functions --
public:

  void pick_point( int x, int y );
  void pick_point( const Core::Point& world_pick_point );

  void adjust_contrast_brightness( int dx, int dy );

  // Auto adjust the view states so the slices are fully visible
  void adjust_view( Core::VolumeSliceHandle target_slice );

  // Move the active slices to the center of the volume
  void adjust_depth( Core::VolumeSliceHandle target_slice );

  // Auto orient the 3D view for the given slice
  void auto_orient( Core::VolumeSliceHandle target_slice );

  // MOVE_SLICE_BY:
  // Move the active slice by the given offset in world coordinates.
  // Called when the viewer is locked to other viewers of the same mode.
  void move_slice_by( double depth_offset );

  // RESET_ACTIVE_SLICE:
  // Bring the active slice into boundary ( if it's out of boundary ).
  // The active slice can only go out of boundary when the viewer is locked to other viewers.
  void reset_active_slice();

public:

  Viewer* viewer_;

  // SIGNALS_BLOCK_COUNT_:
  // Counts the number of times state change signals are blocked
  // Used when state variables are being changed due to internal program logic.
  size_t signals_block_count_;

  // Counts the number of times the slice number is locked (unchangeable)
  size_t slice_lock_count_;

  bool adjusting_contrast_brightness_;

  typedef std::multimap< std::string, boost::signals2::connection > connection_map_type;
  connection_map_type layer_connection_map_;

  typedef std::map< std::string, Core::VolumeSliceHandle > volume_slice_map_type;
  volume_slice_map_type volume_slices_;

  Core::VolumeSliceHandle active_layer_slice_;

  // Indexed view state variables for quick access
  Core::StateViewBaseHandle view_states_[ 4 ];

  Viewer::mouse_event_handler_type mouse_move_handler_;
  Viewer::mouse_event_handler_type mouse_press_handler_;
  Viewer::mouse_event_handler_type mouse_release_handler_;
  Viewer::enter_event_handler_type mouse_enter_handler_;
  Viewer::leave_event_handler_type mouse_leave_handler_;
  Viewer::wheel_event_handler_type wheel_event_handler_;
  Viewer::key_press_event_handler_type key_press_event_handler_;
  Viewer::key_release_event_handler_type key_release_event_handler_;
  Viewer::cursor_handler_type cursor_handler_;

  ViewManipulatorHandle view_manipulator_;

  boost::mutex mouse_pressed_mutex_;
  bool mouse_pressed_;

  // This flag is set before loading states of the viewer.
  bool loading_;
};

void ViewerPrivate::adjust_contrast_brightness( int dx, int dy )
{
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( ( ! active_layer ) ||
       ( active_layer->get_type() != Core::VolumeType::DATA_E && active_layer->get_type() != Core::VolumeType::LARGE_DATA_E ) ||
       ( ! active_layer->is_visible( this->viewer_->get_viewer_id() ) ) )
  {
    return;
  }

  if ( active_layer->get_type() != Core::VolumeType::DATA_E )
  {
    DataLayer* data_layer = static_cast< DataLayer* >( active_layer.get() );
    double contrast_step, brightness_step;
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      data_layer->contrast_state_->get_step( contrast_step ); 
      data_layer->brightness_state_->get_step( brightness_step );
    }

    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(), 
      data_layer->contrast_state_, dy * contrast_step );
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      data_layer->brightness_state_, dx * brightness_step );
  }
  else if ( active_layer->get_type() != Core::VolumeType::LARGE_DATA_E )
  {
    LargeVolumeLayer* data_layer = static_cast< LargeVolumeLayer* >( active_layer.get() );
    double contrast_step, brightness_step;
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      data_layer->contrast_state_->get_step( contrast_step ); 
      data_layer->brightness_state_->get_step( brightness_step );
    }

    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(), 
      data_layer->contrast_state_, dy * contrast_step );
    Core::ActionOffset::Dispatch( Core::Interface::GetMouseActionContext(),
      data_layer->brightness_state_, dx * brightness_step );
  }
}

void ViewerPrivate::pick_point( int window_x, int window_y )
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &ViewerPrivate::pick_point, this, window_x, window_y ) );
    return;
  }

  if( this->viewer_->is_volume_view() )
  {
    // Trigger redraw to get 3D world pick point based on last render pass
    // Only need to redraw scene (not overlay) since we don't want to pick on the overlay
    Core::PickPointHandle pick_point( new Core::PickPoint( window_x, window_y ) );
    this->redraw_scene( pick_point );

    // ActionPickPoint will be dispatched when renderer signals with 3D pick point
  }
  else if ( this->active_layer_slice_ )
  {
    double width =  static_cast<double>( this->viewer_->get_width() );
    double height = static_cast<double>( this->viewer_->get_height() );

    Core::VolumeSlice* volume_slice = this->active_layer_slice_.get();
    // Scale the mouse position to [-1, 1]
    double xpos = window_x * 2.0 / ( width - 1.0 ) - 1.0;
    double ypos = ( height - 1 - window_y ) * 2.0 / ( height - 1.0 ) - 1.0;
    double left, right, bottom, top;
    Core::StateView2D* view_2d = dynamic_cast<Core::StateView2D*>( 
      this->viewer_->get_active_view_state().get() );
    view_2d->get().compute_clipping_planes( width / height, left, right, bottom, top );
    Core::Matrix proj, inv_proj;
    Core::Transform::BuildOrtho2DMatrix( proj, left, right, bottom, top );
    Core::Matrix::Invert( proj, inv_proj );
    Core::Point pos( xpos, ypos, 0 );
    pos = inv_proj * pos;
    volume_slice->get_world_coord( pos.x(), pos.y(), pos );

    this->pick_point( pos );
  }
}

void ViewerPrivate::pick_point( const Core::Point& world_pick_point )
{
  ActionPickPoint::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->viewer_->get_viewer_id(), world_pick_point );
}

void ViewerPrivate::insert_layer( LayerHandle layer )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  Core::VolumeSliceHandle volume_slice;

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  if ( this->viewer_->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( this->viewer_->view_mode_state_->get() == Viewer::SAGITTAL_C )
  {
    slice_type = Core::VolumeSliceType::SAGITTAL_E;
  }

  this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
    layer->opacity_state_->state_changed_signal_.connect( 
    boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );
    
  this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
    layer->visible_state_[ this->viewer_->get_viewer_id() ]->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_overlay, this->viewer_ ) ) ) );

  this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
    layer->visible_state_[ this->viewer_->get_viewer_id() ]->state_changed_signal_.connect(
    boost::bind( &ViewerPrivate::layer_state_changed, this, 
    ViewModeType::ALL_E ) ) ) );
    
  this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
    layer->master_visible_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_overlay, this->viewer_ ) ) ) );

  this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
    layer->master_visible_state_->state_changed_signal_.connect(
    boost::bind( &ViewerPrivate::layer_state_changed, this, 
    ViewModeType::ALL_E ) ) ) );


  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; ++i )
  {
    this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
      layer->visible_state_[ i ]->state_changed_signal_.connect( boost::bind(
      &ViewerPrivate::layer_state_changed, this, ViewModeType::VOLUME_E ) ) ) );
  }
  
  this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
    layer->layer_updated_signal_.connect( boost::bind(
    &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );

  switch( layer->get_type() )
  {
  case Core::VolumeType::LARGE_DATA_E:
    {
      LargeVolumeLayer* data_layer = dynamic_cast< LargeVolumeLayer* >( layer.get() );
      volume_slice.reset( new Core::LargeVolumeSlice( 
        boost::dynamic_pointer_cast<Core::LargeVolume>( data_layer->get_volume() ), slice_type ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        data_layer->contrast_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        data_layer->brightness_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );

	  this->layer_connection_map_.insert(std::make_pair(layer->get_layer_id(),
		  data_layer->color_state_->state_changed_signal_.connect(
		  boost::bind(&ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E))));
    }
    break;
  case Core::VolumeType::DATA_E:
    {
      DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
      Core::DataVolumeHandle data_volume = data_layer->get_data_volume();
      volume_slice.reset( new Core::DataVolumeSlice( data_volume, slice_type ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        data_layer->contrast_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        data_layer->brightness_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        data_layer->volume_rendered_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::VOLUME_E ) ) ) );

	  this->layer_connection_map_.insert(std::make_pair(layer->get_layer_id(),
		  data_layer->color_state_->state_changed_signal_.connect(
		  boost::bind(&ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E))));
    }
    break;
  case Core::VolumeType::MASK_E:
    {
      MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
      Core::MaskVolumeHandle mask_volume = mask_layer->get_mask_volume();
      Core::MaskVolumeSliceHandle mask_volume_slice( 
        new Core::MaskVolumeSlice( mask_volume, slice_type ) );
      volume_slice = mask_volume_slice;

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_layer->color_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::ALL_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_layer->color_state_->state_changed_signal_.connect(
        boost::bind( &Viewer::redraw_overlay, this->viewer_ ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_layer->border_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::NON_VOLUME_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_layer->fill_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::NON_VOLUME_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_layer->show_isosurface_state_->state_changed_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::VOLUME_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_volume_slice->cache_updated_signal_.connect( boost::bind(
        &ViewerPrivate::layer_state_changed, this, ViewModeType::NON_VOLUME_E ) ) ) );

      this->layer_connection_map_.insert( std::make_pair( layer->get_layer_id(),
        mask_layer->isosurface_updated_signal_.connect(
        boost::bind( &ViewerPrivate::layer_state_changed, this, ViewModeType::VOLUME_E ) ) ) );
    }
    break;
  default:
    // Should never reach here.
    assert( false );
  }

  this->volume_slices_[ layer->get_layer_id() ] = volume_slice;

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
  else if ( !this->viewer_->is_volume_view() )
  { 
    volume_slice->move_slice_to( this->active_layer_slice_->depth() );
  }

  if ( !this->viewer_->is_volume_view() && layer->has_valid_data() )
  {
    this->viewer_->redraw_scene();
  }
}

void ViewerPrivate::delete_layers( std::vector< std::string > layers )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
  bool layer_deleted = false;
  for ( size_t i = 0; i < layers.size(); ++i )
  {
    std::string layer_id = layers[ i ];

    // Disconnect from the signals of the layer states
    std::pair< connection_map_type::iterator, connection_map_type::iterator > range = 
      this->layer_connection_map_.equal_range( layer_id );

    for ( connection_map_type::iterator it = range.first; it != range.second; ++it )
    {
      ( *it ).second.disconnect();
    }
    this->layer_connection_map_.erase( layer_id );

    volume_slice_map_type::iterator it = this->volume_slices_.find( layer_id );
    if ( it == this->volume_slices_.end() ) continue;
    if ( this->active_layer_slice_ == ( *it ).second )
    {
      this->active_layer_slice_.reset();
    }
    this->volume_slices_.erase( it );
    layer_deleted = true;
  }

  lock.unlock();

  if ( layer_deleted )
  {
    if ( !LayerManager::Instance()->get_active_layer() )
    {
      this->viewer_->redraw_all();
    }
    else
    {
      this->viewer_->redraw_scene();
    }
  }
}

void ViewerPrivate::set_active_layer( LayerHandle layer )
{
  if ( !layer )
  {
    this->active_layer_slice_.reset();
    return;
  }
  
  Core::VolumeSliceHandle new_active_slice = this->viewer_->
    get_volume_slice( layer->get_layer_id() );
  assert( new_active_slice );

  if ( this->active_layer_slice_ == new_active_slice )
  {
    if ( !this->viewer_->is_volume_view() )
    {
      this->viewer_->redraw_overlay();
      if ( this->viewer_->viewer_visible_state_->get() )
      {
        this->viewer_->slice_changed_signal_( this->viewer_->get_viewer_id() );
      }
    }
    return;
  }
  this->active_layer_slice_ = new_active_slice;

  // Update slice number ranges
  if ( !this->viewer_->is_volume_view() )
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
        this->viewer_->slice_number_state_->set_range(
          0, static_cast< int >( this->active_layer_slice_->number_of_slices() ) - 1 );
      }
      Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
        this->viewer_->get_active_view_state().get() );

      this->active_layer_slice_->move_slice_to( view2d_state->get().center().z(), true );
      if ( needs_redraw && this->viewer_->slice_number_state_->get() == 
        static_cast< int >( this->active_layer_slice_->get_slice_number() ) )
      {
        this->set_slice_number( static_cast< int >( 
          this->active_layer_slice_->get_slice_number() ) );
      }
      else
      {
        this->viewer_->slice_number_state_->set( static_cast< int >( 
          this->active_layer_slice_->get_slice_number() ) );
      }
    }

    if ( needs_redraw )
    {
      this->viewer_->redraw_all();
      if ( this->viewer_->viewer_visible_state_->get() )
      {
        this->viewer_->slice_changed_signal_( this->viewer_->get_viewer_id() );
      }
    }
    else
    {
      this->viewer_->redraw_overlay();
    }
  }
}

void ViewerPrivate::change_view_mode( std::string mode, Core::ActionSource source )
{
  if ( this->signals_block_count_ > 0 || this->loading_ ) return;
  
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->viewer_->lock_state_->set( false );
  }

  if ( !this->cursor_handler_ || !this->cursor_handler_( this->viewer_->shared_from_this() ) )
  {
    this->viewer_->set_cursor( mode == Viewer::VOLUME_C ? Core::CursorShape::ARROW_E : 
      Core::CursorShape::CROSS_E );
  }
  
  if ( mode == Viewer::VOLUME_C )
  {
    return;
  }

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  Core::StateView2DHandle view2d = this->viewer_->axial_view_state_;
  if ( mode == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
    view2d = this->viewer_->coronal_view_state_;
  }
  else if ( mode == Viewer::SAGITTAL_C )
  {
    slice_type =  Core::VolumeSliceType::SAGITTAL_E;
    view2d = this->viewer_->sagittal_view_state_;
  }

  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->viewer_->flip_horizontal_state_->set( view2d->get().scalex() < 0.0 );
    this->viewer_->flip_vertical_state_->set( view2d->get().scaley() < 0.0 );
  }

  volume_slice_map_type::iterator it = this->volume_slices_.begin();
  volume_slice_map_type::iterator it_end = this->volume_slices_.end();
  while ( it != it_end )
  {
    ( *it ).second->set_slice_type( slice_type );
    ++it;
  }

  if ( this->active_layer_slice_ )
  {
    // NOTE: The following state changes are due to internal program logic, 
    // so they should not go through the action mechanism.

    Core::ScopedCounter block_counter( this->signals_block_count_ );

    {
      Core::ScopedCounter slice_lock_counter( this->slice_lock_count_ );
      this->viewer_->slice_number_state_->set_range(
        0, static_cast< int >( this->active_layer_slice_->number_of_slices() ) - 1 );
    }

    Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
      this->viewer_->get_active_view_state().get() );

    this->active_layer_slice_->move_slice_to( view2d_state->get().center().z(), true );

    // Force an update even if the slice number is the same
    if ( this->viewer_->slice_number_state_->get() == 
      static_cast< int >( this->active_layer_slice_->get_slice_number() ) )
    {
      this->set_slice_number( static_cast< int >( this->active_layer_slice_->get_slice_number() ) );
    }
    else
    {
      this->viewer_->slice_number_state_->set( static_cast< int >( 
        this->active_layer_slice_->get_slice_number() ) );
    }
  } // end if ( this->active_layer_slice_ )
}

void ViewerPrivate::set_slice_number( int num, Core::ActionSource source )
{
  if ( this->loading_ ) return;

  const std::string& view_mode = this->viewer_->view_mode_state_->get();

  if ( this->slice_lock_count_ > 0 || 
    view_mode == Viewer::VOLUME_C || 
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
      this->viewer_->get_active_view_state().get() );

    Core::View2D view2d( view2d_state->get() );
    depth_offset = this->active_layer_slice_->depth() - view2d.center().z();
    view2d.center().z( this->active_layer_slice_->depth() );
    view2d_state->set( view2d ) ;

    // Move other layer slices to the new position
    for ( volume_slice_map_type::iterator it = this->volume_slices_.begin();
      it != this->volume_slices_.end(); ++it )
    {
      if ( ( *it ).second == this->active_layer_slice_ )
        continue;
      ( *it ).second->move_slice_to( this->active_layer_slice_->depth() );
    }
  }

  if ( this->viewer_->lock_state_->get() )
  {
    std::vector< size_t > locked_viewers = ViewerManager::Instance()->
      get_locked_viewers( this->viewer_->view_mode_state_->index() );
    for ( size_t i = 0; i < locked_viewers.size(); i++ )
    {
      size_t viewer_id = locked_viewers[ i ];
      if ( this->viewer_->get_viewer_id() != viewer_id )
      {
        ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
        viewer->private_->move_slice_by( depth_offset );
      }
    }
  }

  if ( this->viewer_->viewer_visible_state_->get() && this->signals_block_count_ == 0 )
  {
    this->viewer_->slice_changed_signal_( this->viewer_->get_viewer_id() );
  }
}

void ViewerPrivate::change_visibility( bool visible )
{
  if ( this->signals_block_count_ > 0 || this->loading_ ) return;

  if ( !visible && this->viewer_->lock_state_->get() )
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    // Unlock the viewer when it becomes invisible
    this->viewer_->lock_state_->set( false );
  }

  if ( visible)
  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );
    this->reset_active_slice();
  }

  this->viewer_->slice_changed_signal_( this->viewer_->get_viewer_id() );
}

void ViewerPrivate::viewer_lock_state_changed( bool locked )
{
  if ( this->signals_block_count_ > 0 || this->loading_ || locked )
  {
    return;
  }

  this->reset_active_slice();
}

void ViewerPrivate::layer_state_changed( int affected_view_modes )
{
  bool volume_view = this->viewer_->is_volume_view();

  // NOTE: Normally layer state changes wouldn't require cursor updates,
  // so we only let the handler do what's necessary.
  if ( this->cursor_handler_ )
  {
    this->cursor_handler_( this->viewer_->shared_from_this() );
  }

  if ( ( volume_view && 
    ( affected_view_modes & ViewModeType::VOLUME_E ) != 0 ) ||
    ( !volume_view && 
    ( affected_view_modes & ViewModeType::NON_VOLUME_E ) != 0 ) )
  {
    this->viewer_->redraw_scene();
  }
}

void ViewerPrivate::adjust_view( Core::VolumeSliceHandle target_slice )
{
  if ( !target_slice )
  {
    CORE_LOG_ERROR( "Invalid volume slice handle" );
    return;
  }

  Core::VolumeSliceHandle volume_slice = target_slice->clone();

  double aspect = 1.0;
  double width =  static_cast<double>( this->viewer_->get_width() );
  double height = static_cast<double>( this->viewer_->get_height() );

  if ( width != 0.0 && height != 0.0 ) 
  {
    aspect = width / height;
  }
  double scale, scalex, scaley;
  Core::Point center;
  double slice_width, slice_height;

  volume_slice->set_slice_type( Core::VolumeSliceType::AXIAL_E );
  slice_width = volume_slice->nx() > 1 ? ( volume_slice->right() - volume_slice->left() ) * 
      volume_slice->nx() / ( volume_slice->nx() - 1 ) : 0;
  slice_height = volume_slice->ny() > 1 ? ( volume_slice->top() - volume_slice->bottom() ) * 
    volume_slice->ny() / ( volume_slice->ny() - 1 ) : 0;
  center = Core::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
    ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 
    this->viewer_->axial_view_state_->get().center().z() );
  scale = 1.0 / Core::Max( Core::Abs( slice_height ), Core::Abs( slice_width ) / aspect );
  scalex = scale * Core::Sign( this->viewer_->axial_view_state_->get().scalex() );
  scaley = scale * Core::Sign( this->viewer_->axial_view_state_->get().scaley() );
  this->viewer_->axial_view_state_->set( Core::View2D( center, scalex, scaley ) );

  volume_slice->set_slice_type( Core::VolumeSliceType::CORONAL_E );
  slice_width = volume_slice->nx() > 1 ? ( volume_slice->right() - volume_slice->left() ) * 
    volume_slice->nx() / ( volume_slice->nx() - 1 ) : 0;
  slice_height = volume_slice->ny() > 1 ? ( volume_slice->top() - volume_slice->bottom() ) * 
    volume_slice->ny() / ( volume_slice->ny() - 1 ) : 0;
  center = Core::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
    ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 
    this->viewer_->coronal_view_state_->get().center().z() );
  scale = 1.0 / Core::Max( Core::Abs( slice_height ), Core::Abs( slice_width ) / aspect );
  scalex = scale * Core::Sign( this->viewer_->coronal_view_state_->get().scalex() );
  scaley = scale * Core::Sign( this->viewer_->coronal_view_state_->get().scaley() );
  this->viewer_->coronal_view_state_->set( Core::View2D( center, scalex, scaley ) );

  volume_slice->set_slice_type( Core::VolumeSliceType::SAGITTAL_E );
  slice_width = volume_slice->nx() > 1 ? ( volume_slice->right() - volume_slice->left() ) * 
    volume_slice->nx() / ( volume_slice->nx() - 1 ) : 0;
  slice_height = volume_slice->ny() > 1 ? ( volume_slice->top() - volume_slice->bottom() ) * 
    volume_slice->ny() / ( volume_slice->ny() - 1 ) : 0;
  center = Core::Point( ( volume_slice->left() + volume_slice->right() ) * 0.5, 
    ( volume_slice->bottom() + volume_slice->top() ) * 0.5, 
    this->viewer_->sagittal_view_state_->get().center().z() );
  scale = 1.0 / Core::Max( Core::Abs( slice_height ), Core::Abs( slice_width ) / aspect );
  scalex = scale * Core::Sign( this->viewer_->sagittal_view_state_->get().scalex() );
  scaley = scale * Core::Sign( this->viewer_->sagittal_view_state_->get().scaley() );
  this->viewer_->sagittal_view_state_->set( Core::View2D( center, scalex, scaley ) );

  Core::VolumeHandle volume = volume_slice->get_volume();
  Core::Point corner1 = volume->apply_grid_transform( Core::Point( -0.5, -0.5, -0.5 ) );
  Core::Point corner2 = volume->apply_grid_transform(
    Core::Point( static_cast< double >( volume->get_nx() - 0.5 ), 
    static_cast< double >( volume->get_ny() - 0.5 ), static_cast< double >( volume->get_nz() - 0.5 ) ) );
  Core::View3D view3d( this->viewer_->volume_view_state_->get() );
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
  view3d.eyep( view3d.eyep() + eye_vec * eye_offset );
  this->viewer_->volume_view_state_->set( view3d );
}

void ViewerPrivate::adjust_depth( Core::VolumeSliceHandle target_slice )
{
  if ( !target_slice )
  {
    CORE_LOG_ERROR( "Invalid volume slice handle" );
    return;
  }

  Core::VolumeSliceHandle volume_slice = target_slice->clone();

  volume_slice->set_slice_type( Core::VolumeSliceType::AXIAL_E );
  volume_slice->set_slice_number( volume_slice->number_of_slices() / 2 );
  Core::View2D view2d( this->viewer_->axial_view_state_->get() );
  view2d.center().z( volume_slice->depth() );
  this->viewer_->axial_view_state_->set( view2d );

  volume_slice->set_slice_type( Core::VolumeSliceType::CORONAL_E );
  volume_slice->set_slice_number( volume_slice->number_of_slices() / 2 );
  view2d = this->viewer_->coronal_view_state_->get();
  view2d.center().z( volume_slice->depth() );
  this->viewer_->coronal_view_state_->set( view2d );

  volume_slice->set_slice_type( Core::VolumeSliceType::SAGITTAL_E );
  volume_slice->set_slice_number( volume_slice->number_of_slices() / 2 );
  view2d = this->viewer_->sagittal_view_state_->get();
  view2d.center().z( volume_slice->depth() );
  this->viewer_->sagittal_view_state_->set( view2d );
}

void ViewerPrivate::auto_orient( Core::VolumeSliceHandle target_slice )
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
  this->viewer_->volume_view_state_->set( view3d );
}

void ViewerPrivate::move_slice_by( double depth_offset )
{
  if ( depth_offset == 0.0 || 
    this->viewer_->is_volume_view() || 
    !this->active_layer_slice_ )
  {
    return;
  }

  {
    Core::ScopedCounter block_counter( this->signals_block_count_ );

    Core::StateView2D* view2d_state = dynamic_cast< Core::StateView2D* >( 
      this->viewer_->get_active_view_state().get() );
    view2d_state->dolly( depth_offset );
    double depth = view2d_state->get().center().z();

    // Move all the slices to the new position
    for ( volume_slice_map_type::iterator it = this->volume_slices_.begin();
      it != this->volume_slices_.end(); ++it )
    {
      ( *it ).second->move_slice_to( depth );
    }

    if ( !this->active_layer_slice_->out_of_boundary() )
    {
      Core::ScopedCounter slice_lock_counter( this->slice_lock_count_ );
      this->viewer_->slice_number_state_->set( static_cast< int >( 
        this->active_layer_slice_->get_slice_number() ) );
    }
  }

  this->viewer_->redraw_all();

  // NOTE: No need to trigger slice_changed_signal_ here because the viewer that caused
  // the slice change will trigger it.

}

void ViewerPrivate::reset_active_slice()
{
  if ( !this->viewer_->is_volume_view() && 
    this->active_layer_slice_ &&
    this->active_layer_slice_->out_of_boundary() )
  {
    Core::StateView2D* view2d_state =   dynamic_cast< Core::StateView2D* >( 
      this->viewer_->get_active_view_state().get() );
    this->active_layer_slice_->move_slice_to( view2d_state->get().center().z(), true );
    if ( this->active_layer_slice_->get_slice_number() ==
      static_cast< size_t >( this->viewer_->slice_number_state_->get() ) )
    {
      this->set_slice_number( this->viewer_->slice_number_state_->get() );
      this->viewer_->redraw_scene();
    }
    else
    {
      this->viewer_->slice_number_state_->set( static_cast< int >( 
        this->active_layer_slice_->get_slice_number() ) );
    }
  }
}

void ViewerPrivate::set_viewer_labels()
{
  Core::OptionLabelPairVector label_options;
  label_options.push_back( std::make_pair( Viewer::SAGITTAL_C, 
    PreferencesManager::Instance()->x_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( Viewer::CORONAL_C, 
    PreferencesManager::Instance()->y_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( Viewer::AXIAL_C, 
    PreferencesManager::Instance()->z_axis_label_state_->get() ) );
  label_options.push_back( std::make_pair( Viewer::VOLUME_C, 
    "Volume" ) );

  this->viewer_->view_mode_state_->set_option_list( label_options );
}

void ViewerPrivate::redraw_scene( Core::PickPointHandle pick_point )
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &ViewerPrivate::redraw_scene, this, pick_point ) );
    return;
  }

  if ( this->signals_block_count_ == 0 && this->viewer_->viewer_visible_state_->get() )
  {
    if( pick_point )
    {
      this->viewer_->redraw_scene_pick_signal_( pick_point );
    }
    else
    {
      this->viewer_->redraw_scene_signal_();
    }
  }
}

void ViewerPrivate::reset()
{
  ASSERT_IS_APPLICATION_THREAD();

  this->volume_slices_.clear();
  this->layer_connection_map_.clear();
  this->active_layer_slice_.reset();
  this->viewer_->redraw_all();
}

void ViewerPrivate::handle_flip_horizontal_changed( bool flip )
{
  if ( this->signals_block_count_ > 0 || this->loading_ )
  {
    return;
  }
  
  Core::StateView2DHandle view2d_state = boost::dynamic_pointer_cast< Core::StateView2D >(
    this->viewer_->get_active_view_state() );
  if ( view2d_state )
  {
    Core::View2D view2d = view2d_state->get();
    double abs_scalex = Core::Abs( view2d.scalex() );
    view2d.scalex( flip ? -abs_scalex : abs_scalex );
    view2d_state->set( view2d );
  }
}

void ViewerPrivate::handle_flip_vertical_changed( bool flip )
{
  if ( this->signals_block_count_ > 0 || this->loading_ )
  {
    return;
  }

  Core::StateView2DHandle view2d_state = boost::dynamic_pointer_cast< Core::StateView2D >(
    this->viewer_->get_active_view_state() );
  if ( view2d_state )
  {
    Core::View2D view2d = view2d_state->get();
    double abs_scaley = Core::Abs( view2d.scaley() );
    view2d.scaley( flip ? -abs_scaley : abs_scaley );
    view2d_state->set( view2d );
  }
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class Viewer
//////////////////////////////////////////////////////////////////////////

const std::string Viewer::AXIAL_C( "axial" );
const std::string Viewer::CORONAL_C( "coronal" );
const std::string Viewer::SAGITTAL_C( "sagittal" );
const std::string Viewer::VOLUME_C( "volume" );

Viewer::Viewer( size_t viewer_id, bool visible, const std::string& mode ) :
  Core::AbstractViewer(  viewer_id ),
  private_( new ViewerPrivate )
{
  this->private_->viewer_ = this;
  this->private_->loading_ = false;
  this->private_->signals_block_count_ = 0;
  this->private_->adjusting_contrast_brightness_ = false;
  this->private_->slice_lock_count_ = 0;
  this->private_->view_manipulator_ = ViewManipulatorHandle( new ViewManipulator( this ) );
  this->private_->mouse_pressed_ = false;

  std::string sagittal = SAGITTAL_C + "=" + PreferencesManager::Instance()->x_axis_label_state_->get();
  std::string coronal = CORONAL_C + "=" + PreferencesManager::Instance()->y_axis_label_state_->get();
  std::string axial = AXIAL_C + "=" + PreferencesManager::Instance()->z_axis_label_state_->get();
  
  // Indicate that this statehandler contains data that is part of the project
  //this->mark_as_project_data();

  this->add_state( "view_mode", view_mode_state_, mode, sagittal + "|" + coronal 
     + "|" + axial + "|" + VOLUME_C + "=Volume" );
  this->view_mode_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 1 );
     
  this->add_connection( PreferencesManager::Instance()->x_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &ViewerPrivate::set_viewer_labels, this->private_ ) ) );
  this->add_connection( PreferencesManager::Instance()->y_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &ViewerPrivate::set_viewer_labels, this->private_ ) ) );
  this->add_connection( PreferencesManager::Instance()->z_axis_label_state_->state_changed_signal_.
    connect( boost::bind( &ViewerPrivate::set_viewer_labels, this->private_ ) ) );

  this->add_connection( PreferencesManager::Instance()->grid_size_state_->state_changed_signal_.
    connect( boost::bind( &Viewer::redraw_overlay, this ) ) );
  this->add_connection( PreferencesManager::Instance()->show_slice_number_state_->
    state_changed_signal_.connect( boost::bind( &Viewer::redraw_overlay, this ) ) );
  this->add_connection( PreferencesManager::Instance()->background_color_state_->
    state_changed_signal_.connect( boost::bind( &Viewer::redraw_all, this ) ) );
  this->add_connection( PreferencesManager::Instance()->zero_based_slice_numbers_state_->
    state_changed_signal_.connect( boost::bind( &Viewer::redraw_overlay, this ) ) );
  
  const std::vector< Core::StateColorHandle >& color_states = PreferencesManager::Instance()->color_states_;
  for ( size_t i = 0; i < color_states.size(); ++i )
  {
    this->add_connection( color_states[ i ]->state_changed_signal_.connect( 
      boost::bind( &Viewer::redraw_scene, this ) ) );
  }
  
  this->add_state( "axial_view", axial_view_state_ );
  this->add_state( "coronal_view", coronal_view_state_ );
  this->add_state( "sagittal_view", sagittal_view_state_ );
  this->add_state( "volume_view", volume_view_state_ );

  this->add_state( "flip_horizontal", this->flip_horizontal_state_, false );
  this->flip_horizontal_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->add_state( "flip_vertical", this->flip_vertical_state_, false );
  this->flip_vertical_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->add_connection( this->flip_horizontal_state_->value_changed_signal_.connect( boost::bind(
    &ViewerPrivate::handle_flip_horizontal_changed, this->private_, _1 ) ) );
  this->add_connection( this->flip_vertical_state_->value_changed_signal_.connect( boost::bind(
    &ViewerPrivate::handle_flip_vertical_changed, this->private_, _1 ) ) );

  this->private_->view_states_[ 0 ] = this->sagittal_view_state_;
  this->private_->view_states_[ 1 ] = this->coronal_view_state_;
  this->private_->view_states_[ 2 ] = this->axial_view_state_;
  this->private_->view_states_[ 3 ] = this->volume_view_state_;

  this->add_state( "slice_number", this->slice_number_state_, 0, 0, 0, 1 );
  this->slice_number_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->add_state( "slice_grid", this->slice_grid_state_, false );
  this->add_state( "slice_visible", this->slice_visible_state_, visible);
  this->add_state( "slice_picking_visible", this->slice_picking_visible_state_, true );

  this->add_state( "volume_slices_visible", this->volume_slices_visible_state_, true );
  this->add_state( "volume_isosurfaces_visible", this->volume_isosurfaces_visible_state_, true );
  this->add_state( "volume_volume_rendering_visible", 
    this->volume_volume_rendering_visible_state_, false );
  this->add_state( "volume_light_visible", this->volume_light_visible_state_, true );
  this->add_state( "volume_enable_fog", this->volume_enable_fog_state_, false );
  this->add_state( "volume_enable_clipping", this->volume_enable_clipping_state_, true );
  this->add_state( "volume_show_invisible_slices", this->volume_show_invisible_slices_state_, true );
  this->add_state( "volume_show_bounding_box", this->volume_show_bounding_box_state_, true );

  this->add_state( "lock", this->lock_state_, false );
  this->add_state( "overlay_visible", this->overlay_visible_state_, true );
    
  this->add_state( "is_picking_target", this->is_picking_target_state_, false );
  this->is_picking_target_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  this->add_connection( LayerManager::Instance()->layer_inserted_signal_.connect( 
    boost::bind( &ViewerPrivate::insert_layer, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layers_deleted_signal_.connect(
    boost::bind( &ViewerPrivate::delete_layers, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
    boost::bind( &ViewerPrivate::set_active_layer, this->private_, _1 ) ) );
  this->add_connection( LayerManager::Instance()->layers_reordered_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( LayerManager::Instance()->groups_reordered_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );

  this->add_connection( this->view_mode_state_->value_changed_signal_.connect(
    boost::bind( &ViewerPrivate::change_view_mode, this->private_, _2, _3 ) ) );
    
  this->add_connection( this->slice_number_state_->value_changed_signal_.connect(
    boost::bind( &ViewerPrivate::set_slice_number, this->private_, _1, _2 ) ) );
    
  this->add_connection( this->viewer_visible_state_->value_changed_signal_.connect(
    boost::bind( &ViewerPrivate::change_visibility, this->private_, _1 ) ) );
  this->add_connection( this->lock_state_->value_changed_signal_.connect(
    boost::bind( &ViewerPrivate::viewer_lock_state_changed, this->private_, _1 ) ) );

  // Connect state variables that should trigger redraw_all.
  this->add_connection( this->view_mode_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_all, this ) ) );
  this->add_connection( this->axial_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_all, this ) ) );
  this->add_connection( this->coronal_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_all, this ) ) );
  this->add_connection( this->sagittal_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_all, this) ) );
  this->add_connection( this->slice_number_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_all, this ) ) );
  this->add_connection( this->volume_view_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_all, this ) ) );
    
  // Connect state variables that should trigger redraw_scene.
  this->add_connection( this->volume_light_visible_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_enable_fog_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_enable_clipping_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_slices_visible_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_isosurfaces_visible_state_->state_changed_signal_.
    connect( boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_volume_rendering_visible_state_->state_changed_signal_.
    connect( boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_show_invisible_slices_state_->state_changed_signal_.
    connect( boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( this->volume_show_bounding_box_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );
  this->add_connection( Core::LargeVolumeCache::Instance()->brick_loaded_signal_.connect(
    boost::bind( &Viewer::redraw_scene, this ) ) );

  // Connect state variables that should trigger redraw_overlay
  this->add_connection( this->slice_grid_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_overlay, this) ) );
  this->add_connection( this->slice_picking_visible_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_overlay, this ) ) );
  this->add_connection( this->overlay_visible_state_->state_changed_signal_.connect(
    boost::bind( &Viewer::redraw_overlay, this ) ) );

  this->add_connection( Core::Application::Instance()->reset_signal_.connect(
    boost::bind( &ViewerPrivate::reset, this->private_ ) ) );
}

Viewer::~Viewer()
{
  this->disconnect_all();
  this->reset_mouse_handlers();
}

void Viewer::resize( int width, int height )
{
  AbstractViewer::resize( width, height );
  this->private_->view_manipulator_->resize( width, height );
}

void Viewer::install_renderer( Core::AbstractRendererHandle renderer )
{
  AbstractViewer::install_renderer( renderer ); 

  Core::RendererBaseHandle renderer_base = 
    boost::dynamic_pointer_cast< Core::RendererBase >( renderer );
  if( renderer_base )
  {
    this->add_connection( renderer_base->volume_pick_point_signal_.connect(
      boost::bind( &ViewerPrivate::pick_point, this->private_, _1 ) ) );
  }
}

void Viewer::mouse_move_event( const Core::MouseHistory& mouse_history, int button, int buttons,
    int modifiers )
{
  if ( !this->private_->mouse_move_handler_.empty() )
  {
    // if the registered handler handled the event, no further process needed
    if ( this->private_->mouse_move_handler_( this->shared_from_this(), mouse_history, 
      button, buttons, modifiers ) )
    {
      return;
    }
  }

  this->update_status_bar( mouse_history.current_.x_, mouse_history.current_.y_ );

  if ( this->private_->adjusting_contrast_brightness_ )
  {
    this->private_->adjust_contrast_brightness( mouse_history.current_.x_ - mouse_history.previous_.x_,
      mouse_history.previous_.y_ - mouse_history.current_.y_ );
    return;
  }
  
  // default handling here
  this->private_->view_manipulator_->mouse_move( mouse_history, button, buttons, modifiers );
}

void Viewer::mouse_press_event( const Core::MouseHistory& mouse_history, int button, int buttons,
    int modifiers )
{
  {
    boost::mutex::scoped_lock lock( this->private_->mouse_pressed_mutex_ );
    this->private_->mouse_pressed_ = true;
  }
  
  if ( !this->private_->mouse_press_handler_.empty() )
  {
    // if the registered handler handled the event, no further process needed
    if ( this->private_->mouse_press_handler_( this->shared_from_this(), mouse_history, 
      button, buttons, modifiers ) )
    {
      return;
    }
  }

  if ( button == Core::MouseButton::RIGHT_BUTTON_E && 
    ( modifiers == Core::KeyModifier::CONTROL_MODIFIER_E ) )
  {
    this->private_->pick_point( mouse_history.current_.x_, mouse_history.current_.y_ );
    return;
  }

  if ( button == Core::MouseButton::LEFT_BUTTON_E &&
    modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    !( this->is_volume_view() ) )
  {
    this->private_->adjusting_contrast_brightness_ = true;
    return;
  }
  
  // default handling here
  this->private_->view_manipulator_->mouse_press( mouse_history, button, buttons, modifiers );
}

void Viewer::mouse_release_event( const Core::MouseHistory& mouse_history, int button, int buttons,
    int modifiers )
{
  {
    boost::mutex::scoped_lock lock( this->private_->mouse_pressed_mutex_ );
    this->private_->mouse_pressed_ = false;
  }
  
  if ( !this->private_->mouse_release_handler_.empty() )
  {
    // if the registered handler handled the event, no further process needed
    if ( this->private_->mouse_release_handler_( this->shared_from_this(), mouse_history, 
      button, buttons, modifiers ) )
    {
      return;
    }
  }

  if ( button == Core::MouseButton::LEFT_BUTTON_E )
  {
    this->private_->adjusting_contrast_brightness_ = false;
  }
  
  // default handling here
  this->private_->view_manipulator_->mouse_release( mouse_history, button, buttons, modifiers );
}

void Viewer::mouse_enter_event( int x, int y )
{
  if ( this->private_->mouse_enter_handler_ )
  {
    this->private_->mouse_enter_handler_( this->shared_from_this(), x, y );
  }
}

void Viewer::mouse_leave_event()
{
#ifdef __APPLE__
  // Mac doesn't give matching mouse release event if I leave the window with the mouse pressed.  
  {
    boost::mutex::scoped_lock lock( this->private_->mouse_pressed_mutex_ );
    this->private_->mouse_pressed_ = false;
  }
#endif

  if ( this->private_->mouse_leave_handler_ )
  {
    this->private_->mouse_leave_handler_( this->shared_from_this() );
  }
}

bool Viewer::wheel_event( int delta, int x, int y, int buttons, int modifiers )
{
  if ( !this->private_->wheel_event_handler_.empty() )
  {
    if ( this->private_->wheel_event_handler_( this->shared_from_this(), 
      delta, x, y, buttons, modifiers ) )
    {
      return true;
    }
  }

  if ( delta != 0 )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    if ( PreferencesManager::Instance()->reverse_slice_navigation_state_->get() )
    {
      ActionOffsetSlice::Dispatch( Core::Interface::GetMouseActionContext(),
        this->shared_from_this(), -delta );   
    }
    else
    {
      ActionOffsetSlice::Dispatch( Core::Interface::GetMouseActionContext(),
        this->shared_from_this(), delta );
    }
    // Update the status bar display.
    // 'update_status_bar' reposts itself to the application thread, so it's guaranteed that 
    // by the time it actually runs, the slice number has been updated.
    this->update_status_bar( x, y );
  }

  return true;
}

bool Viewer::key_press_event( int key, int modifiers, int x, int y )
{
  if ( !this->private_->key_press_event_handler_.empty() )
  {
    if ( this->private_->key_press_event_handler_( this->shared_from_this(), key, modifiers ) )
    {
      return true;
    }
  }
  
  bool handled_successfully = false;
  
  if ( ( modifiers == Core::KeyModifier::SHIFT_MODIFIER_E ) || 
     ( modifiers == Core::KeyModifier::NO_MODIFIER_E ) || 
     ( key == Core::Key::KEY_LEFT_E ) || ( key == Core::Key::KEY_RIGHT_E ) ||
     ( key == Core::Key::KEY_DOWN_E ) || ( key == Core::Key::KEY_UP_E ) )
  {
    switch ( key )
    {
      case Core::Key::KEY_LESS_E:
      case Core::Key::KEY_COMMA_E:
      case Core::Key::KEY_DOWN_E:
      {
        Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
        int direction = 1;
        if ( PreferencesManager::Instance()->reverse_slice_navigation_state_->get() ) direction = -1;
      
        if ( modifiers & Core::KeyModifier::SHIFT_MODIFIER_E )
        {
          ActionOffsetSlice::Dispatch( Core::Interface::GetKeyboardActionContext(),
            this->shared_from_this(), 
            -direction*( PreferencesManager::Instance()->slice_step_multiplier_state_->get() ) );     
        }
        else
        {
          ActionOffsetSlice::Dispatch( Core::Interface::GetKeyboardActionContext(),
            this->shared_from_this(), -direction );
        }
        handled_successfully = true;
        break;    
      }
      case Core::Key::KEY_GREATER_E:
      case Core::Key::KEY_PERIOD_E:
      case Core::Key::KEY_UP_E:
      {
        Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
        int direction = 1;
        if ( PreferencesManager::Instance()->reverse_slice_navigation_state_->get() ) direction = -1;

        if ( modifiers & Core::KeyModifier::SHIFT_MODIFIER_E )
        {
          ActionOffsetSlice::Dispatch( Core::Interface::GetKeyboardActionContext(),
            this->shared_from_this(), 
            direction * ( PreferencesManager::Instance()->slice_step_multiplier_state_->get() ) );        
        }
        else
        {   
          ActionOffsetSlice::Dispatch( Core::Interface::GetKeyboardActionContext(),
            this->shared_from_this(), direction );
        }
        handled_successfully = true;
        break;
      }
        
      case Core::Key::KEY_LEFT_E:
      {
        if ( PreferencesManager::Instance()->active_layer_navigation_state_->get() )
        {
        ActionShiftActiveLayer::Dispatch( Core::Interface::GetKeyboardActionContext(), false );
        }
        handled_successfully = true;
        break;
      }
        
      case Core::Key::KEY_RIGHT_E:
      {
        if ( PreferencesManager::Instance()->active_layer_navigation_state_->get() )
        {
        ActionShiftActiveLayer::Dispatch( Core::Interface::GetKeyboardActionContext(), true );
        }
        handled_successfully = true;
        break;
      }
    
      case Core::Key::KEY_G_E:
      {
        Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
          this->slice_grid_state_ );
        handled_successfully = true;
        break;
      }
    
      case Core::Key::KEY_L_E:
      {
        Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
          this->lock_state_ );
        handled_successfully = true;
        break;
      }
    
      case Core::Key::KEY_S_E:
      {
        // Need to lock the query into the state engine, as this function is called
        // from a variety of threads
        Core::StateEngine::lock_type( Core::StateEngine::Instance()->GetMutex() );
        if ( this->view_mode_state_->get() == Viewer::VOLUME_C )
        {
          Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
            this->volume_slices_visible_state_ ); 
        }
        else
        {
          Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
            this->slice_visible_state_ );   
        }
        handled_successfully = true;
        break;
      }
      
      case Core::Key::KEY_T_E:
      {
        Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
          this->overlay_visible_state_ );   
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_I_E:
      {
        Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
          this->volume_isosurfaces_visible_state_ );    
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_H_E:
      {
        Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
          this->volume_light_visible_state_ );    
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_P_E:
      {
        Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
          this->slice_picking_visible_state_ );   
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_0_E:
      case Core::Key::KEY_A_E:
      {
        ActionAutoView::Dispatch( Core::Interface::GetKeyboardActionContext(), 
          this->get_viewer_id() );  
        handled_successfully = true;
        break;
      }
        
      case Core::Key::KEY_SPACE_E:
      {
        LayerHandle layer = LayerManager::Instance()->get_active_layer();
        if ( layer )
        {
          if ( this->get_viewer_id() < layer->visible_state_.size() )
          {
            Core::ActionToggle::Dispatch( Core::Interface::GetKeyboardActionContext(),
              layer->master_visible_state_ );
          }
        }
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_V_E:
      {
        Core::ActionSet::Dispatch( Core::Interface::GetKeyboardActionContext(), 
          this->view_mode_state_, Viewer::VOLUME_C ); 
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_X_E:
      {
        Core::ActionSet::Dispatch( Core::Interface::GetKeyboardActionContext(), 
          this->view_mode_state_, Viewer::SAGITTAL_C ); 
        handled_successfully = true;
        break;
      }

      case Core::Key::KEY_Y_E:
      {
        Core::ActionSet::Dispatch( Core::Interface::GetKeyboardActionContext(), 
          this->view_mode_state_, Viewer::CORONAL_C );  
        handled_successfully = true;
        break;
      }     

      case Core::Key::KEY_Z_E:
      {
        Core::ActionSet::Dispatch( Core::Interface::GetKeyboardActionContext(), 
          this->view_mode_state_, Viewer::AXIAL_C );  
        handled_successfully = true;
        break;
      } 
    }
    
    if( handled_successfully ) 
    {
      this->update_status_bar( x, y );
      return true;
    }
    
  }

  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    key == Core::Key::KEY_O_E )
  {
    ActionComputeIsosurface::Dispatch( Core::Interface::GetKeyboardActionContext() );
    return true;
  }
  
  // function wasn't handled, hence return false.
  return false;
}

bool Viewer::key_release_event( int key, int modifiers, int x, int y )
{
  if ( !this->private_->key_release_event_handler_.empty() )
  {
    if ( this->private_->key_release_event_handler_( this->shared_from_this(), key, modifiers ) )
    {
      return true;
    }
  }

  // Not handled
  return false;
}

void Viewer::set_mouse_move_handler( mouse_event_handler_type func )
{
  this->private_->mouse_move_handler_ = func;
}

void Viewer::set_mouse_press_handler( mouse_event_handler_type func )
{
  this->private_->mouse_press_handler_ = func;
}

void Viewer::set_mouse_release_handler( mouse_event_handler_type func )
{
  this->private_->mouse_release_handler_ = func;
}

void Viewer::set_mouse_enter_handler( enter_event_handler_type func )
{
  this->private_->mouse_enter_handler_ = func;
}

void Viewer::set_mouse_leave_handler( leave_event_handler_type func )
{
  this->private_->mouse_leave_handler_ = func;
}

void Viewer::set_wheel_event_handler( wheel_event_handler_type func )
{
  this->private_->wheel_event_handler_ = func;
}

void Viewer::set_key_press_event_handler( key_press_event_handler_type func )
{
  this->private_->key_press_event_handler_ = func;
}

void Viewer::set_key_release_event_handler( key_release_event_handler_type func )
{
  this->private_->key_release_event_handler_ = func;
}

void Viewer::set_cursor_handler( cursor_handler_type func )
{
  this->private_->cursor_handler_ = func;
}

void Viewer::reset_mouse_handlers()
{
  this->private_->mouse_move_handler_ = 0;
  this->private_->mouse_press_handler_ = 0;
  this->private_->mouse_release_handler_ = 0;
  this->private_->mouse_enter_handler_ = 0;
  this->private_->mouse_leave_handler_ = 0;
  this->private_->wheel_event_handler_ = 0;
  this->private_->key_press_event_handler_ = 0;
  this->private_->key_release_event_handler_ = 0;
  this->private_->cursor_handler_ = 0;
}

bool Viewer::is_busy()
{
  boost::mutex::scoped_lock lock( this->private_->mouse_pressed_mutex_ ); 
  return this->private_->mouse_pressed_;
}

void Viewer::update_status_bar( int x, int y, const std::string& layer_id )
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( 
      &Viewer::update_status_bar, this, x, y, layer_id ) );
    return;
  }

  Core::VolumeSliceHandle volume_slice;
  if ( layer_id == "" )
  {
    volume_slice = this->private_->active_layer_slice_;
  }
  else
  {
    volume_slice = this->get_volume_slice( layer_id );
  }

  if ( !this->is_volume_view() && volume_slice &&
    volume_slice->is_valid() )
  {
    double xpos, ypos;
    this->window_to_world( x, y, xpos, ypos );

    int i, j;
    volume_slice->world_to_index( xpos, ypos, i, j );
    Core::Point index;
    if ( i >= 0 && static_cast<size_t>( i ) < volume_slice->nx() && 
       j >= 0 && static_cast<size_t>( j ) < volume_slice->ny() )
    {
      volume_slice->to_index( static_cast<size_t>( i ), static_cast<size_t>( j ), index );
      Core::Point world_pos = volume_slice->apply_grid_transform( index );
      double value = 0.0;
      if ( volume_slice->volume_type() == Core::VolumeType::DATA_E )
      {
        Core::DataVolumeSlice* data_slice = dynamic_cast< 
          Core::DataVolumeSlice* >( volume_slice.get() );
        value = data_slice->get_data_at( static_cast<size_t>( i ), static_cast<size_t>( j ) );
      }
      else if ( volume_slice->volume_type() == Core::VolumeType::MASK_E )
      {
        Core::MaskVolumeSlice* mask_slice = dynamic_cast< 
          Core::MaskVolumeSlice* >( volume_slice.get() );
        value = mask_slice->get_mask_at( static_cast<size_t>( i ), static_cast<size_t>( j ) );
      }
      DataPointInfoHandle data_point( new DataPointInfo( index, world_pos, value ) );
      StatusBar::Instance()->set_data_point_info( data_point );
      return;
    }
  }

  StatusBar::Instance()->set_data_point_info( DataPointInfoHandle() );
}

bool Viewer::is_volume_view() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->view_mode_state_->get() == VOLUME_C;
}

Core::StateViewBaseHandle Viewer::get_active_view_state() const
{
  return this->private_->view_states_[ this->view_mode_state_->index() ];
}

Core::VolumeSliceHandle Viewer::get_volume_slice( const std::string& layer_id )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ViewerPrivate::volume_slice_map_type::iterator it = 
    this->private_->volume_slices_.find( layer_id );
  if ( it != this->private_->volume_slices_.end() )
  {
    return ( *it ).second;
  }
  return Core::VolumeSliceHandle();
}

void Viewer::redraw_scene()
{
  this->private_->redraw_scene( Core::PickPointHandle() );
}

void Viewer::redraw_overlay()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Viewer::redraw_overlay, this ) );
    return;
  }

  if ( this->private_->signals_block_count_ == 0 &&
    this->viewer_visible_state_->get() )
  {
    this->redraw_overlay_signal_();
  }
}

void Viewer::redraw_all()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Viewer::redraw_all, this ) );
    return;
  }

  if ( this->private_->signals_block_count_ == 0 &&
    this->viewer_visible_state_->get() )
  {
    this->redraw_all_signal_();
  }
}


void Viewer::auto_view()
{
  if ( !this->private_->active_layer_slice_ )
  {
    return;
  }
  
  {
    Core::ScopedCounter block_counter( this->private_->signals_block_count_ );
    this->private_->adjust_view( this->private_->active_layer_slice_ );
  }
  
  this->redraw_all();
}

void Viewer::move_slice_to( const Core::Point& pt )
{
  if ( !this->is_volume_view() && this->private_->active_layer_slice_ )
  {
    this->private_->active_layer_slice_->move_slice_to( pt, true );
    this->slice_number_state_->set( static_cast< int >( 
      this->private_->active_layer_slice_->get_slice_number() ) );
  }
}

int Viewer::offset_slice( int delta )
{
  // This function should only be called by ActionOffsetSlice. 
  // The following assertion is to ensure that.
  assert ( Core::Application::IsApplicationThread() );

  if ( !this->is_volume_view() && delta != 0 && this->private_->active_layer_slice_ )
  {
    if ( this->private_->active_layer_slice_->out_of_boundary() )
    {
      this->private_->reset_active_slice();
      return 0;
    }

    int old_slice = this->slice_number_state_->get();
    this->slice_number_state_->offset( delta );
    return this->slice_number_state_->get() - old_slice;
  }

  return 0;
}

Core::VolumeSliceHandle Viewer::get_active_volume_slice() const
{
  return this->private_->active_layer_slice_;
}

void Viewer::window_to_world( int x, int y, double& world_x, double& world_y ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if ( this->is_volume_view() )
  {
    // NOTE: Log a warning instead of throwing an exception to avoid crash
    CORE_LOG_WARNING( "Viewer::window_to_world called when viewer is in volume view." );
    return;
  }

  // Scale the mouse position to [-1, 1]
  double width =  static_cast<double>( this->get_width() );
  double height = static_cast<double>( this->get_height() );

  double xpos = x * 2.0 / ( width - 1 ) - 1.0;
  double ypos = ( height - 1 - y ) * 2.0 / ( height - 1.0 ) - 1.0;

  Core::Matrix proj, inv_proj;
  this->get_projection_matrix( proj );
  Core::Matrix::Invert( proj, inv_proj );
  Core::Point pos( xpos, ypos, 0 );
  pos = inv_proj * pos;

  world_x = pos.x();
  world_y = pos.y();
}

void Viewer::world_to_window( double world_x, double world_y, double& x, double& y ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if ( this->is_volume_view() )
  {
    // NOTE: Log a warning instead of throwing an exception to avoid crash
    CORE_LOG_WARNING( "Viewer::world_to_window called when viewer is in volume view." );
    return;
  }

  Core::Matrix proj;
  this->get_projection_matrix( proj );

  Core::Point pos( world_x, world_y, 0 );
  pos = proj * pos;

  // Now have mouse position in range [-1, 1]
  // Scale the mouse position to [0, nx|ny]
  double width =  static_cast<double>( this->get_width() );
  double height = static_cast<double>( this->get_height() );
  
  x = static_cast< int >( ( ( pos.x() + 1 ) * ( width - 1 ) ) / 2.0 ); 
  y = static_cast< int >( height - 1 - ( ( ( pos.y() + 1 ) * ( height - 1.0 ) ) / 2.0 ) ); 
}

void Viewer::get_projection_matrix( Core::Matrix& proj_mat ) const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if ( this->is_volume_view() )
  {
    // NOTE: Log a warning instead of throwing an exception to avoid crash
    CORE_LOG_WARNING( "Viewer::get_projection_matrix called when viewer is in volume view." );
    return;
  }

  double left, right, bottom, top;
  Core::StateView2D* view_2d = dynamic_cast<Core::StateView2D*>( 
    this->get_active_view_state().get() );
  view_2d->get().compute_clipping_planes( this->get_width() / 
    ( this->get_height() * 1.0 ), left, right, bottom, top );

  Core::Transform::BuildOrtho2DMatrix( proj_mat, left, right, bottom, top );
}

void Viewer::snap_to_axis()
{
  ASSERT_IS_APPLICATION_THREAD();

  Core::View3D view_3d = this->volume_view_state_->get();

  // Snap the eye position
  Core::Vector eye_vec = view_3d.eyep() - view_3d.lookat();
  double eye_dist = eye_vec.normalize();
  int closest_eye_axis = 0;
  for ( int i = 1; i < 3; ++i )
  {
    if ( Core::Abs( eye_vec[ i ] ) > Core::Abs( eye_vec[ closest_eye_axis ] ) )
    {
      closest_eye_axis = i;
    }
  }

  // Snap the up vector
  // NOTE: The up vector must not line up with the eye vector
  Core::Vector up = view_3d.up();
  int closest_up_axis = 0;
  if ( Core::Abs( up[ ( closest_eye_axis + 1 ) % 3 ] ) > 
    Core::Abs( up[ ( closest_eye_axis + 2 ) % 3 ] ) )
  {
    closest_up_axis = ( closest_eye_axis + 1 ) % 3;
  }
  else
  {
    closest_up_axis = ( closest_eye_axis + 2 ) % 3;
  }

  Core::Vector new_eye_dir( 0.0, 0.0, 0.0 );
  new_eye_dir[ closest_eye_axis ] = Core::Sign( eye_vec[ closest_eye_axis ] ) * eye_dist;
  Core::Vector new_up( 0.0, 0.0, 0.0 );
  new_up[ closest_up_axis ] = Core::Sign( up[ closest_up_axis ] );
  
  view_3d.eyep( view_3d.lookat() + new_eye_dir );
  view_3d.up( new_up );

  this->volume_view_state_->set( view_3d );
}

void Viewer::update_slice_volume( LayerHandle layer )
{
  Core::VolumeSliceHandle volume_slice = this->get_volume_slice( layer->get_layer_id() );
  assert( volume_slice );
  volume_slice->set_volume( layer->get_volume() );
}

bool Viewer::pre_load_states( const Core::StateIO& state_io )
{
  // Set the loading flag so the intermediate state changes during session
  // loading wouldn't cause any updates.
  this->private_->loading_ = true;
  return true;
}

bool Viewer::post_load_states( const Core::StateIO& state_io )
{
  // Update all the slices
  if ( !this->is_volume_view() && this->private_->active_layer_slice_ )
  {
    const std::string& mode = this->view_mode_state_->get();
    Core::StateView2DHandle view2d_state = this->axial_view_state_;
    Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
    if ( mode == Viewer::CORONAL_C )
    {
      slice_type = Core::VolumeSliceType::CORONAL_E;
      view2d_state = this->coronal_view_state_;
    }
    else if ( mode == Viewer::SAGITTAL_C )
    {
      slice_type =  Core::VolumeSliceType::SAGITTAL_E;
      view2d_state = this->sagittal_view_state_;
    }
    double depth = view2d_state->get().center().z();

    ViewerPrivate::volume_slice_map_type::const_iterator it = this->private_->volume_slices_.begin();
    while ( it != this->private_->volume_slices_.end() )
    {
      Core::VolumeSliceHandle vol_slice = ( *it++ ).second;
      vol_slice->set_slice_type( slice_type );
      vol_slice->move_slice_to( depth, vol_slice == this->private_->active_layer_slice_ );
    }
    this->slice_number_state_->set_range( 0, static_cast< int >( 
      this->private_->active_layer_slice_->number_of_slices() ) - 1 );
    this->slice_number_state_->set( static_cast< int >( 
      this->private_->active_layer_slice_->get_slice_number() ) );
  }

  // Set the view flip states
  Core::StateView2DHandle view2d = boost::dynamic_pointer_cast< Core::StateView2D >( 
    this->get_active_view_state() );
  if ( view2d )
  {
    this->flip_horizontal_state_->set( view2d->get().scalex() < 0.0 );
    this->flip_vertical_state_->set( view2d->get().scaley() < 0.0 );
  }

  this->private_->loading_ = false;
  return true;
}

} // end namespace Seg3D
