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

#include <algorithm>
#include <cstdlib>
#include <stack>

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
#include <Application/Tools/PaintTool.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Tools/Actions/ActionPaint.h>
#include <Application/Tools/Actions/ActionFloodFill.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, PaintTool )

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
  void handle_data_constraint_changed();
  void handle_data_cstr_visibility_changed();
  void handle_data_cstr_range_changed();

  bool check_paintable( ViewerHandle viewer );

  bool start_painting();
  void stop_painting();
  void flood_fill( Core::ActionContextHandle context, bool erase, 
    ViewerHandle viewer = ViewerHandle() );

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

  bool initialized_;
  bool brush_mask_changed_;
  bool painting_;
  bool erase_;
  bool brush_visible_;
  bool paintable_;
  bool data_layer_;
  size_t signal_block_count_;

  bool has_data_constraint_;
  bool has_mask_constraint1_;
  bool has_mask_constraint2_;
  double min_val_;
  double max_val_;
  bool negative_data_constraint_;
  bool negative_mask_constraint1_;
  bool negative_mask_constraint2_;

  Core::MaskVolumeSliceHandle target_slice_;
  Core::MaskVolumeSliceHandle mask_constraint1_slice_;
  Core::MaskVolumeSliceHandle mask_constraint2_slice_;
  Core::DataVolumeSliceHandle data_constraint_slice_;
  ViewerHandle viewer_;

  PaintTool* paint_tool_;
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

void PaintToolPrivate::build_brush_mask()
{
  this->radius_ = this->paint_tool_->brush_radius_state_->get();
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

void PaintToolPrivate::initialize()
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
      
    CORE_LOG_SUCCESS("If brush is not tracking with cursor, activate another window and then reactivate the Seg3D window.");
  }
}

void PaintToolPrivate::upload_mask_texture()
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

void PaintToolPrivate::handle_brush_radius_changed()
{
  lock_type lock( this->get_mutex() );
  this->build_brush_mask();
  this->brush_mask_changed_ = true;
  this->update_same_mode_viewers();
}

