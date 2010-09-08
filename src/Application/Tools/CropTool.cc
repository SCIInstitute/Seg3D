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

#include <Core/Utils/ScopedCounter.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/RenderResources/RenderResources.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/CropTool.h>
#include <Application/Tools/Actions/ActionCrop.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, CropTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class CropToolPrivate
//////////////////////////////////////////////////////////////////////////

class CropToolPrivate
{
public:
  void handle_target_group_changed();
  void handle_cropbox_changed();

  size_t signal_block_count_;
  CropTool* tool_;
};

void CropToolPrivate::handle_target_group_changed()
{
  const std::string& group_id = this->tool_->target_group_state_->get();
  if ( group_id == "" || group_id == Tool::NONE_OPTION_C )
  {
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  LayerGroupHandle layer_group = LayerManager::Instance()->get_layer_group( group_id );
  const Core::GridTransform& grid_trans = layer_group->get_grid_transform();
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );
  this->tool_->input_dimensions_state_[ 0 ]->set( nx );
  this->tool_->input_dimensions_state_[ 1 ]->set( ny );
  this->tool_->input_dimensions_state_[ 2 ]->set( nz );

  // Compute the bounding box of the layer group in world space.
  // NOTE: The bounding box is extended by half a voxel in each direction
  Core::Point origin( -0.5, -0.5, -0.5 );
  origin = grid_trans * origin;
  Core::Point end( nx - 0.5, ny - 0.5, nz - 0.5 );
  end = grid_trans * end;

  this->tool_->cropbox_origin_state_[ 0 ]->set_range( origin[ 0 ], end[ 0 ] );
  this->tool_->cropbox_origin_state_[ 1 ]->set_range( origin[ 1 ], end[ 1 ] );
  this->tool_->cropbox_origin_state_[ 2 ]->set_range( origin[ 2 ], end[ 2 ] );
  this->tool_->cropbox_size_state_[ 0 ]->set_range( 0, end[ 0 ] - origin[ 0 ] );
  this->tool_->cropbox_size_state_[ 1 ]->set_range( 0, end[ 1 ] - origin[ 1 ] );
  this->tool_->cropbox_size_state_[ 2 ]->set_range( 0, end[ 2 ] - origin[ 2 ] );
}

void CropToolPrivate::handle_cropbox_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}


//////////////////////////////////////////////////////////////////////////
// Class CropTool
//////////////////////////////////////////////////////////////////////////

CropTool::CropTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_E, toolid ),
  private_( new CropToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->signal_block_count_ = 0;

  this->add_state( "input_x", this->input_dimensions_state_[ 0 ], 0 );
  this->add_state( "input_y", this->input_dimensions_state_[ 1 ], 0 );
  this->add_state( "input_z", this->input_dimensions_state_[ 2 ], 0 );
  this->add_state( "crop_origin_x", this->cropbox_origin_state_[ 0 ], -1000.0, -1000.0, 1000.0, 0.01 );
  this->add_state( "crop_origin_y", this->cropbox_origin_state_[ 1 ], -1000.0, -1000.0, 1000.0, 0.01 );
  this->add_state( "crop_origin_z", this->cropbox_origin_state_[ 2 ], -1000.0, -1000.0, 1000.0, 0.01 );
  this->add_state( "crop_width", this->cropbox_size_state_[ 0 ], 1000.0, 0.0, 1000.0, 0.01 );
  this->add_state( "crop_height", this->cropbox_size_state_[ 1 ], 1000.0, 0.0, 1000.0, 0.01 );
  this->add_state( "crop_depth", this->cropbox_size_state_[ 2 ], 1000.0, 0.0, 1000.0, 0.01 );

  this->add_state( "replace", this->replace_state_, false );

  this->add_connection( this->target_group_state_->state_changed_signal_.connect(
    boost::bind( &CropToolPrivate::handle_target_group_changed, this->private_ ) ) );

  for ( int i = 0; i < 3; ++i )
  {
    this->add_connection( this->cropbox_origin_state_[ i ]->state_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_changed, this->private_ ) ) );
    this->add_connection( this->cropbox_size_state_[ i ]->state_changed_signal_.connect(
      boost::bind( &CropToolPrivate::handle_cropbox_changed, this->private_ ) ) );
  }
  
  this->private_->handle_target_group_changed();
}

CropTool::~CropTool()
{
  this->disconnect_all();
}

void CropTool::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  Core::Point origin( this->cropbox_origin_state_[ 0 ]->get(),
    this->cropbox_origin_state_[ 1 ]->get(), this->cropbox_origin_state_[ 2 ]->get() );
  Core::Vector size( this->cropbox_size_state_[ 0 ]->get(),
    this->cropbox_size_state_[ 1 ]->get(), this->cropbox_size_state_[ 2 ]->get() );
  ActionCrop::Dispatch( context, this->target_layers_state_->get(), 
    origin, size, this->replace_state_->get() );
}

void CropTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );

  Core::VolumeSliceType slice_type( Core::VolumeSliceType::AXIAL_E );
  Core::Point crop_origin, crop_end;

  {
    Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
    std::string target_group_id = this->target_group_state_->get();
    if ( viewer->is_volume_view() || 
      target_group_id == "" || 
      target_group_id == Tool::NONE_OPTION_C )
    {
      return;
    }
    if ( viewer->view_mode_state_->get() == Viewer::SAGITTAL_C )
    {
      slice_type = Core::VolumeSliceType::SAGITTAL_E;
    }
    else if ( viewer->view_mode_state_->get() == Viewer::CORONAL_C )
    {
      slice_type = Core::VolumeSliceType::CORONAL_E;
    }

    for ( int i = 0; i < 3; ++i )
    {
      crop_origin[ i ] = this->cropbox_origin_state_[ i ]->get();
      crop_end[ i ] = crop_origin[ i ] + this->cropbox_size_state_[ i ]->get();
    }
  }

  double start_x, start_y, end_x, end_y;
  Core::VolumeSlice::ProjectOntoSlice( slice_type, crop_origin, start_x, start_y );
  Core::VolumeSlice::ProjectOntoSlice( slice_type, crop_end, end_x, end_y );

  glPushAttrib( GL_LINE_BIT | GL_TRANSFORM_BIT );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  glLineWidth( 2.0f );
  glColor3f( 1.0f, 0.0f, 0.0f );

  glBegin( GL_LINE_LOOP );
  glVertex2d( start_x, start_y );
  glVertex2d( end_x, start_y );
  glVertex2d( end_x, end_y );
  glVertex2d( start_x, end_y );
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

bool CropTool::has_2d_visual()
{
  return true;
}

} // end namespace Seg3D
