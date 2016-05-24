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

#ifndef APPLICATION_VIEWER_VIEWMANIPULATOR_H
#define APPLICATION_VIEWER_VIEWMANIPULATOR_H

#include <boost/shared_ptr.hpp>

#include <Core/Geometry/Quaternion.h>
#include <Core/Geometry/Vector.h>
#include <Core/Viewer/Mouse.h>

namespace Seg3D
{

class Viewer;
class ViewManipulator;
typedef boost::shared_ptr< ViewManipulator > ViewManipulatorHandle;

class ViewManipulator
{
public:
  ViewManipulator( Viewer* viewer );
  ~ViewManipulator();

  void resize( int width, int height );
  void set_scale_factor( double scale_factor );
  void set_flip_y( bool flip_y );
  void set_camera_mode( bool camera_mode );

  void mouse_move( const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers );
  void mouse_press( const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers );
  void mouse_release( const Core::MouseHistory& mouse_history, int button, int buttons, int modifiers );

private:

  bool compute_rotation( int x0, int y0, int x1, int y1, 
    Core::Vector& axis, double& angle ) const;
  double compute_scaling( int x0, int y0, int x1, int y1 ) const;
  Core::Vector compute_translation( int x0, int y0, int x1, int y1, bool is_view3d ) const;

  void compute_3d_viewplane();
  Core::Vector project_point_onto_sphere( int x, int y ) const;
  inline int remap_y( int y ) const;

  int width_;
  int height_;

  double scale_factor_;

  bool flip_y_;
  bool camera_mode_;

  bool translate_active_;
  bool rotate_active_;
  bool scale_active_;

  Viewer* viewer_;

  Core::Vector viewplane_u_;
  Core::Vector viewplane_v_;
};

inline void ViewManipulator::resize( int width, int height )
{
  this->width_ = width;
  this->height_ = height;
}

inline void ViewManipulator::set_scale_factor( double scale_factor )
{
  this->scale_factor_ = scale_factor;
}

inline void ViewManipulator::set_flip_y( bool flip_y )
{
  this->flip_y_ = flip_y;
}

inline void ViewManipulator::set_camera_mode( bool camera_mode )
{
  this->camera_mode_ = camera_mode;
}

inline int ViewManipulator::remap_y( int y ) const
{
  return this->flip_y_ ? ( this->height_ - 1 - y ) : y;
}

} // end namespace Seg3D

#endif
