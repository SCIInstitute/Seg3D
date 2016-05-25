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

#ifndef QTUTILS_WIDGETS_QTDOUBLECLICKTOOLBUTTON_H
#define QTUTILS_WIDGETS_QTDOUBLECLICKTOOLBUTTON_H

#ifndef Q_MOC_RUN

// QT includes
#include <QToolButton>

// Core includes
#include <boost/function.hpp>

#endif

namespace QtUtils
{
  
class QtDoubleClickToolButton : public QToolButton
{
  Q_OBJECT

public:
  // - Constructor / Destructor
  QtDoubleClickToolButton( QWidget *parent = 0 ); 
  virtual ~QtDoubleClickToolButton();

  /// SET_DOUBLE_CLICK_FUNCTION:
  /// Set the function that needs to be called when double clicking on this button.
  void set_double_click_function( boost::function< void() > function );

protected:
  virtual void mouseDoubleClickEvent( QMouseEvent *event );

private:
  boost::function< void() > function_;  
};
  
} // end namespace QtUtils

#endif
