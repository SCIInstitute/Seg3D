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

#ifndef CORE_ACTION_ACTIONCONTEXT_H
#define CORE_ACTION_ACTIONCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Notifier.h>
#include <Core/Action/ActionResult.h>

namespace Core
{

// ENUM ActionStatus
/// This enum lists the possible outcomes of preforming an action

CORE_ENUM_CLASS
(
  ActionStatus,

  /// ACTION_SUCCESS - Everything went fine and the action was executed
  SUCCESS_E = 0,

  /// ACTION_ERROR - The action did not execute properly and failed
  ERROR_E = 1,

  /// ACTION_INVALID - The action did not get validated
  INVALID_E = 2,

  /// ACTION_UNAVAILABLE - The action could not be executed, because
  /// resources are not available
  UNAVAILABLE_E = 3
)

// ENUM ActionSource
/// This enum lists the possible sources of where an action can be triggered
/// from. Depending on the source the action may change. For instance, an
/// action run from the provenance buffer requires a different path for recording
/// provenance.
CORE_ENUM_CLASS
(
  ActionSource,

  /// NONE_E - It did not result from an action.
  NONE_E = 0,

  /// INTERFACE_WIDGET_E - The action is run from the interface,
  /// the interface may not need an update, if the GUI already did the update.
  INTERFACE_WIDGET_E = 1,

  /// INTERFACE_MOUSE_E - The action is run from the interface, but through a mouse
  /// interaction
  INTERFACE_MOUSE_E = 2,

  /// INTERFACE_KEYBOARD_E - The action is run from the interface, but through a keyboard
  /// interaction
  INTERFACE_KEYBOARD_E = 3,

  /// INTERFACE_MENU_E - The action is run from the interface, through a menu.
  INTERFACE_MENU_E = 4,

  /// SCRIPT_E -The action is run from a script, which means that
  /// the interface needs to be updated and as well that actions need to be
  /// queued. Hence for this source the required resource lock needs to be
  /// returned, so the script can wait for the action to be completed.
  SCRIPT_E = 5,

  /// COMMANDLINE_E - This action is run from the command line, it
  /// needs to update the interface, but does not allow queuing
  COMMANDLINE_E = 6,

  /// PROVENANCE_E - The action is run from the provenance buffer
  /// Hence it should not be recorded again into the provenance buffer
  PROVENANCE_E = 7,

  /// UNDOBUFFER_E - The action is run from the undo buffer
  UNDOBUFFER_E = 8
)

class ActionContext;
typedef boost::shared_ptr< ActionContext > ActionContextHandle;

// CLASS ACTIONCONTEXT:
/// The action context contains all the information for the action to relay
/// information back to the point of origin. As this type of information is not
/// cached in provenance or in session files, the ActionContext is a separate
/// entity with a pointer to the Action which describes the action that needs
/// to be done. Action itself contains the pointers to the functions that need
/// to be executed and with which parameters, but not where to report errors,
/// the source of the action ect. The latter information is contained in the
/// ActionContext. Each source needs to derive its own ActionContext from this
/// class and generate the specifics of where information needs to be relayed
/// to.

class ActionContext : public boost::noncopyable
{

  // -- Constructor/destructor --
public:
  // Wrap a context around an action
  ActionContext();

  // Virtual destructor for memory management
  virtual ~ActionContext();

  // -- Reporting functions --
public:
  virtual void report_error( const std::string& error );
  virtual void report_warning( const std::string& warning );
  virtual void report_message( const std::string& message );

  // -- Report back status and results --
public:
  virtual void report_status( ActionStatus status );
  virtual void report_result( const ActionResultHandle& result );
  virtual void report_need_resource( NotifierHandle notifier );

  // -- Report that action was done --
public:
  virtual void report_done();

  // -- Source/Status information --
public:
  virtual ActionStatus status() const;
  virtual ActionSource source() const;

  // -- Utilities
public:
  virtual Core::NotifierHandle get_resource_notifier();
  virtual void reset_context();
  virtual Core::ActionResultHandle get_result();
  virtual std::string get_error_message();

  // -- shortcuts for checking status --
public:
  bool is_success()
  {
    return status_ == ActionStatus::SUCCESS_E;
  }
  bool is_invalid()
  {
    return status_ == ActionStatus::INVALID_E;
  }
  bool is_unavailable()
  {
    return status_ == ActionStatus::UNAVAILABLE_E;
  }
  bool is_error()
  {
    return status_ == ActionStatus::ERROR_E;
  }

  // -- Status information --
protected:
  // The last status report from the action engine
  ActionStatus status_;
  std::string error_msg_;
  Core::NotifierHandle notifier_;
  Core::ActionResultHandle result_;
};

} // end namespace Core

#endif
