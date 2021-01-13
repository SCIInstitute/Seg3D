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

#include <algorithm>
#include <cstdlib>
#include <stack>
#include <iostream>

#include <boost/algorithm/string/predicate.hpp>

#include <Core/Application/Application.h>
#include <Core/Viewer/Mouse.h>
#include <Core/Graphics/Texture.h>
#include <Core/Graphics/Algorithm.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Utils/Exception.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Volume/MaskVolumeSlice.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Tools/detail/MaskShader.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/Actions/ActionPaint.h>
#include <Application/Tools/Actions/ActionFloodFill.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>

#include <Application/Tools/GrowCutTool.h>
#include <Application/Tools/Actions/ActionGrowCut.h>
#include <Application/Tools/Actions/ActionGrowCutInitialize.h>
#include <Application/Tools/Actions/ActionGrowCutPostProcess.h>
#include <Application/Tools/Algorithm/GrowCutter.h>

#include <Application/Layer/Actions/ActionComputeIsosurface.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, GrowCutTool )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Declaration and implementation of class GrowCutToolPrivate
//////////////////////////////////////////////////////////////////////////

class GrowCutToolPrivate : public Core::RecursiveLockable
{
public:

  ~GrowCutToolPrivate();

  // BUILD_BRUSH_MASK:
  // Build mask for the brush which can later be used for painting.
  void build_brush_mask();

  void initialize();
  void upload_mask_texture();

  void handle_brush_radius_changed();

  bool check_paintable( ViewerHandle viewer );

  bool start_painting( bool foreground );
  void stop_painting();

  void finalize_paint();

  //PAINT:
  // Paint on the target layer with the brush centered at (x0, y0).
  void paint( const PaintInfo& paint_info, int xc, int yc, int& paint_count );

  void interpolated_paint( const PaintInfo& paint_info, int x0, int y0,
                           int x1, int y1, int& paint_count );

  // UPDATE_SAME_MODE_VIEWERS:
  // Cause viewers with the same view mode as the one that contains the paint brush
  // to redraw overlay to display the current position of the brush.
  void update_same_mode_viewers();

  void setup_paint_info( PaintInfo& paint_info, int x0, int y0, int x1, int y1 );

  //To highlight the created speedline image
  void handle_layers_initialized( std::string foreground, std::string background,
                                  std::string output, GrowCutToolWeakHandle tool );

  void reinitialize_growcut();

  void handle_layers_changed();

  bool initialized_;
  bool brush_mask_changed_;
  bool painting_;
  bool erase_;
  bool foreground_;
  bool brush_visible_;
  bool paintable_;
  bool data_layer_;
  size_t signal_block_count_;

  std::string target_layer_id_;
  Core::MaskVolumeSliceHandle target_slice_;
  ViewerHandle viewer_;

  GrowCutTool* tool_;
  int center_x_;
  int center_y_;

  // The radius used to build the mask. It might be different from the value stored in the
  // state variable.
  int radius_;

  std::vector< unsigned char > brush_mask_;
  Core::Texture2DHandle brush_tex_;
  MaskShaderHandle shader_;

  std::vector<int> x_;
  std::vector<int> y_;

  const static int INVALID_VIEWER_C;

  GrowCutterHandle grow_cutter_;

  Core::ConnectionHandler connection_handler_;
};

const int GrowCutToolPrivate::INVALID_VIEWER_C = -1;

// Set the pixel at (x, y) of the buffer to the given value.
// The size of the buffer must be dimension * dimension
inline void SetPixel( std::vector< unsigned char >& buffer, int x, int y,
                      int dimension, unsigned char value = 1 )
{
  buffer[ y * dimension + x ] = value;
}

// Bresenham algorithm for drawing a circle.
static void BresenhamCircle( std::vector< unsigned char >& buffer, int radius )
{
  const int x0 = radius;
  const int y0 = radius;
  const int dimension = radius * 2 + 1;

  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;

  SetPixel( buffer, x0, y0 + radius, dimension );
  SetPixel( buffer, x0, y0 - radius, dimension );
  SetPixel( buffer, x0 + radius, y0, dimension );
  SetPixel( buffer, x0 - radius, y0, dimension );

  while ( x < y )
  {
    if ( f >= 0 )
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    SetPixel( buffer, x0 + x, y0 + y, dimension );
    SetPixel( buffer, x0 - x, y0 + y, dimension );
    SetPixel( buffer, x0 + x, y0 - y, dimension );
    SetPixel( buffer, x0 - x, y0 - y, dimension );
    SetPixel( buffer, x0 + y, y0 + x, dimension );
    SetPixel( buffer, x0 - y, y0 + x, dimension );
    SetPixel( buffer, x0 + y, y0 - x, dimension );
    SetPixel( buffer, x0 - y, y0 - x, dimension );
  }
}

