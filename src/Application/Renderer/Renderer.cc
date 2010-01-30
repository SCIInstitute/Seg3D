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

#include <cstdlib>

// application includes
#include <Application/Renderer/Renderer.h>
#include <Application/Renderer/RenderResources.h>
#include <Utils/EventHandler/DefaultEventHandlerContext.h>

namespace Seg3D {

class RendererEventHandlerContext : public Utils::DefaultEventHandlerContext
{
  public:
     
     RendererEventHandlerContext() : DefaultEventHandlerContext() { }
     virtual ~RendererEventHandlerContext() { }
  
    virtual void post_event(Utils::EventHandle& event)
    {
      boost::unique_lock<boost::mutex> lock(event_queue_mutex_);
      
      // discard the previous rendering event
      //if (!event_queue_.empty())
      //{
      //  event_queue_.pop();
      //}
      
      event_queue_.push(event);
      event_queue_new_event_.notify_all();
    }
};

int Renderer::red = 1;

Renderer::Renderer() :
  EventHandler(), active_render_texture_(0), width_(0), 
  height_(0), redraw_needed_(false), resized_(false)
{
  red_ = (red++);
}

Renderer::~Renderer() 
{
}

void
Renderer::initialize()
{
  if (!RenderResources::Instance()->create_render_context(context_))
  {
    SCI_THROW_EXCEPTION("Failed to create a valid rendering context");
  }

  // lock the shared render context
  boost::unique_lock<RenderResources::mutex_type> lock(RenderResources::Instance()->shared_context_mutex());

  textures_[0] = TextureHandle(new Texture2D());
  textures_[1] = TextureHandle(new Texture2D());
  depth_buffer_ = RenderBufferHandle(new RenderBuffer());
  frame_buffer_ = FrameBufferObjectHandle(new FrameBufferObject());
  frame_buffer_->attach_render_buffer(depth_buffer_, GL_DEPTH_ATTACHMENT_EXT);
  
  // release the lock
  lock.unlock();
  
  // starting the rendering thread
  start_eventhandler();
}

void
Renderer::redraw()
{
  if (!is_eventhandler_thread())
  {
    boost::unique_lock<boost::mutex> lock(redraw_needed_mutex_);
    redraw_needed_ = true;
    post_event(boost::bind(&Renderer::redraw, this));
    return;
  }
  
  {
    boost::unique_lock<boost::mutex> lock(redraw_needed_mutex_);
    redraw_needed_ = false;
  }
  
  // make the rendering context current
  context_->make_current();

  // lock the active render texture
  boost::unique_lock<Texture::mutex_type> texture_lock(textures_[active_render_texture_]->get_mutex());

  frame_buffer_->attach_texture(textures_[active_render_texture_]);
  
  // bind the framebuffer object
  frame_buffer_->enable();
  //glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

  // do some rendering
  // ...
  // ...
  glViewport(0, 0, width_, height_); 
  glClearColor(0.15f*red_,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  //unsigned char* pixels = new unsigned char[(width_)*(height_)*3];
  //glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
  //unsigned char val = pixels[width_*height_+1];
  
  glFlush();
  
  //err = glGetError();
  
  //if (pixels != NULL)
  //{
  //  delete[] pixels;
  //}
   
  frame_buffer_->disable(); 
  
  // release the lock on the active render texture
  texture_lock.unlock();
  
  glFinish();
  
  // signal rendering completed
  rendering_completed_signal(textures_[active_render_texture_]);
   
  // swap render textures 
  active_render_texture_ = (~active_render_texture_)&1;
}

void
Renderer::resize(int width, int height)
{
  if (!is_eventhandler_thread())
  {
    post_event(boost::bind(&Renderer::resize, this, width, height));
    return;
  }
  
  if ( width == 0 || height == 0
      || (width_ == width && height_ == height) )
  {
    return;
  }
    
  {
    boost::unique_lock<RenderResources::mutex_type> lock(RenderResources::Instance()->shared_context_mutex());
    textures_[0] = TextureHandle(new Texture2D());
    textures_[1] = TextureHandle(new Texture2D());
  }
  textures_[0]->set_image(width, height, 1, GL_RGBA);
  textures_[1]->set_image(width, height, 1, GL_RGBA);
  
  depth_buffer_->set_storage(width, height, GL_DEPTH_COMPONENT);

  width_ = width;
  height_ = height;
  resized_ = true;
  
  redraw();
}

} // end namespace Seg3D


