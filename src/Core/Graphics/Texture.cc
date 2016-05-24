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

#include <Core/Graphics/Texture.h>
#include <Core/RenderResources/RenderResources.h>

namespace Core
{

Texture::Texture()
{
  glGenTextures( 1, &this->texture_id_ );
  if ( this->texture_id_ == 0 )
  {
    CORE_LOG_ERROR( "Failed to create a new texture object." );
  }
}

Texture::~Texture()
{
  // NOTE: This object can be owned by any thread, however it needs to be deleted in the
  // right context. This function will do this for us.
  RenderResources::Instance()->delete_texture( this->texture_id_ );
}

void Texture::enable()
{
  glEnable( this->target_ );
  glBindTexture( this->target_, this->texture_id_ );
}

void Texture::disable()
{
  glDisable( this->target_ );
}

void Texture::bind()
{
  glBindTexture( this->target_, this->texture_id_ );
  CORE_CHECK_OPENGL_ERROR();
}

void Texture::unbind()
{
  glBindTexture( this->target_, 0 );
}
  
void Texture::set_parameter(unsigned int param_name, int param_value)
{
  this->safe_bind();
  glTexParameteri( this->target_, param_name, param_value );
  this->safe_unbind();
}

void Texture::set_mag_filter( int filter )
{
  this->safe_bind();
  glTexParameteri( this->target_, GL_TEXTURE_MAG_FILTER, filter );
  this->safe_unbind();
}

void Texture::set_min_filter( int filter )
{
  this->safe_bind();
  glTexParameteri( this->target_, GL_TEXTURE_MIN_FILTER, filter );
  this->safe_unbind();
}

void Texture::set_wrap_r( int wrap_mode )
{
  this->safe_bind();
  glTexParameteri( this->target_, GL_TEXTURE_WRAP_R, wrap_mode );
  this->safe_unbind();
}

void Texture::set_wrap_s( int wrap_mode )
{
  this->safe_bind();
  glTexParameteri( this->target_, GL_TEXTURE_WRAP_S, wrap_mode );
  this->safe_unbind();
}

void Texture::set_wrap_t( int wrap_mode )
{
  this->safe_bind();
  glTexParameteri( this->target_, GL_TEXTURE_WRAP_T, wrap_mode );
  this->safe_unbind();
}

void Texture::safe_bind()
{
  glGetIntegerv( this->query_target_, &this->saved_id_ );
  if ( static_cast< int > ( this->texture_id_ ) != this->saved_id_ )
  {
    glBindTexture( this->target_, this->texture_id_ );
  }
}

void Texture::safe_unbind()
{
  if ( static_cast< int > ( this->texture_id_ ) != this->saved_id_ )
  {
    glBindTexture( this->target_, this->saved_id_ );
  }
}

unsigned int Texture::GetActiveTextureUnit()
{
  int tex_unit;
  glGetIntegerv( GL_ACTIVE_TEXTURE, &tex_unit );
  return static_cast< unsigned int >( tex_unit - GL_TEXTURE0 );
}

void Texture::SetActiveTextureUnit( unsigned int unit )
{
  glActiveTexture( GL_TEXTURE0 + unit );
}

void Texture::SetClientActiveTextureUnit( unsigned int unit )
{
  glClientActiveTexture( GL_TEXTURE0 + unit );
}

Texture1D::Texture1D() :
  Texture()
{
  this->target_ = GL_TEXTURE_1D;
  this->query_target_ = GL_TEXTURE_BINDING_1D;

  set_mag_filter( GL_LINEAR );
  set_min_filter( GL_LINEAR );
  set_wrap_s( GL_CLAMP );
}

void Texture1D::set_image( int width, int internal_format, const void *pixels, 
              unsigned int format, unsigned int type, int level )
{
  this->safe_bind();
  glTexImage1D( this->target_, level, internal_format, width, 0, format, type, pixels );
  this->safe_unbind();
  CORE_CHECK_OPENGL_ERROR();
}

void Texture1D::set_sub_image( int xoffset, int width, const void* data, 
                unsigned int format, unsigned int type, int level )
{
  this->safe_bind();
  glTexSubImage1D( this->target_, level, xoffset, width, format, type, data );
  this->safe_unbind();
  CORE_CHECK_OPENGL_ERROR();
}

Texture2D::Texture2D() :
  Texture()
{
  this->target_ = GL_TEXTURE_2D;
  this->query_target_ = GL_TEXTURE_BINDING_2D;

  set_mag_filter( GL_LINEAR );
  set_min_filter( GL_LINEAR );
  set_wrap_s( GL_CLAMP );
  set_wrap_t( GL_CLAMP );
}

void Texture2D::set_image(int width, int height, int internal_format, const void *pixels,
                unsigned int format, unsigned int type, int level)
{
  this->safe_bind();
  glTexImage2D(target_, level, internal_format, width, height, 0, format, type, pixels);
  this->safe_unbind();
  CORE_CHECK_OPENGL_ERROR();
}

void Texture2D::set_sub_image( int xoffset, int yoffset, int width, int height, 
                const void* data, unsigned int format, unsigned int type, int level /* = 0 */ )
{
  this->safe_bind();
  glTexSubImage2D( this->target_, level, xoffset, yoffset, width, height, format, type, data );
  this->safe_unbind();
  CORE_CHECK_OPENGL_ERROR();
}

Texture3D::Texture3D() :
  Texture()
{
  this->target_ = GL_TEXTURE_3D;
  this->query_target_ = GL_TEXTURE_BINDING_3D;

  set_mag_filter( GL_LINEAR );
  set_min_filter( GL_LINEAR );
  set_wrap_s( GL_CLAMP );
  set_wrap_t( GL_CLAMP );
  set_wrap_r( GL_CLAMP );
}

void Texture3D::set_image(int width, int height, int depth, int internal_format, const void *pixels,
  unsigned int format, unsigned int type, int level)
{
  this->safe_bind();
  glTexImage3D(target_, level, internal_format, width, height, depth, 0, format, type, pixels);
  this->safe_unbind();
  CORE_CHECK_OPENGL_ERROR();
}

void Texture3D::set_sub_image( int xoffset, int yoffset, int zoffset, int width, int height, int depth, 
                const void* data, unsigned int format, unsigned int type, int level /* = 0 */ )
{
  this->safe_bind();
  glTexSubImage3D( this->target_, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data );
  this->safe_unbind();
  CORE_CHECK_OPENGL_ERROR();
}

} // end namespace Core
