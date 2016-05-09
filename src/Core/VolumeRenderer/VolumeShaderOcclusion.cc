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

#include <GL/glew.h>

#include <Core/VolumeRenderer/VolumeShaderOcclusion.h>
#include <Core/Utils/Log.h>

namespace Core
{

VolumeShaderOcclusion::VolumeShaderOcclusion() :
  ShaderBase()
{
}

VolumeShaderOcclusion::~VolumeShaderOcclusion()
{
}

bool VolumeShaderOcclusion::get_vertex_shader_source( std::string& source )
{
  const char VERT_SHADER_SOURCE_C[] =
  {
#include "VolumeShaderOcclusion_vert"
  };
  source = std::string( VERT_SHADER_SOURCE_C );
  return true;
  }

bool VolumeShaderOcclusion::get_fragment_shader_source( std::string& source )
  {
  const char FRAG_SHADER_SOURCE_C[] =
  {
#include "VolumeShaderOcclusion_frag"
  };
  source = std::string( FRAG_SHADER_SOURCE_C );
  return true;
}

bool VolumeShaderOcclusion::post_initialize()
{
  this->enable();
  this->tex_bbox_min_loc_ = this->get_uniform_location( "tex_bbox_min" );
  this->tex_bbox_size_loc_ = this->get_uniform_location( "tex_bbox_size" );
  this->texel_size_loc_ = this->get_uniform_location( "texel_size" );
  this->voxel_size_loc_ = this->get_uniform_location( "voxel_size" );
  this->vol_tex_loc_ = this->get_uniform_location( "vol_tex" );
  this->diffuse_lut_loc_ = this->get_uniform_location( "diffuse_lut" );
  this->specular_lut_loc_ = this->get_uniform_location( "specular_lut" );
  this->occlusion_sample_lut_loc_ = this->get_uniform_location( "occlusion_sample_lut" );
  this->occlusion_buffer_loc_ = this->get_uniform_location( "occlusion_buffer" );
  this->num_of_occlusion_samples_loc_ = this->get_uniform_location( "num_of_occlusion_samples" );
  this->occlusion_extent_loc_ = this->get_uniform_location( "occlusion_extent" );
  this->slice_distance_loc_ = this->get_uniform_location( "normalized_slice_distance" );
  this->clip_plane_loc_ = this->get_uniform_location( "clip_plane" );
  this->enable_clip_plane_loc_ = this->get_uniform_location( "enable_clip_plane" );
  this->enable_clipping_loc_ = this->get_uniform_location( "enable_clipping" );
  this->disable();
  return true;
}

void VolumeShaderOcclusion::set_volume_texture( int tex_unit )
{
  glUniform1i( this->vol_tex_loc_, tex_unit );
}

void VolumeShaderOcclusion::set_diffuse_texture( int tex_unit )
{
  glUniform1i( this->diffuse_lut_loc_, tex_unit );
}

void VolumeShaderOcclusion::set_specular_texture( int tex_unit )
{
  glUniform1i( this->specular_lut_loc_, tex_unit );
}

void VolumeShaderOcclusion::set_occlusion_sample_texture( int tex_unit )
{
  glUniform1i( this->occlusion_sample_lut_loc_, tex_unit );
}

void VolumeShaderOcclusion::set_occlusion_buffer_texture( int tex_unit )
{
  glUniform1i( this->occlusion_buffer_loc_, tex_unit );
}

void VolumeShaderOcclusion::set_texture_bbox_min( float x, float y, float z )
{
  glUniform3f( this->tex_bbox_min_loc_, x, y, z );
}

void VolumeShaderOcclusion::set_texture_bbox_size( float x, float y, float z )
{
  glUniform3f( this->tex_bbox_size_loc_, x, y, z );
}

void VolumeShaderOcclusion::set_texel_size( float x, float y, float z )
{
  glUniform3f( this->texel_size_loc_, x, y, z );
}

void VolumeShaderOcclusion::set_voxel_size( float x, float y, float z )
{
  glUniform3f( this->voxel_size_loc_, x, y, z );
}

void VolumeShaderOcclusion::set_slice_distance( float slice_distance )
{
  glUniform1f( this->slice_distance_loc_, slice_distance );
}

void VolumeShaderOcclusion::set_num_of_occlusion_samples( int num_of_samples )
{
  glUniform1i( this->num_of_occlusion_samples_loc_, num_of_samples );
}

void VolumeShaderOcclusion::set_occlusion_extent( float extent )
{
  glUniform1f( this->occlusion_extent_loc_, extent );
}

void VolumeShaderOcclusion::set_clip_plane( const float clip_planes[ 6 ][ 4 ] )
{
  glUniform4fv( this->clip_plane_loc_, 6, &clip_planes[ 0 ][ 0 ] );
}

void VolumeShaderOcclusion::set_enable_clip_plane( const int enabled[ 6 ] )
{
  glUniform1iv( this->enable_clip_plane_loc_, 6, enabled );
}

void VolumeShaderOcclusion::set_enable_clipping( bool enabled )
{
  glUniform1i( this->enable_clipping_loc_, enabled );
}

} // end namespace Seg3D
