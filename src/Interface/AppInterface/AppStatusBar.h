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

#ifndef INTERFACE_APPINTERFACE_APPSTATUSBAR_H
#define INTERFACE_APPINTERFACE_APPSTATUSBAR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <QStatusBar>
#include <qlabel.h>
#include <QAction>
#include <QMainWindow>

namespace Seg3D {


class AppStatusBar : public QObject
{
  Q_OBJECT
  
 
  // -- constructor / destructor --    
  public:
    AppStatusBar(QMainWindow* parent = 0);
    virtual ~AppStatusBar();
  

  
  public Q_SLOTS:
    void set_coordinates_label(int, int);
    void set_coordinates_mode(int);
    void set_focus_label(int);

  // -- status bar components -- //
  private:
    QLabel *coordinates_label_;
    QLabel *focus_label_;
  
  // -- build status bar widgets -- //    
  private:
    void build_coordinates_label();
    void build_focus_label();
  
};
  
  
} // end namespace Seg3D

    
    

#endif
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

