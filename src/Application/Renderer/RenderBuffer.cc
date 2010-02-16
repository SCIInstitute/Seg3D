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

#include <Application/Renderer/RenderBuffer.h>

namespace Seg3D {

const unsigned int RenderBuffer::TARGET_ = GL_RENDERBUFFER_EXT;

RenderBuffer::RenderBuffer()
{
  glGenRenderbuffersEXT(1, &id_);
  
  _safe_bind();
  _safe_unbind();
}

RenderBuffer::~RenderBuffer()
{
  glDeleteRenderbuffersEXT(1, &id_);
}

void RenderBuffer::bind()
{
  glBindRenderbufferEXT(TARGET_, id_);
}

void RenderBuffer::set_storage(int width, int height, unsigned int internal_format, int samples)
{
  _safe_bind();
  if (samples > 1)
  {
    glRenderbufferStorageMultisampleEXT(TARGET_, samples, internal_format, width, height);
  }
  else
  {
    glRenderbufferStorageEXT(TARGET_, internal_format, width, height);
  }
  _safe_unbind();
}

void RenderBuffer::unbind()
{
  glBindRenderbufferEXT(TARGET_, 0);
}

void RenderBuffer::_safe_bind()
{
  glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &saved_id_);
  if (static_cast<int>(id_) != saved_id_)
  {
    glBindRenderbufferEXT(TARGET_, id_);
  }
}

void RenderBuffer::_safe_unbind()
{
  if (static_cast<int>(id_) != saved_id_)
  {
    glBindRenderbufferEXT(TARGET_, saved_id_);
  }
}

} // end namespace Seg3D