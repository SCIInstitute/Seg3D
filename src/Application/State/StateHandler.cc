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

#include <Application/State/StateHandler.h>
#include <Application/State/StateManager.h>

namespace Seg3D {

StateHandler::StateHandler(const std::string& stateid_prefix) :
  stateid_prefix_(stateid_prefix)
{
}

StateHandler::~StateHandler()
{
}

bool 
StateHandler::add_statebase(const std::string& key, StateBaseHandle& state) const
{
  // Step (1): Generate a new unique ID for this state
  std::string stateid = stateid_prefix_+std::string("::")+key;

  // Step (2): Make the state variable aware of its key
  state->set_stateid(stateid);

  StateBaseHandle old_state;
  // Step (3): Import the previous setting from the current variable
  StateManager::instance()->get_state(stateid,old_state);
  if (old_state.get())
  { // use the string representation as intermediate
    state->import_from_string(old_state->export_to_string());
  }

  // Step (4): Add the state to the StateManager
  return (StateManager::instance()->add_state(stateid,state)); 
}

}
