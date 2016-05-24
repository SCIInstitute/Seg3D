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

#include <Core/Graphics/Renderbuffer.h>
#include <Core/RenderResources/RenderResources.h>


namespace Core
{

const unsigned int Renderbuffer::TARGET_ = GL_RENDERBUFFER_EXT;

Renderbuffer::Renderbuffer()
{
  glGenRenderbuffersEXT( 1, &id_ );

  safe_bind();
  safe_unbind();
}

Renderbuffer::~Renderbuffer()
{
  // NOTE: This object can be owned by any thread, however it needs to be deleted in the
  // right context. This function will do this for us.
  RenderResources::Instance()->delete_renderbuffer( this->id_ );
}

void Renderbuffer::bind()
{
  glBindRenderbufferEXT( TARGET_, id_ );
}

void Renderbuffer::set_storage(int width, int height, unsigned int internal_format, int samples)
{
  safe_bind();
  if (samples > 1)
  {
    glRenderbufferStorageMultisampleEXT(TARGET_, samples, internal_format, width, height);
  }
  else
  {
    glRenderbufferStorageEXT(TARGET_, internal_format, width, height);
  }
  safe_unbind();
}

void Renderbuffer::unbind()
{
  glBindRenderbufferEXT( TARGET_, 0 );
}

void Renderbuffer::safe_bind()
{
  glGetIntegerv( GL_RENDERBUFFER_BINDING_EXT, &saved_id_ );
  if ( static_cast< int > ( id_ ) != saved_id_ )
  {
    glBindRenderbufferEXT( TARGET_, id_ );
  }
}

void Renderbuffer::safe_unbind()
{
  if ( static_cast< int > ( id_ ) != saved_id_ )
  {
    glBindRenderbufferEXT( TARGET_, saved_id_ );
  }
}

} // end namespace Core
