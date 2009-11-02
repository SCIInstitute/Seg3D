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

#include <Interface/AppInterface/WorkflowInterface.h>
#include <Interface/AppInterface/ToolManagerInterface.h>
#include <Interface/AppInterface/LayerManagerInterface.h>
#include <Interface/AppInterface/ViewerInterface.h>

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
    AppInterface(QApplication* app);
  
    // virtual constructor needed by Qt to destroy this object
    virtual ~AppInterface();
    
  public:
  
    ViewerInterface* get_viewer_interface() {  return viewer_interface_; }
    WorkflowInterface* get_workflow_interface() {  return workflow_interface_; }
    LayerManagerInterface* get_layermanager_interface() { return layermanager_interface_; }
    ToolManagerInterface* get_toolmanager_interface() { return toolmanager_interface_; }
  
  private:

    // Pointers to the various components
    ViewerInterface*          viewer_interface_;
    WorkflowInterface*        workflow_interface_;
    ToolManagerInterface*     toolmanager_interface_;
    LayerManagerInterface*    layermanager_interface_;
    
};

} //end namespace

#endif

