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
#include <Utils/RenderResources/RenderResourcesEventHandler.h>

namespace Utils
{

RenderResourcesEventHandler::RenderResourcesEventHandler()
{
  RenderResources::Instance()->create_render_context( context_ );
  start_eventhandler();
}

RenderResourcesEventHandler::~RenderResourcesEventHandler()
{
}

void RenderResourcesEventHandler::initialize_eventhandler()
{
  context_->make_current();
}

void RenderResourcesEventHandler::delete_texture( unsigned int texture_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResourcesEventHandler::delete_texture, this, texture_id ) );
    return;
  }
  
  RenderResources::lock_type lock( RenderResources::GetMutex() );
  glDeleteTextures( 1, &texture_id );
}

void RenderResourcesEventHandler::delete_renderbuffer( unsigned int renderbuffer_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResourcesEventHandler::delete_renderbuffer, this, 
      renderbuffer_id ) );
    return;
  }
  
  RenderResources::lock_type lock( RenderResources::GetMutex() );
  glDeleteRenderbuffersEXT( 1, &renderbuffer_id );
}

void RenderResourcesEventHandler::delete_buffer_object( unsigned int buffer_object_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResourcesEventHandler::delete_buffer_object, this, 
      buffer_object_id ) );
    return;
  }
  
  RenderResources::lock_type lock( RenderResources::GetMutex() );
  glDeleteBuffers( 1, &buffer_object_id );
}

void RenderResourcesEventHandler::delete_framebuffer_object( unsigned int framebuffer_object_id )
{
  if ( ! (is_eventhandler_thread() ) )
  {
    post_event( boost::bind( &RenderResourcesEventHandler::delete_framebuffer_object, this, 
      framebuffer_object_id ) );
    return;
  }
  
  RenderResources::lock_type lock( RenderResources::GetMutex() );
  glDeleteFramebuffersEXT( 1, &framebuffer_object_id );
}

} // end namespace Utils

