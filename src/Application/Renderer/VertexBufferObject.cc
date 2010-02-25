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

#include <boost/bind.hpp>

#include <Application/Renderer/VertexBufferObject.h>

namespace Seg3D
{

VertexBufferObject::VertexBufferObject( GLenum target, GLenum array_type ) :
  target_( target ), array_type_( array_type )
{
  assert(target_ == GL_ARRAY_BUFFER || target_ == GL_ELEMENT_ARRAY_BUFFER);
  glGenBuffers( 1, &( this->id_ ) );
  this->saved_id_ = 0;

  if ( target_ == GL_ARRAY_BUFFER )
  {
    this->query_target_ = GL_ARRAY_BUFFER_BINDING;
  }
  else
  {
    this->query_target_ = GL_ELEMENT_ARRAY_BUFFER_BINDING;
  }

  switch( array_type_ )
  {
  case GL_VERTEX_ARRAY:
    this->gl_array_pointer_func4_ = boost::bind( glVertexPointer, _1, _2, _3, _4 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func4, this );
    break;
  case GL_COLOR_ARRAY:
    this->gl_array_pointer_func4_ = boost::bind( glColorPointer, _1, _2, _3, _4 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func4, this );
    break;
  case GL_TEXTURE_COORD_ARRAY:
    this->gl_array_pointer_func4_ = boost::bind( glTexCoordPointer, _1, _2, _3, _4 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func4, this );
    break;
  case GL_SECONDARY_COLOR_ARRAY:
    this->gl_array_pointer_func4_ = boost::bind( glSecondaryColorPointer, _1, _2, _3, _4 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func4, this );
    break;
  case GL_NORMAL_ARRAY:
    this->gl_array_pointer_func3_ = boost::bind( glNormalPointer, _1, _2, _3 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func3, this );
    break;
  case GL_FOG_COORD_ARRAY:
    this->gl_array_pointer_func3_ = boost::bind( glFogCoordPointer, _1, _2, _3 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func3, this );
    break;
  case GL_INDEX_ARRAY:
    this->gl_array_pointer_func3_ = boost::bind( glIndexPointer, _1, _2, _3 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func3, this );
    break;
  case GL_EDGE_FLAG_ARRAY:
    this->gl_array_pointer_func2_ = boost::bind( glEdgeFlagPointer, _1, _2 );
    this->invoke_gl_array_pointer_func_ = boost::bind(
        &VertexBufferObject::invoke_array_pointer_func2, this );
    break;
  default:
    assert(false);
  }

  this->safe_bind();
  this->safe_unbind();
}

VertexBufferObject::~VertexBufferObject( void )
{
  glDeleteBuffers( 1, &( this->id_ ) );
}

void VertexBufferObject::safe_bind()
{
  glGetIntegerv( query_target_, &( this->saved_id_ ) );
  if ( this->id_ != this->saved_id_ )
  {
    glBindBuffer( this->target_, this->id_ );
  }
}

void VertexBufferObject::safe_unbind()
{
  if ( this->id_ != this->saved_id_ )
  {
    glBindBuffer( this->target_, this->saved_id_ );
  }
}

void VertexBufferObject::bind()
{
  glBindBuffer( this->target_, this->id_ );
}

void VertexBufferObject::enable()
{
  glBindBuffer( this->target_, this->id_ );
  if ( this->target_ == GL_ARRAY_BUFFER )
  {
    this->invoke_gl_array_pointer_func_();
    glEnableClientState( this->array_type_ );
  }
}

void VertexBufferObject::invoke_array_pointer_func4()
{
  this->gl_array_pointer_func4_( this->vertex_size_, this->data_type_, 0, 0 );
}

void VertexBufferObject::invoke_array_pointer_func3()
{
  this->gl_array_pointer_func3_( this->data_type_, 0, 0 );
}

void VertexBufferObject::invoke_array_pointer_func2()
{
  this->gl_array_pointer_func2_( 0, 0 );
}

void VertexBufferObject::set_buffer_data( GLenum data_type, GLint vertex_size, GLsizeiptr size,
    const GLvoid* data, GLenum usage )
{
  this->data_type_ = data_type;
  this->vertex_size_ = vertex_size;

  this->safe_bind();
  glBufferData( this->target_, size, data, usage );
  this->safe_unbind();
}

void VertexBufferObject::set_buffer_sub_data( GLintptr offset, GLsizeiptr size, const GLvoid* data )
{
  this->safe_bind();
  glBufferSubData( this->target_, offset, size, data );
  this->safe_unbind();
}

void VertexBufferObject::disable()
{
  if ( this->target_ == GL_ARRAY_BUFFER )
  {
    glDisableClientState( this->array_type_ );
  }
  glBindBuffer( this->target_, 0 );
}

void VertexBufferObject::draw_arrays( GLenum mode, GLint first, GLsizei count )
{
  assert(this->target_ == GL_ARRAY_BUFFER);
  glDrawArrays( mode, first, count );
}

void VertexBufferObject::multi_draw_arrays( GLenum mode, GLint* first, GLsizei* count,
    GLsizei primcount )
{
  assert(this->target_ == GL_ARRAY_BUFFER);
  glMultiDrawArrays( mode, first, count, primcount );
}

void VertexBufferObject::draw_elements( GLenum mode, GLsizei count )
{
  assert(this->target_ == GL_ELEMENT_ARRAY_BUFFER);
  glDrawElements( mode, count, this->data_type_, NULL );
}

void VertexBufferObject::draw_range_elements( GLenum mode, GLuint start, GLuint end, GLsizei count )
{
  assert(this->target_ == GL_ELEMENT_ARRAY_BUFFER);
  glDrawRangeElements( mode, start, end, count, this->data_type_, NULL );
}

void VertexBufferObject::multi_draw_elements( GLenum mode, GLsizei* count, GLsizei primcount )
{
  assert(this->target_ == GL_ELEMENT_ARRAY_BUFFER);
  glMultiDrawElements( mode, count, this->data_type_, NULL, primcount );
}

} // end namespace Seg3D
