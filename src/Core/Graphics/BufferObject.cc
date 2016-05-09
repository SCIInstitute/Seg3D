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

#include <Core/Graphics/BufferObject.h>
#include <Core/RenderResources/RenderResources.h>


namespace Core
{

BufferObject::BufferObject() :
  copy_constructed_( false )
{
  glGenBuffers( 1, &( this->id_ ) );
  CORE_LOG_DEBUG( std::string( "Buffer object generated: " ) + 
    ExportToString( this->id_ ) );
}

BufferObject::BufferObject( const BufferObjectHandle& bo ) :
  id_( bo->id_ ), copy_constructed_( true ), buffer_object_( bo )
{
}

BufferObject::~BufferObject()
{
  if ( !this->copy_constructed_ )
  {
    // NOTE: This object can be owned by any thread, however it needs to be deleted in the
    // right context. This function will do this for us.
    RenderResources::Instance()->delete_buffer_object( this->id_ );
  }
}

void BufferObject::set_buffer_data( GLsizeiptr size, const GLvoid* data, GLenum usage )
{
  this->safe_bind();
  glBufferData( this->target_, size, data, usage );
  this->safe_unbind();
}

void BufferObject::set_buffer_sub_data( GLintptr offset, GLsizeiptr size, const GLvoid* data )
{
  this->safe_bind();
  glBufferSubData( this->target_, offset, size, data );
  this->safe_unbind();
}

void BufferObject::bind()
{
  glBindBuffer( this->target_, this->id_ );
}

void BufferObject::unbind()
{
  glBindBuffer( this->target_, 0 );
}

void* BufferObject::map_buffer( GLenum access )
{
  this->safe_bind();
  void* buffer = glMapBuffer( this->target_, access );
  this->safe_unbind();
  return buffer;
}

GLboolean BufferObject::unmap_buffer()
{
  this->safe_bind();
  GLboolean result = glUnmapBuffer( this->target_ );
  this->safe_unbind();
  return result;
}

void BufferObject::safe_bind()
{
  glGetIntegerv( this->query_target_, &( this->saved_id_ ) );
  if ( this->id_ != static_cast<unsigned int>( this->saved_id_ ) )
  {
    glBindBuffer( this->target_, this->id_ );
  }
}

void BufferObject::safe_unbind()
{
  if ( this->id_ != static_cast<unsigned int>( this->saved_id_ ) )
  {
    glBindBuffer( this->target_, this->saved_id_ );
  }
}

} // end namespace Core
