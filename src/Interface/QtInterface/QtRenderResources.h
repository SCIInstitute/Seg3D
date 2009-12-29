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

#ifndef INTERFACE_QTINTERFACE_QTRENDERRESOURCES_H
#define INTERFACE_QTINTERFACE_QTRENDERRESOURCES_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QtOpenGL>
#include <QtGui>

// Application includes
#include <Application/Renderer/RenderResourcesContext.h>
#include <Application/Renderer/RenderContext.h>

// Interface includes
#include <Interface/QtInterface/QtRenderWidget.h>

namespace Seg3D {

// Forward declarations
class QtRenderContext;
class QtRenderResourcesContext;
typedef boost::shared_ptr<QtRenderContext> QtRenderContextHandle;
typedef boost::shared_ptr<QtRenderResourcesContext> QtRenderResourcesContextHandle;

// Shared pointer to one of Qt's internal resources
// NOTE: As GLContext objects are not managed by Qt we
// need to do this ourselves using a smart pointer

typedef boost::shared_ptr<QGLContext> QGLContextHandle;


class QtRenderContext : public RenderContext {

// -- constructor/ destructor --
  public:
    QtRenderContext(QGLContextHandle& context);
    virtual ~QtRenderContext();

// -- context functions --
    // IS_VALID:
    // Test whether the context is valid
    virtual bool is_valid();

    // MAKE_CURRENT:
    // Set the rendering context current to this thread
    virtual void make_current();

    // DONE_CURRENT:
    // Indicate that rendering using this context is done for now
    virtual void done_current();
    
    // SWAP_BUFFERS:
    // Swap the front and back buffers
    virtual void swap_buffers();

  private:
    QGLContextHandle context_;
};


class QtRenderResourcesContext : public RenderResourcesContext {

  public:
    QtRenderResourcesContext();
    virtual ~QtRenderResourcesContext();

  public:
    // CREATE_RENDER_CONTEXT:
    // Generate a render context for one of the viewers
    virtual bool create_render_context(RenderContextHandle& context);

    // CREATE_QT_RENDER_WIDGET:
    // Get the Qt render context directly
    // NOTE: The viewers access this directly
    QtRenderWidget* create_qt_render_widget(QWidget* parent);

    // VALID_RENDER_RESOURCES:
    // Check whether valid render resources were installed
    virtual bool valid_render_resources();

  private:
    // The Qt render context format options
    QGLFormat format_;
  
    // The handle to the first qt widget that defines all the sharing
    // between contexts
    QPointer<QtRenderWidget> shared_widget_;
    
};

} // end namespace Seg3D

#endif
