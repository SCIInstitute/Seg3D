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

class RenderResourcesPrivate
{
public:
  // A Handle to resource that generated the contexts
  RenderResourcesContextHandle resources_context_;

  // An GL context for deleting shared objects
  // NOTE: Since objects need to be deleted inside an OpenGL context
  // this context has been specially constructed for deleting objects
  // when their handles go out of scope. As these objects are maintained
  // through the program using handles, the OpenGL context in which they
  // were created may be deleted already. Hence we delete them all in
  // a separate thread using a separate OpenGL context.
  RenderContextHandle delete_context_;

  // Mutex and condition variable to make sure the RenderResources thread has 
  // completed initialization before continuing the main thread.
  boost::mutex thread_mutex_;
  boost::condition_variable thread_condition_variable_;

  bool gl_capable_;
};

RenderResources::RenderResources() :
  private_( new RenderResourcesPrivate )
{
  this->private_->gl_capable_ = false;
}

RenderResources::~RenderResources()
{
}

void RenderResources::initialize_eventhandler()
{
  boost::unique_lock< boost::mutex > lock( this->private_->thread_mutex_ );
  this->private_->delete_context_->make_current();
  this->private_->gl_capable_ = true;

  int err = glewInit();
  if ( err != GLEW_OK )
  {
    this->private_->gl_capable_ = false;
    CORE_LOG_ERROR( "glewInit failed with error code " + Core::ExportToString( err ) );
  }

  // Check OpenGL capabilities
  if ( !GLEW_VERSION_2_0 )
  {
    this->private_->gl_capable_ = false;
    CORE_LOG_ERROR( "OpenGL 2.1 required but not found." );
  }
  if ( !GLEW_EXT_framebuffer_object )
  {
    this->private_->gl_capable_ = false;
    CORE_LOG_ERROR( "GL_EXT_framebuffer_object required but not found." );
  }
  if ( !GLEW_ARB_pixel_buffer_object )
  {
    this->private_->gl_capable_ = false;
    CORE_LOG_ERROR( "GLEW_ARB_pixel_buffer_object required but not found." );
  }

  this->private_->thread_condition_variable_.notify_one();
}

bool RenderResources::create_render_context( RenderContextHandle& context )
{
  lock_type lock( this->get_mutex() );

  // The context gets setup through the GUI system and is GUI dependent
  // if this function is accessed before the GUI system is setup, something
  // is wrong in the program logic, hence warn the user
  if ( ! this->private_->resources_context_.get() )
  {
    CORE_THROW_LOGICERROR(
      "No render resources were installed to create an opengl context" );
  }

  return this->private_->resources_context_->create_render_context( context );
}

RenderContextHandle RenderResources::get_current_context()
{
  return this->private_->resources_context_->get_current_context();
}

void RenderResources::install_resources_context( RenderResourcesContextHandle resources_context )
{
  // Check whether we got a proper render context
  if ( !resources_context.get() )
  {
    CORE_THROW_LOGICERROR("Cannot install an empty render resources context");
  }

  if ( this->private_->resources_context_ )
  {
    CORE_THROW_LOGICERROR( "A RenderResourcesContext has already been installed" );
  }
  
  this->private_->resources_context_ = resources_context;

  // Create GL context for the event handler thread and start it
  this->private_->resources_context_->create_render_context( this->private_->delete_context_ );
  boost::unique_lock< boost::mutex > lock( this->private_->thread_mutex_ );
  this->start_eventhandler();
  this->private_->thread_condition_variable_.wait( lock );
}

bool RenderResources::valid_render_resources()
{
  return ( this->private_->resources_context_ && 
         this->private_->resources_context_->valid_render_resources() && 
         this->private_->delete_context_ &&
         this->private_->gl_capable_ );
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
  CORE_CHECK_OPENGL_ERROR();
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
  CORE_CHECK_OPENGL_ERROR();
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
  CORE_CHECK_OPENGL_ERROR();
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
  CORE_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_program( unsigned int program_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResources::delete_program, this, 
      program_id ) );
    return;
  }

  lock_type lock( RenderResources::GetMutex() );
  glDeleteProgram( program_id );
  CORE_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_shader( unsigned int shader_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResources::delete_shader, this, 
      shader_id ) );
    return;
  }

  lock_type lock( RenderResources::GetMutex() );
  glDeleteShader( shader_id );
  CORE_CHECK_OPENGL_ERROR();
}

RenderResources::mutex_type& RenderResources::GetMutex() 
{ 
  return Instance()->get_mutex(); 
}

} // end namespace Core

