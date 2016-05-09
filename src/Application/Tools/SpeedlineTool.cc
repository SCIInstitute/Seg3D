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
#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionClear.h>
#include <Core/State/Actions/ActionSetAt.h>
#include <Core/State/Actions/ActionRemove.h>
#include <Core/Interface/Interface.h>
#include <Core/Utils/AtomicCounter.h>

// Geometry includes
#include <Core/Geometry/Path.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/SpeedlineTool.h>
#include <Application/Tools/Actions/ActionSpeedline.h>
#include <Application/Tools/Actions/ActionPolyline.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

// Action associated with tool
#include <Application/Filters/Actions/ActionGradientMagnitudeFilter.h>

#include "Core/RenderResources/RenderResources.h"

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, SpeedlineTool )

namespace Seg3D
{

using namespace Core;

//////////////////////////////////////////////////////////////////////////
// Class SpeedlineToolPrivate
//////////////////////////////////////////////////////////////////////////

class SpeedlineToolPrivate
{
public:
  SpeedlineToolPrivate();
  ~SpeedlineToolPrivate();

  void handle_vertices_changed();
  void handle_path_changed();
  void handle_slice_changed();
  void handle_speed_params_changed();
  void handle_roi_mask_layer_changed( std::string layer_id );
  void handle_target_data_layer_changed( std::string layer_id );

  bool find_vertex( ViewerHandle viewer, int x, int y, int& index );
  bool find_closest_vertex( ViewerHandle viewer, int x, int y, int& index );
  void execute_fill_erase( ActionContextHandle context, bool erase, 
                           ViewerHandle viewer = ViewerHandle() );
  void execute_path();

  bool get_update_paths(); 
  void set_update_paths( bool update_all_paths ); 

  SpeedlineTool* tool_;
  bool moving_vertex_; //interface thread
  int vertex_index_;

  size_t slice_no_[ 6 ];
  bool initialized_;
  boost::signals2::connection viewer_connection_[ 6 ];

  // To ensure that the Speedline action performed in order
  //AtomicCounterHandle action_counter_;
};

SpeedlineToolPrivate::SpeedlineToolPrivate() :
  moving_vertex_(false),
  vertex_index_(-1),
  initialized_(false)
{
  //action_counter_ = AtomicCounterHandle( new AtomicCounter( 0 ) );
}

SpeedlineToolPrivate::~SpeedlineToolPrivate()
{
}

void SpeedlineToolPrivate::handle_vertices_changed()
{
  this->execute_path();
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void SpeedlineToolPrivate::handle_path_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void SpeedlineToolPrivate::handle_slice_changed()
{
  this->execute_path();
}

void SpeedlineToolPrivate::handle_speed_params_changed()
{
  this->execute_path();
}

void SpeedlineToolPrivate::handle_roi_mask_layer_changed( std::string layer_id )
{
  if ( layer_id != Tool::NONE_OPTION_C )
  {
    this->execute_path();
  }
}

void SpeedlineToolPrivate::handle_target_data_layer_changed( std::string layer_id )
{
  this->tool_->valid_target_data_layer_state_->set( layer_id != Tool::NONE_OPTION_C );
}

// TODO: copied code - move to external helper function
bool SpeedlineToolPrivate::find_vertex( ViewerHandle viewer, int x, int y, int& index )
{
  // Step 1. Compute the size of a pixel in world space
  double x0, y0, x1, y1;
  viewer->window_to_world( 0, 0, x0, y0 );
  viewer->window_to_world( 1, 1, x1, y1 );
  double pixel_width = Abs( x1 - x0 );
  double pixel_height = Abs( y1 - y0 );

  // Step 2. Compute the mouse position in world space
  double world_x, world_y;
  viewer->window_to_world( x, y, world_x, world_y );

  // Step 3. Search for the first vertex that's within 2 pixels of current mouse position
  double range_x = pixel_width * 4;
  double range_y = pixel_height * 4;

  std::vector< Point > vertices = this->tool_->vertices_state_->get();

  VolumeSliceType slice_type( VolumeSliceType::AXIAL_E );
  if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = VolumeSliceType::CORONAL_E;
  }
  else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
  {
    slice_type = VolumeSliceType::SAGITTAL_E;
  }
  
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    double pt_x, pt_y;
    VolumeSlice::ProjectOntoSlice( slice_type, vertices[ i ], pt_x, pt_y );
    if ( Abs( pt_x - world_x ) <= range_x &&
         Abs( pt_y - world_y ) <= range_y )
    {
      index = static_cast< int >( i );
      return true;
    }
  }

