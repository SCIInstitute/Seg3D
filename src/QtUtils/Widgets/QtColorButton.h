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

#ifndef QTUTILS_WIDGETS_QTCOLORBUTTON_H
#define QTUTILS_WIDGETS_QTCOLORBUTTON_H

#ifndef Q_MOC_RUN

// QT includes
#include <QToolButton>

// Core includes
#include <Core/Geometry/Color.h>

#endif

namespace QtUtils
{
  
class QtColorButton :
  public QToolButton
{
  Q_OBJECT
  
Q_SIGNALS:
  void color_changed( Core::Color );
  void color_changed( int );
  void button_clicked( Core::Color, bool );
  void index( int );

public:
  // - Constructor / Destructor
  QtColorButton( QWidget *parent = 0, int index = 0, 
    Core::Color button_color = Core::Color(), int height = 0, int width = 0 );
  virtual ~QtColorButton();
  
public Q_SLOTS:
  /// SET_COLOR:
  /// This function sets the color and the stylesheet of the button to reflect the desired color
  void set_color( Core::Color );

  /// GET_COLOR:
  /// This function returns the current color of the button
  Core::Color get_color(){ return button_color_; }
  
private Q_SLOTS:
  /// TRIGGER_SIGNAL:
  /// This function is called when the button needs to signal that it has been toggled.
  void trigger_signal( bool );

private:
  Core::Color button_color_;
  const int index_;
  
};
  
} // end namespace QtUtils

#endif
