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

#include <Application/Renderer/IsosurfaceShader.h>

namespace Seg3D
{

IsosurfaceShader::IsosurfaceShader() :
  ShaderBase()
{
}

IsosurfaceShader::~IsosurfaceShader()
{
}

bool IsosurfaceShader::get_vertex_shader_source( std::string& source )
{
  const char VERT_SHADER_SOURCE_C[] =
  {
#include "IsosurfaceShader_vert"
#include "Lighting_vert"
#include "Fog_vert"
  };
  source = std::string( VERT_SHADER_SOURCE_C );
  return true;
  }

bool IsosurfaceShader::get_fragment_shader_source( std::string& source )
  {
  const char FRAG_SHADER_SOURCE_C[] =
  {
#include "IsosurfaceShader_frag"
#include "Lighting_frag"
#include "Fog_frag"
  };
  source = std::string( FRAG_SHADER_SOURCE_C );
  return true;
}

bool IsosurfaceShader::pre_link()
{
  this->bind_attrib_location( 1, "value" );
  return true;
}

bool IsosurfaceShader::post_initialize()
{
  this->enable();
  this->enable_lighting_loc_ = this->get_uniform_location( "enable_lighting" );
  this->use_colormap_loc_ = this->get_uniform_location( "use_colormap" );
  this->colormap_loc_ = this->get_uniform_location( "colormap" );
  this->min_val_loc_ = this->get_uniform_location( "min_val" );
  this->val_range_loc_ = this->get_uniform_location( "val_range" );
  this->enable_fog_loc_ = this->get_uniform_location( "enable_fog" );
  this->fog_range_loc_ = this->get_uniform_location( "fog_range" );
  this->opacity_loc_ = this->get_uniform_location( "opacity" );
  this->disable();
  return true;
}

void IsosurfaceShader::set_lighting( bool enabled )
{
  glUniform1i( this->enable_lighting_loc_, enabled );
}

void IsosurfaceShader::set_use_colormap( bool enable )
{
  glUniform1i( this->use_colormap_loc_, enable );
}

void IsosurfaceShader::set_min_val( float min_val )
{
  glUniform1f( this->min_val_loc_, min_val );
}

void IsosurfaceShader::set_val_range( float val_range )
{
  glUniform1f( this->val_range_loc_, val_range );
}

void IsosurfaceShader::set_colormap_texture( int tex_unit )
{
  glUniform1i( this->colormap_loc_, tex_unit );
}

void IsosurfaceShader::set_fog( bool enabled )
{
  glUniform1i( this->enable_fog_loc_, enabled );
}

void IsosurfaceShader::set_fog_range( float znear, float zfar )
{
  glUniform2f( this->fog_range_loc_, znear, zfar );
}
  
void IsosurfaceShader::set_opacity( float opacity )
{
  glUniform1f( this->opacity_loc_, opacity );
}

} // end namespace Seg3D
