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

#include <boost/date_time.hpp>

#include <Core/Math/MathFunctions.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Volume/DataVolumeBrick.h>
#include <Core/VolumeRenderer/VolumeRendererOcclusion.h>
#include <Core/VolumeRenderer/VolumeShaderOcclusion.h>
#include <Core/Geometry/Algorithm.h>
#include <Core/Graphics/FramebufferObject.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class VolumeRendererOcclusionPrivate
//////////////////////////////////////////////////////////////////////////

class VolumeRendererOcclusionPrivate
{
public:
  void resize_fbo();
  void update_disk_sample_lut();

  bool valid_;
  VolumeShaderOcclusionHandle volume_shader_;
  FramebufferObjectHandle fbo_;
  Texture2DHandle eye_buffer_;
  Texture2DHandle occlusion_buffer_[ 2 ];
  int width_;
  int height_;

  Texture1DHandle disk_sample_lut_;
  int num_of_samples_;
  int grid_resolution_;

  int current_occlusion_buffer_;
  int next_occlusion_buffer_;
};

static const GLenum EYE_BUFFER_ATTACHMENT_C = GL_COLOR_ATTACHMENT2_EXT;
static const GLenum OCCLUSION_BUFFER_ATTACHMENT_C[ 2 ] =
{
  GL_COLOR_ATTACHMENT0_EXT,
  GL_COLOR_ATTACHMENT1_EXT
};

void VolumeRendererOcclusionPrivate::resize_fbo()
{
  RenderResources::lock_type lock( RenderResources::GetMutex() );

  int buffer_format = GL_RGBA;
  //if ( GLEW_ARB_texture_float )
  //{
  //  buffer_format = GL_RGBA16F_ARB;
  //}
  //else
  //{
  //  buffer_format = GL_RGBA16;
  //}

  this->eye_buffer_->set_image( this->width_, this->height_, buffer_format );
  this->occlusion_buffer_[ 0 ]->set_image( this->width_, this->height_, buffer_format );
  this->occlusion_buffer_[ 1 ]->set_image( this->width_, this->height_, buffer_format );
  this->fbo_->attach_texture( this->occlusion_buffer_[ 0 ], OCCLUSION_BUFFER_ATTACHMENT_C[ 0 ] );
  this->fbo_->attach_texture( this->occlusion_buffer_[ 1 ], OCCLUSION_BUFFER_ATTACHMENT_C[ 1 ] );
  this->fbo_->attach_texture( this->eye_buffer_, EYE_BUFFER_ATTACHMENT_C );
}

// Sample a disk of radius 1 with a uniform grid. The resulting samples are put into the vector.
static void SampleDisk( const int grid_resolution, std::vector< PointF >& points )
{
  assert( grid_resolution >= 1 );
  const float cell_size = 2.0f / ( grid_resolution + 1 );

  PointF sample_start;

  // If the grid resolution is odd, the grid passes through the center of the disk.
  if ( ( grid_resolution & 1 ) != 0 )
  {
    points.push_back( PointF( 0.0f, 0.0f, 0.0f ) );
    for ( int i = 1; i <= grid_resolution / 2; ++i )
    {
      points.push_back( PointF( cell_size * i, 0.0f, 0.0f ) );
      points.push_back( PointF( -cell_size * i, 0.0f, 0.0f ) );
      points.push_back( PointF( 0.0f, cell_size * i, 0.0f ) );
      points.push_back( PointF( 0.0f, -cell_size * i, 0.0f ) );
    }
    sample_start = PointF( cell_size, cell_size, 0.0f );
  }
  else
  {
    sample_start = PointF( cell_size * 0.5f, cell_size * 0.5f, 0.0f );
  }

  for ( float x = sample_start.x(); x < 1.0f; x += cell_size )
  {
    for ( float y = sample_start.y(); y < 1.0f; y += cell_size )
    {
      if ( x * x + y * y <= 1.0f )
      {
        points.push_back( PointF( x, y, 0.0f ) );
        points.push_back( PointF( -x, y, 0.0f ) );
        points.push_back( PointF( x, -y, 0.0f ) );
        points.push_back( PointF( -x, -y, 0.0f ) );
      }
      else
      {
        break;
      }
    }
  }
}

