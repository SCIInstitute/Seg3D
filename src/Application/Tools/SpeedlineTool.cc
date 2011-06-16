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
#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionClear.h>
#include <Core/State/Actions/ActionSetAt.h>
#include <Core/State/Actions/ActionRemove.h>

// Geometry includes
#include <Core/Geometry/Path.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/SpeedlineTool.h>
#include <Application/Tools/Actions/ActionSpeedline.h>
#include <Application/Tools/Actions/ActionPolyline.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

// Action associated with tool
#include <Application/Filters/Actions/ActionGradientMagnitudeFilter.h>
#include "Core/RenderResources/RenderResources.h"

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, SpeedlineTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class SpeedlineToolPrivate
//////////////////////////////////////////////////////////////////////////

class SpeedlineToolPrivate
{
public:
  void handle_vertices_changed();
  void handle_path_changed();
  bool find_vertex( ViewerHandle viewer, int x, int y, int& index );
  bool find_closest_vertex( ViewerHandle viewer, int x, int y, int& index );
  void execute( Core::ActionContextHandle context, bool erase, 
    ViewerHandle viewer = ViewerHandle() );
  void execute_path( bool update_all_paths );

  SpeedlineTool* tool_;
  bool moving_vertex_; //interface thread
  int vertex_index_;

};

void SpeedlineToolPrivate::handle_vertices_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void SpeedlineToolPrivate::handle_path_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

bool SpeedlineToolPrivate::find_vertex( ViewerHandle viewer, int x, int y, int& index )
{
  // Step 1. Compute the size of a pixel in world space
  double x0, y0, x1, y1;
  viewer->window_to_world( 0, 0, x0, y0 );
  viewer->window_to_world( 1, 1, x1, y1 );
  double pixel_width = Core::Abs( x1 - x0 );
  double pixel_height = Core::Abs( y1 - y0 );

  // Step 2. Compute the mouse position in world space
  double world_x, world_y;
  viewer->window_to_world( x, y, world_x, world_y );

  // Step 3. Search for the first vertex that's within 2 pixels of current mouse position
  double range_x = pixel_width * 4;
  double range_y = pixel_height * 4;

  std::vector< Core::Point > vertices = this->tool_->vertices_state_->get();
  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
  {
    slice_type = Core::VolumeSliceType::SAGITTAL_E;
  }
  
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    double pt_x, pt_y;
    Core::VolumeSlice::ProjectOntoSlice( slice_type, vertices[ i ], pt_x, pt_y );
    if ( Core::Abs( pt_x - world_x ) <= range_x &&
      Core::Abs( pt_y - world_y ) <= range_y )
    {
      index = static_cast< int >( i );
      return true;
    }
  }

  index = -1;
  return false;
}