void GrowCutToolPrivate::build_brush_mask()
{
  this->radius_ = this->tool_->brush_radius_state_->get();
  int brush_size = this->radius_ * 2 + 1;
  this->brush_mask_.resize( brush_size * brush_size );
  if ( brush_size == 1 )
  {
    this->brush_mask_[ 0 ] = 1;
    return;
  }
  memset( &this->brush_mask_[ 0 ], 0, sizeof( unsigned char ) * this->brush_mask_.size() );
  BresenhamCircle( this->brush_mask_, this->radius_ );
  Core::FloodFill( &this->brush_mask_[ 0 ], brush_size, brush_size,
                   this->radius_, this->radius_, ( unsigned char )( 1 ) );
}

void GrowCutToolPrivate::initialize()
{
  if ( !this->initialized_ )
  {
    lock_type internal_lock( this->get_mutex() );
    if ( this->initialized_ )
    {
      return;
    }

    {
      Core::RenderResources::lock_type rr_lock( Core::RenderResources::GetMutex() );
      this->brush_tex_ = Core::Texture2DHandle( new Core::Texture2D );
      this->shader_.reset( new MaskShader );
      this->shader_->initialize();
    }

    this->brush_tex_->set_mag_filter( GL_NEAREST );
    this->brush_tex_->set_min_filter( GL_NEAREST );

    this->shader_->enable();
    this->shader_->set_border_width( 2 );
    this->shader_->set_texture( 0 );
    this->shader_->disable();

    this->initialized_ = true;
  }
}

void GrowCutToolPrivate::upload_mask_texture()
{
  if ( !this->brush_mask_changed_ )
  {
    return;
  }

  lock_type internal_lock( this->get_mutex() );

  if ( !this->brush_mask_changed_ )
  {
    return;
  }

  int brush_size = this->radius_ * 2 + 1;
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  {
    Core::Texture::lock_type tex_lock( this->brush_tex_->get_mutex() );
    Core::RenderResources::lock_type rr_lock( Core::RenderResources::GetMutex() );
    this->brush_tex_->set_image( brush_size, brush_size, GL_ALPHA, &this->brush_mask_[ 0 ],
                                 GL_ALPHA, GL_UNSIGNED_BYTE );
  }

  this->brush_mask_changed_ = false;
}

void GrowCutToolPrivate::handle_brush_radius_changed()
{
  lock_type lock( this->get_mutex() );
  this->build_brush_mask();
  this->brush_mask_changed_ = true;
  this->update_same_mode_viewers();
}

void GrowCutToolPrivate::paint( const PaintInfo& paint_info, int xc, int yc, int& paint_count )
{
  int brush_size = paint_info.brush_radius_ * 2 + 1;
  int x_min = xc - paint_info.brush_radius_;
  int x_max = xc + paint_info.brush_radius_;
  int y_min = yc - paint_info.brush_radius_;
  int y_max = yc + paint_info.brush_radius_;
  if ( x_min >= static_cast< int >( paint_info.target_slice_->nx() ) ||
       x_max < 0 || y_max < 0 ||
       y_min >= static_cast< int >( paint_info.target_slice_->ny() ) )
  {
    return;
  }

  // Compute the start and end indices of the region of the brush that
  // intersects with the mask volume slice.
  size_t x_start = static_cast< size_t >( Core::Max( x_min, 0 ) - x_min );
  size_t x_end = static_cast< size_t >( Core::Min( x_max, static_cast< int >(
                                                     paint_info.target_slice_->nx() - 1 ) ) - x_min );
  size_t y_start = static_cast< size_t >( Core::Max( y_min, 0 ) - y_min );
  size_t y_end = static_cast< size_t >( Core::Min( y_max, static_cast< int >(
                                                     paint_info.target_slice_->ny() - 1 ) ) - y_min );

  unsigned char* buffer = paint_info.target_slice_->get_cached_data();
  size_t nx = paint_info.target_slice_->nx();

  size_t slice_x, slice_y, slice_index;
  size_t current_index = paint_info.target_slice_->to_index( 0, 0 );
  size_t x_stride = paint_info.target_slice_->to_index( 1, 0 ) - current_index;
  size_t y_stride = paint_info.target_slice_->to_index( 0, 1 ) - current_index;
  size_t row_start = paint_info.target_slice_->to_index( x_min + x_start, y_min + y_start );
  for ( size_t y = y_start; y <= y_end; y++, row_start += y_stride )
  {
    current_index = row_start;
    for ( size_t x = x_start; x <= x_end; x++, current_index += x_stride )
    {
      if ( this->brush_mask_[ y * brush_size + x ] != 0 )
      {
        slice_x = x_min + x;
        slice_y = y_min + y;
        slice_index = slice_y * nx + slice_x;

        if ( paint_info.erase_ )
        {
          this->reinitialize_growcut();
          buffer[ slice_index ] = 0;
        }
        else
        {
          buffer[ slice_index ] = 1;
        }
      }
    }
  }

  paint_count++;
}

