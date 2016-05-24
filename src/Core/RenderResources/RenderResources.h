/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef CORE_RENDERRESOURCES_RENDERRESOURCES_H
#define CORE_RENDERRESOURCES_RENDERRESOURCES_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Lockable.h>
#include <Core/EventHandler/EventHandler.h>
#include <Core/RenderResources/RenderContext.h>
#include <Core/RenderResources/RenderResourcesContext.h>


namespace Core
{

// CLASS RENDERRESOURCES
// The resources for rendering in an abstract interface

// Forward declaration
class RenderResources;
class RenderResourcesPrivate;
typedef boost::shared_ptr< RenderResourcesPrivate > RenderResourcesPrivateHandle;

// Class definition
class RenderResources : private EventHandler, public RecursiveLockable
{
  CORE_SINGLETON( RenderResources );
  
  // -- constructor/destructor --
private:
  RenderResources();
  virtual ~RenderResources();

  // -- overloaded event handler --
private:
  // INITIALIZE_EVENTHANDLER:
  /// This function initializes the event handler associated with the singleton
  /// class. It is used to ensure that the OpenGL context for deleting objects has
  /// been initialized.
  virtual void initialize_eventhandler();

  // -- context handling --
public:
  // CREATE_RENDER_CONTEXT:
  /// Generate a render context for one of the viewers
  bool create_render_context( RenderContextHandle& context );

  // INSTALL_RESOURCES_CONTEXT:
  /// Install a context from the UI system in the resource, so this
  /// class can generate virtual OpenGL contexts.
  void install_resources_context( RenderResourcesContextHandle resources_context );

  // VALID_RENDER_RESOURCES:
  /// Check whether valid render resources have been installed
  bool valid_render_resources();
  
  // GET_CURRENT_CONTEXT:
  /// Get the current render context of the calling thread
  /// NOTE: The returned handle should only be used in the scope where this function is
  /// being called, because it may become invalid later.
  RenderContextHandle get_current_context();
  
  // GET_VRAM_SIZE:
  /// Return the total video memory size on the GPU.
  unsigned long get_vram_size();

  // -- thread safe deletion of GL objects --
private:
  friend class Texture;
  friend class BufferObject;
  friend class FramebufferObject;
  friend class Renderbuffer;
  friend class GLSLShader;
  friend class GLSLProgram;

  // INITIALIZE_ON_EVENT_THREAD:
  /// Initialize the delete context on the event handler thread.  This method takes care
  /// of any initialization that must take place either AFTER the eventhandler
  /// thread has been started OR the delete_context_ has been created.
  void initialize_on_event_thread();
  
  // DELETE_TEXTURE:
  /// Delete a texture within the right context
  void delete_texture( unsigned int texture_id );
  
  // DELETE_BUFFER_OBJECT:
  /// Delete a buffer object within the right context
  void delete_buffer_object( unsigned int buffer_id );

  // DELETE_FRAMEBUFFER_OBJECT:
  /// Delete a framebuffer object within the right context
  void delete_framebuffer_object( unsigned int framebuffer_id );

  // DELETE_RENDERBUFFER:
  /// Delete a renderbuffer within the right context
  void delete_renderbuffer( unsigned int renderbuffer_id );

  // DELETE_PROGRAM:
  /// Delete the given GLSL program.
  void delete_program( unsigned int program_id );

  // DELETE_SHADER:
  /// Delete the given GLSL shader.
  void delete_shader( unsigned int shader_id );

private:
  RenderResourcesPrivateHandle private_;
  
public:
  // GETMUTEX:
  /// Get the shared mutex for the opengl resources
  /// NOTE: Do not lock the RenderResources while StateEngine is locked as this will cause deadlock
  static mutex_type& GetMutex();
};

#define CORE_CHECK_OPENGL_ERROR()\
{\
  GLenum err = glGetError();\
  if (err != GL_NO_ERROR)\
  {\
    CORE_LOG_DEBUG(std::string("OpenGL error ") + Core::ExportToString(err) + ": " + \
      reinterpret_cast<const char*>(gluErrorString(err)));\
  }\
}

} // end namespace Core

#endif