bool SpeedlineToolPrivate::find_closest_vertex( ViewerHandle viewer, int x, int y, int& index )
{
  // Step 1. Compute the mouse position in world space
  double world_x, world_y;
  viewer->window_to_world( x, y, world_x, world_y );
  
  // Step 2. Search for the closest vertex to the current mouse position
  std::vector< Core::Point > vertices = this->tool_->vertices_state_->get();
  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
  {
    slice_type = Core::VolumeSliceType::SAGITTAL_E;
  }
  
  int closest_index = -1;
  double min_dist;
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    double pt_x, pt_y;
    Core::VolumeSlice::ProjectOntoSlice( slice_type, vertices[ i ], pt_x, pt_y );
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
  
void SpeedlineToolPrivate::execute( Core::ActionContextHandle context, 
                  bool erase, ViewerHandle viewer )
{
  ASSERT_IS_INTERFACE_THREAD();
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  if ( !this->tool_->valid_target_mask_state_->get() )
  {
    return;
  }
  
  // If no viewer specified, use the current active viewer
  if ( !viewer )
  {
    int active_viewer = ViewerManager::Instance()->active_viewer_state_->get();
    if ( active_viewer < 0 )
    {
      return;
    }
    viewer = ViewerManager::Instance()->get_viewer( static_cast< size_t >( active_viewer ) );
  }
  
  if ( !viewer || viewer->is_volume_view() )
  {
    return;
  }
  
  Core::MaskVolumeSliceHandle volume_slice = boost::dynamic_pointer_cast
    < Core::MaskVolumeSlice >( viewer->get_volume_slice( 
    this->tool_->mask_state_->get() ) );
  if ( !volume_slice )
  {
    return;
  }


  // Target mask being filled must be visible.
  LayerHandle target_mask_layer = LayerManager::Instance()->get_layer_by_id( 
    this->tool_->mask_state_->get() );


  if ( !target_mask_layer->is_visible( viewer->get_viewer_id() )  )
  {
    return;
  }
  
  if ( target_mask_layer->locked_state_->get() ) 
  {
    return;
  }

  const std::vector< Core::Point >& vertices = this->tool_->vertices_state_->get();
  size_t num_of_vertices = vertices.size();

  if ( num_of_vertices < 2 )
  {
    return;
  }

  Core::Path paths;
  std::vector< Core::Point > full_path;

  paths = this->tool_->path_state_->get();

  for ( unsigned int i = 0; i < paths.get_path_num(); ++i )
  {
    Core::SinglePath single_path = paths.get_one_path( i );
    for ( unsigned int j = 0; j < single_path.get_points_num_on_path(); ++j )
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
    vertices_2d.push_back( ActionPolyline::VertexCoord( 
      static_cast< float >( x ), static_cast< float >( y ), 0 ) );
  }
  
  ActionPolyline::Dispatch( context, this->tool_->mask_state_->get(),
    volume_slice->get_slice_type(), volume_slice->get_slice_number(), erase, vertices_2d );
}

void SpeedlineToolPrivate::execute_path( bool update_all_paths )
{
  Core::ActionContextHandle context = Core::Interface::GetMouseActionContext();
  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  //Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  if ( !this->tool_->valid_target_state_->get() || !this->tool_->valid_gradient_state_->get() )
  {
    return;
  }

  // If no viewer specified, use the current active viewer
  if ( !viewer )
  {
    int active_viewer = ViewerManager::Instance()->active_viewer_state_->get();
    if ( active_viewer < 0 )
    {
      return;
    }
    viewer = ViewerManager::Instance()->get_viewer( static_cast< size_t >( active_viewer ) );
  }

  if ( !viewer || viewer->is_volume_view() )
  {
    return;
  }

  Core::DataVolumeSliceHandle volume_slice = boost::dynamic_pointer_cast
    < Core::DataVolumeSlice >( viewer->get_volume_slice( 
    this->tool_->target_layer_state_->get() ) );
  if ( !volume_slice )
  {
    return;
  }

  // Either gradient or target mask must be visible. 
  // Both gradient and target mask must not be locked.
  LayerHandle target_layer = LayerManager::Instance()->get_layer_by_id( 
    this->tool_->target_layer_state_->get() );

  LayerHandle gradient_layer = LayerManager::Instance()->get_layer_by_id( 
    this->tool_->gradient_state_->get() );

  if ( !target_layer->is_visible( viewer->get_viewer_id() ) &&
    !gradient_layer->is_visible( viewer->get_viewer_id() ) ) 
  {
    return;
  }

  if ( target_layer->locked_state_->get() || gradient_layer->locked_state_->get() ) 
  {
    return;
  }

  const std::vector< Core::Point >& vertices = this->tool_->vertices_state_->get();
  size_t num_of_vertices = vertices.size();

  if ( num_of_vertices < 1 )
  {
    return;
  }

  ActionSpeedline::Dispatch( context, this->tool_->gradient_state_->get(),
    volume_slice->get_slice_type(), volume_slice->get_slice_number(), vertices, 
    this->tool_->current_vertex_index_state_->get(),
    this->tool_->itk_path_state_->get(),
    this->tool_->iterations_state_->get(),
    this->tool_->termination_state_->get(),
    update_all_paths,
    this->tool_->toolid() );
}

//////////////////////////////////////////////////////////////////////////
// Class SpeedlineTool
//////////////////////////////////////////////////////////////////////////

void SpeedlineTool::execute_gradient( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionGradientMagnitudeFilter::Dispatch( context,
    this->target_layer_state_->get(),
    false,
    false
    );    
}

SpeedlineTool::SpeedlineTool( const std::string& toolid ) 
:
SingleTargetTool( Core::VolumeType::DATA_E, toolid ),
private_( new SpeedlineToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->moving_vertex_ = false;

  this->add_state( "vertices", this->vertices_state_ );

  //this->add_connection( this->vertices_state_->state_changed_signal_.connect(
  //  boost::bind( &SpeedlineToolPrivate::handle_vertices_changed, this->private_ ) ) );

  this->add_connection( this->vertices_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::execute_path, this->private_, false ) ) );

  this->add_state( "termination_state", this->termination_state_, 1.0, 0.0, 2.0, 0.01 );
  this->add_state( "iterations", this->iterations_state_, 1000, 1, 2000, 1 );

  std::vector< LayerIDNamePair > empty_list( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  // Whether we use a mask to find which components to use
  this->add_state( "gradient", this->gradient_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_dependent_layer_input( this->gradient_state_, Core::VolumeType::DATA_E, false );
  this->add_state( "valid_gradient_layer", this->valid_gradient_state_, false );
  this->add_connection( this->gradient_state_->value_changed_signal_.connect(
    boost::bind( &SpeedlineTool::handle_gradient_layer_changed, this, _2 ) ) );

  this->add_state( "mask", this->mask_state_, Tool::NONE_OPTION_C, empty_list );
  this->add_dependent_layer_input( this->mask_state_, Core::VolumeType::MASK_E, false );
  this->add_state( "valid_mask_layer", this->valid_target_mask_state_, false );
  this->add_connection( this->mask_state_->value_changed_signal_.connect(
    boost::bind( &SpeedlineTool::handle_target_mask_layer_changed, this, _2 ) ) );

  this->add_state( "itk_path", this->itk_path_state_, Core::Path() );
  this->add_state( "path", this->path_state_,  Core::Path() );
  this->add_connection( this->path_state_->state_changed_signal_.connect(
    boost::bind( &SpeedlineToolPrivate::handle_path_changed, this->private_ ) ) );

  this->add_state( "current_vertex_index", this->current_vertex_index_state_, -1 );

  // activate and deactivate method overload, application thread
  for ( size_t i = 0; i < 6; ++i )
  {
    this->add_connection( 
      ViewerManager::Instance()->get_viewer( i )->slice_number_state_->value_changed_signal_.connect(
      boost::bind( &SpeedlineTool::handle_slice_changed, this ) )
    );
  }
}

SpeedlineTool::~SpeedlineTool()
{
  this->disconnect_all();
}

void SpeedlineTool::handle_gradient_layer_changed( std::string layer_id )
{
  this->valid_gradient_state_->set( layer_id != Tool::NONE_OPTION_C );
}

void SpeedlineTool::handle_target_mask_layer_changed( std::string layer_id )
{
  this->valid_target_mask_state_->set( layer_id != Tool::NONE_OPTION_C );
}

void SpeedlineTool::fill( Core::ActionContextHandle context )
{
  this->private_->execute( context, false );
}

void SpeedlineTool::erase( Core::ActionContextHandle context )
{
  this->private_->execute( context, true );
}

bool SpeedlineTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  switch ( key )
  {
    case Core::Key::KEY_F_E:
    {
      this->private_->execute( Core::Interface::GetKeyboardActionContext(), false, viewer );
      return true;
    }
    case Core::Key::KEY_E_E:
    {
      this->private_->execute( Core::Interface::GetKeyboardActionContext(), true, viewer );
      return true;
    }
  }
  return false;
}