  index = -1;
  return false;
}

// TODO: copied code - move to external helper function
bool SpeedlineToolPrivate::find_closest_vertex( ViewerHandle viewer, int x, int y, int& index )
{
  // Step 1. Compute the mouse position in world space
  double world_x, world_y;
  viewer->window_to_world( x, y, world_x, world_y );
  
  // Step 2. Search for the closest vertex to the current mouse position
  std::vector< Point > vertices = this->tool_->vertices_state_->get();
  VolumeSliceType slice_type( VolumeSliceType::AXIAL_E );
  if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = VolumeSliceType::CORONAL_E;
  }
  else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
  {
    slice_type = VolumeSliceType::SAGITTAL_E;
  }
  
  int closest_index = -1;
  double min_dist;
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    double pt_x, pt_y;
    VolumeSlice::ProjectOntoSlice( slice_type, vertices[ i ], pt_x, pt_y );
    double dist_x = pt_x - world_x;
    double dist_y = pt_y - world_y;
    double distance = dist_x * dist_x + dist_y * dist_y;
    if ( i == 0 )
    {
      closest_index = 0;
      min_dist = distance;
    }
    else if ( distance < min_dist )
    {
      closest_index = static_cast< int >( i );
      min_dist = distance;
    }
  }
  
  index = closest_index;
  return index >= 0;
}

void SpeedlineToolPrivate::execute_fill_erase( Core::ActionContextHandle context,
                                               bool erase, ViewerHandle viewer )
{
  ASSERT_IS_INTERFACE_THREAD();
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  if ( ! this->tool_->valid_target_state_->get() ) return;

  // If no viewer specified, use the current active viewer
  if ( ! viewer )
  {
    int active_viewer = ViewerManager::Instance()->active_viewer_state_->get();
    if ( active_viewer < 0 ) return;

    viewer = ViewerManager::Instance()->get_viewer( static_cast< size_t >( active_viewer ) );
  }

  if ( ! viewer || viewer->is_volume_view() ) return;

  Core::MaskVolumeSliceHandle volume_slice = boost::dynamic_pointer_cast< Core::MaskVolumeSlice >(
    viewer->get_volume_slice( this->tool_->target_layer_state_->get() ) );
  if ( ! volume_slice ) return;

  // Target mask being filled must be visible.
  LayerHandle target_mask_layer = LayerManager::Instance()->find_layer_by_id( this->tool_->target_layer_state_->get() );

  if ( ! target_mask_layer->is_visible( viewer->get_viewer_id() )  ) return;

  if ( target_mask_layer->locked_state_->get() ) return;

  const std::vector< Core::Point >& vertices = this->tool_->vertices_state_->get();
  //const std::vector< Core::Point >& vertices = this->tool_->path_vertices_state_->get();
  size_t num_of_vertices = vertices.size();

  if ( num_of_vertices < 3 ) return;

  Path paths = this->tool_->path_state_->get();
  std::vector< SinglePath > all_paths = paths.get_all_paths();
  std::vector< Point > full_path;

  for ( auto &single_path : all_paths )
  {
    size_t num_of_points_on_single_path = single_path.get_points_num_on_path();
    for ( int j = num_of_points_on_single_path - 1; j >= 0; --j )
    {
      full_path.push_back( single_path.get_a_point( j ) );
    }
  }

  size_t num_of_points_on_paths = full_path.size();

  double world_x, world_y;
  int x, y;
  std::vector< ActionPolyline::VertexCoord > vertices_2d;
  for ( size_t i = 0; i < num_of_points_on_paths; ++i )
  {
    volume_slice->project_onto_slice( full_path[ i ], world_x, world_y );
    volume_slice->world_to_index( world_x, world_y, x, y );
    vertices_2d.push_back( ActionPolyline::VertexCoord( static_cast< float >( x ), static_cast< float >( y ), 0 ) );
  }

  ActionPolyline::Dispatch( context, this->tool_->target_layer_state_->get(),
                            volume_slice->get_slice_type(), volume_slice->get_slice_number(), erase, vertices_2d );
}

