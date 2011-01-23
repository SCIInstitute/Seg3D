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

#include <Core/VolumeRenderer/VolumeShader.h>
#include <Core/Utils/Log.h>

namespace Core
{

const char* VolumeShader::FRAG_SHADER_SOURCE_C[] =
{
#include "VolumeShader_frag"
#include "Fog_frag"
};

const char* VolumeShader::VERT_SHADER_SOURCE_C[] =
{
#include "VolumeShader_vert"
};

VolumeShader::VolumeShader() :
  valid_( false )
{
}

VolumeShader::~VolumeShader()
{
}

bool VolumeShader::initialize()
{
  this->glsl_frag_shader_ = Core::GLSLShaderHandle( new Core::GLSLFragmentShader );
  this->glsl_frag_shader_->set_source( sizeof( FRAG_SHADER_SOURCE_C ) / sizeof( char* ),
    FRAG_SHADER_SOURCE_C );
  if ( !this->glsl_frag_shader_->compile() )
  {
    std::string error_info = this->glsl_frag_shader_->get_info_log();
    CORE_LOG_ERROR( std::string( "Failed compiling VolumeShader source: \n" ) + error_info );
    this->glsl_frag_shader_.reset();
    return false;
  }

  this->glsl_vert_shader_.reset( new Core::GLSLVertexShader );
  this->glsl_vert_shader_->set_source( sizeof( VERT_SHADER_SOURCE_C ) / sizeof( char* ),
    VERT_SHADER_SOURCE_C );
  if ( !this->glsl_vert_shader_->compile() )
  {
    std::string error_info = this->glsl_vert_shader_->get_info_log();
    CORE_LOG_ERROR( std::string( "Failed compiling VolumeShader source: \n" ) + error_info );
    this->glsl_frag_shader_.reset();
    this->glsl_vert_shader_.reset();
    return false;
  }
  
  this->glsl_prog_ = Core::GLSLProgramHandle( new Core::GLSLProgram );
  this->glsl_prog_->attach_shader( this->glsl_vert_shader_ );
  this->glsl_prog_->attach_shader( this->glsl_frag_shader_ );
  if ( !this->glsl_prog_->link() )
  {
    std::string error_info = this->glsl_prog_->get_info_log();
    CORE_LOG_ERROR( std::string( "Failed linking VolumeShader program: \n" ) + error_info );
    this->glsl_vert_shader_.reset();
    this->glsl_frag_shader_.reset();
    this->glsl_prog_.reset();
    return false;
  }

  this->glsl_prog_->enable();
  this->vol_tex_loc_ = this->glsl_prog_->get_uniform_location( "vol_tex" );
  this->enable_lighting_loc_ = this->glsl_prog_->get_uniform_location( "enable_lighting" );
  this->enable_fog_loc_ = this->glsl_prog_->get_uniform_location( "enable_fog" );
  this->tex_bbox_min_loc_ = this->glsl_prog_->get_uniform_location( "tex_bbox_min" );
  this->tex_bbox_size_loc_ = this->glsl_prog_->get_uniform_location( "tex_bbox_size" );
  this->texel_size_loc_ = this->glsl_prog_->get_uniform_location( "texel_size" );
  this->voxel_size_loc_ = this->glsl_prog_->get_uniform_location( "voxel_size" );
  this->glsl_prog_->disable();

  this->valid_ = true;
  return true;
}

void VolumeShader::enable()
{
  assert( this->valid_ );
  this->glsl_prog_->enable();
}

void VolumeShader::disable()
{
  assert( this->valid_ );
  this->glsl_prog_->disable();
}

void VolumeShader::set_volume_texture( int tex_unit )
{
  glUniform1i( this->vol_tex_loc_, tex_unit );
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

} // end namespace Seg3D