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

#ifndef QTUTILS_UTILS_QTTRANSFERFUNCTIONWIDGET_H
#define QTUTILS_UTILS_QTTRANSFERFUNCTIONWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

// Glew includes
#include <GL/glew.h>

// Qt includes
#undef __GLEW_H__
#include <QtOpenGL>
#define __GLEW_H__

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/VolumeRenderer/TransferFunction.h>

#endif

namespace QtUtils
{

class QtTransferFunctionWidget;
class QtTransferFunctionWidgetPrivate;
typedef boost::shared_ptr<QtTransferFunctionWidgetPrivate> QtTransferFunctionWidgetPrivateHandle;

class QtTransferFunctionWidget : public QGLWidget, private Core::ConnectionHandler
{
  Q_OBJECT

  // -- constructor/ destructor --
public:
  QtTransferFunctionWidget( const QGLFormat& format, QWidget* parent, QGLWidget* share, 
    Core::TransferFunctionHandle tf );
    
  virtual ~QtTransferFunctionWidget();

protected:

  /// INITIALIZEGL:
  /// This function is called by Qt when the widget is initialized
  virtual void initializeGL();
  
  /// PAINTGL:
  /// This function is called whenever Qt has to repaint the contents of 
  /// the widget displaying the Qt scene
  virtual void paintGL();
  
  /// RESIZEGL:
  /// This function gets called whenever the Qt widget is resized
  virtual void resizeGL( int width, int height );

  // -- internals of the QtTransferFunctionWidget --
private:
  QtTransferFunctionWidgetPrivateHandle private_;
};

} // end namespace QtUtils

#endif
