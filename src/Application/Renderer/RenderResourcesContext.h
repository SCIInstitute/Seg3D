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

#ifndef APPLICATION_RENDER_RENDERRESOURCESCONTEXT_H
#define APPLICATION_RENDER_RENDERRESOURCESCONTEXT_H

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Core/Log.h>
#include <Application/Renderer/RenderContext.h>

namespace Seg3D {

// Forward declarations
class RenderResourcesContext;
typedef boost::shared_ptr<RenderResourcesContext> RenderResourcesContextHandle;

// Class definitions

class RenderResourcesContext : public boost::noncopyable {
    
// -- constructor/ destructor --
  public:  
    RenderResourcesContext();
    virtual ~RenderResourcesContext();
  
// -- functions implemented by GUI system --

  protected:
  
    friend class RenderResources;
    // CREATE_RENDER_CONTEXT:
    // Generate a render context for one of the viewers
    virtual bool create_render_context(RenderContextHandle& context) = 0;
    
    // SHARED_RENDER_CONTEXT:
    // Get the handle to the main shared render context
    virtual bool shared_render_context(RenderContextHandle& context) = 0;
        
};

} // end namespace Utils

#endif
