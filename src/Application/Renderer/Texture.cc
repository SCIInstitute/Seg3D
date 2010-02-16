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

#include <Application/Renderer/Texture.h>

namespace Seg3D {

Texture::Texture()
{
  glGenTextures(1, &texture_id_);
}

Texture::~Texture()
{
  glDeleteTextures(1, &texture_id_);
}

void Texture::enable()
{
  glEnable(target_);
  glBindTexture(target_, texture_id_);
}

void Texture::disable()
{
  glDisable(target_);
}

void Texture::set_parameter(unsigned int param_name, int param_value)
{
  _safe_bind();
  glTexParameteri(target_, param_name, param_value);
  _safe_unbind();
}

void Texture::set_mag_filter(int filter)
{
  _safe_bind();
  glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, filter);
  _safe_unbind();
}

void Texture::set_min_filter(int filter)
{
  _safe_bind();
  glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, filter);
  _safe_unbind();  
}

void Texture::set_wrap_r(int wrap_mode)
{
  _safe_bind();
  glTexParameteri(target_, GL_TEXTURE_WRAP_R, wrap_mode);
  _safe_unbind();
}

void Texture::set_wrap_s(int wrap_mode)
{
  _safe_bind();
  glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrap_mode);
  _safe_unbind();
}

void Texture::set_wrap_t(int wrap_mode)
{
  _safe_bind();
  glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrap_mode);
  _safe_unbind();
}

void Texture::_safe_bind()
{
  glGetIntegerv(query_target_, &saved_id_);
  if (static_cast<int>(texture_id_) != saved_id_)
  {
    glBindTexture(target_, texture_id_);
  }
}

void Texture::_safe_unbind()
{
  if (static_cast<int>(texture_id_) != saved_id_)
  {
    glBindTexture(target_, saved_id_);
  }
}

Texture1D::Texture1D() :
  Texture()
{
  target_ = GL_TEXTURE_1D;
  query_target_ = GL_TEXTURE_BINDING_1D;
  
  set_mag_filter(GL_LINEAR);
  set_min_filter(GL_LINEAR);
  set_wrap_s(GL_CLAMP);
}

void Texture1D::set_image(int width, int height, int depth, int internal_format, const void *pixels,
                                           unsigned int format, unsigned int type, int level)
{
  _safe_bind();
  glTexImage1D(target_, level, internal_format, width, 0, format, type, pixels);
  _safe_unbind();
}

Texture2D::Texture2D() :
  Texture()
{
  target_ = GL_TEXTURE_2D;
  query_target_ = GL_TEXTURE_BINDING_2D;

  set_mag_filter(GL_LINEAR);
  set_min_filter(GL_LINEAR);
  set_wrap_s(GL_CLAMP);
  set_wrap_t(GL_CLAMP);
}

void Texture2D::set_image(int width, int height, int depth, int internal_format, const void *pixels,
                                           unsigned int format, unsigned int type, int level)
{
  _safe_bind();
  glTexImage2D(target_, level, internal_format, width, height, 0, format, type, pixels);
  _safe_unbind();
}

Texture3D::Texture3D() :
  Texture()
{
  target_ = GL_TEXTURE_3D;
  query_target_ = GL_TEXTURE_BINDING_3D;

  set_mag_filter(GL_LINEAR);
  set_min_filter(GL_LINEAR);
  set_wrap_s(GL_CLAMP);
  set_wrap_t(GL_CLAMP);
  set_wrap_r(GL_CLAMP);
}

void Texture3D::set_image(int width, int height, int depth, int internal_format, const void *pixels,
                                           unsigned int format, unsigned int type, int level)
{
  _safe_bind();
  glTexImage3D(target_, level, internal_format, width, height, depth, 0, format, type, pixels);
  _safe_unbind();
}

} // end namespace Seg3D