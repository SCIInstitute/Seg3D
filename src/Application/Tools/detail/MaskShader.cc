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

#include <Application/Tools/detail/MaskShader.h>

#include <Core/Utils/Log.h>

namespace Seg3D
{

const char* MaskShader::FRAG_SHADER_SOURCE_C[] =
{
#include "MaskShader_frag"
};

MaskShader::MaskShader() :
  valid_( false )
{
}

MaskShader::~MaskShader()
{
}

bool MaskShader::initialize()
{
  this->glsl_frag_shader_ = Core::GLSLShaderHandle( new Core::GLSLFragmentShader );
  this->glsl_frag_shader_->set_source( sizeof( FRAG_SHADER_SOURCE_C ) / sizeof( char* ),
    FRAG_SHADER_SOURCE_C );
  if ( !this->glsl_frag_shader_->compile() )
  {
    std::string error_info = this->glsl_frag_shader_->get_info_log();
    CORE_LOG_ERROR( std::string( "Failed compiling MaskShader source: \n" ) + error_info );
    this->glsl_frag_shader_.reset();
    return false;
  }

  this->glsl_prog_ = Core::GLSLProgramHandle( new Core::GLSLProgram );
  this->glsl_prog_->attach_shader( this->glsl_frag_shader_ );
  if ( !this->glsl_prog_->link() )
  {
    std::string error_info = this->glsl_prog_->get_info_log();
    CORE_LOG_ERROR( std::string( "Failed linking MaskShader program: \n" ) + error_info );
    this->glsl_frag_shader_.reset();
    this->glsl_prog_.reset();
    return false;
  }

  this->glsl_prog_->enable();
  this->tex_loc_ = this->glsl_prog_->get_uniform_location( "tex" );
  this->opacity_loc_ = this->glsl_prog_->get_uniform_location( "opacity" );
  this->pixel_size_loc_ = this->glsl_prog_->get_uniform_location( "pixel_size" );
  this->border_width_loc_ = this->glsl_prog_->get_uniform_location( "border_width" );
  this->color_loc_ = this->glsl_prog_->get_uniform_location( "color" );
  this->glsl_prog_->disable();

  this->valid_ = true;
  return true;
}

void MaskShader::enable()
{
  assert( this->valid_ );
  this->glsl_prog_->enable();
}

void MaskShader::disable()
{
  assert( this->valid_ );
  this->glsl_prog_->disable();
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