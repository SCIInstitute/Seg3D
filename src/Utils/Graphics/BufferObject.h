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

#ifndef UTILS_GRAPHICS_BUFFEROBJECT_H
#define UTILS_GRAPHICS_BUFFEROBJECT_H

#include <boost/shared_ptr.hpp>

#include <GL/glew.h>

namespace Utils
{

class BufferObject;
typedef boost::shared_ptr< BufferObject > BufferObjectHandle;

class BufferObject
{
protected:
  // Default constructor
  BufferObject();

  // Copy constructor
  // Allows binding buffer objects to different targets. The destructor will not
  // delete the underlying OpenGL object. It is deleted by the original instance.
  BufferObject( const BufferObjectHandle& bo );

  virtual ~BufferObject();

public:
  void set_buffer_data( GLsizeiptr size, const GLvoid* data, GLenum usage );
  void set_buffer_sub_data( GLintptr offset, GLsizeiptr size, const GLvoid* data );

  void bind();
  void unbind();

  void* map_buffer(GLenum access);
  GLboolean unmap_buffer();

protected:
  void safe_bind();
  void safe_unbind();

protected:
  GLenum target_;
  GLenum query_target_;

private:
  BufferObject& operator=( const BufferObject& bo );

private:
  GLuint id_;
  GLint saved_id_;
  bool copy_constructed_;
  BufferObjectHandle buffer_object_;
};

} // end namespace Utils

#endif
