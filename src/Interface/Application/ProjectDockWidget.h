/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_APPLICATION_PROJECTDOCKWIDGET_H
#define INTERFACE_APPLICATION_PROJECTDOCKWIDGET_H

#ifndef Q_MOC_RUN

// QT includes
#include <QtCore/QPointer>

// STL includes
#include <string>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDockWidget.h>

#endif

namespace Seg3D
{
  
class ProjectDockWidgetPrivate;

class ProjectDockWidget : public QtUtils::QtCustomDockWidget, public Core::ConnectionHandler
{
  Q_OBJECT
  
  // -- constructor/destructor --
public:
  ProjectDockWidget( QWidget *parent = 0 );
  virtual ~ProjectDockWidget();
  

private:
  /// SET_AUTO_SAVE_LABEL:
  /// function that sets the autosave label's text
  void set_auto_save_label( int duration );

  /// GET_DATE:
  /// function for getting the current date to check the session names against
  std::string get_date();

  /// SET_FILE_SIZE_LABEL:
  /// function that actually sets the label to reflect the file size change
  void set_file_size_label( long long file_size );
  

  /// UPDATE_WIDGET:
  /// Update the widget
  void update_widget();
  
private Q_SLOTS:

  /// ENABLE_LOAD_DELETE_AND_EXPORT_BUTTONS:
  /// this function enables the load, export and delete buttons when the user clicks on a session
  void enable_load_delete_and_export_buttons( int row, int column );

  /// SAVE_PROJECT:
  /// this function calls the Save Project Session action
  void save_session();
  
  /// SAVE_NOTE:
  /// this function calls ProjectManagers save note function and then clears the editor window
  void save_note();
    
  /// LOAD_SESSION:
  /// This function calls the LoadSession action that loads the session
  void load_session();
  
  /// DELETE_SESSION:
  /// This function calls the DeleteSession action that deletes a session
  void delete_session();
  
  /// CALL_LOAD_SESSION:
  /// This is an internal function that allows loading session by double clicking on the session
  /// name.
  void call_load_session( int row, int column );
  
  /// ENABLE_SAVE_NOTES_BUTTON:
  /// This is an internal function that enables the button for saving notes when at least 3
  /// characters have been entered.
  void enable_save_notes_button();

  /// EXPORT_PROJECT:
  /// this function is an internal function that opens the export project wizard
  void export_project();

  /// DISABLE_LOAD_DELETE_AND_EXPORT_BUTTONS:
  /// this function disables the load, export and delete buttons for when a session is not selected
  void disable_load_delete_and_export_buttons();
  
  // -- internals --
private:
  ProjectDockWidgetPrivate* private_;
  
  // -- functions that handle callbacks from Application thread --
public:
  typedef QPointer< ProjectDockWidget > qpointer_type;

  /// HANDLEAUTOSAVETIMECHANGED:
  /// This function handles signals that the autosave time has changed and routes them, on the 
  /// correct thread to the set_auto_save_label function so that it can update the label's text
  static void HandleAutoSaveTimeChanged( qpointer_type qpointer, int duration );

  /// HANDLEFILESIZECHANGED:
  /// This function handles the signal that the file size has changed and sets the ui to reflect it
  static void HandleFileSizeChanged( qpointer_type qpointer, long long file_size ); 

  /// HANDLEPROJECTCHANGED:
  /// This function is called when the signal that a new project has been loaded is triggered
  static void HandleProjectChanged( qpointer_type qpointer );
  
  /// HANDLEUPDATEWIDGET:
  /// This function updates the widget
  static void HandleUpdateWidget( qpointer_type qpointer );
  
};
  
} // end namespace Seg3D

#endif 