void VolumeRendererOcclusionPrivate::update_disk_sample_lut()
{
  std::vector< PointF > samples;
  SampleDisk( this->grid_resolution_, samples );
  this->num_of_samples_ = static_cast< int >( samples.size() );

  RenderResources::lock_type lock( RenderResources::GetMutex() );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  this->disk_sample_lut_->set_image( this->num_of_samples_, GL_RGB16F_ARB, &samples[ 0 ], GL_RGB, GL_FLOAT );
}

//////////////////////////////////////////////////////////////////////////
// Class VolumeRendererOcclusion
//////////////////////////////////////////////////////////////////////////

VolumeRendererOcclusion::VolumeRendererOcclusion() :
  private_( new VolumeRendererOcclusionPrivate )
{
  this->private_->width_ = 0;
  this->private_->height_ = 0;
  this->private_->current_occlusion_buffer_ = 0;
  this->private_->next_occlusion_buffer_ = 1;
  this->private_->grid_resolution_ = 0;
  this->private_->num_of_samples_ = 0;
  this->private_->valid_ = false;
}

VolumeRendererOcclusion::~VolumeRendererOcclusion()
{
}

void VolumeRendererOcclusion::initialize()
{
  if ( !GLEW_EXT_draw_buffers2 )
  {
    CORE_LOG_WARNING( "VolumeRendererOcclusion is disabled because "
      "GL_EXT_draw_buffers2 is not found." );
    return;
  }

  RenderResources::lock_type lock( RenderResources::GetMutex() );

  this->private_->fbo_.reset( new FramebufferObject );
  this->private_->eye_buffer_.reset( new Texture2D );
  this->private_->eye_buffer_->set_mag_filter( GL_NEAREST );
  this->private_->eye_buffer_->set_min_filter( GL_NEAREST );
  this->private_->occlusion_buffer_[ 0 ].reset( new Texture2D );
  this->private_->occlusion_buffer_[ 0 ]->set_min_filter( GL_LINEAR );
  this->private_->occlusion_buffer_[ 0 ]->set_mag_filter( GL_LINEAR );
  this->private_->occlusion_buffer_[ 0 ]->set_wrap_s( GL_CLAMP_TO_EDGE );
  this->private_->occlusion_buffer_[ 0 ]->set_wrap_t( GL_CLAMP_TO_EDGE );
  this->private_->occlusion_buffer_[ 1 ].reset( new Texture2D );
  this->private_->occlusion_buffer_[ 1 ]->set_mag_filter( GL_LINEAR );
  this->private_->occlusion_buffer_[ 1 ]->set_min_filter( GL_LINEAR );
  this->private_->occlusion_buffer_[ 1 ]->set_wrap_s( GL_CLAMP_TO_EDGE );
  this->private_->occlusion_buffer_[ 1 ]->set_wrap_t( GL_CLAMP_TO_EDGE );

  this->private_->disk_sample_lut_.reset( new Texture1D );
  this->private_->disk_sample_lut_->set_mag_filter( GL_NEAREST );
  this->private_->disk_sample_lut_->set_min_filter( GL_NEAREST );

  this->private_->volume_shader_.reset( new VolumeShaderOcclusion );
  this->private_->volume_shader_->initialize();
  this->private_->volume_shader_->enable();
  this->private_->volume_shader_->set_volume_texture( 0 );
  this->private_->volume_shader_->set_diffuse_texture( 1 );
  this->private_->volume_shader_->set_occlusion_sample_texture( 2 );
  this->private_->volume_shader_->set_occlusion_buffer_texture( 3 );
  this->private_->volume_shader_->set_specular_texture( 4 );
  this->private_->volume_shader_->disable();

  this->private_->valid_ = true;
}