void SpeedlineTool::reset( Core::ActionContextHandle context )
{
  Core::ActionClear::Dispatch( context, this->vertices_state_ );
  Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
    this->path_state_, Core::Path(), Core::ActionSource::NONE_E ) );
  Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
    this->itk_path_state_, Core::Path(), Core::ActionSource::NONE_E ) );
}

bool SpeedlineTool::handle_mouse_press( ViewerHandle viewer, 
                    const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() )
  {
    return false;
  }

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
    if ( this->private_->find_closest_vertex( viewer, mouse_history.current_.x_, 
      mouse_history.current_.y_, this->private_->vertex_index_ ) )
    {
      this->private_->moving_vertex_ = true;
      viewer->set_cursor( Core::CursorShape::CLOSED_HAND_E );
      return true;      
    }
  }
  else if ( !( modifiers & Core::KeyModifier::SHIFT_MODIFIER_E ) &&
    button == Core::MouseButton::LEFT_BUTTON_E )
  {
    Core::VolumeSliceHandle active_slice = viewer->get_active_volume_slice();
    if ( active_slice && !active_slice->out_of_boundary() )
    {
      double world_x, world_y;
      viewer->window_to_world( mouse_history.current_.x_, 
        mouse_history.current_.y_, world_x, world_y );
      Core::Point pt;
      active_slice->get_world_coord( world_x, world_y,  pt );
      //
      //double dmin = DBL_MAX;
      //double proj_min = DBL_MAX;
      std::vector<Core::Point> points = this->vertices_state_->get();
      //
      size_t idx = 0;
      //for ( size_t j = 0; j < points.size(); j++ )
      //{
      //  size_t k = j + 1;
      //  if ( k ==  points.size() ) k = 0;
      //  
      //  Core::Vector edge_dir = points[ j ] - points[ k ];
      //  double edge_length = edge_dir.normalize();
      //  double alpha = Dot( points[ j ] - pt, points[ j ] - points[ k ] )/
      //    ( edge_length * edge_length );
      //    
      //  double dist = 0.0;
      //  double proj_len = 0.0;
      //  if ( alpha < 0.0 ) 
      //  {
      //    Core::Vector dir = points[ j ] - pt;
      //    dist = dir.length2();
      //    proj_len = Core::Abs( Dot( edge_dir, dir ) );
      //  }
      //  else if ( alpha > 1.0 )
      //  {
      //    Core::Vector dir = points[ k ] - pt;
      //    dist = dir.length2();
      //    proj_len = Core::Abs( Dot( edge_dir, dir ) );
      //  }
      //  else 
      //  {
      //    dist = ( ( points[ j ] - pt ) - alpha * ( points[ j ] - points[ k ] ) ).length2();
      //  }
      //  
      //  if ( dist < dmin ||
      //    ( dist == dmin && proj_len < proj_min ) )
      //  {
      //    dmin = dist;
      //    proj_min = proj_len;
      //    idx = k;
      //  }
      //}
      //points.insert( points.begin() + idx, pt );

      points.push_back( pt );
      idx = points.size() - 1;
      
      //only update the current path, not every one
      this->private_->vertex_index_ = static_cast< int >( idx );

      Core::Application::PostEvent( boost::bind( &Core::StateInt::set,
        this->current_vertex_index_state_, static_cast< int >(idx), Core::ActionSource::NONE_E ) );
      Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(),
        this->vertices_state_, points );

      viewer->set_cursor( Core::CursorShape::OPEN_HAND_E );

      return true;
    }
  }

  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    button == Core::MouseButton::RIGHT_BUTTON_E )
  {
    if ( this->private_->vertex_index_ != -1 )
    {
      Core::Point pt = this->vertices_state_->get()[ this->private_->vertex_index_ ];



      //if ( this->private_->vertex_index_ == 0 || 
      //    ( this->private_->vertex_index_ == this->vertices_state_->get().size() - 1 ) )
      //{
      //  this->private_->vertex_index_ = 0;
      //} 
      //else 
      //{

      //  this->private_->vertex_index_ = this->private_->vertex_index_ - 1;
      //}

      // only one point left
      if ( this->private_->vertex_index_ == 0 &&  ( this->vertices_state_->get().size() == 1 ) )
      {
        this->private_->vertex_index_ = -1;
      }
      else if ( this->private_->vertex_index_ == 0 &&  ( this->vertices_state_->get().size() > 1 ) )
  
      {
        this->private_->vertex_index_ = 0;
      } 
      else if ( ( this->private_->vertex_index_ == this->vertices_state_->get().size() - 1 ) 
        && ( this->vertices_state_->get().size() > 1 ) )
      {
        this->private_->vertex_index_ = 0;
      }
      else 
      {

        this->private_->vertex_index_ = this->private_->vertex_index_ - 1;
      }

      
      Core::Application::PostEvent( boost::bind( &Core::StateInt::set,
        this->current_vertex_index_state_, static_cast< int >( this->private_->vertex_index_ ), Core::ActionSource::NONE_E ) );

      Core::ActionRemove::Dispatch( Core::Interface::GetMouseActionContext(),
        this->vertices_state_, pt );

      // Set to "not hovered over" state since the point no longer exists
      viewer->set_cursor( Core::CursorShape::CROSS_E );
    
      //this->private_->vertex_index_ = -1;

      return true;
    }   
  }

  return false;
}

