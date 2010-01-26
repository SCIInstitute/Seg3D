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

#ifndef APPLICATION_VIEWER_VIEWER_H
#define APPLICATION_VIEWER_VIEWER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <vector>

// Boost includes 
#include <boost/shared_ptr.hpp>

// Application includes
#include <Application/State/StateHandler.h>
#include <Application/Viewer/ViewerRenderer.h>

namespace Seg3D {

// Forward declarations
class Viewer;
typedef boost::shared_ptr<Viewer> ViewerHandle;


// Class declarations

class Viewer : public StateHandler {

// -- constructor/destructor --
  public:
    Viewer(const std::string& key);
    virtual ~Viewer();

// -- State information --
  public:
    StateOptionHandle         viewer_type_;
    StateBoolHandle    viewer_flip_x_;
    StateBoolHandle    viewer_flip_y_;
    StateBoolHandle    viewer_grid_;
    StateBoolHandle    viewer_lock_;
    
// -- Renderer information --
  public:
    
    ViewerRendererHandle renderer() { return renderer_; }
    void set_renderer(ViewerRendererHandle renderer) { renderer_ = renderer; }


  private:
    // A handle to the renderer that is used to render the data
    ViewerRendererHandle renderer_;
    
        
};

} // end namespace Seg3D

#endif
