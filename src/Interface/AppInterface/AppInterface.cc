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

// Application includes
#include <Application/Interface/Interface.h>

// Interface includes
#include <Interface/AppInterface/AppInterface.h>

namespace Seg3D {

AppInterface::AppInterface(QWidget* app) :
  QMainWindow(app)
{
  // Store the pointer to this class, so callbacks can use it in the future
  instance_ = this;

  // Set the window information
  // and set the version numbers
  setWindowTitle(QString("Seg3D Version ")+SEG3D_VERSION+QString(" ")+
                 SEG3D_BITS+QString(" ")+SEG3D_DEBUG_VERSION);
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

  controller_interface_ =       new AppController(this);
  controller_interface_->show();

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

void 
AppInterface::HandleOpenWindow(std::string window_id)
{
  // Ensure that this request is forwarded to the interface thread
  if (!(Interface::IsInterfaceThread()))
  {
    PostInterface(boost::bind(&AppInterface::HandleOpenWindow,window_id));
    return;
  }

  if (instance_.data())
  {
    if (window_id == "Controller")
    {
      if (instance_->controller_interface_.isNull())
      {
        instance_->controller_interface_ = new AppController(instance_.data());
        instance_->controller_interface_->show();
      }
    }
    else if (window_id == "Project")
    {
      if (instance_->project_dock_window_.isNull())
      {
        instance_->project_dock_window_ = new ProjectDockWidget(instance_.data());
        instance_->addDockWidget(Qt::LeftDockWidgetArea, 
                            instance_->project_dock_window_, Qt::Horizontal);
      }
    }
  }
}

void 
AppInterface::HandleCloseWindow(std::string window_id)
{
  // Ensure that this request is forwarded to the interface thread
  if (!(Interface::IsInterfaceThread()))
  {
    PostInterface(boost::bind(&AppInterface::HandleCloseWindow,window_id));
    return;
  }
}

// Need to define singleton interface
QPointer<AppInterface>  AppInterface::instance_;

} //end namespace