void PaintToolPrivate::paint( const PaintInfo& paint_info, int xc, int yc, int& paint_count )
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
  Core::DataBlockHandle constraint_data_block;
  Core::MaskDataBlockHandle constraint1_mask_block;
  Core::MaskDataBlockHandle constraint2_mask_block;
  if ( paint_info.data_constraint_slice_ )
  {
    Core::DataVolumeHandle data_volume = boost::dynamic_pointer_cast
      < Core::DataVolume >( paint_info.data_constraint_slice_->get_volume() );
    constraint_data_block = data_volume->get_data_block();
  }
  if ( paint_info.mask_constraint1_slice_ )
  {
    Core::MaskVolumeHandle mask_volume = boost::dynamic_pointer_cast
      < Core::MaskVolume >( paint_info.mask_constraint1_slice_->get_volume() );
    constraint1_mask_block = mask_volume->get_mask_data_block();
  }
  if ( paint_info.mask_constraint2_slice_ )
  {
    Core::MaskVolumeHandle mask_volume = boost::dynamic_pointer_cast
      < Core::MaskVolume >( paint_info.mask_constraint2_slice_->get_volume() );
    constraint2_mask_block = mask_volume->get_mask_data_block();
  }

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

        if ( constraint_data_block )
        {
          double val = constraint_data_block->get_data_at( current_index );
          bool in_range = val >= paint_info.min_val_ && val <= paint_info.max_val_;
          in_range = paint_info.negative_data_constraint_ ? !in_range : in_range;
          if ( !in_range ) continue;
        }

        if ( constraint1_mask_block )
        {
          bool has_mask = constraint1_mask_block->get_mask_at( current_index );
          if ( ( has_mask && paint_info.negative_mask_constraint1_ ) ||
            ( !has_mask && !paint_info.negative_mask_constraint1_ ) )
          {
            continue;
          }
        }

        if ( constraint2_mask_block )
        {
          bool has_mask = constraint2_mask_block->get_mask_at( current_index );
          if ( ( has_mask && paint_info.negative_mask_constraint2_ ) ||
            ( !has_mask && !paint_info.negative_mask_constraint2_ ) )
          {
            continue;
          }
        }

        if ( paint_info.erase_ )
        {
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

void PaintToolPrivate::interpolated_paint( const PaintInfo& paint_info, int x0, int y0, 
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

void PaintToolPrivate::update_same_mode_viewers()
{
  if ( !this->viewer_ || this->viewer_->is_volume_view() )
  {
    return;
  }

  const std::string& view_mode = this->viewer_->view_mode_state_->get();
  ViewerManager::Instance()->update_viewers_overlay( view_mode );
}

void PaintToolPrivate::handle_data_constraint_changed()
{
  if ( this->paint_tool_->show_data_cstr_bound_state_->get() )
  {
    ViewerManager::Instance()->update_2d_viewers_overlay();
  }
  
  if ( this->paint_tool_->data_constraint_layer_state_->get() == Tool::NONE_OPTION_C )
  {
    return;
  }

  const std::string& layer_id = this->paint_tool_->data_constraint_layer_state_->get();
  LayerHandle layer = LayerManager::Instance()->find_layer_by_id( layer_id );
  if ( !layer )
  {
    CORE_THROW_LOGICERROR( "Data layer '" + layer_id + "' does not exist" );
  }
  DataLayer* data_layer = static_cast< DataLayer* >( layer.get() );
  double min_val = data_layer->get_data_volume()->get_data_block()->get_min();
  double max_val = data_layer->get_data_volume()->get_data_block()->get_max();
  this->paint_tool_->lower_threshold_state_->set_range( min_val, max_val );
  this->paint_tool_->upper_threshold_state_->set_range( min_val, max_val );
  double step = ( max_val - min_val ) / 100.0 ;
  this->paint_tool_->lower_threshold_state_->set_step( step );
  this->paint_tool_->upper_threshold_state_->set_step( step );

}

bool PaintToolPrivate::start_painting()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  this->painting_ = true;
  const std::string& target_layer_id = this->paint_tool_->target_layer_state_->get();
  this->target_slice_ = boost::dynamic_pointer_cast< Core::MaskVolumeSlice >( 
    this->viewer_->get_volume_slice( target_layer_id ) );;
  if ( !this->target_slice_ )
  {
    CORE_THROW_LOGICERROR( "Mask layer with ID '" + 
      target_layer_id + "' does not exist" );
  }
  if ( this->target_slice_->out_of_boundary() )
  {
    this->painting_ = false;
    this->target_slice_.reset();
    return false;
  }

  const std::string& mask_constraint1_id = this->paint_tool_->
    mask_constraint1_layer_state_->get();
  const std::string& mask_constraint2_id = this->paint_tool_->
    mask_constraint2_layer_state_->get();
  const std::string& data_constraint_id = this->paint_tool_->
    data_constraint_layer_state_->get();
  if ( mask_constraint1_id != Tool::NONE_OPTION_C )
  {
    this->mask_constraint1_slice_ = boost::dynamic_pointer_cast< Core::MaskVolumeSlice >(
      this->viewer_->get_volume_slice( mask_constraint1_id ) );
  }
  if ( mask_constraint2_id != Tool::NONE_OPTION_C )
  {
    this->mask_constraint2_slice_ = boost::dynamic_pointer_cast< Core::MaskVolumeSlice >(
      this->viewer_->get_volume_slice( mask_constraint2_id ) );
  }
  if ( data_constraint_id != Tool::NONE_OPTION_C )
  {
    this->data_constraint_slice_ = boost::dynamic_pointer_cast< Core::DataVolumeSlice >(
      this->viewer_->get_volume_slice( data_constraint_id ) );
  }

  this->has_mask_constraint1_ = this->mask_constraint1_slice_.get() != 0;
  this->has_mask_constraint2_ = this->mask_constraint2_slice_.get() != 0;
  this->has_data_constraint_ = this->data_constraint_slice_.get() != 0;
  this->min_val_ = this->paint_tool_->lower_threshold_state_->get();
  this->max_val_ = this->paint_tool_->upper_threshold_state_->get();
  this->negative_data_constraint_ = this->paint_tool_->negative_data_constraint_state_->get();
  this->negative_mask_constraint1_ = this->paint_tool_->negative_mask_constraint1_state_->get();
  this->negative_mask_constraint2_ = this->paint_tool_->negative_mask_constraint2_state_->get();

  return true;
}

void PaintToolPrivate::stop_painting()
{
  this->painting_ = false;
  this->target_slice_->release_cached_data();
  this->target_slice_.reset();
  this->mask_constraint1_slice_.reset();
  this->mask_constraint2_slice_.reset();
  this->data_constraint_slice_.reset();
  this->x_.clear();
  this->y_.clear();
}

void PaintToolPrivate::setup_paint_info( PaintInfo& paint_info, int x0, int y0, int x1, int y1 )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  paint_info.target_layer_id_ = this->paint_tool_->target_layer_state_->get();
  paint_info.target_slice_ = this->target_slice_;
  paint_info.data_constraint_layer_id_ = this->paint_tool_->data_constraint_layer_state_->get();
  paint_info.data_constraint_slice_ = this->data_constraint_slice_;
  paint_info.min_val_ = this->paint_tool_->lower_threshold_state_->get();
  paint_info.max_val_ = this->paint_tool_->upper_threshold_state_->get();
  paint_info.negative_data_constraint_ = this->paint_tool_->negative_data_constraint_state_->get();
  paint_info.mask_constraint1_layer_id_ = this->paint_tool_->mask_constraint1_layer_state_->get();
  paint_info.mask_constraint1_slice_ = this->mask_constraint1_slice_;
  paint_info.negative_mask_constraint1_ = this->paint_tool_->negative_mask_constraint1_state_->get();
  paint_info.mask_constraint2_layer_id_ = this->paint_tool_->mask_constraint2_layer_state_->get();
  paint_info.mask_constraint2_slice_ = this->mask_constraint2_slice_;
  paint_info.negative_mask_constraint2_ = this->paint_tool_->negative_mask_constraint2_state_->get();
  paint_info.brush_radius_ = this->paint_tool_->brush_radius_state_->get();
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

void PaintToolPrivate::handle_data_cstr_visibility_changed()
{
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void PaintToolPrivate::handle_data_cstr_range_changed()
{
  if ( !this->paint_tool_->show_data_cstr_bound_state_->get() ||
    this->paint_tool_->data_constraint_layer_state_->get() == Tool::NONE_OPTION_C )
  {
    return;
  }
  
  ViewerManager::Instance()->update_2d_viewers_overlay();
}

void PaintToolPrivate::flood_fill( Core::ActionContextHandle context, 
                  bool erase, ViewerHandle viewer )
{
  FloodFillInfo ff_params;
  ff_params.erase_ = erase;
  
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    ff_params.target_layer_id_ = this->paint_tool_->target_layer_state_->get();
    if ( ff_params.target_layer_id_ == Tool::NONE_OPTION_C )
    {
      return;
    }

    if ( !viewer )
    {
      viewer = ViewerManager::Instance()->get_active_viewer();
    }
    
    if ( viewer->is_volume_view() )
    {
      context->report_error( "Can't flood fill in the volume view." );
      return;
    }

    Core::VolumeSliceHandle vol_slice = viewer->get_volume_slice( ff_params.target_layer_id_ );
    if ( vol_slice->out_of_boundary() )
    {
      context->report_error( "The mask layer is out of boundary in the active viewer." );
      return;
    }

    LayerHandle layer = LayerManager::Instance()->find_layer_by_id( ff_params.target_layer_id_ );
    if ( !layer->is_visible( viewer->get_viewer_id() ) )
    {
      context->report_error( "Layer not visible in the active viewer" );
      return;
    }
    if ( layer->locked_state_->get() )
    {
      context->report_error( "Layer is locked" );
      return;
    }
    
    ff_params.slice_type_ = vol_slice->get_slice_type();
    ff_params.slice_number_ = vol_slice->get_slice_number();
    ff_params.seeds_ = this->paint_tool_->seed_points_state_->get();
    ff_params.data_constraint_layer_id_ = this->paint_tool_->data_constraint_layer_state_->get();
    ff_params.min_val_ = this->paint_tool_->lower_threshold_state_->get();
    ff_params.max_val_ = this->paint_tool_->upper_threshold_state_->get();
    ff_params.negative_data_constraint_ = this->paint_tool_->negative_data_constraint_state_->get();
    ff_params.mask_constraint1_layer_id_ = this->paint_tool_->mask_constraint1_layer_state_->get();
    ff_params.negative_mask_constraint1_ = this->paint_tool_->negative_mask_constraint1_state_->get();
    ff_params.mask_constraint2_layer_id_ = this->paint_tool_->mask_constraint2_layer_state_->get();
    ff_params.negative_mask_constraint2_ = this->paint_tool_->negative_mask_constraint2_state_->get();
  }

  ActionFloodFill::Dispatch( context, ff_params );
}

bool PaintToolPrivate::check_paintable( ViewerHandle viewer )
{
  bool paintable = false;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    std::string layer_id = this->paint_tool_->target_layer_state_->get();
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
    lock_type lock(  this->get_mutex() );
    this->paintable_ = paintable;
  }
  return paintable;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class PaintTool
//////////////////////////////////////////////////////////////////////////

PaintTool::PaintTool( const std::string& toolid ) :
  SeedPointsTool( Core::VolumeType::MASK_E, toolid ),
  private_( new PaintToolPrivate )
{
  this->private_->paint_tool_ = this;
  this->private_->initialized_ = false;
  this->private_->brush_mask_changed_ = true;
  this->private_->painting_ = false;
  this->private_->brush_visible_ = true;
  this->private_->paintable_ = false;
  this->private_->data_layer_ = false;
  this->private_->signal_block_count_ = 0;

  std::vector< LayerIDNamePair > empty_names( 1, 
    std::make_pair( Tool::NONE_OPTION_C, Tool::NONE_OPTION_C ) );

  this->add_state( "data_constraint", this->data_constraint_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_extra_layer_input( this->data_constraint_layer_state_, Core::VolumeType::DATA_E );

  this->add_state( "mask_constraint1", this->mask_constraint1_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_extra_layer_input( this->mask_constraint1_layer_state_, Core::VolumeType::MASK_E );

  this->add_state( "mask_constraint2", this->mask_constraint2_layer_state_, Tool::NONE_OPTION_C, empty_names );
  this->add_extra_layer_input( this->mask_constraint2_layer_state_, Core::VolumeType::MASK_E );

  this->add_state( "negative_data_constraint", this->negative_data_constraint_state_, false );
  this->add_state( "negative_mask_constraint1", this->negative_mask_constraint1_state_, false );
  this->add_state( "negative_mask_constraint2", this->negative_mask_constraint2_state_, false );
  this->add_state( "show_data_constraint_bound", this->show_data_cstr_bound_state_, true );

  this->add_state( "brush_radius", this->brush_radius_state_, 3, 0, 150, 1 );
  double inf = std::numeric_limits< double >::infinity();
  this->add_state( "upper_threshold", this->upper_threshold_state_, inf, -inf, inf, 0.01 );
  this->add_state( "lower_threshold", this->lower_threshold_state_, -inf, -inf, inf, 0.01 );
  this->add_state( "erase", this->erase_state_, false );
  
  this->add_connection( this->data_constraint_layer_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_data_constraint_changed, this->private_.get() ) ) );
  this->add_connection( this->show_data_cstr_bound_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_data_cstr_visibility_changed, this->private_ ) ) );
  this->add_connection( this->lower_threshold_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_data_cstr_range_changed, this->private_ ) ) );
  this->add_connection( this->upper_threshold_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_data_cstr_range_changed, this->private_ ) ) );
  this->add_connection( this->negative_data_constraint_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_data_cstr_range_changed, this->private_ ) ) );

  this->private_->build_brush_mask();

  this->add_connection( this->brush_radius_state_->state_changed_signal_.connect(
    boost::bind( &PaintToolPrivate::handle_brush_radius_changed, this->private_ ) ) );
    
}