void SpeedlineToolPrivate::execute_path()
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );

  ActionContextHandle context = Interface::GetMouseActionContext();
  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();

  if ( ! this->tool_->valid_target_data_layer_state_->get() ) return;

  // If no viewer specified, use the current active viewer
  if ( ! viewer )
  {
    int active_viewer = ViewerManager::Instance()->active_viewer_state_->get();
    if ( active_viewer < 0 ) return;

    viewer = ViewerManager::Instance()->get_viewer( static_cast< size_t >( active_viewer ) );
  }

  if ( ! viewer || viewer->is_volume_view() ) return;

  DataVolumeSliceHandle volume_slice = boost::dynamic_pointer_cast< DataVolumeSlice >(
    viewer->get_volume_slice( this->tool_->target_data_layer_state_->get() ) );

  if ( ! volume_slice ) return;

  const std::vector< Point > vertices = this->tool_->vertices_state_->get();
  if (vertices.size() < 2) return;

  ActionSpeedline::Dispatch( context,
                             this->tool_->target_data_layer_state_->get(),
                             this->tool_->mask_state_->get(),
                             this->tool_->roi_mask_state_->get(),
                             volume_slice->get_slice_type(),
                             volume_slice->get_slice_number(),
                             vertices,
                             this->tool_->grad_mag_weight_state_->get(),
                             this->tool_->zero_cross_weight_state_->get(),
                             this->tool_->grad_dir_weight_state_->get(),
                             this->tool_->use_image_spacing_state_->get(),
                             this->tool_->use_face_conn_state_->get(),
                             this->tool_->path_state_->get_stateid()
                           );
}

//////////////////////////////////////////////////////////////////////////
// Class SpeedlineTool
//////////////////////////////////////////////////////////////////////////

SpeedlineTool::SpeedlineTool( const std::string& toolid ) :
  SingleTargetTool( VolumeType::MASK_E, toolid ),
  private_( new SpeedlineToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->moving_vertex_ = false;

  for ( size_t i = 0; i < 6; ++i )
    this->private_->slice_no_[ i ] = 0;

  this->add_state( "vertices", this->vertices_state_ );
  this->vertices_state_->set_session_priority( StateBase::DEFAULT_LOAD_E + 40 );
  this->add_connection( this->vertices_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_vertices_changed, this->private_ ) ) );

  // defaults from livewire tool test case
  this->add_state( "grad_mag_weight", this->grad_mag_weight_state_, 0.43, 0.0, 10.0, 0.01 );
  this->add_connection( this->grad_mag_weight_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_speed_params_changed, this->private_ ) ) );
  this->add_state( "zero_cross_weight", this->zero_cross_weight_state_, 0.43, 0.0, 10.0, 0.01 );
  this->add_connection( this->zero_cross_weight_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_speed_params_changed, this->private_ ) ) );
  this->add_state( "grad_dir_weight", this->grad_dir_weight_state_, 0.14, 0.0, 2.0, 0.01 );
  this->add_connection( this->grad_dir_weight_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_speed_params_changed, this->private_ ) ) );

  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  this->add_state( "mask", this->mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->mask_state_->set_session_priority( StateBase::DEFAULT_LOAD_E + 50 );
  this->add_extra_layer_input( this->mask_state_, VolumeType::MASK_E, false, false );

  this->add_state( "roi_mask", this->roi_mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->roi_mask_state_->set_session_priority( StateBase::DEFAULT_LOAD_E + 30 );
  this->add_extra_layer_input( this->roi_mask_state_, VolumeType::MASK_E, false, false );

  // When speed image changes, recompute speedline.
  this->add_connection( this->roi_mask_state_->value_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_roi_mask_layer_changed, this->private_, _2 ) ) );

  this->add_state( "data_layer", this->target_data_layer_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_extra_layer_input( this->target_data_layer_state_, VolumeType::DATA_E, false, false );

  this->add_state( "valid_target_data_layer", this->valid_target_data_layer_state_, false );
  this->add_connection( this->target_data_layer_state_->value_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_target_data_layer_changed, this->private_, _2 ) ) );

  this->add_state( "path", this->path_state_, Path() );
  this->add_connection( this->path_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_path_changed, this->private_ ) ) );

  this->add_state( "use_face_conn", this->use_face_conn_state_, true );
  this->add_connection( this->use_face_conn_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_speed_params_changed, this->private_ ) ) );
  this->add_state( "use_image_spacing", this->use_image_spacing_state_, true );
  this->add_connection( this->use_image_spacing_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_speed_params_changed, this->private_ ) ) );
}

