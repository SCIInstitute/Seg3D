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

#ifndef APPLICATION_ACTIONMANAGER_ACTIONMANAGERDISPATCHER_H
#define APPLICATION_ACTIONMANAGER_ACTIONMANAGERDISPATCHER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// For making the class noncopyable
#include <boost/utility.hpp>

#include <Application/ActionManager/ActionManager.h>
#include <Application/ActionManager/Action.h>

namespace Seg3D {


class ActionManagerDispatcher : public boost::noncopyable {

// -- Constructor
  public:
    ActionManagerDispatcher();

// -- Action handling --
  
  public:
    // TYPEDEFS 
    // The type of the main action signal

    typedef boost::signals2::signal<void (ActionHandle)> post_action_signal_type;

    // POST_ACTION:
    // Post an action into the main signal stack of the application
    // All actions in the program are funneled through this signal stack
    
    bool post_action(ActionHandle action);        

    // POST_ACTION_SIGNAL:
    // This is the main signal stack for actions that are posted inside the
    // application. Any observer that wants to listen into the actions that
    // are being issued by th program needs to connect to this signal as all
    // GUI events, Application events, and Layer Data events are passed through
    // this single application signal.
    
    post_action_signal_type post_action_signal_;

};

// FUNCTION PostAction:
// This function is a short cut to posting an action using the dispatcher
 
inline bool PostAction(ActionHandle action)
{
  return (ActionManager::instance()->dispatcher_->post_action(action));
}

} // namespace Seg3D

#endif