PaintTool::~PaintTool()
{
  this->disconnect_all();
}

void PaintTool::redraw( size_t viewer_id, const Core::Matrix& proj_mat,
  int viewer_width, int viewer_height )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( viewer_id );
  ViewerHandle current_viewer;
  double world_x, world_y;
  int radius;
  bool brush_visible = false;
  
  int center_x, center_y;
  
  {
    PaintToolPrivate::lock_type private_lock( this->private_->get_mutex() );
    current_viewer = this->private_->viewer_;
    radius = this->private_->radius_;
    center_x = this->private_->center_x_;
    center_y = this->private_->center_y_;
    
  }
  
  if ( current_viewer && !current_viewer->is_volume_view() )
  {
    current_viewer->window_to_world( center_x, center_y, world_x, world_y );
  }
  
  this->private_->initialize();
  this->private_->upload_mask_texture();
  
  std::string target_layer_id;
  std::string data_constraint_layer_id;
  double min_val, max_val;
  bool negative_data_constraint;
  std::string current_viewer_mode;
  std::string redraw_viewer_mode;
  bool show_data_cstr_bound;
  LayerHandle target_layer;
  bool layer_visible = false;

  {
    Core::StateEngine::lock_type se_lock( Core::StateEngine::GetMutex() );
    if ( current_viewer )
    {
      current_viewer_mode = current_viewer->view_mode_state_->get();
    }
    redraw_viewer_mode = viewer->view_mode_state_->get();
    target_layer_id = this->target_layer_state_->get();

    // If no target layer is selected, return
    if ( target_layer_id == Tool::NONE_OPTION_C )
    {
      return;
    }
    target_layer = LayerManager::Instance()->find_layer_by_id( target_layer_id );
    if ( !target_layer )
    {
      // NOTE: Since the rendering is happening on a different thread, occasionally
      // when a layer is deleted, the state variables of the paint tool may have not
      // been updated yet, and thus the target layer might be invalid. We should just
      // ignore this kind of errors. Log it in case it's caused by something else.
      CORE_LOG_ERROR( "Layer with ID '" + this->target_layer_state_->get() +
        "' does not exist" );
      return;
    }

    layer_visible = target_layer->has_valid_data() && target_layer->is_visible( viewer_id );
    if ( current_viewer )
    {
      brush_visible = layer_visible && target_layer->is_visible( current_viewer->get_viewer_id() ) &&
        !target_layer->locked_state_->get();
    }
    
    data_constraint_layer_id = this->data_constraint_layer_state_->get();
    min_val = this->lower_threshold_state_->get();
    max_val = this->upper_threshold_state_->get();
    negative_data_constraint = this->negative_data_constraint_state_->get();
    show_data_cstr_bound = this->show_data_cstr_bound_state_->get();
  }

  Core::MaskVolumeSliceHandle target_slice = boost::dynamic_pointer_cast
    < Core::MaskVolumeSlice >( viewer->get_volume_slice( target_layer_id ) );
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
  
  MaskLayer* target_mask_layer = static_cast< MaskLayer* >( target_layer.get() );
  Core::Color color = PreferencesManager::Instance()->get_color( 
    target_mask_layer->color_state_->get() );

  if ( data_constraint_layer_id != Tool::NONE_OPTION_C && 
    show_data_cstr_bound && layer_visible )
  {
    Core::DataVolumeSliceHandle data_constraint_slice = boost::dynamic_pointer_cast
      < Core::DataVolumeSlice >( viewer->get_volume_slice( data_constraint_layer_id ) );
    std::vector< unsigned char > data_constraint_mask;
    data_constraint_slice->create_threshold_mask( data_constraint_mask,
      min_val, max_val, negative_data_constraint );
    Core::Texture2DHandle data_constraint_tex;
    {
      Core::RenderResources::lock_type rr_lock( Core::RenderResources::GetMutex() );
      data_constraint_tex.reset( new Core::Texture2D );
      data_constraint_tex->bind();
      data_constraint_tex->set_min_filter( GL_NEAREST );
      data_constraint_tex->set_mag_filter( GL_NEAREST );
      data_constraint_tex->set_image( static_cast< int >( data_constraint_slice->nx() ), 
        static_cast< int >( data_constraint_slice->ny() ), GL_ALPHA, 
        &data_constraint_mask[ 0 ], GL_ALPHA, GL_UNSIGNED_BYTE );
    }

    double left = target_slice->left() - 0.5 * voxel_width;
    double right = target_slice->right() + 0.5 * voxel_width;
    double bottom = target_slice->bottom() - 0.5 * voxel_height;
    double top = target_slice->top() + 0.5 * voxel_height;
    double slice_width = right - left;
    double slice_height = top - bottom;
    Core::Vector slice_x( slice_width, 0.0, 0.0 );
    slice_x = proj_mat * slice_x;
    double slice_screen_width = Core::Abs( slice_x.x() ) / 2.0 * viewer_width;
    double slice_screen_height = slice_height / slice_width * slice_screen_width;
    this->private_->shader_->set_pixel_size( static_cast< float >( 1.0 / slice_screen_width ), 
      static_cast< float >( 1.0 /slice_screen_height ) );
    this->private_->shader_->set_color( static_cast< float >( 1.0 - color.r() / 255 ), 
      static_cast< float >( 1.0 - color.g() / 255 ), static_cast< float >( 1.0 - color.b() / 255 ) );
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
    data_constraint_tex->unbind();
    CORE_CHECK_OPENGL_ERROR();
  }
  
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
      static_cast< float >( 1.0 /brush_screen_height ) );
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
    SeedPointsTool::redraw( viewer_id, proj_mat, viewer_width, viewer_height );
    CORE_CHECK_OPENGL_ERROR();
  }
  
  glFinish();
}

