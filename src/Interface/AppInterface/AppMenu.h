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

#ifndef INTERFACE_APPINTERFACE_APPMENU_H
#define INTERFACE_APPINTERFACE_APPMENU_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <QMenuBar>
#include <QAction>
#include <QMainWindow>


// Interface Includes
#include <Interface/AppInterface/ViewerInterface.h>
#include <Interface/AppInterface/ViewAction.h>
#include <Interface/AppInterface/AppLayerIO.h>

// Core Includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Action/Actions.h>

namespace Seg3D
{

class AppMenu : public QObject, private Core::ConnectionHandler
{
Q_OBJECT
// -- constructor / destructor --    
public:
  AppMenu( QMainWindow* parent = 0 );
  virtual ~AppMenu();
  
public:
  typedef QPointer< AppMenu > qpointer_type;
  
  void set_recent_file_list( std::vector< std::string > files );
  static void SetRecentFileList( qpointer_type app_menu, 
    std::vector< std::string > recent_projects, Core::ActionSource source );
    
  void enable_disable_layer_actions();  
  static void EnableDisableLayerActions( qpointer_type app_menu );

  // -- functions for building menus --
private:
  void create_file_menu( QMenu* menu );
  void create_edit_menu( QMenu* menu );
  void create_view_menu( QMenu* menu );
  void create_window_menu( QMenu* menu );
  void create_tool_menus( QMenuBar* menubar );
  
  QMenu* file_menu_recents_;
  QAction* export_segmentation_qaction_;
  QAction* export_active_data_layer_qaction_;
  
  QAction* copy_qaction_;
  QAction* paste_qaction_;
    
  // Keep a pointer to the main window
  QMainWindow*  main_window_;
  
private Q_SLOTS:
  void new_project_wizard();
  void open_project_from_file();
  void open_project_folder();
};


} // end namespace Seg3D

#endif

