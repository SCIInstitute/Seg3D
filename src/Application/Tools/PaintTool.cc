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

#include <Core/Application/Application.h>
#include <Core/Viewer/Mouse.h>
#include <Core/Graphics/Texture.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Utils/ScopedCounter.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Renderer/SliceShader.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PaintTool.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Declaration and implementation of class PaintToolPrivate
//////////////////////////////////////////////////////////////////////////

class PaintToolPrivate : public Core::RecursiveLockable
{
public:
  // BUILD_BRUSH_MASK:
  // Build mask for the brush which can later be used for painting.
  void build_brush_mask();

  void initialize();
  void upload_mask_texture();
  void handle_brush_radius_changed();

  //PAINT:
  // Paint on the target layer with the brush centered at (x0, y0).
  void paint( int xc, int yc, int& paint_count );

  void interpolated_paint( int x0, int y0, int x1, int y1, int& paint_count );
  void paint_range( int x0, int y0, int x1, int y1 );

  bool initialized_;
  bool brush_mask_changed_;
  bool painting_;
  bool erase_;
  Core::MaskVolumeSliceHandle target_slice_;
  ViewerHandle viewer_;

  PaintTool* paint_tool_;
  int center_x_;
  int center_y_;
  std::vector< unsigned char > brush_mask_;
  Core::Texture2DHandle brush_tex_;
  SliceShaderHandle shader_;

  const static int INVALID_VIEWER_C;
};

