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

// Glew includes
#include <GL/glew.h>

// Utils includes
#include <Utils/Core/Exception.h>
#include <Utils/Core/Log.h>

// Application includes

// Interface includes
#include <Interface/QtInterface/QtRenderWidget.h>

namespace Seg3D {

QtRenderWidget::QtRenderWidget(const QGLFormat& format, QWidget* parent, QtRenderWidget* share) :
  QGLWidget(format,parent,share)
{
}

QtRenderWidget::~QtRenderWidget()
{
}

void
QtRenderWidget::initializeGL()
{
  glewInit();
}

void
QtRenderWidget::paintGL()
{
  glClearColor(0.25,0.25,0.25,0.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void
QtRenderWidget::resizeGL(int width,int height)
{
  glClearColor(0.25,0.25,0.25,0.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

} // end namespace Seg3D
