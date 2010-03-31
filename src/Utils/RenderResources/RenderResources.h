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

#ifndef UTILS_RENDERRESOURCES_RENDERRESOURCES_H
#define UTILS_RENDERRESOURCES_RENDERRESOURCES_H

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
#include <Utils/RenderResources/RenderContext.h>
#include <Utils/RenderResources/RenderResourcesContext.h>


namespace Utils
{

// CLASS RENDERRESOURCES
// The resources for rendering in an abstract interface

// Forward declaration
class RenderResources;

// Class definition
class RenderResources : public Singleton< RenderResources >, private EventHandler
{

  // -- constructor --
private:
  friend class Singleton< RenderResources >;
  RenderResources();
  virtual ~RenderResources();

  virtual void initialize_eventhandler();

  // -- context handling --
public:
  // CREATE_RENDER_CONTEXT:
  // Generate a render context for one of the viewers
  bool create_render_context( RenderContextHandle& context );

  // INSTALL_RESOURCES_CONTEXT:
  // Install a context from the UI system in the resource, so this
  // class can generate virtual OpenGL contexts.
  void install_resources_context( RenderResourcesContextHandle resources_context );

  // VALID_RENDER_RESOURCES:
  // Check whether valid render resources have been installed
  bool valid_render_resources();

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

  // A Handle to resource that generated the contexts
  RenderResourcesContextHandle resources_context_;

  RenderContextHandle render_context_;

  // -- render locks --
public:

  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  // GET_MUTEX:
  // reference to the mutex that protects the common pool of textures
  mutex_type& get_mutex()
  {
    return mutex_;
  }

private:
  // We need a lock that protects against multiple threads reserving
  // OpenGL resources at the same time
  mutex_type mutex_;

  // -- OpenGL initialization --
public:

  // Initialize the OpenGL environment
  void init_gl();

private:

  // State variable indicating whether the OpenGL environment has been initialized
  bool gl_initialized_;
  
public:
  // GETMUTEX:
  // Get the shared mutex for the opengl resources
  static mutex_type& GetMutex() 
  { 
    return Instance()->get_mutex(); 
  }

};

} // end namespace Utils

#endif
