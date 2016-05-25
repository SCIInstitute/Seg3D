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

#ifndef INTERFACE_APPLICATION_DROPSPACEWIDGET_H
#define INTERFACE_APPLICATION_DROPSPACEWIDGET_H

#ifndef Q_MOC_RUN

// QT includes
#include <QWidget>

#endif

namespace Seg3D
{
  
class DropSpaceWidget :
public QWidget
{
  Q_OBJECT
  
public:
  DropSpaceWidget( QWidget *parent, int height = 45, int grow_speed = 5, int shrink_speed = 15 );
  virtual ~DropSpaceWidget();

public:
  /// HIDE:
  /// This overloaded function adds some animation to the hide function for this widget
  void hide();
  void instant_hide();

  /// SHOW:
  /// This overloaded function adds some animation to the show function for this widget
  void show();
  
  void set_height( int height ){ this->max_height_ = height; }

  
private Q_SLOTS:
  /// CHANGE_SIZE:
  /// This is the function that does the actual size changing for the animation
  void change_size();
  
private:
  //QTimer *timer_;
  bool changing_size_;
  bool open_;
  int max_height_;
  int grow_speed_;
  int shrink_speed_;
};
  
} // end namespace Seg3D

#endif
