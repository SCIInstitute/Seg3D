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

#ifndef INTERFACE_APPINTERFACE_APPINTERFACE_H
#define INTERFACE_APPINTERFACE_APPINTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QtGui>
#include <QMainWindow>
#include <QDockWidget>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Application Interface components. 

#include <Interface/AppInterface/ViewerInterface.h>
#include <Interface/AppInterface/HistoryDockWidget.h>
#include <Interface/AppInterface/ProjectDockWidget.h>
#include <Interface/AppInterface/ToolsDockWidget.h>
#include <Interface/AppInterface/LayerManagerDockWidget.h>
#include <Interface/AppInterface/MeasurementDockWidget.h>

#include <Interface/AppInterface/AppMenu.h>
#include <Interface/AppInterface/AppStatusBar.h>
#include <Interface/AppController/AppController.h>

namespace Seg3D {

// -- AppInterface --
// This class is the main application window
// All the menus and viewers dock into this window
// The event loop of this window is the main event
// loop for the program

// Although there is only one instance of this class
// in the program 

class AppInterface : public QMainWindow
{
  Q_OBJECT
  
  public:
    // Constructor
    AppInterface(QWidget* app = 0);
  
    // virtual constructor needed by Qt to destroy this object
    virtual ~AppInterface();
  
  public:
  
  ViewerInterface* get_viewer_interface() {  return viewer_interface_; }
  HistoryDockWidget* get_history_dock_widget() { return history_dock_window_; }
  ProjectDockWidget* get_project_dock_widget() { return project_dock_window_; }
  ToolsDockWidget* get_tools_dock_widget() { return tools_dock_window_; }
  LayerManagerDockWidget* layer_manager_dock_widget() { return layer_manager_dock_window_; }
  MeasurementDockWidget* measurement_dock_widget() { return measurement_dock_window_; }
  
  public Q_SLOTS:
    void full_screen_toggle(bool);

  private:

    // Pointer to the main canvas of the main window
    QPointer<ViewerInterface>          viewer_interface_;
    QPointer<AppController>            controller_interface_;

    // The dock widgets
    QPointer<HistoryDockWidget>        history_dock_window_;
    QPointer<ProjectDockWidget>        project_dock_window_;
    QPointer<ToolsDockWidget>          tools_dock_window_;
    QPointer<LayerManagerDockWidget>   layer_manager_dock_window_;
    QPointer<MeasurementDockWidget>    measurement_dock_window_;

    // Application menu, statusbar
    QPointer<AppMenu>      application_menu_;
    QPointer<AppStatusBar> status_bar_;
    

// -- Qt singleton implementation --
  public:
    // Obtain the pointer to this class
    static AppInterface* Instance() { return instance_.data(); }
    
  private:  
    // Storing where the singleton is located
    static QPointer<AppInterface> instance_;

// -- Main Window management functions --
  public:
    
    // Reopen a specific window after the user has closed it
    static void HandleOpenWindow(std::string windowid);

    // Close a dock or a window
    static void HandleCloseWindow(std::string windowid);
};

} //end namespace

#endif

