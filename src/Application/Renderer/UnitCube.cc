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

#include <Application/Renderer/UnitCube.h>

namespace Seg3D
{

const GLfloat UnitCube::VERTICES_C[ 8 ][ 3 ] =
{
  { 0, 0, 0 },
  { 0, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 1, 1, 0 },
  { 1, 1, 1 }
};

const GLubyte UnitCube::FACES_C[ 6 ][ 4 ] =
{
  { 0, 2, 6, 4 }, // back
  { 0, 1, 3, 2 }, // left 
  { 2, 3, 7, 6 }, // top 
  { 1, 5, 7, 3 }, // front
  { 4, 6, 7, 5 }, // right
  { 0, 4, 5, 1 }  // bottom
};

UnitCube::UnitCube()
{
  this->vertices_buffer_ = VertexBufferObjectHandle( new VertexBufferObject( GL_ARRAY_BUFFER,
      GL_VERTEX_ARRAY ) );
  this->vertices_buffer_->set_buffer_data( GL_FLOAT, 3, sizeof(GLfloat) * 8 * 3,
      reinterpret_cast< const GLvoid* > ( &VERTICES_C[ 0 ][ 0 ] ), GL_STATIC_DRAW );

  this->colors_buffer_ = VertexBufferObjectHandle( new VertexBufferObject( GL_ARRAY_BUFFER,
      GL_COLOR_ARRAY ) );
  this->colors_buffer_->set_buffer_data( GL_FLOAT, 3, sizeof(GLfloat) * 8 * 3,
      reinterpret_cast< const GLvoid* > ( &VERTICES_C[ 0 ][ 0 ] ), GL_STATIC_DRAW );

  this->faces_buffer_ = VertexBufferObjectHandle( new VertexBufferObject(
      GL_ELEMENT_ARRAY_BUFFER, GL_INDEX_ARRAY ) );
  this->faces_buffer_->set_buffer_data( GL_UNSIGNED_BYTE, 1, sizeof(GLubyte) * 6 * 4,
      reinterpret_cast< const GLvoid* > ( &FACES_C[ 0 ][ 0 ] ), GL_STATIC_DRAW );
}

UnitCube::~UnitCube()
{
}

void UnitCube::draw()
{
  this->vertices_buffer_->enable();
  this->colors_buffer_->enable();
  this->faces_buffer_->enable();
  this->faces_buffer_->draw_elements( GL_QUADS, 24 );
  this->faces_buffer_->disable();
  this->colors_buffer_->disable();
  this->vertices_buffer_->disable();
}

} // end namespace Seg3D