void VolumeRendererOcclusion::render( DataVolumeHandle volume, const VolumeRenderingParam& param )
{
  if ( !this->private_->valid_ )  return;

  boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

  std::vector< BrickEntry > brick_queue;
  this->process_volume( volume, param.sampling_rate_, param.view_, 
    param.orthographic_, true, brick_queue );
  size_t num_bricks = brick_queue.size();
  if ( num_bricks == 0 )
  {
    return;
  }

  Vector voxel_size = this->get_voxel_size();

  int viewport[ 4 ];
  glGetIntegerv( GL_VIEWPORT, viewport );
  if ( viewport[ 2 ] == 0 || viewport[ 3 ] == 0 ) return;
  if ( viewport[ 2 ] != this->private_->width_ || viewport[ 3 ] != this->private_->height_ )
  {
    this->private_->width_ = viewport[ 2 ];
    this->private_->height_ = viewport[ 3 ];
    this->private_->resize_fbo();
  }

  if ( !this->private_->fbo_->check_status() )
  {
    return;
  }

  if ( param.grid_resolution_ != this->private_->grid_resolution_ )
  {
    this->private_->grid_resolution_ = param.grid_resolution_;
    this->private_->update_disk_sample_lut();
  }
  
  glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | 
    GL_VIEWPORT_BIT | GL_TRANSFORM_BIT );
  glPushAttrib( GL_COLOR_BUFFER_BIT );  // Push the draw buffer state
  glDisable( GL_DEPTH_TEST );
  glDepthMask( GL_FALSE );
  glDisable( GL_CULL_FACE );

  // Get the background color
  float bkg_color[ 4 ];
  glGetFloatv( GL_COLOR_CLEAR_VALUE, bkg_color );

  this->private_->fbo_->safe_bind();
  glViewport( 0, 0, this->private_->width_, this->private_->height_ );

  // Initialize eye and occlusion buffers

  // Eye buffer initialized to 0
  glDrawBuffer( EYE_BUFFER_ATTACHMENT_C );
  glClearColor( 0, 0, 0, 0 ); 
  glClear( GL_COLOR_BUFFER_BIT );

  // Occlusion buffers initialized to 1
  glDrawBuffers( 2, OCCLUSION_BUFFER_ATTACHMENT_C );
  glClearColor( 1, 1, 1, 1 ); 
  glClear( GL_COLOR_BUFFER_BIT );


  glBlendFunc( GL_ONE_MINUS_DST_ALPHA, GL_ONE );
  glEnableIndexedEXT( GL_BLEND, 0 );
  glDisableIndexedEXT( GL_BLEND, 1 );
  GLenum render_targets[ 2 ] = { EYE_BUFFER_ATTACHMENT_C, GL_NONE };

  unsigned int old_tex_unit = Texture::GetActiveTextureUnit();
  TextureHandle diffuse_lut = param.transfer_function_->get_diffuse_lut();
  TextureHandle specular_lut = param.transfer_function_->get_specular_lut();
  Texture::lock_type diffuse_lock( diffuse_lut->get_mutex() );
  Texture::lock_type specular_lock( specular_lut->get_mutex() );
  Texture::SetActiveTextureUnit( 1 );
  diffuse_lut->bind();
  Texture::SetActiveTextureUnit( 4 );
  specular_lut->bind();
  Texture::SetActiveTextureUnit( 2 );
  this->private_->disk_sample_lut_->bind();
  
  this->private_->volume_shader_->enable();
  this->private_->volume_shader_->set_voxel_size( 
    static_cast< float >( voxel_size[ 0 ] ),
    static_cast< float >( voxel_size[ 1 ] ),
    static_cast< float >( voxel_size[ 2 ] ) );
  this->private_->volume_shader_->set_slice_distance( static_cast< float >(
    this->get_normalized_sample_distance() ) );
  this->private_->volume_shader_->set_occlusion_extent( static_cast< float >(
    this->get_sample_distance() * Tan( DegreeToRadian( param.occlusion_angle_ ) ) ) );
  this->private_->volume_shader_->set_num_of_occlusion_samples( this->private_->num_of_samples_ );
  this->private_->volume_shader_->set_clip_plane( param.clip_plane_ );
  this->private_->volume_shader_->set_enable_clip_plane( param.enable_clip_plane_ );
  this->private_->volume_shader_->set_enable_clipping( param.enable_clipping_ );

  glEnableClientState( GL_VERTEX_ARRAY );
  for ( size_t i = 0; i < num_bricks; ++i )
  {
    std::vector< PointF > polygon_vertices;
    std::vector< int > first_vec, count_vec;
    DataVolumeBrickHandle brick = brick_queue[ i ].brick_;
    this->slice_brick( brick, polygon_vertices, first_vec, count_vec );

    BBox texture_bbox = brick->get_texture_bbox();
    Texture3DHandle brick_texture = brick->get_texture();
    VectorF texel_size( brick->get_texel_size() );
    VectorF texture_size( texture_bbox.diagonal() );
    this->private_->volume_shader_->set_texture_bbox_min( static_cast< float >( texture_bbox.min().x() ),
      static_cast< float >( texture_bbox.min().y() ), static_cast< float >( texture_bbox.min().z() ) );
    this->private_->volume_shader_->set_texture_bbox_size( texture_size[ 0 ], texture_size[ 1 ], texture_size[ 2 ] );
    this->private_->volume_shader_->set_texel_size( texel_size[ 0 ], texel_size[ 1 ], texel_size[ 2 ] );

    Texture::lock_type tex_lock( brick_texture->get_mutex() );

    Texture::SetActiveTextureUnit( 0 );
    brick_texture->bind();
    Texture::SetActiveTextureUnit( 3 );

    glVertexPointer( 3, GL_FLOAT, 0, &polygon_vertices[ 0 ][ 0 ] );
    for ( size_t j = 0; j < count_vec.size(); ++j )
    {
      render_targets[ 1 ] = OCCLUSION_BUFFER_ATTACHMENT_C[ this->private_->next_occlusion_buffer_ ];
      glDrawBuffers( 2, render_targets );
      this->private_->occlusion_buffer_[ this->private_->current_occlusion_buffer_ ]->bind();
      glDrawArrays( GL_POLYGON, first_vec[ j ], count_vec[ j ] );
      this->private_->occlusion_buffer_[ this->private_->current_occlusion_buffer_ ]->unbind();
      this->private_->next_occlusion_buffer_ = !this->private_->next_occlusion_buffer_;
      this->private_->current_occlusion_buffer_ = !this->private_->current_occlusion_buffer_;
    }

    Texture::SetActiveTextureUnit( 0 );
    brick_texture->unbind();
  }
  glDisableClientState( GL_VERTEX_ARRAY );

  this->private_->volume_shader_->disable();


  Texture::SetActiveTextureUnit( 1 );
  diffuse_lut->unbind();
  Texture::SetActiveTextureUnit( 4 );
  specular_lut->unbind();
  Texture::SetActiveTextureUnit( 2 );
  this->private_->disk_sample_lut_->unbind();
  Texture::SetActiveTextureUnit( old_tex_unit );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, 1.0, 0.0, 1.0 );

  for ( int i = 0; i < 6; ++i )
  {
    glDisable( GL_CLIP_PLANE0 + i );
  }

  // Render the background
  glDrawBuffer( EYE_BUFFER_ATTACHMENT_C );
  glColor4fv( bkg_color );
  glBegin( GL_QUADS );
  glVertex2f( 0.0f, 0.0f );
  glVertex2f( 1.0f, 0.0f );
  glVertex2f( 1.0f, 1.0f );
  glVertex2f( 0.0f, 1.0f );
  glEnd();

  this->private_->fbo_->safe_unbind();

  // Render the final image to the frame buffer
  glPopAttrib();  // Pop the draw buffer state
  glDisable( GL_BLEND );
  glViewport( viewport[ 0 ], viewport[ 1 ], viewport[ 2 ], viewport[ 3 ] );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  this->private_->eye_buffer_->enable();
  glBegin( GL_QUADS );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( 1.0f, 0.0f );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( 1.0f, 1.0f );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, 1.0f );
  glEnd();
  this->private_->eye_buffer_->disable();

  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  glPopAttrib();  // Pop all the saved states

  boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration vr_time = end_time - start_time;
  CORE_LOG_MESSAGE( "Volume rendering finished in " + 
    ExportToString( vr_time.total_milliseconds() ) + " milliseconds." );
}

} // end namespace Core
