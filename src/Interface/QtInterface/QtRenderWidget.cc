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
#include <Application/Interface/Interface.h>

namespace Seg3D {

QtRenderWidget::QtRenderWidget(const QGLFormat& format, QWidget* parent, QtRenderWidget* share) :
  QGLWidget(format,parent,share)
{
  setAutoFillBackground(false);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
}

QtRenderWidget::~QtRenderWidget()
{
  rendering_completed_connection_.disconnect();
}

void 
QtRenderWidget::rendering_completed_slot(TextureHandle texture)
{
  // if not in the interface thread, post an event to the interface thread
  if (!Interface::IsInterfaceThread())
  {
    Interface::PostEvent(boost::bind(&QtRenderWidget::rendering_completed_slot, this, texture));
    return;
  }
  
  //boost::unique_lock<boost::recursive_mutex> lock(renderer_texture_mutex_);
  renderer_texture_ = texture;
  updateGL();
}

void
QtRenderWidget::initializeGL()
{
  RenderResources::Instance()->init_gl();
  
  renderer_ = RendererHandle(new Renderer());
  rendering_completed_connection_ 
    = renderer_->rendering_completed_signal.connect(boost::bind(&QtRenderWidget::rendering_completed_slot, this, _1));
  renderer_->redraw();
}

void
QtRenderWidget::paintGL()
{
  //return;
  
  
  //glDrawBuffer(GL_BACK);
  //glClearColor(0.25,0.25,1.0,0.0);
  //glClear(GL_COLOR_BUFFER_BIT);
  //return;

  if (!renderer_texture_.get())
  {
    return;
  }
  
  //boost::unique_lock<boost::recursive_mutex> lock(renderer_texture_mutex_);  
  renderer_texture_->lock();
  
  // draw a window size quad and map the render texture onto it
  QSize view_size = QWidget::size();
  int width = view_size.width();
  int height = view_size.height();
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //glActiveTexture(GL_TEXTURE0);
  //renderer_texture_->enable();
  //renderer_texture_->disable();
  //glFinish();
  renderer_texture_->enable();
  GLenum err = glGetError();
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  //GLenum err = glGetError();
  //const GLubyte* err_str = gluErrorString(err);
  glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 1.f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(width, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(width, height);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, height);
  glEnd();
  renderer_texture_->disable();
  
  renderer_texture_->unlock();
}

void
QtRenderWidget::resizeGL(int width,int height)
{
  //return;
  //glClearColor(0.25,0.25,0.25,0.0);
  //glClear(GL_COLOR_BUFFER_BIT);
  
  glViewport(0, 0, width, height);  
  paintGL();
  //
  if (renderer_.get())
  {
    renderer_texture_ = TextureHandle();
    renderer_->resize(width, height);
  }
  
}

} // end namespace Seg3D
