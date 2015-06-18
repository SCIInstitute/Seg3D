/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef Q_MOC_RUN

// Qt includes
#include <QPointer>
#include <QMenuBar>
#include <QAction>
#include <QMainWindow>

// Core Includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Action/Actions.h>

#endif

#include <tuple>

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

  // -- functions for building menus --
private:
  void create_file_menu( QMenuBar* menubar );
  void create_edit_menu( QMenuBar* menubar );
  void create_view_menu( QMenuBar* menubar );
  void create_window_menu( QMenuBar* menubar );
  void create_tool_menus( QMenuBar* menubar );
  void create_help_menu( QMenuBar* menubar );
  
  QMenu* file_menu_recents_;
  QAction* export_segmentation_qaction_;
  QAction* export_isosurface_qaction_;
  QAction* export_active_data_layer_qaction_;
  
  QAction* copy_qaction_;
  QAction* paste_qaction_;
  QAction* punch_qaction_;
  
  QAction* undo_qaction_;
  QAction* redo_qaction_;

  QAction* import_large_volume_qaction_;

  std::vector<QAction*> large_volume_tools_;

  // Keep a pointer to the main window
  QMainWindow*  main_window_;
  
private Q_SLOTS:
  /// NEW_PROJECT:
  /// Create a new project and launch the wizard that walks the user through the process.
  void new_project();
  
  /// OPEN_PROJECT:
  /// Open an existing project
  void open_project();
  
  /// OPEN_PROJECT_FOLDER:
  /// This function launches the native explorer, so one can inspect the project folder
  /// This will call the explorer on Windows and Finder on the Mac
  void open_project_folder();

  /// SAVE_PROJECT:
  /// Triggered by Save Project
  void save_project();

  /// SAVE_PROJECT AS:
  /// Triggered by Save Project As
  void save_project_as();
  
  /// ABOUT:
  /// This launches the about dialog
  void about();
  
  /// MAC_OPEN_ANOTHER_VERSION:
  /// This opens another version of Seg3D on the mac.
  /// NOTE: Double clicking on the application will just highlight the current application
  /// Hence we added the option to launch a second version in the menu
  void mac_open_another_version();

private:
  typedef QPointer< Menu > qpointer_type;
  
  /// SET_RECENT_FILE_LIST:
  /// populate the recent file list with these items
  void set_recent_file_list();

  /// ENABLE_DISABLE_LAYER_ACTIONS:
  /// Switch on/off export options depending on whether mask is available
  void enable_disable_mask_actions( bool mask_layer_found );

  /// ENABLE_DISABLE_ISOSURFACE_ACTIONS:
  /// Switch on/off export actions depending on whether isosurface is available
  void enable_disable_isosurface_actions( bool mask_isosurface_found );

  /// SHOW_HIDE_LARGE_VOLUME_ACTIONS:
  /// Show or hide large volume menus (doesn't include Tools).
  void show_hide_large_volume_actions( bool large_volume_visible );
  
  /// ENABLE_DISABLE_DATA_LAYER_ACTIONS:
  /// switches off the export options based on whether or not the active layer is a data layer
  void enable_disable_data_layer_actions( bool data_layer_found );

  /// UPDATE_UNDO_TAG:
  /// set the undo tag of what the next undo will actually do
  void update_undo_tag( std::string tag );

  /// UPDATE_REDO_TAG:
  /// set the redo tag of what the next undo will actually do
  void update_redo_tag( std::string tag );

private:
  // static utility functions

  /// FINDACTIVELAYER:
  /// determine if active layer is data layer and available
  static bool FindActiveLayer();

  /// FINDMASKLAYER:
  /// determine if there is a mask layer available and if it has an isosurface
  static std::tuple<bool, bool> FindMaskLayer();

public:
  /// SETRECENTFILELIST:
  /// This function is called when the recent file list is modified
  static void SetRecentFileList( qpointer_type app_menu );

  /// ENABLEDISABLELAYERACTIONS:
  /// this function is called when new layers are added or layers are deleted
  static void EnableDisableLayerActions( qpointer_type qpointer );

  /// SHOWHIDELARGEVOLUME:
  /// toggle visibility of large volume menu items
  static void ShowHideLargeVolume( qpointer_type qpointer );

  /// UPDATEUNDOTAG
  /// This function is called when there is a new undo tag
  static void UpdateUndoTag( qpointer_type qpointer, std::string tag );

  /// UPDATEREDOTAG
  /// This function is called when there is a new redo tag
  static void UpdateRedoTag( qpointer_type qpointer, std::string tag );
  
  /// CONFIRMRECENTFILELOAD:
  static void ConfirmRecentFileLoad( qpointer_type qpointer, const std::string& path );
};


} // end namespace Seg3D

#endif
