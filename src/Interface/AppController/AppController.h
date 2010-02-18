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

#ifndef INTERFACE_APPCONTROLLER_APPCONTROLLER_H
#define INTERFACE_APPCONTROLLER_APPCONTROLLER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>

// QT includes
#include <QtGui>

// Utils includes
#include <Utils/Core/Log.h>

// Application includes
// include all the headers associated with the action engine
#include <Application/Action/Actions.h>

namespace Seg3D {

class AppControllerPrivate;
typedef boost::shared_ptr<AppControllerPrivate> AppControllerPrivateHandle;

// Forward declaration
class AppController;

// Class definition
class AppController : public QWidget {
    Q_OBJECT
    
// -- constructor/destructor --
  
  public:  
    AppController( QWidget* parent = 0 );
    virtual ~AppController();

// -- qt slots --
  public Q_SLOTS:
    
    void post_action();
    void post_action_message( std::string message );
    void post_action_usage( std::string usage );

  private:
    AppControllerPrivateHandle private_;

    QTabWidget*   tw_controller_;
    QToolButton*  tb_action_;
    QLineEdit*    le_edit_action_;
    QLabel*       l_action_status_;
    QLabel*       l_action_usage_;
  
    QTableView*   tv_action_history_;
    QTableView*   tv_log_history_;
    
  public:
    typedef QPointer<AppController> qpointer_type;
  
    // These functions are static as they are called from the callback stack
    // and as they may be delay, it is not clear whether the AppController
    // still exists, hence the static functions check for the existence first
    // These functions also relay the function calls to the right thread.
    
    // Force the Controller to update its action history widget
    static void UpdateActionHistory( qpointer_type controller );

    // Force the Controller to update its log history widget
    static void UpdateLogHistory( qpointer_type controller, bool relay, 
                                 int message_type, std::string message );

    // Post a message in the Controller message label
    static void PostActionMessage( qpointer_type controller, std::string message );

    // Post a message in the controller usage label
    static void PostActionUsage( qpointer_type controller, std::string usage );
                          
    static void SetActionType( qpointer_type controller, std::string action_type );
};

} // end namespace Seg3D

#endif
