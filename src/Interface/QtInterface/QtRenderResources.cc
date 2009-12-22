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

#include <Interface/QtInterface/QtRenderResources.h>

namespace Seg3D {

QtRenderContext::QtRenderContext(QGLContextHandle& context) :
  context_(context)
{
}

QtRenderContext::~QtRenderContext()
{
}


bool
QtRenderContext::is_valid()
{
  return(context_->isValid());
}

void 
QtRenderContext::make_current()
{
  context_->makeCurrent();
}

void 
QtRenderContext::done_current()
{
  context_->doneCurrent();
}

void 
QtRenderContext::swap_buffers()
{
  context_->swapBuffers();
}


QtRenderResourcesContext::QtRenderResourcesContext()
{
  // Step 1: set up the render format properties of all the OpenGL contexts that
  // are used. These need to be the same for all the contexts as they need to
  // shared textures and objects.
  
  // For now do not double buffer
  format_.setDoubleBuffer(false);
 
  // Create the shared context
  shared_qt_context_ = QGLContextHandle(new QGLContext(format_));
  shared_qt_context_->create();
  shared_context_ = RenderContextHandle(new QtRenderContext(shared_qt_context_));
}

QtRenderResourcesContext::~QtRenderResourcesContext()
{
  // Nothing to clean up, everything is handled by smart pointers
}

bool 
QtRenderResourcesContext::create_render_context(RenderContextHandle& context)
{
  // Generate a new context
  QGLContextHandle qt_context = QGLContextHandle(new QGLContext(format_));
  qt_context->create(shared_qt_context_.get());
  // Bind the new context in the GUI independent wrapper class
  context = RenderContextHandle(new QtRenderContext(qt_context));
  return (context->is_valid());
}

bool 
QtRenderResourcesContext::shared_render_context(RenderContextHandle& context)
{
  context = shared_context_;
  return (context->is_valid());
}

} // end namespace Seg3D