SpeedlineTool::~SpeedlineTool()
{
  for ( size_t i = 0; i < 6; ++i )
  {
    this->private_->viewer_connection_[ i ].disconnect();
  }

  this->disconnect_all();
}

// ACTIVATE:
// Activate a tool: this tool is set as the active tool and hence it should
// setup the right mouse tools in the viewers.
void SpeedlineTool::activate()
{
  bool is_recompute = false;

  for ( size_t i = 0; i < 6; ++i )
  {
    this->private_->viewer_connection_[ i ] = 
      ViewerManager::Instance()->get_viewer( i )->slice_number_state_->value_changed_signal_.connect(
        boost::bind( &SpeedlineToolPrivate::handle_slice_changed, this->private_ ) );
  }

  if ( this->valid_target_data_layer_state_->get() )
  {
    for ( size_t i = 0; i < 6; ++i )
    {
      DataVolumeSliceHandle volume_slice = boost::dynamic_pointer_cast
        < DataVolumeSlice >( ViewerManager::Instance()->get_viewer( i )->get_volume_slice( 
        this->target_data_layer_state_->get() ) );
      
      if ( volume_slice->get_slice_number() != this->private_->slice_no_[ i ]  )
      {
        is_recompute = true;
        break;
      }
    }
  }

  if ( is_recompute && this->private_->initialized_ )
  {
    this->private_->execute_path( /*true*/ );
  }

  this->private_->initialized_ = true;
}

// DEACTIVATE:
// Deactivate a tool. A tool is always deactivate before the next one is activated.
void SpeedlineTool::deactivate()
{
  for ( size_t i = 0; i < 6; ++i )
  {
    this->private_->viewer_connection_[ i ].disconnect();
  }

//  if ( this->valid_gradient_state_->get() )
//  {
//    std::vector< int > slice_no_arr;
//    for ( size_t i = 0; i < 6; ++i )
//    {
//      DataVolumeSliceHandle volume_slice = boost::dynamic_pointer_cast
//        < DataVolumeSlice >( ViewerManager::Instance()->get_viewer( i )->get_volume_slice( 
//        this->gradient_state_->get() ) );
//
//      if ( volume_slice != NULL )
//      {
//        this->private_->slice_no_[ i ]  = volume_slice->get_slice_number();
//      } 
//    }
//  }
}

bool SpeedlineTool::post_load_states( const StateIO& state_io )
{
//  this->private_->update_all_paths_ = false;
  return true;
}

void SpeedlineTool::fill( ActionContextHandle context )
{
  this->private_->execute_fill_erase( context, false );
}

void SpeedlineTool::erase( ActionContextHandle context )
{
  this->private_->execute_fill_erase( context, true );
}

bool SpeedlineTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  switch ( key )
  {
    case Key::KEY_F_E:
    {
      this->private_->execute_fill_erase( Interface::GetKeyboardActionContext(), false, viewer );
      return true;
    }
    case Key::KEY_E_E:
    {
      this->private_->execute_fill_erase( Interface::GetKeyboardActionContext(), true, viewer );
      return true;
    }
    case Key::KEY_C_E:
    {
      this->reset( Interface::GetKeyboardActionContext() );
      return true;
    }
  }
  return false;
}

void SpeedlineTool::reset( ActionContextHandle context )
{
  ActionClear::Dispatch( context, this->vertices_state_ );
  Application::PostEvent( boost::bind( &StateSpeedlinePath::set,
    this->path_state_, Path(), ActionSource::NONE_E ) );
}

