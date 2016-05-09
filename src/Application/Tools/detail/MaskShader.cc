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

#include <Application/Tools/detail/MaskShader.h>

#include <Core/Utils/Log.h>

namespace Seg3D
{

MaskShader::MaskShader() :
  ShaderBase()
{
}

MaskShader::~MaskShader()
{
}

bool MaskShader::get_fragment_shader_source( std::string& source )
{
  const char FRAG_SHADER_SOURCE_C[] =
  {
#include "MaskShader_frag"
  };
  source = std::string( FRAG_SHADER_SOURCE_C );
  return true;
  }

bool MaskShader::post_initialize()
  {
  this->enable();
  this->tex_loc_ = this->get_uniform_location( "tex" );
  this->opacity_loc_ = this->get_uniform_location( "opacity" );
  this->pixel_size_loc_ = this->get_uniform_location( "pixel_size" );
  this->border_width_loc_ = this->get_uniform_location( "border_width" );
  this->color_loc_ = this->get_uniform_location( "color" );
  this->disable();
  return true;
}

void MaskShader::set_texture( int tex_unit )
{
  glUniform1i( this->tex_loc_, tex_unit );
}

void MaskShader::set_opacity( float opacity )
{
  glUniform1f( this->opacity_loc_, opacity );
}

void MaskShader::set_pixel_size( float width, float height )
{
  glUniform2f( this->pixel_size_loc_, width, height );
}

void MaskShader::set_border_width( int width )
{
  glUniform1i( this->border_width_loc_, width );
}

void MaskShader::set_color( float r, float g, float b )
{
  glUniform3f( this->color_loc_, r, g, b );
}

} // end namespace Seg3D
