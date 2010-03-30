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

#ifndef UTILS_RENDERRESOURCES_RENDERRESOURCESEVENTHANDLER_H
#define UTILS_RENDERRESOURCES_RENDERRESOURCESEVENTHANDLER_H

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Core/Exception.h>
#include <Utils/Core/Log.h>
#include <Utils/Core/Singleton.h>
#include <Utils/EventHandler/EventHandler.h>
#include <Utils/RenderResources/RenderResources.h>

namespace Utils
{

class RenderResourcesEventHandler;
typedef boost::shared_ptr<RenderResourcesEventHandler> RenderResourcesEventHandlerHandle;

class RenderResourcesEventHandler : public Singleton< RenderResourcesEventHandler >,
  public EventHandler
{
  // -- constructor --
private:
  friend class Singleton< RenderResourcesEventHandler >;
  RenderResourcesEventHandler();
  virtual ~RenderResourcesEventHandler();

  virtual void initialize_eventhandler();
  
public:
  
  // DELETE_TEXTURE:
  // Delete a texture within the right context
  void delete_texture( unsigned int texture_id );
  
  // DELETE_BUFFER_OBJECT:
  // Delete a buffer object within the right context
  void delete_buffer_object( unsigned int buffer_id );

  // DELETE_FRAMEBUFFER_OBJECT:
  // Delete a framebuffer object within the right context
  void delete_framebuffer_object( unsigned int framebuffer_id );

  // DELETE_RENDERBUFFER:
  // Delete a renderbuffer within the right context
  void delete_renderbuffer( unsigned int renderbuffer_id );

private:
  RenderContextHandle context_;
};

} // end namespace Utils

#endif
