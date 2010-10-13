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

#ifndef INTERFACE_APPINTERFACE_APPSTATUSBAR_H
#define INTERFACE_APPINTERFACE_APPSTATUSBAR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

// QT includes
#include <QtGui>

// Interface includes
//#include <Interface/AppInterface/MessageHistoryWidget.h>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Application/StatusBar/StatusBar.h>

namespace Seg3D
{

class AppStatusBarPrivate;

class AppStatusBar : public QObject, private Core::ConnectionHandler
{
Q_OBJECT

// -- constructor / destructor --
public:
  AppStatusBar( QMainWindow* parent = 0 );
  virtual ~AppStatusBar();

private Q_SLOTS:
  void set_coordinates_mode( bool is_world );
  void set_status_report_label( std::string& report );
  void activate_history();

  // -- build status bar widgets -- //
private:
  //MessageHistoryWidget* history_widget_;
  bool show_world_coord_;
  void update_data_point_info( DataPointInfoHandle data_point );
  void update_data_point_label();
  void set_message( int msg_type, std::string message );

private:
  QStatusBar* statusbar_;
  QWidget *statusbar_widget_;
  boost::shared_ptr< AppStatusBarPrivate > private_;
  DataPointInfo data_point_info_;

private Q_SLOTS:
  void fix_icon_status();
  void swap_bars();

private:
  static void SetMessage( QPointer< AppStatusBar > qpointer, 
    int msg_type, std::string message );
};

} // end namespace Seg3D


#endif //INTERFACE_APPINTERFACE_APPSTATUSBAR_H

