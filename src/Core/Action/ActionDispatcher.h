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

#ifndef CORE_ACTION_ACTIONDISPATCHER_H
#define CORE_ACTION_ACTIONDISPATCHER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Action/Action.h>
#include <Core/Action/ActionContext.h>
#include <Core/Action/ActionProgress.h>

namespace Core
{

// CLASS ACTIONDISPATCHER
/// The main dispatcher of actions in the Seg3D framework

// Forward declaration
class ActionDispatcher;
class ActionDispatcherPrivate;
typedef boost::shared_ptr< ActionDispatcherPrivate > ActionDispatcherPrivateHandle;

// Class definition
class ActionDispatcher : public boost::noncopyable
{
  CORE_SINGLETON( ActionDispatcher );

  // -- Constructor
private:
  ActionDispatcher();
  virtual ~ActionDispatcher();

  // -- Action handling --
public:

  // POST_ACTION:
  /// Post an action onto the application thread, the action is posted on the
  /// stack of actions that need to be processed. Each action needs to be
  /// posted with an ActionContextHandle which describes where feedback from
  /// the action needs to posted.
  /// The action context needs to be created before posting the action.
  void post_action( ActionHandle action, ActionContextHandle action_context ); // << THREAD-SAFE SLOT

  // POST_AND_WAIT_ACTION:
  /// Post an action onto the application thread, the action is posted on the
  /// stack of actions that need to be processed. Each action needs to be
  /// posted with an ActionContextHandle which describes where feedback from
  /// the action needs to posted.
  /// The action context needs to be created before posting the action.
  /// This function also waits on the action to be fully completed and hence
  /// needs to be called from a thread that is not needed for action processing
  void post_and_wait_action( ActionHandle action, ActionContextHandle action_context ); // << THREAD-SAFE SLOT

  // POST_ACTIONS:
  /// Post multiple actions in specified order
  void post_actions( std::vector< ActionHandle > actions, ActionContextHandle action_context ); // << THREAD-SAFE SLOT

  // POST_AND_WAIT_ACTIONS:
  /// Post multiple actions in specified order and wait for them to finish
  void post_and_wait_actions( std::vector< ActionHandle > actions,
      ActionContextHandle action_context ); // << THREAD-SAFE SLOT

  // IS_BUSY:
  /// Returns true if there are actions being processed, otherwise false.
  bool is_busy();

  // LAST_ACTION_COMPLETED:
  /// Returns the timestamp of the last action that was completed
  boost::posix_time::ptime last_action_completed() const;


private:
  friend class ActionDispatcherPrivate;

  // RUN_ACTION:
  /// Run the action
  void run_action( ActionHandle action, ActionContextHandle action_context );

  // RUN_ACTIONS:
  /// Run multiple actions in specified order
  void run_actions( std::vector< ActionHandle > actions, ActionContextHandle action_context );

  // -- Action monitoring --

public:
  // TYPEDEFS
  /// The type of the main action signal

  typedef boost::signals2::signal< void( ActionHandle ) > pre_action_signal_type;
  typedef boost::signals2::signal< void( ActionHandle, ActionResultHandle ) > post_action_signal_type;
  typedef boost::signals2::signal< void( ActionProgressHandle ) > action_progress_signal_type;

  // PRE_ACTION_SIGNAL:
  /// Connect an observer that records all the actions in the program before
  /// they are executed
  pre_action_signal_type pre_action_signal_;

  // NOTE: One can observe action before or after they have been issued:
  // generally for provenance and tracking the program one wants to be
  // informed after the action has been posted. However for debugging
  // purposes it may be useful to extract this information before it is
  // issued to the processing kernel. Hence this interface allows both
  // options.

  // POST_ACTION_SIGNAL:
  /// Connect an observer that records all the actions in the program after
  /// they are executed.
  post_action_signal_type post_action_signal_;

  // BEGIN_PROGRESS_SIGNAL:
  /// This signals a slow action that is being processed and progress needs to be reported
  /// This is only in a few instances needed, like loading files, where the load happens inside
  /// the action. The interface can connect to this signal so it can block further input until
  /// the action has been processed
  action_progress_signal_type begin_progress_signal_;

  // END_PROGRESS_SIGNAL:
  /// This signals the end of the slow action.
  action_progress_signal_type end_progress_signal_;

  // REPORT_PROGRESS_SIGNAL:
  /// Issued every time when progress can be reported
  action_progress_signal_type report_progress_signal_;

private:
  ActionDispatcherPrivateHandle private_;

public:
  // FUNCTION PostAction:
  /// This function is a short cut to posting an action using the dispatcher
  static void PostAction( const ActionHandle& action, 
    const ActionContextHandle& action_context );

  // FUNCTION PostAndWaitAction:
  /// This function is a short cut to posting an action using the dispatcher and
  /// waiting until the action has been completed
  static void PostAndWaitAction( const ActionHandle& action, 
    const ActionContextHandle& action_context );

  // Function IsBusy:
  /// This is a short cut function to the "is_busy" function of the singleton.
  static bool IsBusy();

};

} // namespace Core

#endif
