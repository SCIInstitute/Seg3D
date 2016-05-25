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

#include <GL/glew.h>

#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __APPLE__
//#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

// boost includes
#include <boost/thread/condition_variable.hpp>

#include <Core/RenderResources/RenderResources.h>
#include <Core/Utils/Exception.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( RenderResources );

class RenderResourcesPrivate
{
public:
  // initialize_gl:
  // Initialize GLEW and check required OpenGL version and extensions.
  void initialize_gl();
  
  // Query available video memory size.
  void query_video_memory_size();
  
  // A Handle to resource that generated the contexts
  RenderResourcesContextHandle resources_context_;

  // An GL context for deleting shared objects
  // NOTE: Since objects need to be deleted inside an OpenGL context
  // this context has been specially constructed for deleting objects
  // when their handles go out of scope. As these objects are maintained
  // through the program using handles, the OpenGL context in which they
  // were created may be deleted already. Hence we delete them all in
  // a separate thread using a separate OpenGL context.
  RenderContextHandle delete_context_;

  // Mutex and condition variable to make sure the RenderResources thread has 
  // completed initialization before continuing the main thread.
  boost::mutex thread_mutex_;
  boost::condition_variable thread_condition_variable_;

  bool gl_capable_;
  unsigned long vram_size_;
};
  
void RenderResourcesPrivate::initialize_gl()
{
  this->gl_capable_ = true;
  
  int err = glewInit();
  if ( err != GLEW_OK )
  {
    this->gl_capable_ = false;
    CORE_LOG_ERROR( "glewInit failed with error code " + Core::ExportToString( err ) );
  }
  
  // Check OpenGL capabilities
  if ( !GLEW_VERSION_2_0 )
  {
    this->gl_capable_ = false;
    CORE_LOG_ERROR( "OpenGL 2.0 required but not found." );
  }
  if ( !GLEW_EXT_framebuffer_object )
  {
    this->gl_capable_ = false;
    CORE_LOG_ERROR( "GL_EXT_framebuffer_object required but not found." );
  }
  if ( !GLEW_ARB_pixel_buffer_object )
  {
    this->gl_capable_ = false;
    CORE_LOG_ERROR( "GL_ARB_pixel_buffer_object required but not found." );
  }
}
  
void RenderResourcesPrivate::query_video_memory_size()
{
  this->vram_size_ = 0;
  unsigned long vram_size_MB = 0;
  
#if defined(_WIN32)
  const char HARDWARE_DEVICEMAP_VIDEO_C[] = "HARDWARE\\DEVICEMAP\\VIDEO";
  const char MAX_OBJECT_NUMBER_C[] = "MaxObjectNumber";
  const char SYSTEM_CURRENTCONTROLSET_C[] = "system\\currentcontrolset";
  const char HARDWAREINFO_MEMSIZE[] = "HardwareInformation.MemorySize";

  HKEY video_devicemap_key;
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, HARDWARE_DEVICEMAP_VIDEO_C, 0, 
    KEY_READ, &video_devicemap_key ) == ERROR_SUCCESS )
  {
    DWORD type = REG_DWORD;
    DWORD max_object_number = 0;
    DWORD buffer_size = sizeof( DWORD );
    if ( RegQueryValueEx( video_devicemap_key, MAX_OBJECT_NUMBER_C, NULL, &type, 
      reinterpret_cast< LPBYTE >( &max_object_number ), &buffer_size ) 
      == ERROR_SUCCESS )
    {
      for ( DWORD i = 0; i <= max_object_number && this->vram_size_ == 0; ++i )
      {
        DWORD type = REG_SZ;
        std::string video_device_name = "\\Device\\Video" + ExportToString( i );
        if ( RegQueryValueEx( video_devicemap_key, video_device_name.c_str(), NULL, 
          &type, NULL, &buffer_size ) != ERROR_SUCCESS )
        {
          continue;
        }

        std::vector< BYTE > buffer( buffer_size );
        if ( RegQueryValueEx( video_devicemap_key, video_device_name.c_str(), NULL, 
          NULL, &buffer[ 0 ], &buffer_size ) != ERROR_SUCCESS )
        {
          continue;
        }
        std::string video_device_key_name( reinterpret_cast< char* >( &buffer[ 0 ] ) );
        video_device_key_name = StringToLower( video_device_key_name );
        size_t pos = video_device_key_name.find( SYSTEM_CURRENTCONTROLSET_C );
        if ( pos == std::string::npos )
        {
          continue;
        }
        video_device_key_name = video_device_key_name.substr( pos );
        HKEY video_device_key;
        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, video_device_key_name.c_str(),
          0, KEY_READ, &video_device_key ) != ERROR_SUCCESS )
        {
          continue;
        }
        DWORD vram_size = 0;
        buffer_size = sizeof( DWORD );
        type = REG_BINARY;
        
        if ( RegQueryValueEx( video_device_key, HARDWAREINFO_MEMSIZE, NULL, &type, 
          reinterpret_cast< LPBYTE >( &vram_size ), &buffer_size ) == ERROR_SUCCESS )
        {
          this->vram_size_ = vram_size;
          vram_size_MB = this->vram_size_ >> 20;
        }
        RegCloseKey( video_device_key );
      } // end for
      
    }

    RegCloseKey( video_devicemap_key );
  }
