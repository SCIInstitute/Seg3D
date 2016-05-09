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

// Core includes
#include <Core/Utils/AtomicCounter.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>
#include <Core/Application/Application.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/Action/ActionHistory.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Implementation of class  ActionDispatcherPrivate
//////////////////////////////////////////////////////////////////////////

class ActionDispatcherPrivate
{
public:
  // RUN_ACTION:
  // Convenience function for keeping track of actions being executed.
  void run_action(  ActionHandle action, ActionContextHandle action_context );
  void make_timestamp();
  boost::posix_time::ptime get_last_action_completed_timestamp();

  ActionDispatcher* dispatcher_;
  AtomicCounter action_count_;
  boost::posix_time::ptime last_action_completed_;
};

void ActionDispatcherPrivate::run_action( ActionHandle action, 
                     ActionContextHandle action_context )
{
  this->dispatcher_->run_action( action, action_context );
  --this->action_count_;
  this->make_timestamp();
}

void ActionDispatcherPrivate::make_timestamp() 
{
  this->last_action_completed_ = boost::posix_time::second_clock::local_time();
}

boost::posix_time::ptime ActionDispatcherPrivate::get_last_action_completed_timestamp()
{
  return this->last_action_completed_;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of class ActionDispatcher
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( ActionDispatcher );

ActionDispatcher::ActionDispatcher() :
  private_( new ActionDispatcherPrivate )
{
  this->private_->dispatcher_ = this;
  // Connect this class to the ActionHistory
  post_action_signal_.connect( boost::bind( &ActionHistory::record_action, 
    ActionHistory::Instance() , _1, _2 ) );
}

ActionDispatcher::~ActionDispatcher()
{
}

void ActionDispatcher::post_action( ActionHandle action, ActionContextHandle action_context )
{
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  CORE_LOG_DEBUG( std::string("Posting Action: ") + action->export_to_string() );
  ++this->private_->action_count_;
  Application::Instance()->post_event( boost::bind( &ActionDispatcherPrivate::run_action, 
    this->private_.get(), action, action_context ) );
}

void ActionDispatcher::post_and_wait_action( ActionHandle action,
    ActionContextHandle action_context )
{
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  if ( Application::IsApplicationThread() )
  {
    CORE_THROW_LOGICERROR( "post_and_wait_action cannot be posted from the thread"
      " that processes the actions. This will lead to a dead lock");
  }

  CORE_LOG_DEBUG(std::string("Posting Action: ")+action->export_to_string());
  ++this->private_->action_count_;
  Application::Instance()->post_and_wait_event( boost::bind( &ActionDispatcherPrivate::run_action, 
    this->private_.get(), action, action_context ) );
}

void ActionDispatcher::post_actions( std::vector< ActionHandle > actions,
    ActionContextHandle action_context )
{
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  for ( size_t j = 0; j < actions.size(); j++ )
  {
    ++this->private_->action_count_;
    CORE_LOG_DEBUG( std::string("Posting Action sequence: " ) + 
      actions[ j ]->export_to_string() );
  }

  Application::Instance()->post_event( boost::bind( &ActionDispatcher::run_actions, this,
      actions, action_context ) );
}

void ActionDispatcher::post_and_wait_actions( std::vector< ActionHandle > actions,
    ActionContextHandle action_context )
{
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  if ( Application::IsApplicationThread() )
  {
    CORE_THROW_LOGICERROR("Post and Wait actions cannot be posted from the"
      " thread that processes the actions. This will lead to a dead lock");
  }

  for ( size_t j = 0; j < actions.size(); j++ )
  {
    ++this->private_->action_count_;
    CORE_LOG_DEBUG( std::string( "Posting Action sequence: ") +
      actions[ j ]->export_to_string());
  }

  Application::Instance()->post_and_wait_event( boost::bind( &ActionDispatcher::run_actions,
      this, actions, action_context ) );
}

bool ActionDispatcher::is_busy()
{
  return this->private_->action_count_ > 0;
}

void ActionDispatcher::run_action( ActionHandle action, ActionContextHandle action_context )
{
  // Step (1): Some actions require a translation before they can be validated
  // The first step is calling the translation function.
  // NOTE: if translation fails the action is not executed.
  if ( !( action->translate( action_context ) ) )
  {
    // The action context should return unavailable or invalid
    if ( action_context->status() != ActionStatus::UNAVAILABLE_E )
    {
      action_context->report_status( ActionStatus::INVALID_E );
    }
    action_context->report_done();

    // Clear any cached handles
    action->clear_cache();
    return; 
  }
  
  // Step (2): An action needs to be validated before it can be executed.
  // The validation is a separate step as invalid actions should not be
  // posted to the observers that record what the program does.

  if ( !( action->validate( action_context ) ) )
  {
    // The action context should return unavailable or invalid
    if ( action_context->status() != ActionStatus::UNAVAILABLE_E )
    {
      action_context->report_status( ActionStatus::INVALID_E );
    }
    action_context->report_done();

    // Clear any cached handles
    action->clear_cache();
    return;
  }

  // NOTE: Observers that connect to this signal should not change the state of
  // the program as that may invalidate actions that were just run.

  // Step (3): Tell observers what action has been executed
  pre_action_signal_( action );

  // Step (4): Run action from the context that was provided. And if the action
  // was synchronous a done signal is triggered in the context, to inform the
  // program whether the action succeeded.

  ActionResultHandle result;
  if ( !( action->run( action_context, result ) ) )
  {
    action_context->report_status( ActionStatus::ERROR_E );
    action_context->report_done();
    // actions that fail, are aborted here

    // Clear any cached handles
    action->clear_cache();
    return;
  }

  // Step (5): Set the action result if any was returned.

  if ( result.get() )
  {
    action_context->report_result( result );
  }
  
  action_context->report_status( ActionStatus::SUCCESS_E );
  action_context->report_done();

  // Clear any cached handles
  action->clear_cache();

  // NOTE: Observers that connect to this signal should not change the state of
  // the program as that may invalidate actions that were just run.

  // Step (6): Tell observers what action has been executed
  post_action_signal_( action, result );

  return;
}

void ActionDispatcher::run_actions( std::vector< ActionHandle > actions,
    ActionContextHandle action_context )
{
  // Now that we are on the application thread
  // Run the actions one by one.
  for ( size_t j = 0; j < actions.size(); j++ )
  {
    this->private_->run_action( actions[ j ], action_context );
  }
}

void ActionDispatcher::PostAction( const ActionHandle& action, 
  const ActionContextHandle& action_context )
{
  Instance()->post_action( action, action_context );
}

void ActionDispatcher::PostAndWaitAction( const ActionHandle& action, 
  const ActionContextHandle& action_context )
{
  Instance()->post_and_wait_action( action, action_context );
}

bool ActionDispatcher::IsBusy()
{
  return Instance()->is_busy();
}

boost::posix_time::ptime ActionDispatcher::last_action_completed() const
{
  return this->private_->get_last_action_completed_timestamp();
}



} // end namespace Core
