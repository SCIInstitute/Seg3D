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

#ifndef INTERFACE_APPINTERFACE_PROJECTDOCKWIDGET_H
#define INTERFACE_APPINTERFACE_PROJECTDOCKWIDGET_H

// QT includes
#include <QtCore/QPointer>

// STL includes
#include <string>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// QtUtils includes
#include <QtUtils/Widgets/QtCustomDockWidget.h>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D
{
  
class ProjectDockWidgetPrivate;

class ProjectDockWidget : public QtUtils::QtCustomDockWidget, public Core::ConnectionHandler
{
  Q_OBJECT
  
public:
  ProjectDockWidget( QWidget *parent = 0 );
  virtual ~ProjectDockWidget();
  
private:
  typedef QPointer< ProjectDockWidget > qpointer_type;

  // HANDLESESSIONSCHANGED:
  // A function that verifies that we're operating on the proper thread and if not, it moves the 
  // process to the correct one in order to reload the sessions displayed after they have been
  // updated elsewhere.
  static void HandleSessionsChanged( qpointer_type qpointer );
  
  // HANDLENOTESSAVED:
  // A function that verifies that we're operating on the proper thread and if not, it moves the 
  // process to the correct one in order to save reload the notes displayed after they have been
  // updated elsewhere.
  static void HandleNoteSaved( qpointer_type qpointer );

  // HANDLEAUTOSAVETIMECHANGED:
  // this function handles signals that the autosave time has changed and routes them, on the 
  // correct thread to the set_auto_save_label function so that it can update the label's text
  static void HandleAutoSaveTimeChanged( qpointer_type qpointer, int duration );

  // HANDLEFILESIZECHANGED:
  // this function handles the signal that the file size has changed and sets the ui to reflect it
  static void HandleFileSizeChanged( qpointer_type qpointer, long long file_size );
  
  static void HandleAutosaveStateChanged( qpointer_type qpointer, bool state );

private:
  // SET_AUTO_SAVE_LABEL:
  // function that sets the autosave label's text
  void set_auto_save_label( int duration );

  // SET_SMART_SAVE_LABEL:
  // function that updates the gui to reflect the status of the smart autosave state variable
  //void set_smart_save_label();

  // GET_DATE:
  // function for getting the current date to check the session names against
  std::string get_date();

  // SET_FILE_SIZE_LABEL:
  // function that actually sets the label to reflect the file size change
  void set_file_size_label( long long file_size );
  
  void set_autosave_checkbox( bool state );
  
private Q_SLOTS:
  // ENABLE_LOAD_DELETE_AND_EXPORT_BUTTONS:
  // this function enables the load, export and delete buttons when the user clicks on a session
  void enable_load_delete_and_export_buttons( int row, int column );

  // SAVE_PROJECT:
  // this function calls the Save Project Session action
  void save_session();
  
  // SAVE_NOTE:
  // this function calls ProjectManagers save note function and then clears the editor window
  void save_note();
  
  // POPULATE_SESSION_LIST:
  // This function clears the current session list and reloads it from the state variables
  void populate_session_list();
  
  // POPULATE_NOTES_LIST:
  // This function clears the current notes list and reloads it from the state variables
  void populate_notes_list();
  
  // LOAD_SESSION:
  // This function calls the LoadSession action that loads the session
  void load_session();
  
  // DELETE_SESSION:
  // This function calls the DeleteSession action that deletes a session
  void delete_session();
  
  // CALL_LOAD_SESSION:
  // This is an internal function that allows loading session by double clicking on the session
  // name.
  void call_load_session( int row, int column );
  
  // ENABLE_SAVE_NOTES_BUTTON:
  // This is an internal function that enables the button for saving notes when at least 3
  // characters have been entered.
  void enable_save_notes_button();

  // EXPORT_PROJECT:
  // this function is an internal function that opens the export project wizard
  void export_project();

  // DISABLE_LOAD_DELETE_AND_EXPORT_BUTTONS:
  // this function disables the load, export and delete buttons for when a session is not selected
  void disable_load_delete_and_export_buttons();
  
  // SET_AUTOSAVE_CHECKED_STATE:
  // this function dispatches a function that sets the autosave checked state
  void set_autosave_checked_state( bool state );
  

private:
  boost::shared_ptr< ProjectDockWidgetPrivate > private_;
};
  
} // end namespace

#endif // PROJECTDOCKWIDGET_H
