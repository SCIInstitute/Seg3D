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

#ifndef CORE_GRAPHICS_TEXTURE_H
#define CORE_GRAPHICS_TEXTURE_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <GL/glew.h>

#include <Core/Utils/Lockable.h>

namespace Core
{

class Texture;
class Texture1D;
class Texture2D;
class Texture3D;
typedef boost::shared_ptr< Texture > TextureHandle;
typedef boost::shared_ptr< Texture1D > Texture1DHandle;
typedef boost::shared_ptr< Texture2D > Texture2DHandle;
typedef boost::shared_ptr< Texture3D > Texture3DHandle;

// CLASS TEXTURE
// A wrapper of the OpenGL texture

class Texture : public RecursiveLockable
{

  // -- Constructor/Destructor --
protected:
  Texture();
  virtual ~Texture();

public:
  void enable();
  void disable();
  void bind();
  void unbind();
  
  void set_parameter(unsigned int param_name, int param_value);
  void set_mag_filter( int filter );
  void set_min_filter( int filter );
  void set_wrap_s( int wrap_mode );
  void set_wrap_t( int wrap_mode );
  void set_wrap_r( int wrap_mode );

  inline unsigned int get_id() const
  {
    return this->texture_id_;
  }

  inline unsigned int get_target() const
  {
    return this->target_;
  }

protected:
  void safe_bind();
  void safe_unbind();

  int saved_id_;
  unsigned int texture_id_;
  unsigned int target_;
  unsigned int query_target_;

  mutex_type mutex_;

public:
  static unsigned int GetActiveTextureUnit();
  static void SetActiveTextureUnit( unsigned int unit );
  static void SetClientActiveTextureUnit( unsigned int unit );
};

// CLASS TEXTURE1D

class Texture1D : public Texture
{
public:
  Texture1D();
  virtual ~Texture1D() {}

  void set_image(int width, int internal_format = GL_RGBA, const void *pixels = 0, 
    unsigned int format = GL_RGBA, unsigned int type = GL_UNSIGNED_BYTE, int level = 0);
  void set_sub_image( int xoffset, int width, const void* data, unsigned int format, 
    unsigned int type, int level = 0 );
};

// CLASS TEXTURE2D

class Texture2D : public Texture
{
public:
  Texture2D();
  virtual ~Texture2D() {}

  void set_image(int width, int height, int internal_format = GL_RGBA, const void *pixels = 0,
    unsigned int format = GL_RGBA, unsigned int type = GL_UNSIGNED_BYTE, int level = 0);
  void set_sub_image( int xoffset, int yoffset, int width, int height, const void* data, 
    unsigned int format, unsigned int type, int level = 0 );
};

// CLASS TEXTURE3D

class Texture3D : public Texture
{
public:
  Texture3D();
  virtual ~Texture3D() {}

  void set_image(int width, int height, int depth, int internal_format = GL_RGBA, const void *pixels = 0,
    unsigned int format = GL_RGBA, unsigned int type = GL_UNSIGNED_BYTE, int level = 0);
  void set_sub_image( int xoffset, int yoffset, int zoffset, int width, int height, int depth,
    const void* data, unsigned int format, unsigned int type, int level = 0 );
};

} // end namespace Core

#endif
