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
#include <Core/RenderResources/RenderResources.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( RenderResources );

RenderResources::RenderResources() :
  gl_initialized_( false )
{
}

RenderResources::~RenderResources()
{
}

void RenderResources::initialize_eventhandler()
{
  this->render_context_->make_current();
}

bool RenderResources::create_render_context( RenderContextHandle& context )
{
  // The context gets setup through the GUI system and is GUI dependent
  // if this function is accessed before the GUI system is setup, something
  // is wrong in the program logic, hence warn the user
  if ( !resources_context_.get() )
  {
    CORE_THROW_LOGICERROR("No render resources were installed to create an opengl context");
  }

  return ( resources_context_->create_render_context( context ) );
}

void RenderResources::install_resources_context( RenderResourcesContextHandle resources_context )
{
  // Check whether we got a proper render context
  if ( !resources_context.get() )
  {
    CORE_THROW_LOGICERROR("Cannot install an empty render resources context");
  }

  resources_context_ = resources_context;
}

bool RenderResources::valid_render_resources()
{
  return ( resources_context_.get() && resources_context_->valid_render_resources() );
}

void RenderResources::delete_texture( unsigned int texture_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResources::delete_texture, this, texture_id ) );
    return;
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteTextures( 1, &texture_id );
  SCI_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_renderbuffer( unsigned int renderbuffer_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResources::delete_renderbuffer, this, 
      renderbuffer_id ) );
    return;
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteRenderbuffersEXT( 1, &renderbuffer_id );
  SCI_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_buffer_object( unsigned int buffer_object_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResources::delete_buffer_object, this, 
      buffer_object_id ) );
    return;
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteBuffers( 1, &buffer_object_id );
  SCI_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_framebuffer_object( unsigned int framebuffer_object_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResources::delete_framebuffer_object, this, 
      framebuffer_object_id ) );
    return;
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteFramebuffersEXT( 1, &framebuffer_object_id );
  SCI_CHECK_OPENGL_ERROR();
}

std::string RenderResources::get_current_context_string()
{
  if ( this->resources_context_ )
  {
    return this->resources_context_->get_current_context_string();
  }
  return std::string("");
}

void RenderResources::init_gl()
{
  if ( !this->gl_initialized_ )
  {
    lock_type lock( this->get_mutex() );
    if ( !this->gl_initialized_ )
    {
      glewInit();
      this->gl_initialized_ = true;

      // Create GL context for the event handler thread and start it
      this->create_render_context( this->render_context_ );
      this->start_eventhandler();

      // Check OpenGL capabilities
      if ( !GLEW_VERSION_2_1 )
      {
        CORE_THROW_OPENGLEXCEPTION( "Minimum OpenGL version 2.1 required." );
      }
      if ( !GLEW_EXT_framebuffer_object )
      {
        CORE_THROW_OPENGLEXCEPTION( "GL_EXT_framebuffer_object not found." );
      }
    }
  }
}

} // end namespace Core

