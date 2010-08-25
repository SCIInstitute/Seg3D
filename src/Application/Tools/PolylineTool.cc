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
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionClear.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PolylineTool.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, PolylineTool )

namespace Seg3D
{

class PolylineToolPrivate
{
public:
  void handle_vertices_changed();
  bool find_vertex( int x, int y, size_t& index );

  PolylineTool* tool_;
  ViewerHandle viewer_;
  bool moving_vertex_;
  size_t vertex_index_;
};

void PolylineToolPrivate::handle_vertices_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

bool PolylineToolPrivate::find_vertex( int x, int y, size_t& index )
{
  // Step 1. Compute the size of a pixel in world space
  double x0, y0, x1, y1;
  this->viewer_->window_to_world( 0, 0, x0, y0 );
  this->viewer_->window_to_world( 1, 1, x1, y1 );
  double pixel_width = Core::Abs( x1 - x0 );
  double pixel_height = Core::Abs( y1 - y0 );

  // Step 2. Compute the mouse position in world space
  double world_x, world_y;
  this->viewer_->window_to_world( x, y, world_x, world_y );

  // Step 3. Search for the first vertex that's within 2 pixels of current mouse position
  double range_x = pixel_width * 2;
  double range_y = pixel_height * 2;
  std::vector< Core::Point > vertices = this->tool_->vertices_state_->get();
  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  if ( this->viewer_->view_mode_state_->get() == Viewer::CORONAL_C )
  {
    slice_type = Core::VolumeSliceType::CORONAL_E;
  }
  else if ( this->viewer_->view_mode_state_->get() == Viewer::SAGITTAL_C )
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

void PolylineTool::fill( Core::ActionContextHandle context ) const
{
}

void PolylineTool::erase( Core::ActionContextHandle context ) const
{
}

void PolylineTool::reset( Core::ActionContextHandle context ) const
{
  Core::ActionClear::Dispatch( context, this->vertices_state_ );
}

bool PolylineTool::handle_mouse_enter( size_t viewer_id, int x, int y )
{
  this->private_->viewer_ = ViewerManager::Instance()->get_viewer( viewer_id );
  return true;
}

bool PolylineTool::handle_mouse_leave( size_t viewer_id )
{
  this->private_->viewer_.reset();
  return true;
}

bool PolylineTool::handle_mouse_press( const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
  if ( this->private_->viewer_->is_volume_view() )
  {
    return false;
  }

  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    button == Core::MouseButton::LEFT_BUTTON_E )
  {
    Core::VolumeSliceHandle active_slice = this->private_->viewer_->get_active_volume_slice();
    if ( active_slice && !active_slice->out_of_boundary() )
    {
      double world_x, world_y;
      this->private_->viewer_->window_to_world( mouse_history.current_.x_, 
        mouse_history.current_.y_, world_x, world_y );
      Core::Point pt;
      active_slice->get_world_coord( world_x, world_y,  pt );
      Core::ActionAdd::Dispatch( Core::Interface::GetMouseActionContext(),
        this->vertices_state_, pt );

      return true;
    }
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    button == Core::MouseButton::RIGHT_BUTTON_E )
  {
    this->private_->moving_vertex_ = this->private_->find_vertex( mouse_history.current_.x_, 
      mouse_history.current_.y_, this->private_->vertex_index_ );
    return this->private_->moving_vertex_;
  }
  
  return false;
}

bool PolylineTool::handle_mouse_release( const Core::MouseHistory& mouse_history, 
                    int button, int buttons, int modifiers )
{
  if ( this->private_->moving_vertex_ && button == Core::MouseButton::RIGHT_BUTTON_E )
  {
    this->private_->moving_vertex_ = false;
    return true;
  }
  
  return false;
}

bool PolylineTool::handle_mouse_move( const Core::MouseHistory& mouse_history, 
                   int button, int buttons, int modifiers )
{
  if ( this->private_->moving_vertex_ )
  {
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
