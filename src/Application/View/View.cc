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

/*
 *****************************************************************************
 *
 *   View.cc
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/19/2009
 *
 *    
 *****************************************************************************
 */

// STL includes

// Boost includes 

#include <Application/View/View.h>


namespace Seg3D {

View::View() 
{
}
  
View::~View()
{
}

void 
View::add_layer(const std::string& layer_name, bool make_active, unsigned int opacity)
{
  // TODO:  Place the layer in the appropriate location in the stack
  // All masks to the front, data to the back.
  layer_stack_.push_back(layer_name);
  set_opacity(layer_name, opacity);
  if (make_active)
  {
    active_layer_ = layer_name;
  }
}

void
View::remove_layer(const std::string& layer_name)
{
  // TODO: Does the layer_stack_ or opacity_ need to be locked prior to modification?
  LayerStackType::iterator pos;
  pos = find(layer_stack_.begin(), layer_stack_.end(), layer_name);
  if (pos != layer_stack_.end())
  {
    layer_stack_.erase(pos);
  }
  opacity_.erase(layer_name);
}

void
View::redraw() const
{
  // Redraw the view
}

unsigned int 
View::get_opacity(const std::string& layer)
{
  return opacity_[layer];
}

void
View::set_opacity(const std::string& layer, unsigned int new_opacity)
{
  // TODO: should we check to see if layer is a valid layer name?
  if (new_opacity > 100) { opacity_[layer] = 100; }
  else { opacity_[layer] = new_opacity; }
}

} // end namespace Seg3D

