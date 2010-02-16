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


#ifndef APPLICATION_VIEWER_VIEWERMANAGER_H
#define APPLICATION_VIEWER_VIEWERMANAGER_H 1

//#ifdef (_MSC_VER) && (_MSC_VER >= 1020)
//# pragma once
//#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

// Application includes
#include <Utils/Singleton/Singleton.h>
#include <Application/Viewer/Viewer.h>
#include <Application/State/State.h>


namespace Seg3D {

// Forward declarations
class ViewerManager;

// typedefs
class ViewerManager : public StateHandler {

// -- Constructor/Destructor --
  private:
    friend class Utils::Singleton<ViewerManager>;
    ViewerManager();
  
  public:
    virtual ~ViewerManager();
  
// -- Getting information from manager -- 
  
  public:
    ViewerHandle get_viewer(size_t idx);

// -- State information --
  public:    
    StateOptionHandle   layout_state_;
    StateIntHandle      active_viewer_state_;

// -- Viewer information --
  private:
  
    std::vector<ViewerHandle> viewers_;
  
// -- Singleton interface --
  public:
    
    // INSTANCE:
    // Get the singleton interface
    static ViewerManager* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<ViewerManager> instance_;

}; // class ViewerManager

} // end namespace Seg3D

#endif
