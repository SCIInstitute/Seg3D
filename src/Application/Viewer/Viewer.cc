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

// Application includes
#include <Application/Viewer/Viewer.h>
#include <Application/Viewer/ViewManipulator.h>
#include <Application/State/Actions/ActionResizeView.h>

namespace Seg3D {

Viewer::Viewer(const std::string& key) :
  StateHandler(key)
{
  add_state("view_mode", view_mode_state, "axial", "axial|coronal|sagittal|volume");

  add_state("axial_view", axial_view_state);
  add_state("sagittal_view", sagittal_view_state);
  add_state("coronal_view", coronal_view_state);
  add_state("volume_view", volume_view_state);

  add_state("slice_lock", slice_lock_state, true);
  add_state("slice_grid", slice_grid_state, true);
  add_state("slice_visible", slice_visible_state, true);

  add_state("volume_lock", volume_lock_state, true);
  add_state("volume_slices_visible", volume_slices_visible_state, true);
  add_state("volume_isosurfaces_visible", volume_isosurfaces_visible_state, true);
  add_state("volume_volume_rendering_visible", volume_volume_rendering_visible_state, false);

  this->view_manipulator_ = boost::shared_ptr<ViewManipulator>(new ViewManipulator(this));
  //this->view_mode_state->set("volume");
}

Viewer::~Viewer()
{
  disconnect_all();
}

void Viewer::resize( int width, int height )
{
  this->view_manipulator_->resize(width, height);
  ActionResizeView::Dispatch(this->volume_view_state, width, height);
  ActionResizeView::Dispatch(this->axial_view_state, width, height);
  ActionResizeView::Dispatch(this->sagittal_view_state, width, height);
  ActionResizeView::Dispatch(this->coronal_view_state, width, height);
}

void Viewer::mouse_move_event( const MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  if (!mouse_move_handler_.empty())
  {
    // if the registered handler handled the event, no further process needed
    if (mouse_move_handler_(mouse_history, button, buttons, modifiers))
    {
      return;
    }
  }

  // default handling here
  this->view_manipulator_->mouse_move(mouse_history, button, buttons, modifiers);
}

void Viewer::mouse_press_event( const MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  if (!mouse_press_handler_.empty())
  {
    // if the registered handler handled the event, no further process needed
    if (mouse_press_handler_(mouse_history, button, buttons, modifiers))
    {
      return;
    }
  }

  // default handling here
  this->view_manipulator_->mouse_press(mouse_history, button, buttons, modifiers);
}

void Viewer::mouse_release_event( const MouseHistory& mouse_history, int button, int buttons, int modifiers )
{
  if (!mouse_release_handler_.empty())
  {
    // if the registered handler handled the event, no further process needed
    if (mouse_release_handler_(mouse_history, button, buttons, modifiers))
    {
      return;
    }
  }

  // default handling here
  this->view_manipulator_->mouse_release(mouse_history, button, buttons, modifiers);
}

void Viewer::set_mouse_move_handler( mouse_event_handler_type func )
{
  this->mouse_move_handler_ = func;
}

void Viewer::set_mouse_press_handler( mouse_event_handler_type func )
{
  this->mouse_press_handler_ = func;
}

void Viewer::set_mouse_release_handler( mouse_event_handler_type func )
{
  this->mouse_release_handler_ = func;
}

void Viewer::reset_mouse_handlers()
{
  this->mouse_move_handler_ = 0;
  this->mouse_press_handler_ = 0;
  this->mouse_release_handler_ = 0;
}

void Viewer::state_changed()
{
  redraw_signal_();
}

bool Viewer::is_volume_view() const
{
  return this->view_mode_state->get() == "volume";
}

StateViewBaseHandle Viewer::get_active_view_state()
{
  if (this->view_mode_state->get() == "volume")
  {
    return this->volume_view_state;
  }
  else if (this->view_mode_state->get() == "axial")
  {
    return this->axial_view_state;
  }
  else if (this->view_mode_state->get() == "coronal")
  {
    return this->coronal_view_state;
  }
  else
  {
    return this->sagittal_view_state;
  }
}

} // end namespace Seg3D

