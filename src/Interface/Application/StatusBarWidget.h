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

#ifndef INTERFACE_APPLICATION_APPSTATUSBAR_H
#define INTERFACE_APPLICATION_APPSTATUSBAR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

// QT includes
#include <QObject>
#include <QMainWindow>
#include <QStatusBar>
#include <QPointer>

// Core includes
#include <Core/Utils/ConnectionHandler.h>

// Application includes
#include <Application/StatusBar/StatusBar.h>

#endif

namespace Seg3D
{

class StatusBarWidgetPrivate;

class StatusBarWidget : public QObject, private Core::ConnectionHandler
{
Q_OBJECT

// -- constructor / destructor --
public:
  StatusBarWidget( QMainWindow* parent = 0 );
  virtual ~StatusBarWidget();

private:
  /// UPDATE_DATA_POINT_INFO:
  ///
  void update_data_point_info( DataPointInfoHandle data_point );
  
  /// UPDATE_DATA_POINT_LABEL:
  ///
  void update_data_point_label();
  
  /// SET_MESSAGE:
  /// 
  void set_message( int msg_type, std::string message );
  
  /// CHECK_TIME:
  /// 
  void check_time();

private Q_SLOTS:
  /// SET_COORDINATES_MODE:
  /// function that sets which mode the status bar is showing the coordinates in
  void set_coordinates_mode( bool is_world );

  /// SET_STATUS_REPORT_LABEL:
  /// function that actually sets the message on the status bar
  void set_status_report_label( std::string& report );

  /// RESET_ICON:
  /// function that resets the icon after an error has been viewed in the message window
  void reset_icon( bool show_errors );
  
  /// FIX_ICON_STATUS:
  /// function that sets the status report label back to true
  ///void fix_icon_status();
  
  /// SWAP_BARS:
  /// function that swaps between the coordinates bar and the active tools/layer bar
  void swap_bars();
  
  /// CLEAR_LABEL:
  /// function that clears the message that is on the status bar
  void clear_label();
  
  /// SLIDE_IN:
  /// function that animates the status message in
  void slide_in();
  
  /// SLIDE_OUT:
  /// function that animates the status message out
  void slide_out();
  
  /// SLIDE_OUT_THEN_IN:
  /// function that animates the status message out then in
  void slide_out_then_in();
  
  /// SET_FINISHED_ANIMATING:
  /// function that sets the animating status to done;
  void set_finished_animating();
  
private:
  typedef QPointer< StatusBarWidget > qpointer_type;
  boost::shared_ptr< StatusBarWidgetPrivate > private_;
  
private:
  static void SetMessage( qpointer_type qpointer, int msg_type, std::string message );
  static void HandleActionEvent( qpointer_type qpointer );


  
  

};

} // end namespace Seg3D

#endif
