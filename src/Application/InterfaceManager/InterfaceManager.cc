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

#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>

#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/InterfaceManager/Actions/ActionShowWindow.h>
#include <Application/InterfaceManager/Actions/ActionCloseWindow.h>

namespace Seg3D {

InterfaceManager::InterfaceManager() :
  StateHandler("InterfaceManager")
{
}

// THREAD-SAFETY:
// The RunActionFromInterface function will migrate the function call to the
// application thread. Hence there is no need to safe guard this function.
// This code can be run directly form the interface thread.

void
InterfaceManager::dispatch_show_window(const std::string& windowid) const
{
  // Build new action
  ActionShowWindowHandle action(new ActionShowWindow);
  // Set the action parameters
  action->set(windowid);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

void
InterfaceManager::dispatch_close_window(const std::string& windowid) const
{
  // Build new action
  ActionCloseWindowHandle action(new ActionCloseWindow);
  // Set the action parameters
  action->set(windowid);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

void
InterfaceManager::add_windowid(const std::string& windowid)
{
  std::string lower_windowid = Utils::string_to_lower(windowid);
  boost::unique_lock<boost::mutex> lock(windowid_list_mutex_);
  if (windowid_list_.find(lower_windowid) == windowid_list_.end()) 
  {
    windowid_list_.insert(lower_windowid);
  }
}

bool
InterfaceManager::is_windowid(const std::string& windowid)
{
  std::string lower_windowid = Utils::string_to_lower(windowid);
  boost::unique_lock<boost::mutex> lock(windowid_list_mutex_);
  return (windowid_list_.find(lower_windowid) != windowid_list_.end());
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<InterfaceManager> InterfaceManager::instance_;
  
} // end namespace Seg3D
