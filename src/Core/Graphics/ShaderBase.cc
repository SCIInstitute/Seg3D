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

#include <Core/Graphics/GLSLProgram.h>
#include <Core/Graphics/ShaderBase.h>
#include <Core/Utils/Log.h>

namespace Core
{

class ShaderBasePrivate
{
public:
  bool valid_;
  Core::GLSLProgramHandle glsl_prog_;
  Core::GLSLShaderHandle glsl_frag_shader_;
  Core::GLSLShaderHandle glsl_vert_shader_;
};

ShaderBase::ShaderBase() :
  private_( new ShaderBasePrivate )
{
  this->private_->valid_ = false;
}

ShaderBase::~ShaderBase()
{
}

bool ShaderBase::initialize()
{
  this->private_->glsl_prog_.reset( new Core::GLSLProgram );

  std::string vertex_shader_source;
  if ( this->get_vertex_shader_source( vertex_shader_source ) )
  {
    this->private_->glsl_vert_shader_.reset( new Core::GLSLVertexShader );
    this->private_->glsl_vert_shader_->set_source( vertex_shader_source );
    if ( !this->private_->glsl_vert_shader_->compile() )
    {
      std::string error_info = this->private_->glsl_vert_shader_->get_info_log();
      CORE_LOG_ERROR( std::string( "Failed to compile vertex shader: \n" ) + error_info );
#ifdef _WIN32
      assert( false );
#endif
      return false;
    }

    this->private_->glsl_prog_->attach_shader( this->private_->glsl_vert_shader_ );
  }
  
  std::string frag_shader_source;
  if ( this->get_fragment_shader_source( frag_shader_source ) )
  {
    this->private_->glsl_frag_shader_.reset( new Core::GLSLFragmentShader );
    this->private_->glsl_frag_shader_->set_source( frag_shader_source );
    if ( !this->private_->glsl_frag_shader_->compile() )
    {
      std::string error_info = this->private_->glsl_frag_shader_->get_info_log();
      CORE_LOG_ERROR( std::string( "Failed to compile fragment shader: \n" ) + error_info );
#ifdef _WIN32
      assert( false );
#endif
      return false;
    }

    this->private_->glsl_prog_->attach_shader( this->private_->glsl_frag_shader_ );
  }
  
  if ( !this->pre_link() )
  {
    return false;
  }

  if ( !this->private_->glsl_prog_->link() )
  {
    std::string error_info = this->private_->glsl_prog_->get_info_log();
    CORE_LOG_ERROR( std::string( "Failed to link GLSL program: \n" ) + error_info );
#ifdef _WIN32
    assert( false );
#endif
    return false;
  }

  if ( !this->post_initialize() )
  {
    return false;
  }

  this->private_->valid_ = true;
  return true;
}

void ShaderBase::enable()
{
  if ( this->private_->valid_ )
  {
    this->private_->glsl_prog_->enable();
  }
}

void ShaderBase::disable()
{
  if ( this->private_->valid_ )
  {
    this->private_->glsl_prog_->disable();
  }
}

bool ShaderBase::is_valid()
{
  return this->private_->valid_;
}

bool ShaderBase::get_vertex_shader_source( std::string& source )
{
  source = "";
  return false;
}

bool ShaderBase::get_fragment_shader_source( std::string& source )
{
  source = "";
  return false;
}

bool ShaderBase::pre_link()
{
  return true;
}

bool ShaderBase::post_initialize()
{
  return true;
}

int ShaderBase::get_uniform_location( const char* name )
{
  return this->private_->glsl_prog_->get_uniform_location( name );
}

void ShaderBase::bind_attrib_location( unsigned int index, const char* name )
{
  this->private_->glsl_prog_->bind_attrib_location( index, name );
}

} // end namespace Seg3D