bool PaintTool::handle_mouse_enter( ViewerHandle viewer, int x, int y )
{
  PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->viewer_ = viewer;
  if ( this->private_->viewer_->is_volume_view() ) return false;

  this->private_->brush_visible_ = true;
  this->private_->center_x_ = x;
  this->private_->center_y_ = y;

  return true;
}

bool PaintTool::handle_mouse_leave( ViewerHandle viewer )
{
  PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->brush_visible_ = false;
  this->private_->update_same_mode_viewers();
  this->private_->viewer_.reset();
  if ( this->private_->painting_ )
  {
    this->private_->stop_painting();
  }
  return true;
}

void PaintTool::activate()
{
}

void PaintTool::deactivate()
{
  ViewerManager::Instance()->reset_cursor();
}

bool PaintTool::handle_mouse_move( ViewerHandle viewer, const Core::MouseHistory& mouse_history, 
  int button, int buttons, int modifiers )
{
  if ( this->private_->data_layer_ ) return false;

  // NOTE: This function call is running on the interface thread
  // Hence we need to lock as most of the paint tool runs on the Application thread.
  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
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
    data_constraint_layer = this->data_constraint_layer_state_->get();
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

    PaintTool::HandlePaint( this->shared_from_this(), paint_info );
    
    return true;
  }
  else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E )
  {
    return true;
  }
  
  return false;
}