bool SpeedlineTool::handle_mouse_release( ViewerHandle viewer, 
                    const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  if ( viewer->is_volume_view() )
  {
    return false;
  }

  if ( this->private_->moving_vertex_ && 
    ( button == Core::MouseButton::LEFT_BUTTON_E ||
    button == Core::MouseButton::MID_BUTTON_E ) )
  {
    this->private_->moving_vertex_ = false;
    this->private_->find_vertex( viewer, mouse_history.current_.x_, 
      mouse_history.current_.y_, this->private_->vertex_index_ );
    
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
  if ( viewer->is_volume_view() )
  {
    return false;
  }
  
  if ( buttons == Core::MouseButton::NO_BUTTON_E )
  {
    this->private_->find_vertex( viewer, mouse_history.current_.x_, 
      mouse_history.current_.y_, this->private_->vertex_index_ );

    //if ( this->private_->find_closest_vertex( viewer, mouse_history.current_.x_, 
    //  mouse_history.current_.y_, this->private_->vertex_index_ ) )

    if ( this->private_->vertex_index_ >= 0 )
    {
      Core::Application::PostEvent( boost::bind( &Core::StateInt::set,
        this->current_vertex_index_state_, this->private_->vertex_index_, Core::ActionSource::NONE_E ) );
    }
    
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

      Core::Application::PostEvent( boost::bind( &Core::StateInt::set,
        this->current_vertex_index_state_, this->private_->vertex_index_, Core::ActionSource::NONE_E ) );

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


void SpeedlineTool::handle_slice_changed(  )
{
  this->private_->execute_path( true );
}

void SpeedlineTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( viewer->is_volume_view() )
  {
    return;
  }

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
  if ( vertices_num == 0 )
  {
    return;
  }

  size_t paths_num = paths.get_path_num();


  CORE_CHECK_OPENGL_ERROR();
  glPushAttrib( GL_LINE_BIT | GL_POINT_BIT | GL_TRANSFORM_BIT );
  CORE_CHECK_OPENGL_ERROR();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  CORE_CHECK_OPENGL_ERROR();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glPointSize( 4.0f );
  glLineWidth( 1.0f );
  glColor3f( 0.0f, 1.0f, 1.0f );
  glEnable( GL_LINE_SMOOTH );

  //glColor3f( 1.0f, 0.0f, 1.0f );  
  //glBegin( GL_POINTS );
  //for ( size_t i = 0; i < vertices_num; ++i )
  //{
  //  double x_pos, y_pos;
  //  Core::VolumeSlice::ProjectOntoSlice( slice_type, vertices[ i ], x_pos, y_pos );
  //  glVertex2d( x_pos, y_pos );
  //}
  //glEnd();

  
  
  //Core::Point start_p = vertices[ 0 ];
  //Core::Point end_p = vertices[ vertices_num - 1 ];
  //bool is_more_than_two_edges =  vertices_num > 2 ? true : false;

  Core::Point start_p = paths.get_start_point();
  Core::Point end_p = paths.get_end_point();

  if ( start_p == end_p  )
  {
    glColor3f( 0.0f, 1.0f, 1.0f );
    glBegin( GL_POINTS );
    {
      double x_pos, y_pos;
      Core::VolumeSlice::ProjectOntoSlice( slice_type, start_p, x_pos, y_pos );
      glVertex2d( x_pos, y_pos );
    }
    glEnd();
  }
  
  if ( paths_num > 0 )
  {
    bool is_more_than_two_edges =  paths_num > 2 ? true : false;

    for ( unsigned int i = 0; i < paths_num; ++i )
    {
      Core::SinglePath single_path = paths.get_one_path( i );
      Core::Point p0, p1;
      single_path.get_point_on_ends( 0, p0 );
      single_path.get_point_on_ends( 1, p1 );

      if ( p0 == end_p && p1 == start_p && is_more_than_two_edges )
      {
        glColor3f( 1.0f, 0.0f, 0.0f );    // red
      } 
      else
      {
        glColor3f( 0.0f, 1.0f, 0.0f );    // green
      }

      glBegin( GL_LINE_STRIP );
      for ( unsigned int j = 0; j < single_path.get_points_num_on_path(); ++j )
      {
        double x_pos, y_pos;
        Core::VolumeSlice::ProjectOntoSlice( slice_type, single_path.get_a_point( j ), x_pos, y_pos );
        glVertex2d( x_pos, y_pos );
      }
      glEnd();

      glColor3f( 1.0f, 1.0f, 0.0f );
      //glPointSize( 4.0f );
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
