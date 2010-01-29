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

#ifndef APPLICATION_RENDERER_RENDERER_H
#define APPLICATION_RENDERER_RENDERER_H

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// boost includes
#include <boost/thread/mutex.hpp>

// Application includes
#include <Application/Renderer/RenderContext.h>
#include <Application/Viewer/ViewerRenderer.h>
#include <Application/Renderer/Texture.h>
#include <Application/Renderer/RenderBuffer.h>
#include <Application/Renderer/FrameBufferObject.h>
#include <Utils/EventHandler/EventHandler.h>

namespace Seg3D {

// Forward declarations
class Renderer;
typedef boost::shared_ptr<Renderer> RendererHandle;

// Class definitions
class Renderer : public ViewerRenderer, private Utils::EventHandler {

// -- constructor/destructor --
  public:
    Renderer();
    virtual ~Renderer();

  public:
  
    virtual void initialize();
    virtual void redraw();
    
    virtual void resize(int width, int height);

  private:
    
    // Context for rendering images
    RenderContextHandle context_;
    
    TextureHandle textures_[2];
    RenderBufferHandle depth_buffer_;
    FrameBufferObjectHandle frame_buffer_;
    int active_render_texture_;
    
    int width_;
    int height_;
    bool redraw_needed_;
    bool resized_;
    boost::mutex redraw_needed_mutex_;
    
    static int red;
    int red_;
};

} // end namespace Seg3D

#endif
