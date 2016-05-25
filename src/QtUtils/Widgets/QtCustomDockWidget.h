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

#ifndef QTUTILS_WIDGETS_QTCUSTOMDOCKWIDGET_H
#define QTUTILS_WIDGETS_QTCUSTOMDOCKWIDGET_H

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>


// QT includes
#include <QDockWidget>
#include <QCloseEvent>
#include <QKeyEvent>

#endif

namespace QtUtils
{
  
class QtCustomDockWidgetPrivate;
  
class QtCustomDockWidget : public QDockWidget
{
Q_OBJECT
  
Q_SIGNALS:
  void opened();
  void closed();
  
public:
  // - Constructor / Destructor
  QtCustomDockWidget( QWidget *parent = 0 );
  virtual ~QtCustomDockWidget();

  /// CLOSEEVENT:
  /// This function is called by Qt to deliver an event that tells that the
  /// widget is being hidden. 
  virtual void closeEvent( QCloseEvent* event );
  
  /// RESIZEEVENT:
  /// This function is called by Qt to deliver an event that tells that the
  /// widget is being resized. 
  virtual void resizeEvent( QResizeEvent *event );

  /// SHOWEVENT:
  /// This function is called by Qt to deliver an event that tells that the
  /// widget is being resized. 
  virtual void showEvent( QShowEvent *event );

  
public:
  void set_enabled( bool enabled );
  
protected:
  QWidget* dock_base_;
  
private:
  boost::shared_ptr< QtCustomDockWidgetPrivate > private_;
  

};
  
} // end namespace QtUtils

#endif
