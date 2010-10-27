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

#include <Core/Volume/VolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionClear.h>
#include <Core/State/Actions/ActionSetAt.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PolylineTool.h>
#include <Application/Tools/Actions/ActionPolyline.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, PolylineTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class PolylineToolPrivate
//////////////////////////////////////////////////////////////////////////

class PolylineToolPrivate
{
public:
  void handle_vertices_changed();
  bool find_vertex( ViewerHandle viewer, int x, int y, size_t& index );
  void execute( Core::ActionContextHandle context, bool erase, 
    ViewerHandle viewer = ViewerHandle() );

  PolylineTool* tool_;
  bool moving_vertex_;
  size_t vertex_index_;
};

void PolylineToolPrivate::handle_vertices_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

bool PolylineToolPrivate::find_vertex( ViewerHandle viewer, int x, int y, size_t& index )
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
      index = i;
      return true;
    }
  }

  return false;
}

void PolylineToolPrivate::execute( Core::ActionContextHandle context, 
                  bool erase, ViewerHandle viewer )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );

  if ( !this->tool_->valid_target_state_->get() )
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
    this->tool_->target_layer_state_->get() ) );
  if ( !volume_slice )
  {
    return;
  }
  
  const std::vector< Core::Point >& vertices = this->tool_->vertices_state_->get();
  size_t num_of_vertices = vertices.size();
  if ( num_of_vertices < 3 )
  {
    return;
  }
  double world_x, world_y;
  int x, y;
  std::vector< ActionPolyline::VertexCoord > vertices_2d;
  for ( size_t i = 0; i < num_of_vertices; ++i )
  {
    volume_slice->project_onto_slice( vertices[ i ], world_x, world_y );
    volume_slice->world_to_index( world_x, world_y, x, y );
    vertices_2d.push_back( ActionPolyline::VertexCoord( 
      static_cast< float >( x ), static_cast< float >( y ), 0 ) );
  }
  
  ActionPolyline::Dispatch( context, this->tool_->target_layer_state_->get(),
    volume_slice->get_slice_type(), volume_slice->get_slice_number(), erase, vertices_2d );
}

//////////////////////////////////////////////////////////////////////////
// Class PolylineTool
//////////////////////////////////////////////////////////////////////////

PolylineTool::PolylineTool( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::MASK_E, toolid ),
  private_( new PolylineToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->moving_vertex_ = false;

  this->add_state( "vertices", this->vertices_state_ );

  this->add_connection( this->vertices_state_->state_changed_signal_.connect(
    boost::bind( &PolylineToolPrivate::handle_vertices_changed, this->private_ ) ) );
}

PolylineTool::~PolylineTool()
{
  this->disconnect_all();
}

void PolylineTool::fill( Core::ActionContextHandle context )
{
  this->private_->execute( context, false );
}

void PolylineTool::erase( Core::ActionContextHandle context )
{
  this->private_->execute( context, true );
}

bool PolylineTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
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

void PolylineTool::reset( Core::ActionContextHandle context )
{
  Core::ActionClear::Dispatch( context, this->vertices_state_ );
}

bool PolylineTool::handle_mouse_press( ViewerHandle viewer, 
                    const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( viewer->is_volume_view() )
  {
    return false;
  }

  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
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
      Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(),
        this->vertices_state_, pt );

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
      
      double dmin = DBL_MAX;
      std::vector<Core::Point> points = this->vertices_state_->get();
      
      size_t idx = 0;
      for ( size_t j = 0; j < points.size(); j++ )
      {
        size_t k = j + 1;
        if ( k ==  points.size() ) k = 0;
        
        double alpha = Dot( points[ j ] - pt, points[ j ] - points[ k ] )/
          Dot( points[ j ] - points[ k ], points[ j ] - points[ k ] );
          
        double dist = 0.0;
        if ( alpha < 0.0 ) dist = ( points[ j ] - pt ).length2();
        else if ( alpha > 1.0 ) dist = ( points[ k ] - pt ).length2();
        else dist = ( ( points[ j ] - pt ) - alpha * ( points[ j ] - points[ k ] ) ).length2();
        
        if ( dist < dmin )
        {
          dmin = dist;
          idx = k;
        }
      }
      points.insert( points.begin() + idx, pt );
      
      Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(),
        this->vertices_state_, points );

      return true;
    }
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    button == Core::MouseButton::RIGHT_BUTTON_E )
  {
    this->private_->moving_vertex_ = this->private_->find_vertex( viewer, 
      mouse_history.current_.x_, mouse_history.current_.y_, 
      this->private_->vertex_index_ );
    return this->private_->moving_vertex_;
  }
  
  return false;
}

bool PolylineTool::handle_mouse_release( ViewerHandle viewer, 
                    const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  if ( this->private_->moving_vertex_ && button == Core::MouseButton::RIGHT_BUTTON_E )
  {
    this->private_->moving_vertex_ = false;
    return true;
  }
  
  return false;
}

bool PolylineTool::handle_mouse_move( ViewerHandle viewer, 
                   const Core::MouseHistory& mouse_history, 
                   int button, int buttons, int modifiers )
{
  if ( this->private_->moving_vertex_ )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    Core::Point pt = this->vertices_state_->get()[ this->private_->vertex_index_ ];
    std::string view_mode = viewer->view_mode_state_->get();
    double world_x, world_y;
    viewer->window_to_world( mouse_history.current_.x_,
      mouse_history.current_.y_, world_x, world_y );
    lock.unlock();

    if ( view_mode == Viewer::AXIAL_C )
    {
      pt[ 0 ] = world_x;
      pt[ 1 ] = world_y;
    }
    else if ( view_mode == Viewer::CORONAL_C )
    {
      pt[ 0 ] = world_x;
      pt[ 2 ] = world_y;
    }
    else if ( view_mode == Viewer::SAGITTAL_C )
    {
      pt[ 1 ] = world_x;
      pt[ 2 ] = world_y;
    }
    else
    {
      this->private_->moving_vertex_ = false;
      return false;
    }

    Core::ActionSetAt::Dispatch( Core::Interface::GetMouseActionContext(),
      this->vertices_state_, this->private_->vertex_index_, pt );
    return true;
  }
  
  return false;
}

void PolylineTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  if ( viewer->is_volume_view() )
  {
    return;
  }

  std::vector< Core::Point > vertices;
  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    vertices = this->vertices_state_->get();
    if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      slice_type = Core::VolumeSliceType::SAGITTAL_E;
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      slice_type = Core::VolumeSliceType::CORONAL_E;
    }
  }

  if ( vertices.size() == 0 )
  {
    return;
  }
  
  glPushAttrib( GL_LINE_BIT | GL_POINT_BIT | GL_TRANSFORM_BIT );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glPointSize( 5.0f );
  glLineWidth( 2.0f );
  glColor3f( 0.0f, 1.0f, 1.0f );
  glEnable( GL_LINE_SMOOTH );

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

  glBegin( GL_LINE_LOOP );
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    glVertex2d( vertices[ i ].x(), vertices[ i ].y() );
  }
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

bool PolylineTool::has_2d_visual()
{
  return true;
}

} // end namespace Seg3D
