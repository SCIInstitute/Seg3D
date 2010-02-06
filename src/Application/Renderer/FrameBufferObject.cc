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

#include <Application/Renderer/FrameBufferObject.h>

namespace Seg3D {

const unsigned int FrameBufferObject::TARGET_ = GL_FRAMEBUFFER_EXT;

FrameBufferObject::FrameBufferObject()
{
  glGenFramebuffersEXT(1, &id_);
  
  _safe_bind();
  _safe_unbind();
}

FrameBufferObject::~FrameBufferObject()
{
  glDeleteFramebuffersEXT(1, &id_);
}

void FrameBufferObject::enable()
{
  glBindFramebufferEXT(TARGET_, id_);
}

void FrameBufferObject::disable()
{
  glBindFramebufferEXT(TARGET_, 0);
}

void FrameBufferObject::attach_render_buffer(RenderBufferHandle render_buffer, unsigned int attachment)
{
  _safe_bind();
  glFramebufferRenderbufferEXT(TARGET_, attachment, render_buffer->get_target(), render_buffer->get_id());
  _safe_unbind();
}

void FrameBufferObject::attach_texture(TextureHandle texture, unsigned int attachment, int level, int layer)
{
  _safe_bind();
  
  unsigned int texture_target = texture->get_target();
  switch (texture_target)
  {
  case GL_TEXTURE_1D:
    glFramebufferTexture1DEXT(TARGET_, attachment, texture_target, texture->get_id(), level);
    break;
  case GL_TEXTURE_3D:
    glFramebufferTexture3DEXT(TARGET_, attachment, texture_target, texture->get_id(), level, layer);
    break;
  default:
    glFramebufferTexture2DEXT(TARGET_, attachment, texture_target, texture->get_id(), level);
  }
  
  _safe_unbind();
}

void FrameBufferObject::_safe_bind()
{
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &saved_id_);
  if (static_cast<int>(id_) != saved_id_)
  {
    glBindFramebufferEXT(TARGET_, id_);
  }
}

void FrameBufferObject::_safe_unbind()
{
  if (static_cast<int>(id_) != saved_id_)
  {
    glBindFramebufferEXT(TARGET_, saved_id_);
  }
}

} // end namespace Seg3D