bool PaintTool::handle_mouse_press( ViewerHandle viewer, const Core::MouseHistory& mouse_history, 
  int button, int buttons, int modifiers )
{
  // if already painting (or erasing), ignore additional mouse presses
  if ( this->private_->painting_ ) 
  {
    return true;
  }
    
  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
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
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->center_x_ = mouse_history.current_.x_;
    this->private_->center_y_ = mouse_history.current_.y_;
  }

  bool paintable = false;
  bool erase = false;
  {
  
    this->private_->data_layer_ = false;
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    erase = this->erase_state_->get();
    if ( this->target_layer_state_->get() != Tool::NONE_OPTION_C &&
      !this->private_->painting_ )
    {
      size_t viewer_id = this->private_->viewer_->get_viewer_id();
      MaskLayerHandle layer = boost::dynamic_pointer_cast< Core::MaskLayer >(
        LayerManager::Instance()->find_layer_by_id( this->target_layer_state_->get() ) );
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
      PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
      this->private_->painting_ = true;
      this->private_->erase_ = erase;
    }
    else if ( modifiers == Core::KeyModifier::NO_MODIFIER_E &&
      button == Core::MouseButton::RIGHT_BUTTON_E )
    {
      PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
      this->private_->painting_ = true;
      this->private_->erase_ = true;
    }

    if ( this->private_->painting_ )
    {
      PaintToolPrivate::lock_type lock( this->private_->get_mutex() );

      if ( this->private_->start_painting() )
      {
        Core::StateEngine::lock_type state_lock( Core::StateEngine::GetMutex() );
        PaintInfo paint_info;
        this->private_->setup_paint_info( paint_info, mouse_history.current_.x_, 
          mouse_history.current_.y_, mouse_history.current_.x_, 
          mouse_history.current_.y_ );

        PaintTool::HandlePaint( this->shared_from_this(), paint_info );
        return true;
      }
    }
  }

  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->brush_visible_ = this->private_->painting_;
  }

  if ( this->private_->painting_ )
  {
    return true;
  }

  if ( paintable )
  {
    return SeedPointsTool::handle_mouse_press( viewer, mouse_history, button, buttons, modifiers );
  }
  
  return false;
}

