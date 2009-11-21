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

#include <Application/State/StateManager.h>

namespace Seg3D {

StateManager::StateManager()
{
}
  
bool
StateManager::add_state(const std::string& state_id, StateBase* state)
{
  boost::unique_lock<boost::mutex> lock(state_map_lock_);
  
  state_map_type::const_iterator it = state_map_.find(state_id);
  if (it != state_map_.end())
  {
    SCI_LOG_ERROR(std::string("Trying to add state variable '")+state_id+"' twice");
    return (false);
  }

  state_map_[state_id] = state;
}

StateBase* 
StateManager::get_state(const std::string& state_id)
{
  boost::unique_lock<boost::mutex> lock(state_map_lock_);
  
  state_map_type::const_iterator it = state_map_.find(state_id);
  if (it == state_map_.end()) return (0);
  return ((*it).second);
}

void
StateManager::remove_state(const std::string& state_id)
{
  boost::unique_lock<boost::mutex> lock(state_map_lock_);
  
  state_map_type::iterator it = state_map_.begin();
  state_map_type::iterator it_end = state_map_.end();

  while (it != it_end)
  {
    if ((*it).first.size() >= state_id.size())
    {
      if ((*it).first.compare(0,state_id.size(),state_id,0,state_id.size()) == 0)
      {
        if ((*it).first.size() > state_id.size())
        {
          if ((*it).first[state_id.size()] == ':')
          {
            state_map_.erase(it++); 
            continue;
          }
        }
        else
        {
          state_map_.erase(it++); 
          continue;        
        }
      }
    }
    ++it;
  }
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<StateManager> StateManager::instance_;

} // end namespace Seg3D
