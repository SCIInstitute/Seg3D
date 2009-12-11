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

#include "QStatusBar.h"

//  Application includes
#include <Application/Tool/ToolManager.h>
#include <Application/Tool/ToolFactory.h>

// Interface includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/AppStatusBar.h>


namespace Seg3D {
  

AppStatusBar::AppStatusBar(QMainWindow* parent) :
  QObject(parent)
{
  QStatusBar* statusbar = parent->statusBar();
  
  build_coordinates_label();
  build_focus_label();
  
  statusbar->addWidget(coordinates_label_, 0);
  statusbar->addWidget(focus_label_, 1);
  
  set_coordinates_label(234, 232);
}

AppStatusBar::~AppStatusBar()
{
}

  // -- build status bar widgets -- //  
  void AppStatusBar::build_coordinates_label()
  {
    coordinates_label_ = new QLabel("x: 0000  y: 0000 ");
    coordinates_label_->setAlignment(Qt::AlignHCenter);
    coordinates_label_->setMinimumSize(coordinates_label_->sizeHint());
    coordinates_label_->setStyleSheet("margin-left: 4px; margin-right: 4px; ");
  }
    
  void AppStatusBar::build_focus_label()
  {
    focus_label_ = new QLabel("Focus: ");
    focus_label_->setIndent(3);
  }  
  
  // -- public slots -- //
  
  void AppStatusBar::set_coordinates_label(int x_coord_, int y_coord_)
  {
    QString mouseLocation = QString().sprintf("x: %04d  y: %04d\t", x_coord_, y_coord_);
    coordinates_label_->setText(mouseLocation);
  }
  
  void AppStatusBar::set_focus_label(int active_view_)
  {
    QString view;
    
    switch (active_view_) {
      case 1:
        view = "main view";
        break;
      case 2:
        view = "secondary view";
        break;
      default:
        break;
    }
    
    focus_label_->setText(view);
  }
  
  void AppStatusBar::set_coordinates_mode(int mode_)
  {
    switch (mode_){
      case 1:
        // TODO
        break;
      case 2:
        // TODO
        break;

      default:
        break;
    }
  }
  
  
  
  
  
  
  
  
  
  
}  // end namespace Seg3d
