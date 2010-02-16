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

// STL includes

// Boost includes 

#include <Application/Application/Application.h>
#include <Application/Layer/Layer.h>


namespace Seg3D {

Layer::Layer(std::string& name) :
  StateHandler(name)
{
  // Step (1) : Build the viewer dependent state variables
  size_t num_viewers = Application::Instance()->number_of_viewers();

  opacity_state_.resize(num_viewers);
  visibility_state_.resize(num_viewers);
  border_state_.resize(num_viewers);
  fill_state_.resize(num_viewers);

  for (size_t j=0; j<num_viewers; j++)
  {
    std::string key;
    
    key = std::string("opacity")+Utils::to_string(j);
    add_state(key,opacity_state_[j],1.0);

    key = std::string("visibitily")+Utils::to_string(j);
    add_state(key,visibility_state_[j],true);

    key = std::string("border")+Utils::to_string(j);
    add_state(key,border_state_[j],true);

    key = std::string("fill")+Utils::to_string(j);
    add_state(key,border_state_[j],true);
  }

  // Step (2) : Build the layer specific state variables
  
  add_state("name",name_state_,name);
  add_state("lock",lock_state_,false);
  add_state("color_index",color_index_state_,0);
  add_state("contrast",contrast_state_,1.0);
  add_state("brighness",brightness_state_,1.0);
  
}
  
Layer::~Layer()
{
  disconnect_all();
}

} // end namespace Seg3D

