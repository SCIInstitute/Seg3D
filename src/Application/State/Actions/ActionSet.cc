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
#include <Application/State/Actions/ActionSet.h>

namespace Seg3D {

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
SCI_REGISTER_ACTION(Set);

bool 
ActionSet::validate(ActionContextHandle& context)
{
  // Check whether the state exists
  StateBase* state = StateManager::instance()->get_state(stateid_.value());
  // If not this action is invalid
  if (state == 0) 
  {
    context->report_error(
      std::string("Unknown state variable '")+stateid_.value()+"'");
    return (false);
  }
  
  // The Variant parameter can contain both the value send from the State in
  // its right format or a string in case it is send from a script.
  // In any case we need to validate whether the value can be transcribed into
  // the type we want.
  bool changed = false;
  if(!(state->validate_and_compare_variant(statevalue_,changed)))
  {
    context->report_error("The state value cannot be converted into the right type");
    return (false);
  }
  
  // No error to report, but this action does not need to be executed.
  if (changed == false) return (false);
  
  return (true);
}

bool 
ActionSet::run(ActionContextHandle& context)
{
  // Get the state
  StateBase* state = StateManager::instance()->get_state(stateid_.value());
  // Set the value
  state->import_from_variant(statevalue_);
  return (true);
}

} // end namespace Seg3D
