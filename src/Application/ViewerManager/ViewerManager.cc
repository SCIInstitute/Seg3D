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

// Application includes
#include <Application/Viewer/Viewer.h> 
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Interface/Interface.h>
 

namespace Seg3D {

ViewerManager::ViewerManager() :
  StateHandler("view")
{
  add_state("layout",layout_state,"2and3","single|1and1|1and2|1and3|2and2|2and3|3and3");
  
  viewers_.resize(6);
  for (size_t j=0; j< viewers_.size(); j++)
  {
    std::string key = std::string("viewer")+Utils::to_string(j);
    viewers_[j] = ViewerHandle(new Viewer(key));
  }
}
  
ViewerManager::~ViewerManager()
{
}

ViewerHandle 
ViewerManager::get_viewer(size_t idx)
{
  ViewerHandle handle;
  if (idx < viewers_.size()) handle = viewers_[idx];
  return handle;
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<ViewerManager> ViewerManager::instance_;

} // end namespace Seg3D

