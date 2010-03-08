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

#include <Utils/Core/Log.h>
#include <Utils/Graphics/VertexBufferObject.h>

namespace Utils
{

const GLenum VertexBufferObject::GL_ARRAY_TYPES_C[] = 
{
  GL_VERTEX_ARRAY,
  GL_COLOR_ARRAY,
  GL_TEXTURE_COORD_ARRAY,
  GL_SECONDARY_COLOR_ARRAY,
  GL_NORMAL_ARRAY,
  GL_FOG_COORD_ARRAY,
  GL_INDEX_ARRAY,
  GL_EDGE_FLAG_ARRAY
};

VertexBufferObject::VertexBufferObject( GLenum target ) :
  target_( target )
{
  assert( target_ == GL_ARRAY_BUFFER || target_ == GL_ELEMENT_ARRAY_BUFFER );
  glGenBuffers( 1, &( this->id_ ) );
  this->saved_id_ = 0;

  if ( this->target_ == GL_ARRAY_BUFFER )
  {
    this->query_target_ = GL_ARRAY_BUFFER_BINDING;
  }
  else
  {
    this->query_target_ = GL_ELEMENT_ARRAY_BUFFER_BINDING;
  }

  this->safe_bind();
  this->safe_unbind();
}

VertexBufferObject::~VertexBufferObject()
{
  glDeleteBuffers( 1, &( this->id_ ) );
}

void VertexBufferObject::safe_bind()
{
  glGetIntegerv( this->query_target_, &( this->saved_id_ ) );
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

void VertexBufferObject::unbind()
{
  glBindBuffer( this->target_, 0 );
}

void VertexBufferObject::set_buffer_data( GLsizeiptr size, const GLvoid* data, GLenum usage )
{
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

void VertexBufferObject::set_array( VertexAttribArrayType array_type, GLint vertex_size, 
                            GLenum data_type, GLsizei stride, int offset )
{
  if ( this->target_ != GL_ARRAY_BUFFER )
  {
    return;
  }

  VertexArrayInfoHandle array_info( new VertexArrayInfo( array_type ) );

  switch(array_type)
  {
  case VertexAttribArrayType::VERTEX_E:
    array_info->gl_array_pointer_func_ = boost::bind( glVertexPointer, vertex_size,
      data_type, stride, reinterpret_cast<void*>( offset ) );
    break;
  case VertexAttribArrayType::COLOR_E:
    array_info->gl_array_pointer_func_ = boost::bind( glColorPointer, vertex_size,
      data_type, stride, reinterpret_cast<void*>( offset ) );
    break;
  case VertexAttribArrayType::TEXTURE_COORD_E:
    array_info->gl_array_pointer_func_ = boost::bind( glTexCoordPointer, vertex_size,
      data_type, stride, reinterpret_cast<void*>( offset ) );
    break;
  case  VertexAttribArrayType::SECONDARY_COLOR_E:
    array_info->gl_array_pointer_func_ = boost::bind( glSecondaryColorPointer, vertex_size,
      data_type, stride, reinterpret_cast<void*>( offset ) );
    break;
  default:
    SCI_LOG_ERROR( "Incompatible vertex array type and parameters" );
    assert( false );
    return;
  }

  this->vertex_arrays_.push_back( array_info );
}

void VertexBufferObject::set_array( VertexAttribArrayType array_type, 
                            GLenum data_type, GLsizei stride, int offset )
{
  if ( this->target_ != GL_ARRAY_BUFFER )
  {
    return;
  }

  VertexArrayInfoHandle array_info( new VertexArrayInfo( array_type ) );

  switch( array_type )
  {
  case VertexAttribArrayType::NORMAL_E:
    array_info->gl_array_pointer_func_ = boost::bind( glNormalPointer, data_type,
      stride, reinterpret_cast<void*>( offset ) );
    break;
  case VertexAttribArrayType::FOG_COORD_E:
    array_info->gl_array_pointer_func_ = boost::bind( glFogCoordPointer, data_type,
      stride, reinterpret_cast<void*>( offset ) );
    break;
  case VertexAttribArrayType::INDEX_E:
    array_info->gl_array_pointer_func_ = boost::bind( glIndexPointer, data_type,
      stride, reinterpret_cast<void*>( offset ) );
    break;
  default:
    SCI_LOG_ERROR( "Incompatible vertex array type and parameters" );
    assert( false );
    return;
  }

  this->vertex_arrays_.push_back( array_info );
}

void VertexBufferObject::set_array( VertexAttribArrayType array_type, GLsizei stride, int offset )
{
  if ( this->target_ != GL_ARRAY_BUFFER )
  {
    return;
  }

  VertexArrayInfoHandle array_info( new VertexArrayInfo( array_type ) );

  switch( array_type )
  {
  case VertexAttribArrayType::EDGE_FLAG_E:
    array_info->gl_array_pointer_func_ = boost::bind( glEdgeFlagPointer, stride,
      reinterpret_cast<void*>( offset ) );
    break;
  default:
    SCI_LOG_ERROR( "Incompatible vertex array type and parameters" );
    assert( false );
    return;
  }

  this->vertex_arrays_.push_back( array_info );
}

void VertexBufferObject::enable_arrays()
{
  if ( this->target_ != GL_ARRAY_BUFFER )
  {
    assert( false );
    return;
  }

  size_t num_of_arrays = this->vertex_arrays_.size();
  if ( num_of_arrays == 0 )
  {
    assert( false );
    return;
  }

  this->safe_bind();
  for (size_t i = 0; i < num_of_arrays; i++)
  {
    this->vertex_arrays_[i]->gl_array_pointer_func_();
    glEnableClientState( GL_ARRAY_TYPES_C[ this->vertex_arrays_[i]->type_ ] );
  }
  this->safe_unbind();
}

void VertexBufferObject::disable_arrays()
{
  if ( this->target_ != GL_ARRAY_BUFFER )
  {
    return;
  }

  size_t num_of_arrays = this->vertex_arrays_.size();
  if ( num_of_arrays == 0 )
  {
    return;
  }

  for (size_t i = 0; i < num_of_arrays; i++)
  {
    glDisableClientState( GL_ARRAY_TYPES_C[ this->vertex_arrays_[i]->type_ ] );
  }
}

void VertexBufferObject::draw_arrays( GLenum mode, GLint first, GLsizei count )
{
  assert(this->target_ == GL_ARRAY_BUFFER);
  this->enable_arrays();
  glDrawArrays( mode, first, count );
  this->disable_arrays();
}

void VertexBufferObject::multi_draw_arrays( GLenum mode, GLint* first, GLsizei* count,
    GLsizei primcount )
{
  assert(this->target_ == GL_ARRAY_BUFFER);
  this->enable_arrays();
  glMultiDrawArrays( mode, first, count, primcount );
  this->disable_arrays();
}

void VertexBufferObject::draw_elements( GLenum mode, GLsizei count, GLenum data_type, int offset )
{
  assert(this->target_ == GL_ELEMENT_ARRAY_BUFFER);
  this->safe_bind();
  glDrawElements( mode, count, data_type, reinterpret_cast<void*>( offset ) );
  this->safe_unbind();
}

void VertexBufferObject::draw_range_elements( GLenum mode, GLuint start, GLuint end, 
  GLsizei count, GLenum data_type, int offset )
{
  assert(this->target_ == GL_ELEMENT_ARRAY_BUFFER);
  this->safe_bind();
  glDrawRangeElements( mode, start, end, count, data_type, reinterpret_cast<void*>( offset ) );
  this->safe_unbind();
}

void VertexBufferObject::multi_draw_elements( GLenum mode, GLsizei* count, 
  GLenum data_type, const GLvoid** offsets, GLsizei primcount )
{
  assert(this->target_ == GL_ELEMENT_ARRAY_BUFFER);
  this->safe_bind();
  glMultiDrawElements( mode, count, data_type, offsets, primcount );
  this->safe_unbind();
}

} // end namespace Utils
