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

#include <Application/Renderer/SliceShader.h>

#include <Utils/Core/Log.h>

namespace Seg3D
{

const char* SliceShader::FRAG_SHADER_SOURCE_C[] =
{
#include <Application/Renderer/SliceShaderFrag>
};

SliceShader::SliceShader() :
  valid_( false )
{
}

SliceShader::~SliceShader()
{
}

bool SliceShader::initialize()
{
  this->glsl_frag_shader_ = Utils::GLSLShaderHandle( new Utils::GLSLFragmentShader );
  this->glsl_frag_shader_->set_source( sizeof( FRAG_SHADER_SOURCE_C ) / sizeof( char* ),
    FRAG_SHADER_SOURCE_C );
  if ( !this->glsl_frag_shader_->compile() )
  {
    std::string error_info = this->glsl_frag_shader_->get_info_log();
    SCI_LOG_ERROR( std::string( "Failed compling SliceShader source: \n" ) + error_info );
    this->glsl_frag_shader_.reset();
    return false;
  }

  this->glsl_prog_ = Utils::GLSLProgramHandle( new Utils::GLSLProgram );
  this->glsl_prog_->attach_shader( this->glsl_frag_shader_ );
  if ( !this->glsl_prog_->link() )
  {
    std::string error_info = this->glsl_prog_->get_info_log();
    SCI_LOG_ERROR( std::string( "Failed linking SliceShader program: \n" ) + error_info );
    this->glsl_frag_shader_.reset();
    this->glsl_prog_.reset();
    return false;
  }

  this->glsl_prog_->enable();
  this->tex_loc_ = this->glsl_prog_->get_uniform_location( "tex" );
  this->opacity_loc_ = this->glsl_prog_->get_uniform_location( "opacity" );
  this->mask_mode_loc_ = this->glsl_prog_->get_uniform_location( "mask_mode" );
  this->scale_loc_ = this->glsl_prog_->get_uniform_location( "scale" );
  this->bias_loc_ = this->glsl_prog_->get_uniform_location( "bias" );
  this->glsl_prog_->disable();

  this->valid_ = true;
  return true;
}

void SliceShader::enable()
{
  assert( this->valid_ );
  this->glsl_prog_->enable();
}

void SliceShader::disable()
{
  assert( this->valid_ );
  this->glsl_prog_->disable();
}

void SliceShader::set_texture( int tex_unit )
{
  glUniform1i( this->tex_loc_, tex_unit );
}

void SliceShader::set_opacity( float opacity )
{
  glUniform1f( this->opacity_loc_, opacity );
}

void SliceShader::set_mask_mode( bool mask_mode )
{
  glUniform1i( this->mask_mode_loc_, mask_mode );
}

void SliceShader::set_scale( float scale )
{
  glUniform1f( this->scale_loc_, scale );
}

void SliceShader::set_bias( float bias )
{
  glUniform1f( this->bias_loc_, bias );
}

} // end namespace Seg3D