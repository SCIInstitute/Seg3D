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
#include <Core/VolumeRenderer/VolumeRendererSimple.h>
#include <Core/VolumeRenderer/VolumeShaderSimple.h>
#include <Core/Geometry/Algorithm.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class VolumeRendererSimplePrivate
//////////////////////////////////////////////////////////////////////////

class VolumeRendererSimplePrivate
{
public:
  VolumeShaderSimpleHandle volume_shader_;
};

//////////////////////////////////////////////////////////////////////////
// Class VolumeRendererSimple
//////////////////////////////////////////////////////////////////////////

VolumeRendererSimple::VolumeRendererSimple() :
  private_( new VolumeRendererSimplePrivate )
{
}

VolumeRendererSimple::~VolumeRendererSimple()
{
}

void VolumeRendererSimple::initialize()
{
  RenderResources::lock_type lock( RenderResources::GetMutex() );

  this->private_->volume_shader_.reset( new VolumeShaderSimple );
  this->private_->volume_shader_->initialize();
  this->private_->volume_shader_->enable();
  this->private_->volume_shader_->set_volume_texture( 0 );
  this->private_->volume_shader_->set_diffuse_texture( 1 );
  this->private_->volume_shader_->set_specular_texture( 2 );
  this->private_->volume_shader_->disable();
}

void VolumeRendererSimple::render( DataVolumeHandle volume, const VolumeRenderingParam& param )
{
  boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

  std::vector< BrickEntry > brick_queue;
  this->process_volume( volume, param.sampling_rate_, param.view_, 
    param.orthographic_, false, brick_queue );

  size_t num_bricks = brick_queue.size();
  if ( num_bricks == 0 )
  {
    return;
  }
  Vector voxel_size = this->get_voxel_size();
  
  glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT );
  glEnable( GL_DEPTH_TEST );
  glDepthMask( GL_FALSE );
  glDisable( GL_CULL_FACE );

  unsigned int old_tex_unit = Texture::GetActiveTextureUnit();
  TextureHandle diffuse_lut = param.transfer_function_->get_diffuse_lut();
  TextureHandle specular_lut = param.transfer_function_->get_specular_lut();
  Texture::lock_type diffuse_lock( diffuse_lut->get_mutex() );
  Texture::lock_type specular_lock( specular_lut->get_mutex() );
  Texture::SetActiveTextureUnit( 1 );
  diffuse_lut->bind();
  Texture::SetActiveTextureUnit( 2 );
  specular_lut->bind();
  Texture::SetActiveTextureUnit( 0 );
  
  this->private_->volume_shader_->enable();
  this->private_->volume_shader_->set_voxel_size( 
    static_cast< float >( voxel_size[ 0 ] ),
    static_cast< float >( voxel_size[ 1 ] ),
    static_cast< float >( voxel_size[ 2 ] ) );
  this->private_->volume_shader_->set_lighting( param.enable_lighting_ );
  this->private_->volume_shader_->set_fog( param.enable_fog_ );
  this->private_->volume_shader_->set_slice_distance( static_cast< float >(
    this->get_normalized_sample_distance() ) );
  this->private_->volume_shader_->set_fog_range( static_cast< float >( param.znear_ ), 
    static_cast< float >( param.zfar_ ) );
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
    brick_texture->bind();

    glVertexPointer( 3, GL_FLOAT, 0, &polygon_vertices[ 0 ][ 0 ] );
    glMultiDrawArrays( GL_POLYGON, &first_vec[ 0 ], &count_vec[ 0 ], 
      static_cast< GLsizei >( count_vec.size() ) );

    brick_texture->unbind();
  }
  glDisableClientState( GL_VERTEX_ARRAY );

  this->private_->volume_shader_->disable();

  Texture::SetActiveTextureUnit( 1 );
  diffuse_lut->unbind();
  Texture::SetActiveTextureUnit( 2 );
  specular_lut->unbind();
  Texture::SetActiveTextureUnit( old_tex_unit );
  glPopAttrib();

  boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration vr_time = end_time - start_time;
  CORE_LOG_MESSAGE( "Volume rendering finished in " + 
    ExportToString( vr_time.total_milliseconds() ) + " milliseconds." );
}

} // end namespace Core
