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

Layer::Layer(const std::string& name, 
             Utils::VolumeType type, 
             const Utils::VolumeHandle& volume) :
  StateHandler(name),
  type_(type),
  volume_(volume)
{
  // Step (1) : Build the layer specific state variables
  
  // == The name of the layer ==
  add_state("name",name_state_,name);

  // == Visibility information for this layer per viewer ==
  size_t num_viewers = Application::Instance()->number_of_viewers();
  visible_state_.resize(num_viewers);

  for (size_t j=0; j< visible_state_.size(); j++)
  {
    std::string key = std::string("visible")+Utils::to_string(j);
    add_state(key,visible_state_[j],true);
  }
  
  // == The state of the lock ==
  add_state("lock",lock_state_,false);
  
  // == The opacity of the layer ==
  add_state("opacity",opacity_state_,1.0,0.0,1.0,0.05);
  
  // == Selected by the LayerGroup ==
  add_state("selected",selected_state_,true);
  
  // == Which of the submenus is being editted ==
  add_state("edit_mode",edit_mode_state_,"none",
            "none|opacity|color|contrast|appearance");
  
  // Step (2) : Update internal state
  
  // == cache the grid transform in the Layer class ==
  if ( volume_.get() ) grid_transform_ = volume_->grid_transform();

}
  

Layer::~Layer()
{
  // Disconnect all current connections
  disconnect_all();
}


void
Layer::set_volume(Utils::VolumeHandle volume)
{
  volume_ = volume;
  if ( volume_.get() ) grid_transform_ = volume->grid_transform();
}



} // end namespace Seg3D

