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

#include <Core/VolumeRenderer/VolumeShaderSimple.h>
#include <Core/Utils/Log.h>

namespace Core
{

VolumeShaderSimple::VolumeShaderSimple() :
  ShaderBase()
{
}

VolumeShaderSimple::~VolumeShaderSimple()
{
}

bool VolumeShaderSimple::get_vertex_shader_source( std::string& source )
{
  const char VERT_SHADER_SOURCE_C[] =
  {
#include "VolumeShaderSimple_vert"
  };
  source = std::string( VERT_SHADER_SOURCE_C );
  return true;
  }

bool VolumeShaderSimple::get_fragment_shader_source( std::string& source )
  {
  const char FRAG_SHADER_SOURCE_C[] =
  {
#include "VolumeShaderSimple_frag"
  };
  source = std::string( FRAG_SHADER_SOURCE_C );
  return true;
}

bool VolumeShaderSimple::post_initialize()
{
  this->enable();
  this->vol_tex_loc_ = this->get_uniform_location( "vol_tex" );
  this->diffuse_lut_loc_ = this->get_uniform_location( "diffuse_lut" );
  this->specular_lut_loc_ = this->get_uniform_location( "specular_lut" );
  this->enable_lighting_loc_ = this->get_uniform_location( "enable_lighting" );
  this->enable_fog_loc_ = this->get_uniform_location( "enable_fog" );
  this->tex_bbox_min_loc_ = this->get_uniform_location( "tex_bbox_min" );
  this->tex_bbox_size_loc_ = this->get_uniform_location( "tex_bbox_size" );
  this->texel_size_loc_ = this->get_uniform_location( "texel_size" );
  this->voxel_size_loc_ = this->get_uniform_location( "voxel_size" );
  this->slice_distance_loc_ = this->get_uniform_location( "slice_distance" );
  this->fog_range_loc_ = this->get_uniform_location( "fog_range" );
  this->clip_plane_loc_ = this->get_uniform_location( "clip_plane" );
  this->enable_clip_plane_loc_ = this->get_uniform_location( "enable_clip_plane" );
  this->enable_clipping_loc_ = this->get_uniform_location( "enable_clipping" );
  this->disable();
  return true;
}

void VolumeShaderSimple::set_volume_texture( int tex_unit )
{
  glUniform1i( this->vol_tex_loc_, tex_unit );
}

void VolumeShaderSimple::set_diffuse_texture( int tex_unit )
{
  glUniform1i( this->diffuse_lut_loc_, tex_unit );
}

void VolumeShaderSimple::set_specular_texture( int tex_unit )
{
  glUniform1i( this->specular_lut_loc_, tex_unit );
}

void VolumeShaderSimple::set_lighting( bool enabled )
{
  glUniform1i( this->enable_lighting_loc_, enabled );
}

void VolumeShaderSimple::set_fog( bool enabled )
{
  glUniform1i( this->enable_fog_loc_, enabled );
}

void VolumeShaderSimple::set_texture_bbox_min( float x, float y, float z )
{
  glUniform3f( this->tex_bbox_min_loc_, x, y, z );
}

void VolumeShaderSimple::set_texture_bbox_size( float x, float y, float z )
{
  glUniform3f( this->tex_bbox_size_loc_, x, y, z );
}

void VolumeShaderSimple::set_texel_size( float x, float y, float z )
{
  glUniform3f( this->texel_size_loc_, x, y, z );
}

void VolumeShaderSimple::set_voxel_size( float x, float y, float z )
{
  glUniform3f( this->voxel_size_loc_, x, y, z );
}

void VolumeShaderSimple::set_fog_range( float znear, float zfar )
{
  glUniform2f( this->fog_range_loc_, znear, zfar );
}

void VolumeShaderSimple::set_clip_plane( const float clip_planes[ 6 ][ 4 ] )
{
  glUniform4fv( this->clip_plane_loc_, 6, &clip_planes[ 0 ][ 0 ] );
}

void VolumeShaderSimple::set_enable_clip_plane( const int enabled[ 6 ] )
{
  glUniform1iv( this->enable_clip_plane_loc_, 6, enabled );
}

void VolumeShaderSimple::set_enable_clipping( bool enabled )
{
  glUniform1i( this->enable_clipping_loc_, enabled );
}

void VolumeShaderSimple::set_slice_distance( float slice_distance )
{
  glUniform1f( this->slice_distance_loc_, slice_distance );
}

} // end namespace Seg3D
