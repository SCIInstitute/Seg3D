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

// Need instance of main application for inserting events into main application
// thread.
#include <Application/Application/Application.h>
#include <Application/Action/ActionDispatcher.h>

namespace Seg3D {

ActionDispatcher::ActionDispatcher()
{
}


ActionDispatcher::~ActionDispatcher()
{
}


void
ActionDispatcher::post_action(ActionHandle action, 
                              ActionContextHandle action_context)
{ 
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  SCI_LOG_DEBUG(std::string("Posting Action: ")+action->export_to_string());  

  Application::Instance()->post_event(boost::bind
      (&ActionDispatcher::run_action,this,action,action_context));
}


void
ActionDispatcher::post_and_wait_action(ActionHandle action, 
                                       ActionContextHandle action_context)
{ 
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  if (Application::IsApplicationThread())
  {
    SCI_THROW_LOGICERROR("post_and_wait_action cannot be posted from the thread"
                  " that processes the actions. This will lead to a dead lock");
  }

  SCI_LOG_DEBUG(std::string("Posting Action: ")+action->export_to_string());  

  Application::Instance()->post_and_wait_event(boost::bind
      (&ActionDispatcher::run_action,this,action,action_context));
}


void
ActionDispatcher::post_actions(std::vector<ActionHandle> actions, 
                               ActionContextHandle action_context)
{ 
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  for (size_t j = 0;  j < actions.size(); j++)
  {
    SCI_LOG_DEBUG(std::string("Posting Action sequence: ")+
                                              actions[j]->export_to_string());  
  }
  
  Application::Instance()->post_event(boost::bind
      (&ActionDispatcher::run_actions,this,actions,action_context));
}


void
ActionDispatcher::post_and_wait_actions(std::vector<ActionHandle> actions, 
                                        ActionContextHandle action_context)
{ 
  // THREAD SAFETY:
  // Always relay the function call to the application thread as an event, so
  // events are handled in the order that they are posted and one action is fully
  // handled before the next one

  if (Application::IsApplicationThread())
  {
    SCI_THROW_LOGICERROR("Post and Wait actions cannot be posted from the"
          " thread that processes the actions. This will lead to a dead lock");
  }

  for (size_t j = 0;  j < actions.size(); j++)
  {
    SCI_LOG_DEBUG(std::string("Posting Action sequence: ")+
                                              actions[j]->export_to_string());  
  }
  
  Application::Instance()->post_and_wait_event(boost::bind
      (&ActionDispatcher::run_actions,this,actions,action_context));
}


void
ActionDispatcher::run_action(ActionHandle action, 
                             ActionContextHandle action_context)
{ 
  // Step (1): An action needs to be validated before it can be executed. 
  // The validation is a separate step as invalid actions should not be 
  // posted to the observers that record what the program does.

  if (!(action->validate(action_context)))
  {
    // The action  context should return unavailable or invalid
    if (action_context->status() != ACTION_UNAVAILABLE_E)
    {
      action_context->report_status(ACTION_INVALID_E);
    }
    action_context->report_done();
    return;
  }
    
  // NOTE: Observers that connect to this signal should not change the state of
  // the program as that may invalidate actions that were just run.

  // Step (2): Tell observers what action has been executed
  pre_action_signal_(action);

  // Step (3): Run action from the context that was provided. And if the action
  // was synchronous a done signal is triggered in the context, to inform the
  // program whether the action succeeded.

  ActionResultHandle result;
  if(!(action->run(action_context, result)))
  {
    action_context->report_status(ACTION_ERROR_E);
    action_context->report_done();
  }

  // Step (4): Set the action result if any was returned.
  
  if (result.get()) 
  {
    action_context->report_result(result);
  }
  action_context->report_status(ACTION_SUCCESS_E);
  action_context->report_done();
 
  // NOTE: Observers that connect to this signal should not change the state of
  // the program as that may invalidate actions that were just run.

  // Step (5): Tell observers what action has been executed
  post_action_signal_(action,result);

  return;
}


void
ActionDispatcher::run_actions(std::vector<ActionHandle> actions, 
                              ActionContextHandle action_context)
{ 
  // Now that we are on the application thread
  // Run the actions one by one.
  for (size_t j=0; j<actions.size(); j++)
  {
    run_action(actions[j],action_context);
  }
}


void 
PostAction(const ActionHandle& action, 
           const ActionContextHandle& action_context)
{
  ActionDispatcher::Instance()->post_action(action, action_context);
}


void 
PostAndWaitAction(const ActionHandle& action, 
                  const ActionContextHandle& action_context)
{
  ActionDispatcher::Instance()->post_and_wait_action(action, action_context);
}


// Singleton interface needs to be defined somewhere
Utils::Singleton<ActionDispatcher> ActionDispatcher::instance_;

} // end namespace Seg3D
