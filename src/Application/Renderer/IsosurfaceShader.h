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

#ifndef APPLICATION_RENDERER_ISOSURFACESHADER_H
#define APPLICATION_RENDERER_ISOSURFACESHADER_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <Core/Graphics/ShaderBase.h>

namespace Seg3D
{

class IsosurfaceShader;
typedef boost::shared_ptr< IsosurfaceShader > IsosurfaceShaderHandle;

class IsosurfaceShader : public Core::ShaderBase
{
public:
  IsosurfaceShader();
  virtual ~IsosurfaceShader();

  void set_lighting( bool enabled );
  void set_use_colormap( bool enable );
  void set_colormap_texture( int tex_unit );
  void set_min_val( float min_val );
  void set_val_range( float val_range );
  void set_fog( bool enabled );
  void set_fog_range( float znear, float zfar );
  void set_opacity( float opacity );

protected:
  virtual bool get_vertex_shader_source( std::string& source );
  virtual bool get_fragment_shader_source( std::string& source );
  virtual bool pre_link();
  virtual bool post_initialize();

private:
  int enable_lighting_loc_;
  int use_colormap_loc_;
  int colormap_loc_;
  int min_val_loc_;
  int val_range_loc_;
  int enable_fog_loc_;
  int fog_range_loc_;
  int opacity_loc_;
};

} // end namespace Seg3D
#endif