#elif defined(__APPLE__)

  CGLRendererInfoObj info;
  GLint infoCount = 0;

  CGLError e = CGLQueryRendererInfo( CGDisplayIDToOpenGLDisplayMask( CGMainDisplayID() ), &info, &infoCount );
  if (e != kCGLNoError)
  {
    CORE_LOG_WARNING( CGLErrorString(e) );
  }

  for (int i = 0; i < infoCount; ++i)
  {
    GLint vram_size = 0;
    CGLDescribeRenderer(info, i, kCGLRPVideoMemoryMegabytes, &vram_size);
    if ( vram_size > 0 && vram_size > this->vram_size_ )
    {
      vram_size_MB = static_cast< unsigned long >( vram_size );
      this->vram_size_ = vram_size_MB << 20;
    }
  }
  CGLDestroyRendererInfo(info);

#else
  // TODO: Add support for Linux
#endif

  if ( this->vram_size_ == 0 ) 
  {
    CORE_LOG_WARNING( "Failed to query video memory size." );
    CORE_LOG_WARNING( "Assuming system has at least 128 MB of graphics memory." );
    this->vram_size_ = 128 * (1 << 20);
  }
  else 
  {
    CORE_LOG_MESSAGE( "Video Memory Size: " + ExportToString( vram_size_MB ) + " MB." );
  }
}

RenderResources::RenderResources() :
  private_( new RenderResourcesPrivate )
{
  this->private_->gl_capable_ = false;
}

RenderResources::~RenderResources()
{
}

void RenderResources::initialize_eventhandler()
{
  boost::unique_lock< boost::mutex > lock( this->private_->thread_mutex_ );
  this->private_->thread_condition_variable_.notify_one();
}

bool RenderResources::create_render_context( RenderContextHandle& context )
{
  lock_type lock( this->get_mutex() );

  // The context gets setup through the GUI system and is GUI dependent
  // if this function is accessed before the GUI system is setup, something
  // is wrong in the program logic, hence warn the user
  if ( ! this->private_->resources_context_.get() )
  {
    CORE_THROW_LOGICERROR(
      "No render resources were installed to create an opengl context" );
  }

  return this->private_->resources_context_->create_render_context( context );
}

RenderContextHandle RenderResources::get_current_context()
{
  return this->private_->resources_context_->get_current_context();
}
  
unsigned long RenderResources::get_vram_size()
{
  return this->private_->vram_size_;
}

void RenderResources::install_resources_context( RenderResourcesContextHandle resources_context )
{
  // Check whether we got a proper render context
  if ( !resources_context.get() )
  {
    CORE_THROW_LOGICERROR("Cannot install an empty render resources context");
  }

  if ( this->private_->resources_context_ )
  {
    CORE_THROW_LOGICERROR( "A RenderResourcesContext has already been installed" );
  }
  
  this->private_->resources_context_ = resources_context;

  // Start the event handler thread and then create the GL context
  boost::unique_lock< boost::mutex > lock( this->private_->thread_mutex_ );
  this->start_eventhandler();
  this->private_->thread_condition_variable_.wait( lock );
  this->initialize_on_event_thread();
}

