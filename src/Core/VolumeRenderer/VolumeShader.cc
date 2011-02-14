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

#include <GL/glew.h>

#include <Core/VolumeRenderer/VolumeShader.h>
#include <Core/Utils/Log.h>

namespace Core
{

VolumeShader::VolumeShader() :
  ShaderBase()
{
}

VolumeShader::~VolumeShader()
{
}

bool VolumeShader::get_vertex_shader_source( std::string& source )
{
  const char VERT_SHADER_SOURCE_C[] =
  {
#include "VolumeShader_vert"
#include "Fog_vert"
  };
  source = std::string( VERT_SHADER_SOURCE_C );
  return true;
}

bool VolumeShader::get_fragment_shader_source( std::string& source )
{
  const char FRAG_SHADER_SOURCE_C[] =
  {
#include "VolumeShader_frag"
#include "Fog_frag"
  };
  source = std::string( FRAG_SHADER_SOURCE_C );
  return true;
}

bool VolumeShader::post_initialize()
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
  this->scale_bias_loc_ = this->get_uniform_location( "scale_bias" );
  this->sample_rate_loc_ = this->get_uniform_location( "sample_rate" );
  this->fog_range_loc_ = this->get_uniform_location( "fog_range" );
  this->disable();
  return true;
}

void VolumeShader::set_volume_texture( int tex_unit )
{
  glUniform1i( this->vol_tex_loc_, tex_unit );
}

void VolumeShader::set_diffuse_texture( int tex_unit )
{
  glUniform1i( this->diffuse_lut_loc_, tex_unit );
}

void VolumeShader::set_specular_texture( int tex_unit )
{
  glUniform1i( this->specular_lut_loc_, tex_unit );
}

void VolumeShader::set_lighting( bool enabled )
{
  glUniform1i( this->enable_lighting_loc_, enabled );
}

void VolumeShader::set_fog( bool enabled )
{
  glUniform1i( this->enable_fog_loc_, enabled );
}

void VolumeShader::set_texture_bbox_min( float x, float y, float z )
{
  glUniform3f( this->tex_bbox_min_loc_, x, y, z );
}

void VolumeShader::set_texture_bbox_size( float x, float y, float z )
{
  glUniform3f( this->tex_bbox_size_loc_, x, y, z );
}

void VolumeShader::set_texel_size( float x, float y, float z )
{
  glUniform3f( this->texel_size_loc_, x, y, z );
}

void VolumeShader::set_voxel_size( float x, float y, float z )
{
  glUniform3f( this->voxel_size_loc_, x, y, z );
}

void VolumeShader::set_scale_bias( float scale, float bias )
{
  glUniform2f( this->scale_bias_loc_, scale, bias );
}

void VolumeShader::set_sample_rate( float sample_rate )
{
  glUniform1f( this->sample_rate_loc_, sample_rate );
}

void VolumeShader::set_fog_range( float znear, float zfar )
{
  glUniform2f( this->fog_range_loc_, znear, zfar );
}

} // end namespace Seg3D