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

#ifndef INTERFACE_APPINTERFACE_TOOLSDOCKWIDGET_H
#define INTERFACE_APPINTERFACE_TOOLSDOCKWIDGET_H

// QT includes
#include <QtGui>
#include <QDockWidget>
#include <QPointer>

// STL includes
#include <string>
#include <map>

// Boost includes
#include <boost/signals2/signal.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Application includes
#include <Application/Tool/ToolManager.h>

// Interface includes
#include <Interface/AppInterface/ToolWidget.h>
#include <Interface/AppInterface/ToolBoxWidget.h>

namespace Seg3D {

class ToolsDockWidget : public QDockWidget {
    Q_OBJECT
    
// -- constructor/destructor --
  public:
    ToolsDockWidget(QWidget *parent = 0);
    virtual ~ToolsDockWidget();

// -- functions that control the toolbox --
  public:
  
    // Open a tool
    void open_tool(ToolHandle tool);

    // Close a tool
    void close_tool(ToolHandle tool);
    
    // Activate tool
    void activate_tool(ToolHandle tool);

// -- slots --
  public Q_SLOTS:
  
    void tool_changed(int index);

  private:

    boost::signals2::connection open_tool_connection_;
    boost::signals2::connection close_tool_connection_;
    boost::signals2::connection activate_tool_connection_;
  
    // List of tool widgets
    typedef std::map<std::string,ToolWidget*> widget_list_type;
    widget_list_type widget_list_;
    
    // Pointer to the ToolBox Widget
    ToolBoxWidget* toolbox_;
    
  // -- static functions for callbacks into this widget --

  public:
    static void HandleOpenTool(QPointer<ToolsDockWidget> widget,
                               ToolHandle tool);
    static void HandleCloseTool(QPointer<ToolsDockWidget> widget,
                                ToolHandle tool);
    static void HandleActivateTool(QPointer<ToolsDockWidget> widget,
                                   ToolHandle tool);
    
};
  
} // end namespace

#endif // TOOLSDOCKWIDGET_H