void GrowCutToolPrivate::interpolated_paint( const PaintInfo& paint_info, int x0, int y0,
                                             int x1, int y1, int& paint_count )
{
  int delta_x = Core::Abs( x1 - x0 );
  int delta_y = Core::Abs( y1 - y0 );

  if ( delta_x == 0 && delta_y == 0 )
  {
    this->paint( paint_info, x0, y0, paint_count );
    return;
  }

  if ( delta_x > 1 || delta_y > 1 )
  {
    int mid_x = Core::Round( ( x0 + x1 ) * 0.5 );
    int mid_y = Core::Round( ( y0 + y1 ) * 0.5 );
    this->interpolated_paint( paint_info, x0, y0, mid_x, mid_y, paint_count );
    this->interpolated_paint( paint_info, mid_x, mid_y, x1, y1, paint_count );
  }
  else
  {
    this->paint( paint_info, x1, y1, paint_count );
  }
}

void GrowCutToolPrivate::update_same_mode_viewers()
{
  if ( !this->viewer_ || this->viewer_->is_volume_view() )
  {
    return;
  }

  const std::string& view_mode = this->viewer_->view_mode_state_->get();
  ViewerManager::Instance()->update_viewers_overlay( view_mode );
}

bool GrowCutToolPrivate::start_painting( bool foreground )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  this->painting_ = true;

  // get the foreground or background layer
  this->target_layer_id_ = this->tool_->foreground_layer_state_->get();
  if ( !foreground )
  {
    this->target_layer_id_ = this->tool_->background_layer_state_->get();
  }

  this->target_slice_ = boost::dynamic_pointer_cast< Core::MaskVolumeSlice >(
    this->viewer_->get_volume_slice( this->target_layer_id_ ) );
  if ( !this->target_slice_ )
  {
    CORE_THROW_LOGICERROR( "Mask layer with ID '" +
                           this->target_layer_id_ + "' does not exist" );
  }
  if ( this->target_slice_->out_of_boundary() )
  {
    this->painting_ = false;
    this->target_slice_.reset();
    return false;
  }

  return true;
}

void GrowCutToolPrivate::stop_painting()
{
  this->painting_ = false;
  this->x_.clear();
  this->y_.clear();
  this->finalize_paint();
}

void GrowCutToolPrivate::finalize_paint()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &GrowCutToolPrivate::finalize_paint, this ) );
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  this->target_slice_->release_cached_data();
  this->target_slice_.reset();
}

void GrowCutToolPrivate::setup_paint_info( PaintInfo& paint_info, int x0, int y0, int x1, int y1 )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  paint_info.target_layer_id_ = this->tool_->target_layer_state_->get();
  paint_info.target_slice_ = this->target_slice_;
  paint_info.data_constraint_layer_id_ = Tool::NONE_OPTION_C;
  paint_info.min_val_ = 0;
  paint_info.max_val_ = 0;
  paint_info.negative_data_constraint_ = false;
  paint_info.mask_constraint1_layer_id_ = Tool::NONE_OPTION_C;
  paint_info.negative_mask_constraint1_ = false;
  paint_info.mask_constraint2_layer_id_ = Tool::NONE_OPTION_C;
  paint_info.negative_mask_constraint2_ = false;
  paint_info.brush_radius_ = this->tool_->brush_radius_state_->get();
  paint_info.erase_ = this->erase_;

  paint_info.x_.resize( 2 );
  paint_info.y_.resize( 2 );

  double xpos, ypos;
  int slice_x, slice_y;
  this->viewer_->window_to_world( x0, y0, xpos, ypos );
  this->target_slice_->world_to_index( xpos, ypos, slice_x, slice_y );

  paint_info.x_[ 0 ] = slice_x;
  paint_info.y_[ 0 ] = slice_y;

  this->viewer_->window_to_world( x1, y1, xpos, ypos );
  this->target_slice_->world_to_index( xpos, ypos, slice_x, slice_y );

  paint_info.x_[ 1 ] = slice_x;
  paint_info.y_[ 1 ] = slice_y;

  this->x_.push_back( slice_x );
  this->y_.push_back( slice_y );
}

bool GrowCutToolPrivate::check_paintable( ViewerHandle viewer )
{
  bool paintable = false;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::string layer_id = this->tool_->foreground_layer_state_->get();
    if ( layer_id != Tool::NONE_OPTION_C )
    {
      size_t viewer_id = viewer->get_viewer_id();
      MaskLayerHandle layer = boost::dynamic_pointer_cast< Core::MaskLayer >(
        LayerManager::Instance()->find_layer_by_id( layer_id ) );
      if ( layer )
      {
        paintable = layer->is_visible( viewer_id ) && layer->has_valid_data() &&
                    !layer->locked_state_->get();
      }
    }
  }

  {
    lock_type lock( this->get_mutex() );
    this->paintable_ = paintable;
  }
  return paintable;
}

