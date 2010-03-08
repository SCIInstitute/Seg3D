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

#include <Utils/Graphics/PixelBufferObject.h>

namespace Utils
{

PixelBufferObject::PixelBufferObject( PixelBufferType buffer_type )
{
  if ( buffer_type == PixelBufferType::PACK_BUFFER_E )
  {
    this->target_ = GL_PIXEL_PACK_BUFFER;
    this->query_target_ = GL_PIXEL_PACK_BUFFER_BINDING;
  }
  else
  {
    this->target_ = GL_PIXEL_UNPACK_BUFFER;
    this->query_target_ = GL_PIXEL_UNPACK_BUFFER_BINDING;
  }

  glGenBuffers( 1, &( this->id_ ) );
  this->saved_id_ = 0;

  this->safe_bind();
  this->safe_unbind();
}

PixelBufferObject::~PixelBufferObject()
{
  glDeleteBuffers( 1, &( this->id_ ) );
}

void PixelBufferObject::bind()
{
  glBindBuffer( this->target_, this->id_ );
}

void PixelBufferObject::unbind()
{
  glBindBuffer( this->target_, 0 );
}

void PixelBufferObject::safe_bind()
{
  glGetIntegerv( this->query_target_, &( this->saved_id_ ) );
  if ( this->id_ != this->saved_id_ )
  {
    glBindBuffer( this->target_, this->id_ );
  }
}

void PixelBufferObject::safe_unbind()
{
  if ( this->id_ != this->saved_id_ )
  {
    glBindBuffer( this->target_, this->saved_id_ );
  }
}

void PixelBufferObject::set_buffer_data( GLsizeiptr size, const GLvoid* data, GLenum usage )
{
  this->safe_bind();
  glBufferData( this->target_, size, data, usage );
  this->safe_unbind();
}

void PixelBufferObject::set_buffer_sub_data( GLintptr offset, GLsizeiptr size, const GLvoid* data )
{
  this->safe_bind();
  glBufferSubData( this->target_, offset, size, data );
  this->safe_unbind();
}

void* PixelBufferObject::map_buffer( GLenum access )
{
  this->safe_bind();
  void* buffer = glMapBuffer( this->target_, access );
  this->safe_unbind();
  return buffer;
}

GLboolean PixelBufferObject::unmap_buffer()
{
  this->safe_bind();
  GLboolean result = glUnmapBuffer( this->target_ );
  this->safe_unbind();
  return result;
}

} // end namespace Utils