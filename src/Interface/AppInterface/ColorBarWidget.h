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

#ifndef INTERFACE_APPINTERFACE_COLORBARWIDGET_H
#define INTERFACE_APPINTERFACE_COLORBARWIDGET_H

// QT includes
#include <QtGui>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

//Interface includes
#include <Interface/AppPreferences/ColorButton.h>

namespace Seg3D
{

class ColorBarWidgetPrivate;

class ColorBarWidget : public QWidget
{
Q_OBJECT

public:
  ColorBarWidget( QWidget* parent = 0 );
  virtual ~ColorBarWidget();

public Q_SLOTS:
  void set_color_index( int index );

Q_SIGNALS:
  void color_index_changed( int index );
  void color_changed( int index );

private:
  void initialize_buttons();
  void mousePressEvent( QMouseEvent* event );
  
private Q_SLOTS:
  void signal_activation( int active );
  void color_only_changed( int button_index );

private:
  // Internals of the dockwidget
  boost::shared_ptr< ColorBarWidgetPrivate > private_;
};

} // end namespace

#endif // COLORBARWIDGET_H
