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

#ifndef APPLICATION_VIEWER_VIEWERRENDERER_H
#define APPLICATION_VIEWER_VIEWERRENDERER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes 
#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <Utils/Graphics/Texture.h>

namespace Seg3D
{

// Forward declarations
class ViewerRenderer;
typedef boost::shared_ptr< ViewerRenderer > ViewerRendererHandle;

// Class definitions
class ViewerRenderer : public boost::noncopyable
{

public:
  ViewerRenderer();
  virtual ~ViewerRenderer();

public:

  // Functions for drawing content
  virtual void initialize();
  virtual void redraw();

  virtual void resize( int width, int height );

  // Set the index of the corresponding viewer in the ViewerManager
  inline void set_viewer_id( size_t viewer_id )
  {
    this->viewer_id_ = viewer_id;
  }

  // Activate the renderer
  inline void activate() 
  { 
    lock_type lock( this->active_mutex_ );
    this->active_ = true; 
  }

  // Deactivate the renderer
  inline void deactivate()  
  {
    lock_type lock( this->active_mutex_ );
    this->active_ = false; 
  }

  // Return the status of the renderer
  inline bool is_active() 
  { 
    lock_type lock( this->active_mutex_ );
    return this->active_; 
  }

  // -- signals handling --
public:
  typedef boost::signals2::signal< void( Utils::TextureHandle ) > rendering_completed_signal_type;
  rendering_completed_signal_type rendering_completed_signal_;

protected:
  size_t viewer_id_;

  typedef boost::mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

private:
  // Indicates whether the renderer is active. It's set to false in the constructor.
  bool active_;
  // Mutex for protecting the active status
  mutex_type active_mutex_;
};

} // end namespace Seg3D


#endif

