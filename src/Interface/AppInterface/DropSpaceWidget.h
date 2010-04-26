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

#ifndef INTERFACE_APPINTERFACE_DropSpaceWidgetWIDGET_H
#define INTERFACE_APPINTERFACE_DropSpaceWidgetWIDGET_H

// QT includes
#include <QtGui>


namespace Seg3D
{
  
class DropSpaceWidget :
public QWidget
{
  Q_OBJECT
  
public:
  DropSpaceWidget( QWidget *parent );
  virtual ~DropSpaceWidget();

public:
  void hide();
  void show();

  
private Q_SLOTS:
  void change_size();
  
private:
  QTimer *timer_;
  bool changing_size_;
  bool open_;
  QWidget* parent_;
  
};
  
} // end namespace Seg3D

#endif //INTERFACE_APPINTERFACE_DropSpaceWidget_H