void SpeedlineTool::reset_parameters( ActionContextHandle context )
{
  Application::PostEvent( boost::bind( &StateRangedDouble::set,
    this->grad_mag_weight_state_, 0.43, ActionSource::NONE_E ) );
  Application::PostEvent( boost::bind( &StateRangedDouble::set,
    this->zero_cross_weight_state_, 0.43, ActionSource::NONE_E ) );
  Application::PostEvent( boost::bind( &StateRangedDouble::set,
    this->grad_dir_weight_state_, 0.14, ActionSource::NONE_E ) );
  Application::PostEvent( boost::bind( &StateBool::set,
    this->use_face_conn_state_, true, ActionSource::NONE_E ) );
  Application::PostEvent( boost::bind( &StateBool::set,
    this->use_image_spacing_state_, true, ActionSource::NONE_E ) );
}

bool SpeedlineTool::handle_mouse_press( ViewerHandle viewer,
                                       const Core::MouseHistory& mouse_history,
                                       int button, int buttons, int modifiers )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() ) return false;

  if ( ! this->valid_target_data_layer_state_->get() ) return false;

  if ( button == Core::MouseButton::LEFT_BUTTON_E &&
       ( modifiers == Core::KeyModifier::NO_MODIFIER_E ||
         modifiers == Core::KeyModifier::SHIFT_MODIFIER_E ) &&
      this->private_->vertex_index_ != -1 )
  {
    this->private_->moving_vertex_ = true;
    viewer->set_cursor( Core::CursorShape::CLOSED_HAND_E );
    return true;
  }
  else if ( button == Core::MouseButton::MID_BUTTON_E &&
            ( modifiers == Core::KeyModifier::NO_MODIFIER_E ||
              modifiers == Core::KeyModifier::SHIFT_MODIFIER_E ) )
  {
    if ( this->private_->find_closest_vertex( viewer, mouse_history.current_.x_, mouse_history.current_.y_, this->private_->vertex_index_ ) )
    {
      this->private_->moving_vertex_ = true;
      viewer->set_cursor( Core::CursorShape::CLOSED_HAND_E );
      return true;
    }
  }
  else if ( ! ( modifiers & Core::KeyModifier::SHIFT_MODIFIER_E ) && button == Core::MouseButton::LEFT_BUTTON_E )
  {
    Core::VolumeSliceHandle active_slice = viewer->get_active_volume_slice();
    if ( active_slice && ! active_slice->out_of_boundary() )
    {
      double world_x, world_y;
      viewer->window_to_world( mouse_history.current_.x_,
                               mouse_history.current_.y_, world_x, world_y );
      Core::Point pt;
      active_slice->get_world_coord( world_x, world_y,  pt );

      double dmin = DBL_MAX;
      double proj_min = DBL_MAX;
      std::vector<Core::Point> points = this->vertices_state_->get();

      size_t idx = 0;
      for ( size_t j = 0; j < points.size(); j++ )
      {
        size_t k = j + 1;
        if ( k ==  points.size() ) k = 0;

        Core::Vector edge_dir = points[ j ] - points[ k ];
        double edge_length = edge_dir.normalize();
        double alpha = Dot( points[ j ] - pt, points[ j ] - points[ k ] ) / ( edge_length * edge_length );

        double dist = 0.0;
        double proj_len = 0.0;
        if ( alpha < 0.0 )
        {
          Core::Vector dir = points[ j ] - pt;
          dist = dir.length2();
          proj_len = Core::Abs( Dot( edge_dir, dir ) );
        }
        else if ( alpha > 1.0 )
        {
          Core::Vector dir = points[ k ] - pt;
          dist = dir.length2();
          proj_len = Core::Abs( Dot( edge_dir, dir ) );
        }
        else
        {
          dist = ( ( points[ j ] - pt ) - alpha * ( points[ j ] - points[ k ] ) ).length2();
        }

        if ( dist < dmin || ( dist == dmin && proj_len < proj_min ) )
        {
          dmin = dist;
          proj_min = proj_len;
          idx = k;
        }
      }
      points.insert( points.begin() + idx, pt );

      Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(),
                                 this->vertices_state_, points );

      // Set to "hovered over" state since the mouse is hovering over the new point
      viewer->set_cursor( Core::CursorShape::OPEN_HAND_E );
      this->private_->vertex_index_ = static_cast< int >( idx );

      return true;
    }
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
            button == Core::MouseButton::LEFT_BUTTON_E )
  {
    Core::VolumeSliceHandle active_slice = viewer->get_active_volume_slice();
    if ( active_slice && ! active_slice->out_of_boundary() )
    {
      double world_x, world_y;
      viewer->window_to_world( mouse_history.current_.x_,
                               mouse_history.current_.y_,
                               world_x, world_y );
      Core::Point pt;
      active_slice->get_world_coord( world_x, world_y, pt );
      Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(),
                                 this->vertices_state_, pt );
      return true;
    }
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
            button == Core::MouseButton::RIGHT_BUTTON_E )
  {
    if ( this->private_->vertex_index_ != -1 )
    {
      Core::Point pt = this->vertices_state_->get()[ this->private_->vertex_index_ ];
      Core::ActionRemove::Dispatch( Core::Interface::GetMouseActionContext(),
                                    this->vertices_state_, pt );

      // Set to "not hovered over" state since the point no longer exists
      viewer->set_cursor( Core::CursorShape::CROSS_E );
      this->private_->vertex_index_ = -1;

      return true;
    }
  }

  return false;
}

