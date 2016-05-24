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

#ifndef INTERFACE_APPLICATION_OVERLAYWIDGET_H
#define INTERFACE_APPLICATION_OVERLAYWIDGET_H

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>

// Qt includes
#include <QWidget>

#endif

namespace Seg3D
{
  
class OverlayWidgetPrivate; 
  
class OverlayWidget : public QWidget 
{
  Q_OBJECT
  
public:
  OverlayWidget( QWidget *parent );
  virtual ~OverlayWidget();
  
  /// SET_TRANSPARENT:
  ///
  void set_transparent( bool transparent );
  
  /// PAINTEVENT
  /// 
  virtual void paintEvent( QPaintEvent * event );

private:
  boost::shared_ptr< OverlayWidgetPrivate > private_;
    
};
  
} // end namespace

#endif
