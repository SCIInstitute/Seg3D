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

#include <Interface/AppInterface/ToolWidget.h>
#include <Utils/Core/Log.h>
#include <boost/lexical_cast.hpp>

namespace Seg3D {

ToolWidget::ToolWidget() :
  //close_button_(0),
  //help_button_(0),
  main_frame_(0)
{
}

ToolWidget::~ToolWidget()
{
}


bool
ToolWidget::create_widget(QWidget* parent, ToolHandle& tool)
{

  //std::string h = boost::lexical_cast<std::string>(parent->height());
//  SCI_LOG_MESSAGE("The the height of the PARENT tool is: "+ h);
  
  
  // Setup the parent widget: this one will be used for memory management of
  // this widget class
  setParent(parent);

    // Add the handle of the underlying tool to the widget
  set_tool(tool);
  
  // Generate a vertical layout for the tool widget
  QVBoxLayout* vbox = new QVBoxLayout;
  
  // Ensure it has some tight spacing  
  vbox->setSpacing(0);
  vbox->setContentsMargins(0,0,0,0);
  setLayout(vbox);
  
  main_frame_ = new QFrame;  
  
  main_frame_->resize(1, 1);
  vbox->addWidget(main_frame_);
  vbox->addStretch();
  
  
  std::string h = boost::lexical_cast<std::string>(&main_frame_);
  //std::string h = boost::lexical_cast<std::string>(main_frame_->height());
  SCI_LOG_MESSAGE("The the address of main_frame_ tool is: "+ h);
  
  return ( build_widget(main_frame_));
}

bool
ToolWidget::build_widget(QFrame* frame)
{
  return (true);
}

} //end namespace Seg3D

