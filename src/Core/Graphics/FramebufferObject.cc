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

#include <Core/Graphics/FramebufferObject.h>
#include <Core/RenderResources/RenderResources.h>


namespace Core
{

const unsigned int FramebufferObject::TARGET_C = GL_FRAMEBUFFER_EXT;

FramebufferObject::FramebufferObject()
{
  glGenFramebuffersEXT( 1, &id_ );

  safe_bind();
  safe_unbind();
}

FramebufferObject::~FramebufferObject()
{
  // NOTE: This object can be owned by any thread, however it needs to be deleted in the
  // right context. This function will do this for us.
  RenderResources::Instance()->delete_framebuffer_object( this->id_ );
}

void FramebufferObject::enable()
{
  glBindFramebufferEXT( TARGET_C, id_ );
}

void FramebufferObject::disable()
{
  glBindFramebufferEXT( TARGET_C, 0 );
}

void FramebufferObject::attach_renderbuffer( RenderbufferHandle renderbuffer, unsigned int attachment )
{
  this->safe_bind();
  glFramebufferRenderbufferEXT( TARGET_C, attachment, renderbuffer->get_target(), renderbuffer->get_id() );
  this->safe_unbind();
}

void FramebufferObject::detach_renderbuffer( RenderbufferHandle renderbuffer, unsigned int attachment )
{
  this->safe_bind();
  glFramebufferRenderbufferEXT( TARGET_C, attachment, renderbuffer->get_target(), 0 );
  this->safe_unbind();
}

void FramebufferObject::attach_texture(TextureHandle texture, unsigned int attachment, int level, int layer)
{
  this->safe_bind();

  unsigned int texture_target = texture->get_target();
  switch (texture_target)
  {
  case GL_TEXTURE_1D:
    glFramebufferTexture1DEXT(TARGET_C, attachment, texture_target, texture->get_id(), level);
    break;
  case GL_TEXTURE_3D:
    glFramebufferTexture3DEXT(TARGET_C, attachment, texture_target, texture->get_id(), level, layer);
    break;
  default:
    glFramebufferTexture2DEXT(TARGET_C, attachment, texture_target, texture->get_id(), level);
  }

  this->safe_unbind();
}

void FramebufferObject::detach_texture( TextureHandle texture, unsigned int attachment )
{
  this->safe_bind();

  unsigned int texture_target = texture->get_target();
  switch (texture_target)
  {
  case GL_TEXTURE_1D:
    glFramebufferTexture1DEXT( TARGET_C, attachment, texture_target, 0, 0 );
    break;
  case GL_TEXTURE_3D:
    glFramebufferTexture3DEXT( TARGET_C, attachment, texture_target, 0, 0, 0);
    break;
  default:
    glFramebufferTexture2DEXT( TARGET_C, attachment, texture_target, 0, 0 );
  }

  this->safe_unbind();
}

bool FramebufferObject::check_status( GLenum* status )
{
  this->safe_bind();
  GLenum result = glCheckFramebufferStatusEXT( TARGET_C );
  this->safe_unbind();
  if ( status != NULL )
  {
    *status = result;
  }

  return ( result == GL_FRAMEBUFFER_COMPLETE_EXT );
}

void FramebufferObject::safe_bind()
{
  glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &saved_id_ );
  if ( static_cast< int > ( id_ ) != saved_id_ )
  {
    glBindFramebufferEXT( TARGET_C, id_ );
  }
}

void FramebufferObject::safe_unbind()
{
  if ( static_cast< int > ( id_ ) != saved_id_ )
  {
    glBindFramebufferEXT( TARGET_C, saved_id_ );
  }
}

} // end namespace Core
