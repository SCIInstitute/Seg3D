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

#include <Core/Utils/Log.h>

#include <Application/Renderer/SliceShader.h>


namespace Seg3D
{

SliceShader::SliceShader() :
    ShaderBase(), slice_tex_loc_(0), pattern_tex_loc_(0),
    mask_mode_loc_(0), volume_type_loc_(0), mask_color_loc_(0),
    data_color_loc_(0), opacity_loc_(0), scale_bias_loc_(0),
    border_width_loc_(0), pixel_size_loc_(0), enable_lighting_loc_(0),
    enable_fog_loc_(0), fog_range_loc_(0), texture_clamp_loc_(0),
    pick_color_loc_(0)
{
}

SliceShader::~SliceShader()
{
}

bool SliceShader::get_vertex_shader_source( std::string& source )
{
  const char VERT_SHADER_SOURCE_C[] =
  {
#include "SliceShader_vert"
#include "Lighting_vert"
#include "Fog_vert"
  };
  source = std::string( VERT_SHADER_SOURCE_C );
  return true;
  }

bool SliceShader::get_fragment_shader_source( std::string& source )
  {
  const char FRAG_SHADER_SOURCE_C[] =
  {
#include "SliceShader_frag"
#include "Lighting_frag"
#include "Fog_frag"
  };
  source = std::string( FRAG_SHADER_SOURCE_C );
  return true;
}

bool SliceShader::post_initialize()
{
  this->enable();
  this->slice_tex_loc_ = this->get_uniform_location( "slice_tex" );
  this->pattern_tex_loc_ = this->get_uniform_location( "pattern_tex" );
  this->opacity_loc_ = this->get_uniform_location( "opacity" );
  this->mask_mode_loc_ = this->get_uniform_location( "mask_mode" );
  this->scale_bias_loc_ = this->get_uniform_location( "scale_bias" );
  this->pick_color_loc_ = this->get_uniform_location("pick_color");
  this->pixel_size_loc_ = this->get_uniform_location( "pixel_size" );
  this->border_width_loc_ = this->get_uniform_location( "border_width" );
  this->volume_type_loc_ = this->get_uniform_location( "volume_type" );
  this->mask_color_loc_ = this->get_uniform_location( "mask_color" );
  this->data_color_loc_ = this->get_uniform_location("data_color");
  this->enable_lighting_loc_ = this->get_uniform_location( "enable_lighting" );
  this->enable_fog_loc_ = this->get_uniform_location( "enable_fog" );
  this->fog_range_loc_ = this->get_uniform_location( "fog_range" );
  this->texture_clamp_loc_ = this->get_uniform_location( "texture_clamp" );
  this->disable();

  return true;
}

void SliceShader::set_slice_texture( int tex_unit )
{
  glUniform1i( this->slice_tex_loc_, tex_unit );
}

void SliceShader::set_pattern_texture( int tex_unit )
{
  glUniform1i( this->pattern_tex_loc_, tex_unit );
}

void SliceShader::set_opacity( float opacity )
{
  glUniform1f( this->opacity_loc_, opacity );
}

void SliceShader::set_mask_mode( int mask_mode )
{
  glUniform1i( this->mask_mode_loc_, mask_mode );
}

void SliceShader::set_scale_bias( float scale, float bias )
{
  glUniform2f( this->scale_bias_loc_, scale, bias );
}

void SliceShader::set_pick_color(bool pick_color)
{
	glUniform1i(this->pick_color_loc_, pick_color );
}

void SliceShader::set_pixel_size( float width, float height )
{
  glUniform2f( this->pixel_size_loc_, width, height );
}

void SliceShader::set_border_width( int width )
{
  glUniform1i( this->border_width_loc_, width );
}

void SliceShader::set_volume_type( int volume_type )
{
  glUniform1i( this->volume_type_loc_, volume_type );
}

void SliceShader::set_mask_color( float r, float g, float b )
{
  glUniform3f( this->mask_color_loc_, r, g, b );
}

void SliceShader::set_data_color(float r, float g, float b)
{
	glUniform3f(this->data_color_loc_, r, g, b );
}

void SliceShader::set_lighting( bool enabled )
{
  glUniform1i( this->enable_lighting_loc_, enabled );
}

void SliceShader::set_fog( bool enabled )
{
  glUniform1i( this->enable_fog_loc_, enabled );
}

void SliceShader::set_fog_range( float znear, float zfar )
{
  glUniform2f( this->fog_range_loc_, znear, zfar );
}

void SliceShader::set_texture_clamp( float s_min, float s_max, float t_min, float t_max )
{
  glUniform4f( this->texture_clamp_loc_, s_min, s_max, t_min, t_max );
}

} // end namespace Seg3D
