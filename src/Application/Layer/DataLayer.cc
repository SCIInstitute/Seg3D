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
#include <Application/Layer/DataLayer.h>

namespace Seg3D {

DataLayer::DataLayer(const std::string& name, const Utils::VolumeHandle& volume) :
  Layer(name,Utils::DATA_E,volume)
{
  // Step (1) : Build the layer specific state variables
  
  // == Contrast ==
  add_state("contrast",contrast_state_,0.5,0.0,1.0,0.05);

  // == Brightness ==
  add_state("brightness",brightness_state_,0.5,0.0,1.0,0.05);
  
  // == Is this volume rendered through the volume renderer ==
  add_state("volume_rendered",volume_rendered_state_,false);
}
  

DataLayer::~DataLayer()
{
  // Disconnect all current connections
  disconnect_all();
}


} // end namespace Seg3D

