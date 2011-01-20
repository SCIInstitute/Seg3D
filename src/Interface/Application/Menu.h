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

#ifndef INTERFACE_APPLICATION_MENU_H
#define INTERFACE_APPLICATION_MENU_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QPointer>
#include <QMenuBar>
#include <QAction>
#include <QMainWindow>

// Core Includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Action/Actions.h>

namespace Seg3D
{

class Menu;

class Menu : public QObject, private Core::ConnectionHandler
{
Q_OBJECT
// -- constructor / destructor --    
public:
  Menu( QMainWindow* parent = 0 );
  virtual ~Menu();
  
public:
  typedef QPointer< Menu > qpointer_type;
  
  // SET_RECENT_FILE_LIST:
  // populate the recent file list with these items
  void set_recent_file_list();

  // ENABLE_DISABLE_LAYER_ACTIONS:
  // Switch on/off export options depending on what is available
  void enable_disable_mask_actions(); 
  
  // ENABLE_DISABLE_DATA_LAYER_ACTIONS:
  // switches off the export options based on whether or not the active layer is a data layer
  void enable_disable_data_layer_actions();

  // UPDATE_UNDO_TAG:
  // set the undo tag of what the next undo will actually do
  void update_undo_tag( std::string tag );

  // UPDATE_REDO_TAG:
  // set the redo tag of what the next undo will actually do
  void update_redo_tag( std::string tag );

  // -- functions for building menus --
private:
  void create_file_menu( QMenu* menu );
  void create_edit_menu( QMenu* menu );
  void create_view_menu( QMenu* menu );
  void create_window_menu( QMenu* menu );
  void create_tool_menus( QMenuBar* menubar );
  void create_help_menu( QMenu* menu );
  
  QMenu* file_menu_recents_;
  QAction* export_segmentation_qaction_;
  QAction* export_active_data_layer_qaction_;
  
  QAction* copy_qaction_;
  QAction* paste_qaction_;
  QAction* punch_qaction_;
  
  QAction* undo_action_;
  QAction* redo_action_;
  
  // Keep a pointer to the main window
  QMainWindow*  main_window_;
  
private Q_SLOTS:
  void new_project_wizard();
  void open_project_from_file();
  void open_project_folder();
  void save_as_wizard();
  void about();
  void mac_open_another_version();
  
public:

  // SETRECENTFILELIST:
  // This function is called when the recent file list is modified
  static void SetRecentFileList( qpointer_type app_menu );

  // ENABLEDISABLELAYERACTIONS:
  // This function is called when layers are updated
  static void EnableDisableMaskActions( qpointer_type app_menu );
  
  // ENABLEDISABLEDATALAYERACTIONS:
  // this function is called when the active layer is changed
  static void EnableDisableDataLayerActions( qpointer_type qpointer );

  // UPDATEUNDOTAG
  // This function is called when there is a new undo tag
  static void UpdateUndoTag( qpointer_type qpointer, std::string tag );

  // UPDATEREDOTAG
  // This function is called when there is a new redo tag
  static void UpdateRedoTag( qpointer_type qpointer, std::string tag );
  
  // CONFIRMRECENTFILELOAD:
  static void ConfirmRecentFileLoad( qpointer_type qpointer, const std::string& path );
};


} // end namespace Seg3D

#endif