bool PaintTool::handle_mouse_release( ViewerHandle viewer, const Core::MouseHistory& mouse_history, 
  int button, int buttons, int modifiers )
{
  if ( this->private_->data_layer_ ) return false;

  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->viewer_ = viewer;
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

          PaintTool::HandlePaint( this->shared_from_this(), paint_info );
    
          paint_info.x_ = this->private_->x_;
          paint_info.y_ = this->private_->y_;
          
          // Create a record on the action queue
          ActionPaint::Dispatch( Core::Interface::GetMouseActionContext(), paint_info );
        }

        PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
        this->private_->stop_painting();
      }
      return true;
    }
  }
  
  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
    this->private_->brush_visible_ = true;
  }
  this->private_->update_same_mode_viewers();
  return false;
}

bool PaintTool::handle_wheel( ViewerHandle viewer, int delta, 
               int x, int y, int buttons, int modifiers )
{
  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
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

bool PaintTool::handle_update_cursor( ViewerHandle viewer )
{
  {
    PaintToolPrivate::lock_type lock( this->private_->get_mutex() );
    // Only need to update the cursor for the viewer that currently have focus
    if ( this->private_->viewer_ != viewer || this->private_->painting_ ) 
    {
      return false;
    }
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

bool PaintTool::post_load_states()
{
  return true;
}

bool PaintTool::has_2d_visual()
{
  return true;
}

bool PaintTool::paint( const PaintInfo& paint_info )
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

  if ( paint_count > 0)
  {
    paint_info.target_slice_->cache_updated_signal_();
    return true;
  }

  return false;
}

void PaintTool::HandlePaint( PaintToolWeakHandle tool, const PaintInfo info )
{
  if ( !( Core::Application::IsApplicationThread() ) )
  {
    Core::Application::PostEvent( boost::bind( &PaintTool::HandlePaint, tool,
    info ) );
    
    return;
  }

  PaintToolHandle tool_handle = tool.lock();
  if ( tool_handle ) tool_handle->paint( info );
}


void PaintTool::flood_fill( Core::ActionContextHandle context, bool erase )
{
  this->private_->flood_fill( context, erase );
}

bool PaintTool::handle_key_press( ViewerHandle viewer, int key, int modifiers )
{
  if ( this->private_->painting_ )
  {
    return true;
  }
  
  if ( key == Core::Key::KEY_F_E )
  {
    this->private_->flood_fill( Core::Interface::GetKeyboardActionContext(), false, viewer );
    return true;
  }
  else if ( key == Core::Key::KEY_E_E )
  {
    this->private_->flood_fill( Core::Interface::GetKeyboardActionContext(), true, viewer );
    return true;
  }
  
  return SeedPointsTool::handle_key_press( viewer, key, modifiers );
}

} // end namespace Seg3D
