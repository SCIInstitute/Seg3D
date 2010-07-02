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
  
public:
  typedef QPointer< ProjectDockWidget > qpointer_type;
  static void HandleSessionsChanged( qpointer_type qpointer );
  static void HandleNoteSaved( qpointer_type qpointer );
  
private Q_SLOTS:
  void save_project();
  void save_note();
  void populate_session_list();
  void populate_notes_list();
  void load_session();
  void delete_session();
  void call_load_session( QListWidgetItem* item );
  void enable_save_notes_button();
  
private:
  boost::shared_ptr< ProjectDockWidgetPrivate > private_;
};
  
} // end namespace

#endif // PROJECTDOCKWIDGET_H
