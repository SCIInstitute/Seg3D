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

#ifndef APPLICATION_ACTION_ACTIONDISPATCHER_H
#define APPLICATION_ACTION_ACTIONDISPATCHER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/utility.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/shared_ptr.hpp>

#include <Utils/Core/Log.h>
#include <Utils/Singleton/Singleton.h>

#include <Application/Action/Action.h>
#include <Application/Action/ActionContext.h>
#include <Application/Action/ActionFactory.h>

namespace Seg3D {

class ActionDispatcher : public boost::noncopyable {

// -- Constructor
  public:
    ActionDispatcher();

// -- Action handling --
  public:
    // POST_ACTION:
    // Post an action in the application thread. If this function is called from
    // an other thread, the action is posted on the stack of actions that need
    // to be processed. Each action needs to be posted with an ActionContextHandle
    // with describes whether feedback from the action needs to posted.
    // The action context needs to be created before posting the action
    
    void post_action(ActionHandle action, ActionContextHandle action_context); // << THREAD-SAFE SLOT       

    // POST_ACTIONS:
    // Post multiple actions in specified order

    void post_actions(std::vector<ActionHandle> actions, ActionContextHandle action_context); // << THREAD-SAFE SLOT   
  
  
  private:
  
    // RUN_ACTION:
    // Run the action
    
    void run_action(ActionHandle action, ActionContextHandle action_context);      

    // RUN_ACTIONS:
    // Run multiple actions in specified order

    void run_actions(std::vector<ActionHandle> actions, ActionContextHandle action_context);   

// -- Action monitoring --

  public:
    // TYPEDEFS 
    // The type of the main action signal

    typedef boost::signals2::signal<void (ActionHandle )> pre_action_signal_type;

    typedef boost::signals2::signal<void (ActionHandle, ActionResultHandle )> post_action_signal_type;
    
    // PRE_ACTION_SIGNAL:
    // Connect an observer that records all the actions in the program before
    // they are exectuted
    
    pre_action_signal_type pre_action_signal;  

     // NOTE: One can observe action before or after they have been issued:
     // generally for provenance and tracking the program one wants to be 
     // informed after the action has been posted. However for debugging 
     // purposes it may be useful to extract this information before it is
     // issued to the processing kernel. Hence this interface allows both
     // options. 
  
    // POST_ACTION_SIGNAL:
    // Connect an observer that records all the actions in the program after
    // they are exectuted.

    post_action_signal_type post_action_signal;  

// -- Singleton interface --
  public:
    
    static ActionDispatcher* Instance() { return instance_.instance(); } // << SINGLETON
    
  private:
    // Singleton internals
    static Utils::Singleton<ActionDispatcher> instance_;
};

// FUNCTION PostAction:
// This function is a short cut to posting an action using the dispatcher
 
void PostAction(ActionHandle action, ActionContextHandle action_context);

// FUNCTION PostAction:
// This function is a short cut to posting a raw unparsed action
 
void PostAction(std::string& actionstring, ActionContextHandle action_context);

} // namespace Seg3D

#endif
