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

#include <boost/bind.hpp>

#include <Core/Utils/Log.h>
#include <Core/Graphics/VertexBufferObject.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class VertexBufferObject
//////////////////////////////////////////////////////////////////////////

VertexBufferObject::VertexBufferObject() :
  BufferObject()
{
}

VertexBufferObject::VertexBufferObject(const BufferObjectHandle &bo) :
  BufferObject( bo )
{
}

VertexBufferObject::~VertexBufferObject()
{
}

//////////////////////////////////////////////////////////////////////////
// Class VertexAttribArrayBuffer
//////////////////////////////////////////////////////////////////////////

const GLenum VertexAttribArrayBuffer::GL_ARRAY_TYPES_C[] = 
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

VertexAttribArrayBuffer::VertexAttribArrayBuffer() :
  VertexBufferObject()
{
  this->target_ = GL_ARRAY_BUFFER;
  this->query_target_ = GL_ARRAY_BUFFER_BINDING;
  this->safe_bind();
  this->safe_unbind();
}

VertexAttribArrayBuffer::VertexAttribArrayBuffer( const BufferObjectHandle &bo ) :
  VertexBufferObject( bo )
{
  this->target_ = GL_ARRAY_BUFFER;
  this->query_target_ = GL_ARRAY_BUFFER_BINDING;
}

void VertexAttribArrayBuffer::set_array( VertexAttribArrayType array_type, GLint vertex_size, 
                            GLenum data_type, GLsizei stride, int offset )
{
  VertexAttribArrayInfoHandle array_info( new VertexAttribArrayInfo );
  array_info->array_type_ = GL_ARRAY_TYPES_C[ array_type ];
  array_info->generic_ = false;

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
    CORE_LOG_ERROR( "Incompatible array type and parameters" );
    assert( false );
    return;
  }

  this->vertex_arrays_.push_back( array_info );
}

void VertexAttribArrayBuffer::set_array( VertexAttribArrayType array_type, 
                            GLenum data_type, GLsizei stride, int offset )
{
  VertexAttribArrayInfoHandle array_info( new VertexAttribArrayInfo );
  array_info->array_type_ = GL_ARRAY_TYPES_C[ array_type ];
  array_info->generic_ = false;

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
    CORE_LOG_ERROR( "Incompatible array type and parameters" );
    assert( false );
    return;
  }

  this->vertex_arrays_.push_back( array_info );
}

void VertexAttribArrayBuffer::set_array( VertexAttribArrayType array_type, GLsizei stride, int offset )
{
  VertexAttribArrayInfoHandle array_info( new VertexAttribArrayInfo );
  array_info->array_type_ = GL_ARRAY_TYPES_C[ array_type ];
  array_info->generic_ = false;

  switch( array_type )
  {
  case VertexAttribArrayType::EDGE_FLAG_E:
    array_info->gl_array_pointer_func_ = boost::bind( glEdgeFlagPointer, stride,
      reinterpret_cast<void*>( offset ) );
    break;
  default:
    CORE_LOG_ERROR( "Incompatible array type and parameters" );
    assert( false );
    return;
  }

  this->vertex_arrays_.push_back( array_info );
}

void VertexAttribArrayBuffer::set_generic_array( GLuint index, GLint attrib_size, 
    GLenum data_type, GLboolean normalized, GLsizei stride, int offset )
{
  VertexAttribArrayInfoHandle array_info( new VertexAttribArrayInfo );
  array_info->array_type_ = index;
  array_info->generic_ = true;
  array_info->gl_array_pointer_func_ = boost::bind( glVertexAttribPointer, index, 
    attrib_size, data_type, normalized, stride, reinterpret_cast< void* >( offset ) );
  this->vertex_arrays_.push_back( array_info );
}

void VertexAttribArrayBuffer::enable_arrays()
{
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
    if ( this->vertex_arrays_[ i ]->generic_ )
    {
      glEnableVertexAttribArray( this->vertex_arrays_[ i ]->array_type_ );
    }
    else
    {
      glEnableClientState( this->vertex_arrays_[i]->array_type_ );
    }
  }
  this->safe_unbind();
}

void VertexAttribArrayBuffer::disable_arrays()
{
  size_t num_of_arrays = this->vertex_arrays_.size();
  if ( num_of_arrays == 0 )
  {
    return;
  }

  for (size_t i = 0; i < num_of_arrays; i++)
  {
    if ( this->vertex_arrays_[ i ]->generic_ )
    {
      glDisableVertexAttribArray( this->vertex_arrays_[i]->array_type_ );
    }
    else
    {
      glDisableClientState( this->vertex_arrays_[i]->array_type_ );
    }
  }
}

void VertexAttribArrayBuffer::draw_arrays( GLenum mode, GLint first, GLsizei count )
{
  this->enable_arrays();
  glDrawArrays( mode, first, count );
  this->disable_arrays();
}

void VertexAttribArrayBuffer::multi_draw_arrays( GLenum mode, GLint* first, GLsizei* count,
    GLsizei primcount )
{
  this->enable_arrays();
  glMultiDrawArrays( mode, first, count, primcount );
  this->disable_arrays();
}

void VertexAttribArrayBuffer::RestoreDefault()
{
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

//////////////////////////////////////////////////////////////////////////
// Class ElementArrayBuffer
//////////////////////////////////////////////////////////////////////////

ElementArrayBuffer::ElementArrayBuffer() :
  VertexBufferObject()
{
  this->target_ = GL_ELEMENT_ARRAY_BUFFER;
  this->query_target_ = GL_ELEMENT_ARRAY_BUFFER_BINDING;
  this->safe_bind();
  this->safe_unbind();
}

ElementArrayBuffer::ElementArrayBuffer( const BufferObjectHandle& bo ) :
  VertexBufferObject( bo )
{
  this->target_ = GL_ELEMENT_ARRAY_BUFFER;
  this->query_target_ = GL_ELEMENT_ARRAY_BUFFER_BINDING;
}

void ElementArrayBuffer::draw_elements( GLenum mode, GLsizei count, GLenum data_type, int offset )
{
  this->safe_bind();
  glDrawElements( mode, count, data_type, reinterpret_cast<void*>( offset ) );
  this->safe_unbind();
}

void ElementArrayBuffer::draw_range_elements( GLenum mode, GLuint start, GLuint end, 
  GLsizei count, GLenum data_type, int offset )
{
  this->safe_bind();
  glDrawRangeElements( mode, start, end, count, data_type, reinterpret_cast<void*>( offset ) );
  this->safe_unbind();
}

void ElementArrayBuffer::multi_draw_elements( GLenum mode, GLsizei* count, 
  GLenum data_type, const GLvoid** offsets, GLsizei primcount )
{
  this->safe_bind();
  glMultiDrawElements( mode, count, data_type, offsets, primcount );
  this->safe_unbind();
}

void ElementArrayBuffer::RestoreDefault()
{
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

} // end namespace Core
