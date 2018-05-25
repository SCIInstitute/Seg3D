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

#ifndef APPLICATION_RENDERER_SLICESHADER_H
#define APPLICATION_RENDERER_SLICESHADER_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <Core/Graphics/ShaderBase.h>

namespace Seg3D
{

class SliceShader;
typedef boost::shared_ptr< SliceShader > SliceShaderHandle;

class SliceShader : public Core::ShaderBase
{
public:
  SliceShader();
  virtual ~SliceShader();

  void set_slice_texture( int tex_unit );
  void set_pattern_texture( int tex_unit );
  void set_mask_mode( int mask_mode );
  void set_volume_type( int volume_type );
  void set_mask_color( float r, float g, float b );
  void set_data_color(float r, float g, float b);
  void set_opacity( float opacity );
  void set_scale_bias( float scale, float bias );
  void set_pixel_size( float width, float height );
  void set_border_width( int width );
  void set_lighting( bool enabled );
  void set_fog( bool enabled );
  void set_fog_range( float znear, float zfar );
  void set_texture_clamp( float s_min, float s_max, float t_min, float t_max );

protected:
  virtual bool get_vertex_shader_source( std::string& source );
  virtual bool get_fragment_shader_source( std::string& source );
  virtual bool post_initialize();

private:
  int slice_tex_loc_;
  int pattern_tex_loc_;
  int mask_mode_loc_;
  int volume_type_loc_;
  int mask_color_loc_;
  int data_color_loc_;
  int opacity_loc_;
  int scale_bias_loc_;
  int border_width_loc_;
  int pixel_size_loc_;
  int enable_lighting_loc_;
  int enable_fog_loc_;
  int fog_range_loc_;
  int texture_clamp_loc_;
};

} // end namespace Seg3D
#endif
