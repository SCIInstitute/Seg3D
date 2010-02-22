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

// Utils includes
#include <Utils/Math/MathFunctions.h>

#include <Application/State/Actions/ActionRotateView3D.h>
#include <Application/State/Actions/ActionScaleView.h>
#include <Application/State/Actions/ActionTranslateView.h>
#include <Application/Viewer/Viewer.h>
#include <Application/Viewer/ViewManipulator.h>

namespace Seg3D
{

ViewManipulator::ViewManipulator(Viewer* viewer) :
  width_(0), height_(0), scale_factor_(6.0),
  flip_y_(true), camera_mode_(true)
{
  this->viewer_ = viewer;
  this->translate_active_ = false;
  this->scale_active_ = false;
  this->rotate_active_ = false;
}

ViewManipulator::~ViewManipulator()
{
}

void ViewManipulator::mouse_press( const MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  // if there is already an active mouse action, return directly
  if (this->translate_active_ || this->scale_active_ || this->rotate_active_)
  {
    return;
  }
  
  if (this->viewer_->is_volume_view())
  {
    if (button == Viewer::LEFT_BUTTON_E)
    {
      this->translate_active_ = true;
      this->compute_3d_viewplane();
    } 
    else if (button == Viewer::RIGHT_BUTTON_E)
    {
      this->scale_active_ = true;
    }
    else if (button == Viewer::MID_BUTTON_E)
    {
      this->rotate_active_ = true;
    }   
  }
  else
  {
    if ((button == Viewer::LEFT_BUTTON_E)
      && (modifiers == Viewer::SHIFT_MODIFIER_E))
    {
      this->translate_active_ = true;
    }
    else if ((button == Viewer::RIGHT_BUTTON_E)
      && (modifiers == Viewer::SHIFT_MODIFIER_E))
    {
      this->scale_active_ = true;
    }
  }
}

void ViewManipulator::mouse_move( const MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  if (this->translate_active_)
  {
    Utils::Vector offset = this->compute_translation(mouse_history.previous.x, mouse_history.previous.y,
      mouse_history.current.x, mouse_history.current.y, this->viewer_->is_volume_view());
    StateViewBaseHandle view_state = this->viewer_->get_active_view_state();
    ActionTranslateView::Dispatch(view_state, offset);
  }
  else if (this->scale_active_)
  {
    double scale_ratio = this->compute_scaling(mouse_history.previous.x, mouse_history.previous.y,
      mouse_history.current.x, mouse_history.current.y);
    StateViewBaseHandle view_state = this->viewer_->get_active_view_state();
    ActionScaleView::Dispatch(view_state, scale_ratio);
  }
  else if (this->rotate_active_)
  {
    Utils::Vector axis;
    double angle;
    if (this->compute_rotation(mouse_history.previous.x, mouse_history.previous.y,
                          mouse_history.current.x, mouse_history.current.y, axis, angle))
    {
      // dispatch an ActionRotateView3D
      ActionRotateView3D::Dispatch(this->viewer_->volume_view_state, axis, angle);
    }
  } 
}

void ViewManipulator::mouse_release( const MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  switch(button)
  {
  case Viewer::LEFT_BUTTON_E:
    this->translate_active_ = false;
    break;
  case Viewer::RIGHT_BUTTON_E:
    this->scale_active_ = false;
    break;
  case Viewer::MID_BUTTON_E:
    this->rotate_active_ = false;
    break;
  }
}

bool ViewManipulator::compute_rotation( int x0, int y0, int x1, int y1, Utils::Vector& axis, double& angle ) const
{
  Utils::Vector v0 = this->project_point_onto_sphere(x0, y0);
  Utils::Vector v1 = this->project_point_onto_sphere(x1, y1);

  axis = Utils::Cross(v0, v1);
  angle = Utils::Acos(Dot(v0, v1));

  double len = axis.normalize();
  if (len < 1e-8)
  {
    return false;
  }

  angle = angle * 180.0 / Utils::Pi();
  
  return true;
}

double ViewManipulator::compute_scaling( int x0, int y0, int x1, int y1 ) const
{
  double delta_x = static_cast<double>(x1 - x0) / this->width_;
  double delta_y = static_cast<double>(y1 - y0) / this->height_;
  if (this->flip_y_)
  {
    delta_y = -delta_y;
  }
  
  double len = Utils::Sqrt(delta_x * delta_x + delta_y * delta_y);
  double scale = Utils::Abs(delta_x) > Utils::Abs(delta_y) ? delta_x : delta_y;
  
  if (scale > 0)
  {
    scale = 1.0 / (1.0 + len * this->scale_factor_);
  } 
  else
  {
    scale = 1.0 + len * this->scale_factor_;
  }
  
  return scale;
}

Utils::Vector ViewManipulator::compute_translation( int x0, int y0, 
                           int x1, int y1, bool is_view3d ) const
{
  double dx = x1 - x0;
  double dy = this->flip_y_ ? (y0 - y1) : (y1 - y0);

  if (is_view3d)
  {
    dx /= this->width_;
    dy /= this->height_;
    return (this->viewplane_u_ * dx + this->viewplane_v_ * dy);
  }
  else
  {
    double dimension = Utils::Min(this->width_, this->height_);
    dx /= dimension;
    dy /= dimension;
    return Utils::Vector(dx, dy, 0.0);
  }
}

Utils::Vector ViewManipulator::project_point_onto_sphere( int x, int y ) const
{
  Utils::Vector v(x * 2.0 / this->width_ - 1.0, this->remap_y(y) * 2.0 / this->height_ - 1.0, 0.0);
  double len2 = v.length2();
  v[2] = len2 >= 1.0 ? 0.0 : Utils::Sqrt(1.0 - len2);
  v.normalize();

  return v;
}

void ViewManipulator::compute_3d_viewplane()
{
  StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());
  Utils::View3D view3d (this->viewer_->volume_view_state->get());
  lock.unlock();

  Utils::Vector z(view3d.eyep() - view3d.lookat());
  double eye_distance = z.normalize();
  Utils::Vector x(Cross(view3d.up(), z));
  x.normalize();
  Utils::Vector y(Cross(z, x));

  double half_fov = Utils::DegreeToRadian(view3d.fov()) * 0.5;
  double viewplane_height = eye_distance * Utils::Tan(half_fov) * 2;
  double viewplane_width = this->width_ * (viewplane_height / this->height_);

  this->viewplane_u_ = x * viewplane_width;
  this->viewplane_v_ = y * viewplane_height;
}

} // end namespace Seg3D