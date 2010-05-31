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

#ifndef QTINTERFACE_UTILS_QTRENDERRESOURCES_H
#define QTINTERFACE_UTILS_QTRENDERRESOURCES_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Glew include
#include <GL/glew.h>

// Qt includes
#include <QtOpenGL>
#include <QPointer>

// Application includes
#include <Core/RenderResources/RenderResourcesContext.h>
#include <Core/RenderResources/RenderContext.h>
#include <Core/Viewer/AbstractViewer.h>

#include <QtInterface/Utils/QtRenderWidget.h>

namespace QtUtils
{

// CLASS: QtRenderResourcesContext
// Qt specific implementation of the render resources class

// Forward declarations
class QtRenderResourcesContext;
class QtRenderResourcesContextPrivate;
typedef boost::shared_ptr< QtRenderResourcesContext > 
  QtRenderResourcesContextHandle;
typedef boost::shared_ptr< QtRenderResourcesContextPrivate > 
  QtRenderResourcesContextPrivateHandle;

// Class definition
class QtRenderResourcesContext : public Core::RenderResourcesContext
{

public:
  QtRenderResourcesContext();
  virtual ~QtRenderResourcesContext();

public:
  // CREATE_RENDER_CONTEXT:
  // Generate a render context for one of the viewers
  virtual bool create_render_context( Core::RenderContextHandle& context );

  // CREATE_QT_RENDER_WIDGET:
  // Get the Qt render context directly
  // NOTE: The viewers access this directly
  QtRenderWidget* create_qt_render_widget( QWidget* parent, Core::AbstractViewerHandle viewer );

  // VALID_RENDER_RESOURCES:
  // Check whether valid render resources were installed
  virtual bool valid_render_resources();

  // GET_CURRENT_CONTEXT:
  // Get the current render context of the calling thread
  // NOTE: The returned handle should only be used in the scope where this function is
  // being called, because it may become invalid later.
  virtual Core::RenderContextHandle get_current_context();

private:
  QtRenderResourcesContextPrivateHandle private_;

};

} // end namespace QtUtils

#endif