void GrowCutToolPrivate::handle_layers_initialized( std::string foreground, std::string background,
                                                    std::string output, GrowCutToolWeakHandle tool )
{
  // Make sure that the tool still exists
  if ( tool.lock() )
  {

    this->tool_->foreground_layer_state_->set( foreground );
    this->tool_->background_layer_state_->set( background );
    this->tool_->output_layer_state_->set( output );
  }
}

void GrowCutToolPrivate::reinitialize_growcut()
{
  this->grow_cutter_.reset();
}

void GrowCutToolPrivate::handle_layers_changed()
{
  this->reinitialize_growcut();

  this->connection_handler_.disconnect_all();

  MaskLayerHandle layer = boost::dynamic_pointer_cast< Core::MaskLayer >(
    LayerManager::Instance()->find_layer_by_id( this->tool_->foreground_layer_state_->get() ) );
  if ( layer )
  {
    this->connection_handler_.add_connection( layer->layer_updated_signal_.connect(
                                                boost::bind( &GrowCutTool::handle_layer_data_changed, this->tool_ ) ) );
  }

  if ( this->tool_->target_layer_state_->get() == Tool::NONE_OPTION_C )
  {
    this->tool_->registration_warning_state_->set( false );
  }
  else
  {
    LayerHandle target_layer_ = LayerManager::FindLayer( this->tool_->target_layer_state_->get() );

    if ( boost::starts_with( target_layer_->get_layer_name(), "Registered_" ) )
    {
      this->tool_->registration_warning_state_->set( false );
    }
    else
    {
      this->tool_->registration_warning_state_->set( true );
    }
  }
}

GrowCutToolPrivate::~GrowCutToolPrivate()
{
  this->connection_handler_.disconnect_all();
}


//////////////////////////////////////////////////////////////////////////
// Implementation of class GrowCutTool
//////////////////////////////////////////////////////////////////////////