bool RenderResources::valid_render_resources()
{
  return ( this->private_->resources_context_ && 
         this->private_->resources_context_->valid_render_resources() && 
         this->private_->delete_context_ &&
         this->private_->gl_capable_ );
}

void RenderResources::initialize_on_event_thread()
{
  if (!this->is_eventhandler_thread())
  {
    this->post_event( boost::bind( &RenderResources::initialize_on_event_thread, this ) );
    return;
  }

  boost::unique_lock< boost::mutex > lock( this->private_->thread_mutex_ );
  this->private_->resources_context_->create_render_context( this->private_->delete_context_ );
  this->private_->delete_context_->make_current();
  this->private_->initialize_gl();
  this->private_->query_video_memory_size();

}

void RenderResources::delete_texture( unsigned int texture_id )
{
  // If the calling thread doesn't have a valid OpenGL context, 
  // repost the delete operation to the default RenderResources thread.
  // Otherwise, delete the object immediately.
  if ( !this->is_eventhandler_thread() && !this->get_current_context() )
  {
    this->post_event( boost::bind( &RenderResources::delete_texture, this, 
      texture_id ) );
    return;     
  }
  
  lock_type lock( RenderResources::GetMutex() );
  if ( glIsTexture( texture_id ) )
  {
    glDeleteTextures( 1, &texture_id );
    CORE_CHECK_OPENGL_ERROR();
  }
}

void RenderResources::delete_renderbuffer( unsigned int renderbuffer_id )
{
  // If the calling thread doesn't have a valid OpenGL context, 
  // repost the delete operation to the default RenderResources thread.
  // Otherwise, delete the object immediately.
  if ( !this->is_eventhandler_thread() && !this->get_current_context() )
  {
    this->post_event( boost::bind( &RenderResources::delete_renderbuffer, this, 
      renderbuffer_id ) );
    return;     
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteRenderbuffersEXT( 1, &renderbuffer_id );
  CORE_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_buffer_object( unsigned int buffer_object_id )
{
  // If the calling thread doesn't have a valid OpenGL context, 
  // repost the delete operation to the default RenderResources thread.
  // Otherwise, delete the object immediately.
  if ( !this->is_eventhandler_thread() && !this->get_current_context() )
  {
    this->post_event( boost::bind( &RenderResources::delete_buffer_object, this, 
      buffer_object_id ) );
    return;     
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteBuffers( 1, &buffer_object_id );
  CORE_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_framebuffer_object( unsigned int framebuffer_object_id )
{
  // If the calling thread doesn't have a valid OpenGL context, 
  // repost the delete operation to the default RenderResources thread.
  // Otherwise, delete the object immediately.
  if ( !this->is_eventhandler_thread() && !this->get_current_context() )
  {
    this->post_event( boost::bind( &RenderResources::delete_framebuffer_object, this, 
      framebuffer_object_id ) );
    return;     
  }
  
  lock_type lock( RenderResources::GetMutex() );
  glDeleteFramebuffersEXT( 1, &framebuffer_object_id );
  CORE_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_program( unsigned int program_id )
{
  // If the calling thread doesn't have a valid OpenGL context, 
  // repost the delete operation to the default RenderResources thread.
  // Otherwise, delete the object immediately.
  if ( !this->is_eventhandler_thread() && !this->get_current_context() )
  {
    this->post_event( boost::bind( &RenderResources::delete_program, this, 
      program_id ) );
    return;     
  }

  lock_type lock( RenderResources::GetMutex() );
  glDeleteProgram( program_id );
  CORE_CHECK_OPENGL_ERROR();
}

void RenderResources::delete_shader( unsigned int shader_id )
{
  // If the calling thread doesn't have a valid OpenGL context, 
  // repost the delete operation to the default RenderResources thread.
  // Otherwise, delete the object immediately.
  if ( !this->is_eventhandler_thread() && !this->get_current_context() )
  {
    this->post_event( boost::bind( &RenderResources::delete_shader, this, 
      shader_id ) );
    return;     
  }

  lock_type lock( RenderResources::GetMutex() );
  if ( glIsShader( shader_id ) )
  {
    glDeleteShader( shader_id );
  }
  CORE_CHECK_OPENGL_ERROR();
}

RenderResources::mutex_type& RenderResources::GetMutex() 
{ 
  return Instance()->get_mutex(); 
}

} // end namespace Core