bool SpeedlineTool::handle_mouse_release( ViewerHandle viewer,
                                          const Core::MouseHistory& mouse_history,
                                          int button, int buttons, int modifiers )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() ) return false;

  if ( ! this->valid_target_data_layer_state_->get() ) return false;

  if ( this->private_->moving_vertex_ &&
       ( button == Core::MouseButton::LEFT_BUTTON_E ||
         button == Core::MouseButton::MID_BUTTON_E ) )
  {
    this->private_->moving_vertex_ = false;
    this->private_->find_vertex( viewer,
                                 mouse_history.current_.x_,
                                 mouse_history.current_.y_,
                                 this->private_->vertex_index_ );
    viewer->set_cursor( this->private_->vertex_index_ != -1 ?
                        Core::CursorShape::OPEN_HAND_E : Core::CursorShape::CROSS_E );
    return true;
  }

  return false;
}

bool SpeedlineTool::handle_mouse_move( ViewerHandle viewer,
                                       const Core::MouseHistory& mouse_history,
                                       int button, int buttons, int modifiers )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() ) return false;

  if ( ! this->valid_target_data_layer_state_->get() ) return false;

  if ( buttons == Core::MouseButton::NO_BUTTON_E )
  {
    this->private_->find_vertex( viewer, mouse_history.current_.x_,
                                mouse_history.current_.y_, this->private_->vertex_index_ );
    viewer->set_cursor( this->private_->vertex_index_ != -1 ?
                       Core::CursorShape::OPEN_HAND_E : Core::CursorShape::CROSS_E );
  }

  if ( this->private_->moving_vertex_ )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::vector< Core::Point > vertices = this->vertices_state_->get();
    std::string view_mode = viewer->view_mode_state_->get();
    double offset_x, offset_y, world_x0, world_y0, world_x1, world_y1;
    viewer->window_to_world( mouse_history.previous_.x_,
                            mouse_history.previous_.y_, world_x0, world_y0 );
    viewer->window_to_world( mouse_history.current_.x_,
                            mouse_history.current_.y_, world_x1, world_y1 );
    offset_x = world_x1 - world_x0;
    offset_y = world_y1 - world_y0;
    lock.unlock();

    Core::Point pt_offset( 0.0, 0.0, 0.0 );
    if ( view_mode == Viewer::AXIAL_C )
    {
      pt_offset[ 0 ] = offset_x;
      pt_offset[ 1 ] = offset_y;
    }
    else if ( view_mode == Viewer::CORONAL_C )
    {
      pt_offset[ 0 ] = offset_x;
      pt_offset[ 2 ] = offset_y;
    }
    else if ( view_mode == Viewer::SAGITTAL_C )
    {
      pt_offset[ 1 ] = offset_x;
      pt_offset[ 2 ] = offset_y;
    }
    else
    {
      this->private_->moving_vertex_ = false;
      return false;
    }
    
    if ( modifiers == Core::KeyModifier::SHIFT_MODIFIER_E )
    {
      for ( size_t i = 0; i < vertices.size(); ++i )
      {
        vertices[ i ] += pt_offset;
      }
      Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(),
                                this->vertices_state_, vertices );
    }
    else if ( this->private_->vertex_index_ >= 0 )
    {
      Core::Point pt = vertices[ this->private_->vertex_index_ ];
      pt += pt_offset;
      Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(),
                                  this->vertices_state_, this->private_->vertex_index_, pt );
    }
    else 
    {
      this->private_->moving_vertex_ = false;
      return false;
    }
    
    return true;
  }
  
  return false;
}

void SpeedlineTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
                            int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( viewer->is_volume_view() ) return;

  std::vector< Core::Point > vertices;
  Core::Path paths;

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    vertices = this->vertices_state_->get();
    paths = this->path_state_->get();

    if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      slice_type = Core::VolumeSliceType::SAGITTAL_E;
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      slice_type = Core::VolumeSliceType::CORONAL_E;
    }
  }

  size_t vertices_num = vertices.size();
  size_t paths_num = paths.get_path_num();
  Core::Point start_p = paths.get_start_point();
  Core::Point end_p = paths.get_end_point();

  if ( vertices_num == 0 )  // no point
  {
    return;
  }

//  CORE_CHECK_OPENGL_ERROR();
  glPushAttrib( GL_LINE_BIT | GL_POINT_BIT | GL_TRANSFORM_BIT );
//  CORE_CHECK_OPENGL_ERROR();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
//  CORE_CHECK_OPENGL_ERROR();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glPointSize( 5.0f );
  glLineWidth( 1.5f );
  glColor3f( 0.0f, 1.0f, 1.0f );
  glEnable( GL_LINE_SMOOTH );

  if ( start_p == end_p && paths_num == 0 ) // only one point
  {
    glColor3f( 0.0f, 1.0f, 1.0f ); // cyan
//    glBegin( GL_POINTS );
//    {
//      double x_pos, y_pos;
//      Core::VolumeSlice::ProjectOntoSlice( slice_type, start_p, x_pos, y_pos );
//      glVertex2d( x_pos, y_pos );
//    }
//    glEnd();

    glBegin( GL_POINTS );
    for ( size_t i = 0; i < vertices.size(); ++i )
    {
      double x_pos, y_pos;
      Core::VolumeSlice::ProjectOntoSlice( slice_type, vertices[ i ], x_pos, y_pos );
      vertices[ i ][ 0 ] = x_pos;
      vertices[ i ][ 1 ] = y_pos;
      glVertex2d( x_pos, y_pos );
    }
    glEnd();
  }
  else if ( paths_num > 0 )
  {
    for ( unsigned int i = 0; i < paths_num; ++i )
    {
      Core::SinglePath single_path = paths.get_one_path( i );
      Core::Point p0, p1;
      single_path.get_point_on_ends( 0, p0 );
      single_path.get_point_on_ends( 1, p1 );

      glColor3f( 1.0f, 0.5f, 0.0f );    // orange
      glBegin( GL_LINE_STRIP );
      for ( unsigned int j = 0; j < single_path.get_points_num_on_path(); ++j )
      {
        double x_pos, y_pos;
        Core::VolumeSlice::ProjectOntoSlice( slice_type, single_path.get_a_point( j ), x_pos, y_pos );
        glVertex2d( x_pos, y_pos );
      }
      glEnd();

      glColor3f( 0.0f, 1.0f, 1.0f ); // cyan
      glBegin( GL_POINTS );
      {
        double x_pos, y_pos;
        Core::VolumeSlice::ProjectOntoSlice( slice_type, p0, x_pos, y_pos );
        glVertex2d( x_pos, y_pos );
        
        Core::VolumeSlice::ProjectOntoSlice( slice_type, p1, x_pos, y_pos );
        glVertex2d( x_pos, y_pos );
        
      }
      glEnd();
    }
  }
  glPopMatrix();
  glPopAttrib();
}

bool SpeedlineTool::has_2d_visual()
{
  return true;
}


} // end namespace Seg3D
