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
#include <QtGui>
#include <QDockWidget>

// STL includes
#include <string>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D
{
  
class ProjectDockWidgetPrivate;

class ProjectDockWidget : public QDockWidget, public Core::ConnectionHandler
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

  static void HandleAutoSaveTimeChanged( qpointer_type qpointer, double duration );
  static void HandleSmartAutoSaveToggled( qpointer_type qpointer );

private:
  void set_auto_save_label( double duration );

  void set_smart_save_label();
  
private Q_SLOTS:
  // SAVE_PROJECT:
  // this function calls the Save Project Session action
  void save_project();
  
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
  void call_load_session( QListWidgetItem* item );
  
  // ENABLE_SAVE_NOTES_BUTTON:
  // This is an internal function that enables the button for saving notes when at least 3
  // characters have been entered.
  void enable_save_notes_button();
  
private:
  boost::shared_ptr< ProjectDockWidgetPrivate > private_;
};
  
} // end namespace

#endif // PROJECTDOCKWIDGET_H
