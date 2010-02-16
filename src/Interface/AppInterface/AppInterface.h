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
#pragma once
#endif 

// Qt includes
#include <QtGui>
#include <QMainWindow>
#include <QDockWidget>
#include <QPointer>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Application Interface components. 
#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/Action/Actions.h>

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
  
// -- constructor/destructor --
  public:
    // Constructor
    AppInterface();
  
    // virtual constructor needed by Qt to destroy this object
    virtual ~AppInterface();
  
  public:
  
    ViewerInterface*        viewer_interface();
    HistoryDockWidget*      history_dock_widget();
    ProjectDockWidget*      project_dock_widget();
    ToolsDockWidget*        tools_dock_widget();
    LayerManagerDockWidget* layer_manager_dock_widget();
    MeasurementDockWidget*  measurement_dock_widget();
  
// -- functions to handle the properties of the interface --  
  public:
    void set_full_screen(bool full_screen);


  private:
    void add_windowids();
    void show_window(const std::string& windowid);
    void close_window(const std::string& windowid);

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
    
    // Overload the default addDockWidget and upgrade it, so docks are added
    // on top of each other
    void addDockWidget(Qt::DockWidgetArea area, QDockWidget* dock_widget);

// -- Main Window management functions --
  public:
    typedef QPointer<AppInterface> qpointer_type; 

    // NOTE: These functions are static to ensure that they can serve as targets
    // for the signal/slot mechanism. As the main interface may be closed while
    // there are still function callbacks in the loop, these functions test for 
    // the existence of the interface before executing.
    
    // HANDLESHOWWINDOW:
    // Reopen a specific window after the user has closed it
    static void HandleShowWindow(qpointer_type app_interface, 
                                 std::string windowid);

    // HANDLECLOSEWINDOW:
    // Close a dock or a window
    static void HandleCloseWindow(qpointer_type app_interface, 
                                  std::string windowid);
  
    // SETFULLSCREEN:
    // Set full screen mode of the Main Window
    static void SetFullScreen(qpointer_type app_interface, 
                                 bool full_screen, ActionSource source);
};

} //end namespace

#endif

