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

#include "ViewAction.h"

//#include <QAction>


namespace Seg3D  {
  
ViewAction::ViewAction(QAction* parent, int column1, int column2) : 
  QObject(parent)
{
  col1_ = column1;
  col2_ = column2;
  
  connect(parent,SIGNAL(triggered()),this,SLOT(slot()));
}

ViewAction::ViewAction(QAction* parent, bool true_or_false) : 
  QObject(parent)
{
  state_ = true_or_false;
  connect(parent, SIGNAL(triggered(bool)), this, SLOT(slot(bool)));
}
  

void  ViewAction::slot()
{  
  Q_EMIT triggered(col1_,col2_);
}
 
void ViewAction::slot(bool torf)
{
  //Q_EMIT trigaficated(state_);
}
  

ViewAction::~ViewAction(){}


}  //end namespace Seg3d