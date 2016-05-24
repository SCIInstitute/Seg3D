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

#ifndef INTERFACE_APPLICATION_CONTROLLERINTERFACE_H
#define INTERFACE_APPLICATION_CONTROLLERINTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>

// QT includes
#include <QPointer>
#include <QtUtils/Widgets/QtCustomDialog.h>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableView>
#include <QPointer>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Log.h>
#include <Core/Action/Actions.h>

#endif

namespace Seg3D
{

class ControllerInterfacePrivate;
typedef boost::shared_ptr< ControllerInterfacePrivate > ControllerInterfacePrivateHandle;

// Forward declaration
class ControllerInterface;

// Class definition
class ControllerInterface : public QtUtils::QtCustomDialog, private Core::ConnectionHandler
{
Q_OBJECT

// -- constructor/destructor --

public:
  ControllerInterface( QWidget* parent = 0 );
  virtual ~ControllerInterface();

  // -- qt slots --
public Q_SLOTS:

  void post_action();
  void post_action_message( std::string message );
  void post_action_usage( std::string usage );

private:
  ControllerInterfacePrivateHandle private_;

  QTabWidget* tw_controller_;
  QPushButton* tb_action_;
  QLineEdit* le_edit_action_;
  QLabel* l_action_status_;
  QLabel* l_action_usage_;

  QTableView* tv_action_history_;
  QTableView* tv_state_engine_;
  QTableView* tv_log_history_;

public:
  typedef QPointer< ControllerInterface > qpointer_type;

  /// These functions are static as they are called from the callback stack
  /// and as they may be delay, it is not clear whether the AppController
  /// still exists, hence the static functions check for the existence first
  /// These functions also relay the function calls to the right thread.

  /// UPDATEACTIONHISTORY:
  /// Force the Controller to update its action history widget
  static void UpdateActionHistory( qpointer_type controller );

  /// UPDATELOGHISTORY:
  /// Force the Controller to update its log history widget
  static void UpdateLogHistory( qpointer_type controller, bool relay, int message_type,
      std::string message );

  /// UPDATEUNDOBUFFER:
  /// Update the undo buffer
  static void UpdateUndoBuffer( qpointer_type controller );


  /// UPDATESTATEENGINE:
  /// Force the Controller to update the state engine widget
  static void UpdateStateEngine( qpointer_type controller );

  /// POSTACTIONMESSAGE:
  /// Post a message in the Controller message label
  static void PostActionMessage( qpointer_type controller, std::string message );

  /// POSTACTIONUSAGE:
  /// Post a message in the controller usage label
  static void PostActionUsage( qpointer_type controller, std::string usage );

  /// SETACTIONTYPE:
  /// Set the action types available
  static void SetActionType( qpointer_type controller, std::string action_type );
};

} // end namespace Seg3D

#endif
