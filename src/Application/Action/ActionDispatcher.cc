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

void
ActionDispatcher::run_action(ActionHandle action, ActionContextHandle action_context)
{ 
  // THREAD SAFETY:
  // If it is not on the application thread, relay the function call
  // to the application thread
  
  // Synchronization of actions is done by forcing all actions that change the
  // state of the program to run to an dedicated thread: the application thread.
  
  if (!(Application::instance()->is_eventhandler_thread()))
  {
    Application::instance()->post_event(boost::bind
      (&ActionDispatcher::run_action,this,action,action_context));
    return;
  }

  SCI_LOG_DEBUG(std::string("Processing Action: ")+action()->type_name());  

  
  // Step (1): An action needs to be validated before it can be executed. 
  // The validation is a separate step as invalid actions should nor be 
  // posted to the observers recording what the program does

  SCI_LOG_DEBUG("Validating Action");  
  if(!(action()->validate(action_context))) 
  {
    action_context->report_done(false);
    return;
  }

  // NOTE: Observers that connect to this signal should not change the state of
  // the program as that may invalidate actions that were just tested.

  // Step (2): Tell observers what action is about to be executed
  SCI_LOG_DEBUG("Posting Action");  
  post_action_signal_(action());
  
  // Step (3): Run action from the context that was provided. And if the action
  // was synchronous a done signal is triggered in the context, to inform the
  // program whether the action succeeded.
  SCI_LOG_DEBUG("Running Action");    
  bool success = action()->run(action_context);
  
  if (!(action()->properties() & Action::ASYNCHRONOUS_E))
  {
    // Ignore asynchronous actions, they will do their own reporting
    action_context->report_done(success);
  }
  
  return;
}

void
ActionDispatcher::run_actions(std::vector<ActionHandle> actions, ActionContextHandle action_context)
{ 
  // THREAD SAFETY:
  // If it is not on the application thread, relay the function call
  // to the application thread
  
  // Synchronization of actions is done by forcing all actions that change the
  // state of the program to run to an dedicated thread: the application thread.
  
  if (!(Application::instance()->is_eventhandler_thread()))
  {
    Application::instance()->post_event(boost::bind
      (&ActionDispatcher::run_actions,this,actions,action_context));
    return;
  }
  
  // Now that we are on the application thread
  // Run the actions one by one.
  for (size_t j=0; j<actions.size(); j++)
  {
    run_action(actions[j],action_context);
  }
}


void
ActionDispatcher::run_actions(std::vector<ActionHandle> actions, 
                              std::vector<ActionContextHandle> action_contexts)
{ 
  // THREAD SAFETY:
  // If it is not on the application thread, relay the function call
  // to the application thread
  
  // Synchronization of actions is done by forcing all actions that change the
  // state of the program to run to an dedicated thread: the application thread.
  
  if (!(Application::instance()->is_eventhandler_thread()))
  {
    Application::instance()->post_event(boost::bind
      (&ActionDispatcher::run_actions,this,actions,action_contexts));
    return;
  }
  
  if (actions.size() != action_contexts.size())
  {
    SCI_THROW_LOGICERROR("Number of actions does not match number of contexts");
  }
  
  // Now that we are on the application thread
  // Run the actions one by one.
  for (size_t j=0; j<actions.size(); j++)
  {
    run_action(actions[j],action_contexts[j]);
  }
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<ActionDispatcher> ActionDispatcher::instance_;

} // end namespace Seg3D