GrowCutTool::GrowCutTool( const std::string& toolid ) :
  SingleTargetTool( Core::VolumeType::DATA_E, toolid ),
  private_( new GrowCutToolPrivate )
{
  this->private_->tool_ = this;
  this->private_->initialized_ = false;
  this->private_->brush_mask_changed_ = true;
  this->private_->painting_ = false;
  this->private_->brush_visible_ = true;
  this->private_->paintable_ = false;
  this->private_->data_layer_ = false;
  this->private_->signal_block_count_ = 0;

  std::vector< LayerIDNamePair > empty_names( 1,
                                              std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  this->add_state( "foreground_mask", this->foreground_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_extra_layer_input( this->foreground_layer_state_, Core::VolumeType::MASK_E, true, true );

  this->add_state( "background_mask", this->background_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_extra_layer_input( this->background_layer_state_, Core::VolumeType::MASK_E, true, true );

  this->add_state( "output_mask", this->output_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_extra_layer_input( this->output_layer_state_, Core::VolumeType::MASK_E, true, true );

  this->add_state( "brush_radius", this->brush_radius_state_, 3, 0, 150, 1 );
  double inf = std::numeric_limits< double >::infinity();
  this->add_state( "erase", this->erase_state_, false );

  this->add_state( "registration_warning", this->registration_warning_state_, false );

  this->private_->build_brush_mask();

  this->add_connection( this->brush_radius_state_->state_changed_signal_.connect(
                          boost::bind( &GrowCutToolPrivate::handle_brush_radius_changed, this->private_ ) ) );

  // if any of these layers change, we must reinitialize the growcutter
  this->add_connection( this->target_layer_state_->state_changed_signal_.connect(
                          boost::bind( &GrowCutToolPrivate::handle_layers_changed, this->private_ ) ) );
  this->add_connection( this->foreground_layer_state_->state_changed_signal_.connect(
                          boost::bind( &GrowCutToolPrivate::handle_layers_changed, this->private_ ) ) );
  this->add_connection( this->background_layer_state_->state_changed_signal_.connect(
                          boost::bind( &GrowCutToolPrivate::handle_layers_changed, this->private_ ) ) );
}

GrowCutTool::~GrowCutTool()
{
  this->disconnect_all();
}

void GrowCutTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
                          int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  ViewerHandle current_viewer;
  double world_x = 0, world_y = 0;
  int radius;
  bool brush_visible = false;

  int center_x, center_y;

  bool valid = false;
  {
    GrowCutToolPrivate::lock_type private_lock( this->private_->get_mutex() );
    current_viewer = this->private_->viewer_;
    radius = this->private_->radius_;
    center_x = this->private_->center_x_;
    center_y = this->private_->center_y_;
    valid = this->valid_target_state_->get();
  }

  if ( current_viewer && !current_viewer->is_volume_view() )
  {
    current_viewer->window_to_world( center_x, center_y, world_x, world_y );
  }

  this->private_->initialize();
  this->private_->upload_mask_texture();

  std::string foreground_layer_id;
  std::string data_constraint_layer_id;
  std::string current_viewer_mode;
  std::string redraw_viewer_mode;
  LayerHandle foreground_layer;
  bool layer_visible = false;

  {
    Core::StateEngine::lock_type se_lock( Core::StateEngine::GetMutex() );
    if ( current_viewer )
    {
      current_viewer_mode = current_viewer->view_mode_state_->get();
    }
    redraw_viewer_mode = viewer->view_mode_state_->get();
    foreground_layer_id = this->foreground_layer_state_->get();

    if ( !valid )
    {
      return;
    }

    // If no target layer is selected, return
    if ( foreground_layer_id == Tool::NONE_OPTION_C )
    {
      return;
    }
    foreground_layer = LayerManager::Instance()->find_layer_by_id( foreground_layer_id );
    if ( !foreground_layer )
    {
      // NOTE: Since the rendering is happening on a different thread, occasionally
      // when a layer is deleted, the state variables of the paint tool may have not
      // been updated yet, and thus the target layer might be invalid. We should just
      // ignore this kind of errors. Log it in case it's caused by something else.
      CORE_LOG_ERROR( "Layer with ID '" + this->foreground_layer_state_->get() +
                      "' does not exist" );
      return;
    }

    layer_visible = foreground_layer->has_valid_data() && foreground_layer->is_visible( viewer_id );
    if ( current_viewer )
    {
      brush_visible = layer_visible && foreground_layer->is_visible( current_viewer->get_viewer_id() ) &&
                      !foreground_layer->locked_state_->get();
    }
  }

  Core::MaskVolumeSliceHandle target_slice = boost::dynamic_pointer_cast
                                             < Core::MaskVolumeSlice >( viewer->get_volume_slice( foreground_layer_id ) );
  if ( target_slice->out_of_boundary() )
  {
    return;
  }

  float opacity = 1.0f;

  double voxel_width = ( target_slice->right() - target_slice->left() ) /
                       ( target_slice->nx() - 1 );
  double voxel_height = ( target_slice->top() - target_slice->bottom() ) /
                        ( target_slice->ny() - 1 );

  // Lock the shader, because this function can be called from multiple rendering threads
  MaskShader::lock_type shader_lock( this->private_->shader_->get_mutex() );

  this->private_->shader_->enable();
  this->private_->shader_->set_opacity( opacity );

  unsigned int old_tex_unit = Core::Texture::GetActiveTextureUnit();
  Core::Texture::SetActiveTextureUnit( 0 );
  glPushAttrib( GL_TRANSFORM_BIT );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

  CORE_CHECK_OPENGL_ERROR();

  MaskLayer* target_mask_layer = static_cast< MaskLayer* >( foreground_layer.get() );
  Core::Color color = PreferencesManager::Instance()->get_color(
    target_mask_layer->color_state_->get() );

  if ( brush_visible && current_viewer_mode == redraw_viewer_mode )
  {
    // Compute the position of the brush in world space
    // NOTE: The size of the brush needs to be extended by half of the voxel size in each
    // direction in order to visually align with the target mask layer.
    int i, j;
    target_slice->world_to_index( world_x, world_y, i, j );
    double left = target_slice->left() + ( i - radius - 0.5 ) * voxel_width;
    double right = target_slice->left() + ( i + radius + 0.5 ) * voxel_width;
    double bottom = target_slice->bottom() + ( j - radius - 0.5 ) * voxel_height;
    double top = target_slice->bottom() + ( j + radius + 0.5 ) * voxel_height;

    // Compute the size of the brush in window space
    Core::Vector brush_x( right - left, 0.0, 0.0 );
    brush_x = proj_mat * brush_x;
    double brush_screen_width = Core::Abs( brush_x.x() ) / 2.0 * viewer_width;
    double brush_screen_height = ( top - bottom ) / ( right - left ) * brush_screen_width;

    if ( current_viewer->get_viewer_id() != viewer_id )
    {
      this->private_->shader_->set_opacity( 0.5f );
    }
    this->private_->shader_->set_pixel_size( static_cast< float >( 1.0 / brush_screen_width ),
                                             static_cast< float >( 1.0 / brush_screen_height ) );
    this->private_->shader_->set_color( static_cast< float >( color.r() / 255 ),
                                        static_cast< float >( color.g() / 255 ), static_cast< float >( color.b() / 255 ) );

    // Lock the brush texture
    Core::Texture::lock_type tex_lock( this->private_->brush_tex_->get_mutex() );
    this->private_->brush_tex_->bind();
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2d( left, bottom );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2d( right, bottom );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2d( right, top );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2d( left, top );
    glEnd();

    this->private_->brush_tex_->unbind();
    CORE_CHECK_OPENGL_ERROR();
  }

  Core::Texture::SetActiveTextureUnit( old_tex_unit );
  this->private_->shader_->disable();
  glPopMatrix();
  glPopAttrib();

  if ( layer_visible )
  {
    SingleTargetTool::redraw( viewer_id, proj_mat, viewer_width, viewer_height );
    CORE_CHECK_OPENGL_ERROR();
  }

  glFinish();
}

bool GrowCutTool::handle_mouse_enter( ViewerHandle viewer, int x, int y )
{
  GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->viewer_ = viewer;
  if ( this->private_->viewer_->is_volume_view() ) { return false; }
  if ( !this->valid_target_state_->get() ) {return false; }

  this->private_->brush_visible_ = true;
  this->private_->center_x_ = x;
  this->private_->center_y_ = y;

  return true;
}

bool GrowCutTool::handle_mouse_leave( ViewerHandle viewer )
{
  GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->brush_visible_ = false;
  this->private_->update_same_mode_viewers();
  this->private_->viewer_.reset();
  if ( this->private_->painting_ )
  {
    this->private_->stop_painting();
  }
  return true;
}

void GrowCutTool::activate()
{}

void GrowCutTool::deactivate()
{
  ViewerManager::Instance()->reset_cursor();
}

bool GrowCutTool::handle_mouse_move( ViewerHandle viewer, const Core::MouseHistory& mouse_history,
                                     int button, int buttons, int modifiers )
{
  if ( this->private_->data_layer_ )
  {
    viewer->set_cursor( Core::CursorShape::CROSS_E );
    return false;
  }

  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    if ( !this->valid_target_state_->get() )
    {
      viewer->set_cursor( Core::CursorShape::CROSS_E );
      return false;
    }
  }

  // NOTE: This function call is running on the interface thread
  // Hence we need to lock as most of the paint tool runs on the Application thread.
  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->viewer_ = viewer;
    this->private_->center_x_ = mouse_history.current_.x_;
    this->private_->center_y_ = mouse_history.current_.y_;
  }

  // If it is a volume view we cannot do any painting
  if ( this->private_->viewer_->is_volume_view() )
  {
    viewer->set_cursor( Core::CursorShape::ARROW_E );
    return false;
  }

  // Get a handle to the constraint layer
  // NOTE: Again we need to lock the state engine as we are on the interface thread
  // and not on the application thread.
  std::string data_constraint_layer;
  bool cursor_invisible;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    cursor_invisible = PreferencesManager::Instance()->paint_cursor_invisibility_state_->get();
  }

  if ( !this->private_->painting_ )
  {
    if ( this->private_->check_paintable( viewer ) && cursor_invisible )
    {
      viewer->set_cursor( Core::CursorShape::BLANK_E );
    }
    else
    {
      viewer->set_cursor( Core::CursorShape::CROSS_E );
    }

    // When mousing over data with a data constraint the data constraint value
    // will be shown in the status bar
    if ( data_constraint_layer != Tool::NONE_OPTION_C )
    {
      this->private_->viewer_->update_status_bar( mouse_history.current_.x_,
                                                  mouse_history.current_.y_, data_constraint_layer );
    }
    else
    {
      this->private_->viewer_->update_status_bar( mouse_history.current_.x_,
                                                  mouse_history.current_.y_ );
    }
  }

  // If a brush is shown update all viewers that need the brush
  if ( this->private_->brush_visible_ )
  {
    this->private_->update_same_mode_viewers();
  }

  // If we are painting
  if ( this->private_->painting_ )
  {
    PaintInfo paint_info;

    this->private_->setup_paint_info( paint_info, mouse_history.previous_.x_,
                                      mouse_history.previous_.y_, mouse_history.current_.x_,
                                      mouse_history.current_.y_ );

    GrowCutTool::HandlePaint( this->shared_from_this(), paint_info );

    return true;
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E )
  {
    return true;
  }

  return false;
}

