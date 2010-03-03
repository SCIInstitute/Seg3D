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

#include <Application/Renderer/RenderResources.h>
#include <GL/glew.h>

namespace Seg3D
{

RenderResources::RenderResources() :
  gl_initialized_( false )
{
}

RenderResources::~RenderResources()
{
}

bool RenderResources::create_render_context( RenderContextHandle& context )
{
  // The context gets setup through the GUI system and is GUI dependent
  // if this function is accessed before the GUI system is setup, something
  // is wrong in the program logic, hence warn the user
  if ( !resources_context_.get() )
  {
    SCI_THROW_LOGICERROR("No render resources were installed to create an opengl context");
  }

  return ( resources_context_->create_render_context( context ) );
}

void RenderResources::install_resources_context( RenderResourcesContextHandle resources_context )
{
  // Check whether we got a proper render context
  if ( !resources_context.get() )
  {
    SCI_THROW_LOGICERROR("Cannot install an empty render resources context");
  }

  resources_context_ = resources_context;
}

bool RenderResources::valid_render_resources()
{
  return ( resources_context_.get() && resources_context_->valid_render_resources() );
}

void RenderResources::init_gl()
{
  if ( !gl_initialized_ )
  {
    boost::unique_lock< mutex_type > lock( this->shared_context_mutex_ );
    if ( !gl_initialized_ )
    {
      glewInit();
      gl_initialized_ = true;
      if ( !GL_VERSION_1_5 )
      {
        SCI_THROW_OPENGLEXCEPTION("Minimum OpenGL version 1.5 required.");
      }
      if ( !GL_EXT_framebuffer_object )
      {
        SCI_THROW_OPENGLEXCEPTION("GL_EXT_framebuffer_object not found.");
      }
    }
  }
}

OpenGLException::OpenGLException( std::string message,
  unsigned int line, const char* file ) :
Utils::Exception(message, line, file)
{
}

OpenGLException::~OpenGLException()
{
}

std::string OpenGLException::what() const
{
  return std::string( "OpenGLException" );
}

} // end namespace Seg3D

