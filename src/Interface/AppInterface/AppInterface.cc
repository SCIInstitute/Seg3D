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

// For the version numbers
#include "Seg3DConfiguration.h"

#include <Interface/AppInterface/AppInterface.h>

namespace Seg3D {

AppInterface::AppInterface(QApplication* app)
{
  // Set the window information
  // and set the version numbers
  setWindowTitle(QString("Seg3D Version ")+SEG3D_VERSION);
  setWindowIconText(QString("Seg3D"));
  setDocumentMode(true);
  
  // Tell Qt what size to start up in
  resize(1280, 720);
  
  // Tell Qt where to doc the toolbars
  setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
  setCorner(Qt::TopRightCorner,Qt::RightDockWidgetArea);
  setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomRightCorner,Qt::RightDockWidgetArea);
  
    

  viewer_interface_ =           new ViewerInterface(this);
  history_dock_window_ =        new HistoryDockWidget(this);
  project_dock_window_ =        new ProjectDockWidget(this);
  tools_dock_window_ =          new ToolsDockWidget(this);
  layer_manager_dock_window_ =  new LayerManagerDockWidget(this);
  measurement_dock_window_ =    new MeasurementDockWidget(this);

  setCentralWidget(viewer_interface_);

  addDockWidget(Qt::LeftDockWidgetArea, history_dock_window_, Qt::Horizontal);
  addDockWidget(Qt::LeftDockWidgetArea, project_dock_window_, Qt::Horizontal);
  addDockWidget(Qt::LeftDockWidgetArea, tools_dock_window_, Qt::Horizontal);
  
  tabifyDockWidget(project_dock_window_, history_dock_window_);
  tabifyDockWidget(history_dock_window_, tools_dock_window_);
  
  addDockWidget(Qt::RightDockWidgetArea, layer_manager_dock_window_, Qt::Horizontal);
  addDockWidget(Qt::RightDockWidgetArea, measurement_dock_window_, Qt::Horizontal);
  
  tabifyDockWidget(measurement_dock_window_, layer_manager_dock_window_);
  

  

  //showFullScreen();
  application_menu_ = new AppMenu(this, viewer_interface_);
  status_bar_ = new AppStatusBar(this);
  
  // set the viewer_interface_ to a default view of 1 and 3
  viewer_interface_->set_views(1,3);
  
 
  
}

void
AppInterface::full_screen_toggle(bool e)
{
  if(e) AppInterface::showFullScreen();
  else AppInterface::showFullScreen();
}
  

  

AppInterface::~AppInterface()
{
  viewer_interface_->writeSizeSettings();
}







} //end namespace