const int PaintToolPrivate::INVALID_VIEWER_C = -1;

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

  while( x < y )
  {
    if( f >= 0 ) 
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

// Flood fill the buffer with the given value, starting from (x, y). The algorithm stops
// when it gets to the boundary of the buffer, or when it arrives at a point where the
// value equals to the given value.
static void FloodFill( std::vector< unsigned char >& buffer, int x, int y, 
            int dimension, unsigned char value )
{
  if ( x >= dimension || y >= dimension ||
    buffer[ y * dimension + x ] == value )
  {
    return;
  }
  
  buffer[ y * dimension + x ] = value;
  FloodFill( buffer, x + 1, y, dimension, value );
  FloodFill( buffer, x - 1, y, dimension, value );
  FloodFill( buffer, x, y + 1, dimension, value );
  FloodFill( buffer, x, y - 1, dimension, value );
}

void PaintToolPrivate::build_brush_mask()
{
  int radius = this->paint_tool_->brush_radius_state_->get();
  int brush_size = radius * 2 + 1;
  this->brush_mask_.resize( brush_size * brush_size );
  if ( brush_size == 1 )
  {
    this->brush_mask_[ 0 ] = 1;
    return;
  }
  memset( &this->brush_mask_[ 0 ], 0, sizeof( unsigned char ) * this->brush_mask_.size() );
  BresenhamCircle( this->brush_mask_, radius );
  FloodFill( this->brush_mask_, radius, radius, brush_size, 1 );
}

void PaintToolPrivate::initialize()
{
  if ( !this->initialized_ )
  {
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
    this->brush_tex_ = Core::Texture2DHandle( new Core::Texture2D );
    this->brush_tex_->set_mag_filter( GL_NEAREST );
    this->brush_tex_->set_min_filter( GL_NEAREST );

    this->shader_ = SliceShaderHandle( new SliceShader );
    this->shader_->initialize();
    this->shader_->enable();
    this->shader_->set_border_width( 2 );
    this->shader_->set_mask_mode( 1 );
    this->shader_->set_opacity( 1.0f );
    this->shader_->set_slice_texture( 0 );
    this->shader_->set_volume_type( Core::VolumeType::MASK_E );
    this->shader_->disable();

    this->initialized_ = true;
  }
}

void PaintToolPrivate::upload_mask_texture()
{
  if ( !this->brush_mask_changed_ )
  {
    return;
  }
  
  Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
  int radius = this->paint_tool_->brush_radius_state_->get();
  int brush_size = radius * 2 + 1;
  this->brush_tex_->set_image( brush_size, brush_size, GL_ALPHA, &this->brush_mask_[ 0 ],
    GL_ALPHA, GL_UNSIGNED_BYTE );

  this->brush_mask_changed_ = false;
}

void PaintToolPrivate::handle_brush_radius_changed()
{
  lock_type lock( this->get_mutex() );
  this->build_brush_mask();
  this->brush_mask_changed_ = true;
}

void PaintToolPrivate::paint( int xc, int yc, int& paint_count )
{ 
  int radius = this->paint_tool_->brush_radius_state_->get();
  int brush_size = radius * 2 + 1;
  double xpos, ypos;
  this->viewer_->window_to_world( xc, yc, xpos, ypos );
  int x0, y0;
  this->target_slice_->world_to_index( xpos, ypos, x0, y0 );
  int x_min = x0 - radius;
  int x_max = x0 + radius;
  int y_min = y0 - radius;
  int y_max = y0 + radius;
  if ( x_min >= static_cast< int >( this->target_slice_->nx() ) ||
    x_max < 0 || y_max < 0 ||
    y_min >= static_cast< int >( this->target_slice_->ny() ) )
  {
    return;
  }
  
  // Compute the start and end indices of the region of the brush that 
  // intersects with the mask volume slice.
  size_t x_start = static_cast< size_t >( Core::Max( x_min, 0 ) - x_min );
  size_t x_end = static_cast< size_t >( Core::Min( x_max, static_cast< int >( 
    this->target_slice_->nx() - 1 ) ) - x_min );
  size_t y_start = static_cast< size_t >( Core::Max( y_min, 0 ) - y_min );
  size_t y_end = static_cast< size_t >( Core::Min( y_max, static_cast< int >( 
    this->target_slice_->ny() - 1 ) ) - y_min );

  unsigned char* buffer = this->target_slice_->get_cached_data();
  size_t nx = this->target_slice_->nx();
  size_t ny = this->target_slice_->ny();
  for ( size_t y = y_start; y <= y_end; y++ )
  {
    for ( size_t x = x_start; x <= x_end; x++ )
    {
      if ( this->brush_mask_[ y * brush_size + x ] != 0 )
      {
        if ( this->erase_ )
        {
          buffer[ ( y_min + y ) * nx + x_min + x ] = 0;
        }
        else
        {
          buffer[ ( y_min + y ) * nx + x_min + x ] = 1;
        }
      }
    }
  }

  paint_count++;
}

void PaintToolPrivate::interpolated_paint( int x0, int y0, int x1, int y1, int& paint_count )
{
  int delta_x = Core::Abs( x1 - x0 );
  int delta_y = Core::Abs( y1 - y0 );
  int radius = this->paint_tool_->brush_radius_state_->get();
  // If the distance between the two points are greater than radius, 
  // we need to interpolate between them
  if ( ( radius == 0 && ( delta_x > 1 || delta_y > 1 ) ) ||
    ( radius > 0 && (delta_x > radius || delta_y > radius ) ) )
  {
    int mid_x = Core::Round( ( x0 + x1 ) * 0.5 );
    int mid_y = Core::Round( ( y0 + y1 ) * 0.5 );
    this->interpolated_paint( x0, y0, mid_x, mid_y, paint_count );
    this->interpolated_paint( mid_x, mid_y, x1, y1, paint_count );
  }
  else
  {
    this->paint( x1, y1, paint_count );
  }
}

void PaintToolPrivate::paint_range( int x0, int y0, int x1, int y1 )
{
  //if ( !Core::Application::IsApplicationThread() )
  //{
  //  Core::Application::PostEvent( boost::bind( &PaintToolPrivate::paint_range,
  //    this, x0, y0, x1, y1 ) );
  //  return;
  //}

  PaintToolPrivate::lock_type lock( this->get_mutex() );

  if ( !this->painting_ || !this->viewer_ || !this->target_slice_ )
  {
    // Shouldn't be here
    assert( false );
    return;
  }

  int paint_count = 0;

  {
    Core::MaskVolumeSlice::cache_lock_type cache_lock( this->target_slice_->get_cache_mutex() );
    this->interpolated_paint( x0, y0, x1, y1, paint_count );
  }

  if ( paint_count > 0)
  {
    this->target_slice_->cache_updated_signal_();
  }
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class PaintTool
//////////////////////////////////////////////////////////////////////////

const size_t PaintTool::VERSION_NUMBER_C = 1;

// Register the tool into the tool factory
SCI_REGISTER_TOOL(PaintTool)

PaintTool::PaintTool( const std::string& toolid, bool auto_number ) :
  Tool( toolid, VERSION_NUMBER_C, auto_number ),
  signal_block_count_( 0 ),
  private_( new PaintToolPrivate )
{
  // Need to set ranges and default values for all parameters

  std::vector< LayerIDNamePair > empty_names( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  this->add_state( "target", this->target_layer_state_, 
    Tool::NONE_OPTION_C, empty_names );
  this->add_state( "data_constraint", this->data_constraint_layer_state_, 
    Tool::NONE_OPTION_C, empty_names );
  this->add_state( "mask_constraint", this->mask_constraint_layer_state_,
    Tool::NONE_OPTION_C, empty_names );

  this->add_state( "brush_radius", this->brush_radius_state_, 3, 0, 250, 1 );
  this->add_state( "upper_threshold", this->upper_threshold_state_, 1.0, 00.0, 1.0, 0.01 );
  this->add_state( "lower_threshold", this->lower_threshold_state_, 0.0, 00.0, 1.0, 0.01 );
  this->add_state( "erase", this->erase_state_, false );
  
  this->handle_layers_changed();

  this->add_connection( this->target_layer_state_->value_changed_signal_.connect(
    boost::bind( &PaintTool::update_constraint_options, this ) ) );
  
  this->add_connection ( LayerManager::Instance()->layers_changed_signal_.connect(
    boost::bind( &PaintTool::handle_layers_changed, this ) ) );

  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( active_layer && active_layer->type() == Core::VolumeType::MASK_E )
  {
    this->target_layer_state_->set( active_layer->get_layer_id() );
  }

  this->private_->paint_tool_ = this;
  this->private_->build_brush_mask();
  this->private_->initialized_ = false;
  this->private_->brush_mask_changed_ = true;
  this->private_->painting_ = false;

  this->add_connection( this->brush_radius_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_brush_radius_changed, this->private_ ) ) );
}

PaintTool::~PaintTool()
{
  this->disconnect_all();
}

void PaintTool::handle_layers_changed()
{
  this->update_target_options();
  this->update_constraint_options();
}

void PaintTool::update_target_options()
{
  std::vector< LayerIDNamePair > mask_layer_names;
  mask_layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );
  LayerManager::Instance()->get_layer_names( mask_layer_names, Core::VolumeType::MASK_E );

  {
    Core::ScopedCounter counter( this->signal_block_count_ );
    this->target_layer_state_->set_option_list( mask_layer_names );
  }
}

