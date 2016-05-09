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

#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Exception.h>
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Utils/QtRenderResources.h>

namespace QtUtils
{

// CLASS: QtRenderContext

// Forward declarations
// The QtRenderContext class is an implementation of the openGL context
// on top of the abstract interface that is used for managing OpenGL contexts
class QtRenderContext;
typedef boost::shared_ptr< QtRenderContext > QtRenderContextHandle;

// Shared pointer to one of Qt's internal resources
// NOTE: As GLContext objects are not managed by Qt we
// need to do this ourselves using a smart pointer
typedef boost::shared_ptr< QGLContext > QGLContextHandle;

class QtRenderContext : public Core::RenderContext
{
  // -- constructor/ destructor --
public:
  QtRenderContext( QGLContextHandle context );
  virtual ~QtRenderContext();

  // -- context functions --
  // IS_VALID:
  // Test whether the context is valid
  virtual bool is_valid() const;

  // MAKE_CURRENT:
  // Set the rendering context current to this thread
  virtual void make_current();

  // DONE_CURRENT:
  // Indicate that rendering using this context is done for now
  virtual void done_current();

  // SWAP_BUFFERS:
  // Swap the front and back buffers
  virtual void swap_buffers() const;

private:
  QGLContextHandle context_;
};

class QtRenderResourcesContextPrivate
{
public:
  // The Qt render context format options
  QGLFormat format_;

  // The handle to the first qt widget that defines all the sharing
  // between contexts
  QGLWidget* shared_widget_;
};

QtRenderContext::QtRenderContext( QGLContextHandle context ) :
  context_( context )
{
}

QtRenderContext::~QtRenderContext()
{
}

bool QtRenderContext::is_valid() const
{
  return ( context_->isValid() );
}

void QtRenderContext::make_current()
{
  context_->makeCurrent();
}

void QtRenderContext::done_current()
{
  context_->doneCurrent();
}

void QtRenderContext::swap_buffers() const
{
  context_->swapBuffers();
}


// CLASS: QtRenderResourcesContext
// Implementation details of this class

QtRenderResourcesContext::QtRenderResourcesContext() :
  private_( new QtRenderResourcesContextPrivate )
{
  this->private_->format_ = QGLFormat::defaultFormat();
  this->private_->shared_widget_ = new QGLWidget( this->private_->format_ );
}

QtRenderResourcesContext::~QtRenderResourcesContext()
{
  delete this->private_->shared_widget_;
}

bool QtRenderResourcesContext::create_render_context( Core::RenderContextHandle& context )
{
  // Generate a new context
  QGLContextHandle qt_context = QGLContextHandle( new QGLContext( this->private_->format_,
    this->private_->shared_widget_->context()->device() ) );
  qt_context->create( this->private_->shared_widget_->context() );

  // Bind the new context in the GUI independent wrapper class
  context = Core::RenderContextHandle( new QtRenderContext( qt_context ) );

  return context->is_valid();
}

QtRenderWidget* QtRenderResourcesContext::create_qt_render_widget( QWidget* parent, 
                          Core::AbstractViewerHandle viewer )
{
  CORE_LOG_DEBUG( "Create an OpenGL widget" );

  return new QtRenderWidget( this->private_->format_, parent, 
    this->private_->shared_widget_, viewer );
}

QtTransferFunctionWidget* QtRenderResourcesContext::create_qt_transfer_function_widget( 
  QWidget* parent, Core::TransferFunctionHandle tf )
{
  return new QtTransferFunctionWidget( this->private_->format_, parent,
    this->private_->shared_widget_, tf );
}

bool QtRenderResourcesContext::valid_render_resources()
{
  return this->private_->shared_widget_->isValid();
}

class SharedPtrNopDeleter
{
public:
  template< class T >
  void operator()( T* p )
  {
  }
};

Core::RenderContextHandle QtRenderResourcesContext::get_current_context()
{
  const QGLContext* current_context = QGLContext::currentContext();
  if ( current_context != 0 )
  {
    // NOTE: We don't want to delete the context after the handle goes out of scope,
    // so we pass a NOP deleter to shared_ptr
    return Core::RenderContextHandle( new QtRenderContext( QGLContextHandle( 
      const_cast< QGLContext* >( current_context ), SharedPtrNopDeleter() ) ) );
  }
  
  return Core::RenderContextHandle();
}

} // end namespace Seg3D