bool GrowCutTool::handle_mouse_press( ViewerHandle viewer, const Core::MouseHistory& mouse_history,
                                      int button, int buttons, int modifiers )
{

  // if already painting (or erasing), ignore additional mouse presses
  if ( this->private_->painting_ )
  {
    return true;
  }

  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    if ( !this->valid_target_state_->get() ) {return false; }
    this->private_->viewer_ = viewer;
  }

  if ( viewer->is_volume_view() )
  {
    return false;
  }

  double world_x, world_y;
  viewer->window_to_world( mouse_history.current_.x_,
                           mouse_history.current_.y_, world_x, world_y );

  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->center_x_ = mouse_history.current_.x_;
    this->private_->center_y_ = mouse_history.current_.y_;
  }

  bool paintable = false;
  bool erase = false;
  {

    this->private_->data_layer_ = false;
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    bool valid_target = this->valid_target_state_->get();
    if ( !valid_target )
    {
      return false;
    }

    erase = this->erase_state_->get();
    if ( this->target_layer_state_->get() != Tool::NONE_OPTION_C &&
         !this->private_->painting_ )
    {
      size_t viewer_id = this->private_->viewer_->get_viewer_id();
      MaskLayerHandle layer = boost::dynamic_pointer_cast< Core::MaskLayer >(
        LayerManager::Instance()->find_layer_by_id( this->foreground_layer_state_->get() ) );
      if ( layer )
      {
        paintable = layer->is_visible( viewer_id ) && layer->has_valid_data() &&
                    !layer->locked_state_->get();
      }
      else
      {
        this->private_->data_layer_ = true;
        return false;
      }
    }
    else
    {
      this->private_->data_layer_ = true;
      return false;
    }
  }

  if ( paintable )
  {
    if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
         button == Core::MouseButton::LEFT_BUTTON_E )
    {
      GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
      this->private_->painting_ = true;
      this->private_->erase_ = erase;
      this->private_->foreground_ = true;
    }
    else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
              button == Core::MouseButton::RIGHT_BUTTON_E )
    {
      GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
      this->private_->painting_ = true;
      this->private_->erase_ = erase;
      this->private_->foreground_ = false;
    }
    else if ( modifiers == Core::KeyModifier::CONTROL_MODIFIER_E &&
              button == Core::MouseButton::LEFT_BUTTON_E )
    {
      GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
      this->private_->painting_ = true;
      this->private_->erase_ = true;
      this->private_->foreground_ = true;
    }
    else if ( modifiers == Core::KeyModifier::CONTROL_MODIFIER_E &&
              button == Core::MouseButton::RIGHT_BUTTON_E )
    {
      GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
      this->private_->painting_ = true;
      this->private_->erase_ = true;
      this->private_->foreground_ = false;
    }

    if ( this->private_->painting_ )
    {
      GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );

      if ( this->private_->start_painting( this->private_->foreground_ ) )
      {
        Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
        PaintInfo paint_info;
        this->private_->setup_paint_info( paint_info, mouse_history.current_.x_,
                                          mouse_history.current_.y_, mouse_history.current_.x_,
                                          mouse_history.current_.y_ );

        GrowCutTool::HandlePaint( this->shared_from_this(), paint_info );
        return true;
      }
    }
  }

  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->brush_visible_ = this->private_->painting_;
  }

  if ( this->private_->painting_ )
  {
    return true;
  }

  if ( paintable )
  {
    return SingleTargetTool::handle_mouse_press( viewer, mouse_history, button, buttons, modifiers );
  }

  return false;
}

