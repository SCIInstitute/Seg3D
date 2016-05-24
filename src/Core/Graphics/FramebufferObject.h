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

#ifndef CORE_GRAPHICS_FRAMEBUFFEROBJECT_H
#define CORE_GRAPHICS_FRAMEBUFFEROBJECT_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <GL/glew.h>

#include <Core/Graphics/Texture.h>
#include <Core/Graphics/Renderbuffer.h>

namespace Core
{

class FramebufferObject;
typedef boost::shared_ptr< FramebufferObject > FramebufferObjectHandle;

class FramebufferObject : public boost::noncopyable
{

public:

  FramebufferObject();
  ~FramebufferObject();

  void enable();
  void disable();
  void safe_bind();
  void safe_unbind();

  void attach_texture( TextureHandle texture, unsigned int attachment = 
    GL_COLOR_ATTACHMENT0_EXT, int level = 0, int layer = 0 );
  void detach_texture( TextureHandle texture, unsigned int attachment = GL_COLOR_ATTACHMENT0_EXT );
  void attach_renderbuffer( RenderbufferHandle renderbuffer, unsigned int attachment );
  void detach_renderbuffer( RenderbufferHandle renderbuffer, unsigned int attachment );
  bool check_status( GLenum* status = NULL );

private:
  unsigned int id_;
  int saved_id_;

  const static unsigned int TARGET_C;
};

} // end namespace Core

#endif
