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

namespace Seg3D {

Viewer::Viewer(const std::string& key) :
  StateHandler(key)
{
  add_state("view_mode",view_mode_state,"axial|coronal|sagittal|volume","axial");
  
  add_state("axial_view",axial_view_state);
  add_state("sagittal_view",sagittal_view_state);
  add_state("coronal_view",coronal_view_state);
  add_state("volume_view",volume_view_state);

  add_state("slice_lock",slice_lock_state,true);
  add_state("slice_grid",slice_grid_state,true);
  add_state("slice_visible",slice_visible_state,true);

  add_state("volume_lock",volume_lock_state,true);
  add_state("volume_slices_visible",volume_slices_visible_state,true);
  add_state("volume_isosurfaces_visible",volume_isosurfaces_visible_state,true);
  add_state("volume_volume_rendering_visible",volume_volume_rendering_visible_state,false);
}
  
Viewer::~Viewer()
{
  disconnect_all();
}

void 
Viewer::mouse_move_event( int x, int y, int buttons, int modifiers )
{
  if (!mouse_move_handler_.empty())
  {
    // if the registered handler handled the event, no further process needed
    if (mouse_move_handler_(x, y, buttons, modifiers))
    {
      return;
    }
  }
  
  // default handling here

}

void 
Viewer::mouse_press_event( int x, int y, int buttons, int modifiers )
{
  if (!mouse_press_handler_.empty())
  {
    // if the registered handler handled the event, no further process needed
    if (mouse_press_handler_(x, y, buttons, modifiers))
    {
      return;
    }
  }

  // default handling here

}

void 
Viewer::mouse_release_event( int x, int y, int buttons, int modifiers )
{
  if (!mouse_release_handler_.empty())
  {
    // if the registered handler handled the event, no further process needed
    if (mouse_release_handler_(x, y, buttons, modifiers))
    {
      return;
    }
  }

  // default handling here

}

} // end namespace Seg3D