bool GrowCutTool::handle_mouse_release( ViewerHandle viewer, const Core::MouseHistory& mouse_history,
                                        int button, int buttons, int modifiers )
{
  if ( this->private_->data_layer_ ) { return false; }

  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->viewer_ = viewer;

    if ( !this->valid_target_state_->get() ) {return false; }
  }

  if ( this->private_->viewer_->is_volume_view() )
  {
    return false;
  }

  if ( this->private_->painting_ )
  {
    // stop painting if the left mouse button was released, stop erasing if the right button is released
    if ( ( button == Core::MouseButton::RIGHT_BUTTON_E ) ||
         ( button == Core::MouseButton::LEFT_BUTTON_E ) )
    {
      {
        {
          PaintInfo paint_info;

          this->private_->setup_paint_info( paint_info, mouse_history.previous_.x_,
                                            mouse_history.previous_.y_, mouse_history.current_.x_,
                                            mouse_history.current_.y_ );

          GrowCutTool::HandlePaint( this->shared_from_this(), paint_info );

          paint_info.x_ = this->private_->x_;
          paint_info.y_ = this->private_->y_;

          paint_info.target_layer_id_ = this->private_->target_layer_id_;

          // Create a record on the action queue
          ActionPaint::Dispatch( Core::Interface::GetMouseActionContext(), paint_info );
        }

        GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
        this->private_->stop_painting();
      }
      return true;
    }
  }

  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->brush_visible_ = true;
  }
  this->private_->update_same_mode_viewers();
  return false;
}

bool GrowCutTool::handle_wheel( ViewerHandle viewer, int delta,
                                int x, int y, int buttons, int modifiers )
{
  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->viewer_ = viewer;
  }

  if ( this->private_->viewer_->is_volume_view() )
  {
    return false;
  }

  if ( ( modifiers & ~Core::KeyModifier::KEYPAD_MODIFIER_E ) ==
       Core::KeyModifier::NO_MODIFIER_E &&
       !this->private_->painting_ )
  {
    int min_radius, max_radius;
    this->brush_radius_state_->get_range( min_radius, max_radius );
    int brush_radius = this->brush_radius_state_->get() + delta;
    if ( brush_radius >= min_radius && brush_radius <= max_radius )
    {
      Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(),
                                 this->brush_radius_state_, brush_radius );
    }
    return true;
  }

  if ( this->private_->painting_ )
  {
    return true;
  }

  return false;
}

