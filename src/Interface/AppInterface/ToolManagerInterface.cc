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

#include <Interface/AppInterface/ToolManagerInterface.h>

namespace Seg3D {

// -- ToolsWindow --
// The Tools window contains a tab bar with tools that can be opened and closed

ToolManagerInterface::ToolManagerInterface(QWidget* parent) :
  QDockWidget("Tools",parent)
{
  // Set the areas in which we can dock this window
  setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );   
  // We want a vertical title bar to have the most visible space
  setFeatures( DockWidgetVerticalTitleBar | DockWidgetFloatable | DockWidgetMovable );

  // Generate a new top level Window for putting the tabs in
  interface_ = new QWidget(this);
  // We need a layout in here so it will resize automatically with
  // the window
  layout_ = new QVBoxLayout(interface_);
  tabs_   = new QTabBar(interface_);
  
  // Make the spacing very narrow so we have a lot of space
  layout_->setContentsMargins(0,0,0,0);
  layout_->setSpacing(0);  
  // Add the widget to the tabs bar
  layout_->addWidget(tabs_);
  // Force Widget to be on the top of the window
  layout_->addStretch();
  
  // Document mode makes the tab bar look like a browser bar
  // Closable allows tabs to be closed
  tabs_->setDocumentMode(true);
  tabs_->setTabsClosable(true);
  tabs_->setExpanding(false);

  // Example tabs
  tabs_->addTab("2D Painter");
  tabs_->addTab("3D Painter");

  setWidget(interface_);
}

ToolManagerInterface::~ToolManagerInterface()
{
}

} // end namespace
