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

// Core includes
#include <Core/Utils/Log.h>

#include <Application/Application/Application.h>

#include <Application/ActionManager/Action.h>
#include <Application/ActionManager/ActionManagerDispatcher.h>


namespace Seg3D {

ActionManagerDispatcher::ActionManagerDispatcher()
{
}

bool
ActionManagerDispatcher::post_action(ActionHandle action)
{ 
  // If it is not on the application thread, relay the function call
  // to the application thread
  if (!(Application::instance()->is_application_thread()))
  {
    return Application::instance()->post_event_and_get_result(
      boost::bind(&ActionManagerDispatcher::post_action,this,action));
  }
  
  // Validate the action
  if (!(action->validate()))
  {
    return (false);
  }

  SCI_LOG_DEBUG("Posting message on action signal");

  // Trigger a signal
  post_action_signal_(action);
  
  return (true);
}

} // namespace Seg3D