bool GrowCutTool::handle_update_cursor( ViewerHandle viewer )
{
  {
    GrowCutToolPrivate::lock_type lock( this->private_->get_mutex() );
    // Only need to update the cursor for the viewer that currently have focus
    if ( this->private_->viewer_ != viewer || this->private_->painting_ )
    {
      return false;
    }

    if ( this->private_->viewer_->is_volume_view() )
    {
      return false;
    }

    if ( !this->valid_target_state_->get() ) {return false; }
  }

  bool cursor_invisible;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    cursor_invisible = PreferencesManager::Instance()->paint_cursor_invisibility_state_->get();
  }

  if ( viewer->is_volume_view() )
  {
    viewer->set_cursor( Core::CursorShape::ARROW_E );
  }
  else
  {
    if ( this->private_->check_paintable( viewer ) && cursor_invisible )
    {
      viewer->set_cursor( Core::CursorShape::BLANK_E );
    }
    else
    {
      viewer->set_cursor( Core::CursorShape::CROSS_E );
    }
  }

  return true;
}

bool GrowCutTool::post_load_states()
{
  return true;
}

bool GrowCutTool::has_2d_visual()
{
  return true;
}

bool GrowCutTool::paint( const PaintInfo& paint_info )
{
  int paint_count = 0;

  this->brush_radius_state_->set( paint_info.brush_radius_ );
  {
    Core::MaskVolumeSlice::lock_type cache_lock( paint_info.target_slice_->get_mutex() );

    this->private_->paint( paint_info, paint_info.x_[ 0 ], paint_info.y_[ 0 ], paint_count );
    for ( size_t j = 0; j < paint_info.x_.size() - 1; j++ )
    {
      this->private_->interpolated_paint( paint_info,
                                          paint_info.x_[ j ], paint_info.y_[ j ],
                                          paint_info.x_[ j + 1 ], paint_info.y_[ j + 1 ], paint_count );
    }
  }

  if ( paint_count > 0 )
  {
    paint_info.target_slice_->cache_updated_signal_();
    return true;
  }

  return false;
}

void GrowCutTool::HandlePaint( GrowCutToolWeakHandle tool, const PaintInfo info )
{
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &GrowCutTool::HandlePaint, tool,
                                               info ) );

    return;
  }

  GrowCutToolHandle tool_handle = tool.lock();
  if ( tool_handle ) { tool_handle->paint( info ); }
}

bool GrowCutTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  if ( this->private_->painting_ )
  {
    return true;
  }

  if ( key == Core::Key::KEY_R_E )
  {
    this->run_growcut( Core::Interface::GetKeyboardActionContext(), false );
    return true;
  }
  else if ( key == Core::Key::KEY_S_E )
  {
    this->run_smoothing( Core::Interface::GetKeyboardActionContext() );
    return true;
  }
  else if ( key == Core::Key::KEY_B_E )
  {
    this->run_growcut( Core::Interface::GetKeyboardActionContext(), true );
    return true;
  }
  else if ( key == Core::Key::KEY_BRACKETLEFT_E )
  {
    this->handle_wheel( viewer, -1, 0, 0, 0, 0 );
    return true;
  }
  else if ( key == Core::Key::KEY_BRACKETRIGHT_E )
  {
    this->handle_wheel( viewer, +1, 0, 0, 0, 0 );
    return true;
  }

  return SingleTargetTool::handle_key_press( viewer, key, modifiers );
}

void GrowCutTool::initialize_layers( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  LayerHandle target_layer_ = LayerManager::FindLayer( this->target_layer_state_->get() );

  ActionGrowCutInitialize::Dispatch( context, target_layer_->get_layer_group()->get_group_id(),
                                     boost::bind( &GrowCutToolPrivate::handle_layers_initialized,
                                                  this->private_,
                                                  _1, _2, _3, GrowCutToolWeakHandle( this->shared_from_this() ) ) );
}

void GrowCutTool::run_growcut( Core::ActionContextHandle context, bool smoothing )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if ( !this->private_->grow_cutter_ )
  {
    this->private_->grow_cutter_ = GrowCutterHandle( new GrowCutter() );
  }

  ActionGrowCut::Dispatch( context,
                           this->target_layer_state_->get(),
                           this->foreground_layer_state_->get(),
                           this->background_layer_state_->get(),
                           this->output_layer_state_->get(),
                           smoothing,
                           this->private_->grow_cutter_ );
}

void GrowCutTool::run_smoothing( Core::ActionContextHandle context )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ActionGrowCutPostProcess::Dispatch( context, this->output_layer_state_->get(), true );
}

void GrowCutTool::handle_layer_data_changed()
{
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    if ( this->private_->signal_block_count_ > 0 )
    {
      return;
    }
  }
  this->private_->reinitialize_growcut();
}
} // end namespace Seg3D
