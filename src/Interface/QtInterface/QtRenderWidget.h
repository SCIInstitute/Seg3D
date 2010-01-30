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
 
#ifndef INTERFACE_QTINTERFACE_QTRENDERWIDGET_H
#define INTERFACE_QTINTERFACE_QTRENDERWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 
 
#include <Application/Renderer/RenderResources.h> 
#include <Application/Renderer/Renderer.h>
#include <Application/Viewer/ViewerRenderer.h>

#include <QtOpenGL>
 
namespace Seg3D {

class QtRenderWidget : public QGLWidget {
    Q_OBJECT

  public:
  
    QtRenderWidget(const QGLFormat& format, QWidget* parent, QtRenderWidget* share);
    virtual ~QtRenderWidget();
    
    void rendering_completed_slot(TextureHandle texture);
    
    ViewerRendererHandle renderer()
    {
      return renderer_;
    }

  protected:
  
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);

  private:
    RendererHandle renderer_;
    TextureHandle renderer_texture_;
    
    boost::signals2::connection rendering_completed_connection_;
};

} // end namespace Seg3D

#endif