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

#include <Application/Action/ActionHistory.h>

namespace Seg3D {

ActionHistory::ActionHistory() :
  action_history_max_size_(0)
{
  // Connect this class to the ActionDispatcher
  dispatcher_connection_ = ActionDispatcher::Instance()->connect_post_action(
    boost::bind(&ActionHistory::record_action,this,_1));
}

void
ActionHistory::set_max_history_size(size_t size)
{
  boost::unique_lock<boost::mutex> lock(action_history_mutex_);
  action_history_max_size_ = size;
  if (action_history_.size() > action_history_max_size_)
  {
    // remove part of the history
    action_history_.erase(action_history_.begin()+size,action_history_.end());
    // signal that buffer has changed
    history_changed_signal_(); 
  }
}

size_t
ActionHistory::max_history_size()
{
  boost::unique_lock<boost::mutex> lock(action_history_mutex_);
  return (action_history_max_size_ );
}

size_t
ActionHistory::history_size()
{
  boost::unique_lock<boost::mutex> lock(action_history_mutex_);
  return (action_history_.size() );
}

ActionHandle
ActionHistory::action(size_t index)
{
  boost::unique_lock<boost::mutex> lock(action_history_mutex_);
  if (index < action_history_.size())
  {
    return action_history_[index];
  }
  else
  {
    ActionHandle empty_handle;
    return empty_handle;
  }
}

void 
ActionHistory::record_action(ActionHandle action)
{
  boost::unique_lock<boost::mutex> lock(action_history_mutex_);
  action_history_.push_front(action);
  if (action_history_.size() > action_history_max_size_)
  {
    action_history_.pop_back();
  }
  
  SCI_LOG_DEBUG(std::string("Record action into history log: ")+action->type());
  history_changed_signal_();
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<ActionHistory> ActionHistory::instance_;

} // end namespace Seg3D