void PaintTool::update_constraint_options()
{
  if ( this->signal_block_count_ > 0 )
  {
    return;
  }

  std::vector< LayerIDNamePair > mask_layer_names;
  mask_layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  std::vector< LayerIDNamePair > data_layer_names;
  data_layer_names.push_back( std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  if ( this->target_layer_state_->get() != Tool::NONE_OPTION_C )
  {
    LayerHandle target_layer = LayerManager::Instance()->
      get_layer_by_id( this->target_layer_state_->get() );
    if ( !target_layer )
    {
      CORE_THROW_LOGICERROR( std::string( "Layer " ) + 
        this->target_layer_state_->get() + " does not exist" );
    }

    LayerGroupHandle layer_group = target_layer->get_layer_group();
    layer_group->get_layer_names( mask_layer_names, Core::VolumeType::MASK_E,
      target_layer );
    layer_group->get_layer_names( data_layer_names, Core::VolumeType::DATA_E );
  }

  this->data_constraint_layer_state_->set_option_list( data_layer_names );
  this->mask_constraint_layer_state_->set_option_list( mask_layer_names );
}

void PaintTool::activate()
{
}

void PaintTool::deactivate()
{
}

void PaintTool::repaint( size_t viewer_id, const Core::Matrix& proj_mat )
{
  PaintToolPrivate::lock_type lock( this->private_->get_mutex() );

  // Don't draw the tool if the viewer is not the one that the tool is currently in.
  if ( !this->private_->viewer_ || this->private_->viewer_->get_viewer_id() != viewer_id )
  {
    return;
  }
  
  // If no target layer is selected, return
  if ( this->target_layer_state_->get() == Tool::NONE_OPTION_C )
  {
    return;
  }

  LayerHandle target_layer = LayerManager::Instance()->get_layer_by_id( 
    this->target_layer_state_->get() );
  if ( !target_layer )
  {
    CORE_THROW_LOGICERROR( "Layer with ID '" + this->target_layer_state_->get() +
      "' does not exist" );
  }

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  Core::MaskVolumeSliceHandle target_slice = viewer->get_mask_volume_slice( 
    this->target_layer_state_->get() );
  if ( target_slice->out_of_boundary() )
  {
    return;
  }
  
  // Compute the position of the brush in world space
  // NOTE: The size of the brush needs to be extended by half of the voxel size in each
  // direction in order to visually align with the target mask layer.
  int radius = this->brush_radius_state_->get();
  double xpos, ypos;
  viewer->window_to_world( this->private_->center_x_, this->private_->center_y_,
    xpos, ypos );
  int i, j;
  target_slice->world_to_index( xpos, ypos, i, j );
  double voxel_width = ( target_slice->right() - target_slice->left() ) / 
    ( target_slice->nx() - 1 );
  double voxel_height = ( target_slice->top() - target_slice->bottom() ) /
    ( target_slice->ny() - 1 );
  double left = target_slice->left() + ( i - radius - 0.5 ) * voxel_width;
  double right = target_slice->left() + ( i + radius + 0.5 ) * voxel_width;
  double bottom = target_slice->bottom() + ( j - radius - 0.5 ) * voxel_height;
  double top = target_slice->bottom() + ( j + radius + 0.5 ) * voxel_height;

  // Compute the size of the brush in window space
  Core::Vector brush_x( right - left, 0.0, 0.0 );
  brush_x = proj_mat * brush_x;
  double brush_screen_width = brush_x.x() / 2.0 * viewer->get_width();
  double brush_screen_height = ( top - bottom ) / ( right - left ) * brush_screen_width;

  this->private_->initialize();
  this->private_->upload_mask_texture();

  this->private_->shader_->enable();
  this->private_->shader_->set_pixel_size( static_cast< float >( 1.0 / brush_screen_width ), 
    static_cast< float >( 1.0 /brush_screen_height ) );

  unsigned int old_tex_unit = Core::Texture::GetActiveTextureUnit();
  Core::Texture::SetActiveTextureUnit( 0 );
  this->private_->brush_tex_->bind();
  
  MaskLayer* target_mask_layer = static_cast< MaskLayer* >( target_layer.get() );
  Core::Color color = PreferencesManager::Instance()->get_color( 
    target_mask_layer->color_state_->get() );
  this->private_->shader_->set_mask_color( static_cast< float >( color.r() / 255 ), 
    static_cast< float >( color.g() / 255 ), static_cast< float >( color.b() / 255 ) );

  glPushAttrib( GL_TRANSFORM_BIT );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixd( proj_mat.data() );

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
  Core::Texture::SetActiveTextureUnit( old_tex_unit );
  this->private_->shader_->disable();

  glPopMatrix();
  glPopAttrib();
}

bool PaintTool::handle_mouse_enter( size_t viewer_id )
{
  this->private_->viewer_ = ViewerManager::Instance()->get_viewer( viewer_id );
  this->private_->viewer_->set_cursor_visible( false );
  return true;
}

bool PaintTool::handle_mouse_leave( size_t viewer_id )
{
  this->private_->viewer_->set_cursor_visible( true );
  this->private_->viewer_.reset();
  return true;
}

bool PaintTool::handle_mouse_move( const Core::MouseHistory& mouse_history, 
                  int button, int buttons, int modifiers )
{
  this->private_->center_x_ = mouse_history.current_.x_;
  this->private_->center_y_ = mouse_history.current_.y_;

  if ( this->private_->painting_ )
  {
    this->private_->paint_range( mouse_history.previous_.x_, mouse_history.previous_.y_,
      mouse_history.current_.x_, mouse_history.current_.y_ );
    return true;
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E )
  {
    return true;
  }
  
  return false;
}

bool PaintTool::handle_mouse_press( const Core::MouseHistory& mouse_history, 
                   int button, int buttons, int modifiers )
{
  this->private_->center_x_ = mouse_history.current_.x_;
  this->private_->center_y_ = mouse_history.current_.y_;

  if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
    this->target_layer_state_->get() != Tool::NONE_OPTION_C &&
    !this->private_->painting_ )
  {
    if ( button == Core::MouseButton::LEFT_BUTTON_E )
    {
      this->private_->painting_ = true;
      this->private_->erase_ = false;
    }
    else if ( button == Core::MouseButton::RIGHT_BUTTON_E )
    {
      this->private_->painting_ = true;
      this->private_->erase_ = true;
    }

    if ( this->private_->painting_ )
    {
      this->private_->target_slice_ = this->private_->viewer_->get_mask_volume_slice(
        this->target_layer_state_->get() );
      if ( !this->private_->target_slice_ )
      {
        CORE_THROW_LOGICERROR( "Mask layer with ID '" + 
          this->target_layer_state_->get() + "' does not exist" );
      }
      if ( this->private_->target_slice_->out_of_boundary() )
      {
        this->private_->painting_ = false;
        this->private_->target_slice_.reset();
      }
      else
      {
        this->private_->paint_range( this->private_->center_x_, this->private_->center_y_,
          this->private_->center_x_, this->private_->center_y_ );
        return true;
      }
    }
  }

  return this->private_->painting_;
}

bool PaintTool::handle_mouse_release( const Core::MouseHistory& mouse_history, 
                   int button, int buttons, int modifiers )
{
  if ( this->private_->painting_ )
  {
    if ( ( this->private_->erase_ && button == Core::MouseButton::RIGHT_BUTTON_E ) ||
      ( !this->private_->erase_ && button == Core::MouseButton::LEFT_BUTTON_E ) )
    {
      this->private_->painting_ = false;
      this->private_->target_slice_->release_cached_data();
      this->private_->target_slice_.reset();
      return true;
    }
  }
  return false;
}

bool PaintTool::handle_wheel( int delta, int x, int y, int buttons, int modifiers )
{
  if ( modifiers == Core::KeyModifier::CONTROL_MODIFIER_E &&
    !this->private_->painting_ )
  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->brush_radius_state_->set( this->brush_radius_state_->get() + delta );
    return true;
  }

  if ( this->private_->painting_)
  {
    return true;
  }
  
  return false;
}

} // end namespace Seg3D
