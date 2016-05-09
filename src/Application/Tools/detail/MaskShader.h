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

#ifndef APPLICATION_TOOLS_DETAIL_MASKSHADER_H
#define APPLICATION_TOOLS_DETAIL_MASKSHADER_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <Core/Graphics/ShaderBase.h>
#include <Core/Utils/Lockable.h>

namespace Seg3D
{

class MaskShader;
typedef boost::shared_ptr< MaskShader > MaskShaderHandle;

class MaskShader : public Core::Lockable, public Core::ShaderBase
{
public:
  MaskShader();
  virtual ~MaskShader();

  void set_texture( int tex_unit );
  void set_color( float r, float g, float b );
  void set_opacity( float opacity );
  void set_pixel_size( float width, float height );
  void set_border_width( int width );

protected:
  virtual bool get_fragment_shader_source( std::string& source );
  virtual bool post_initialize();

private:
  int tex_loc_;
  int color_loc_;
  int opacity_loc_;
  int border_width_loc_;
  int pixel_size_loc_;
};

} // end namespace Seg3D
#endif
