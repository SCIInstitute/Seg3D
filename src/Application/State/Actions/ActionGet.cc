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
#include <Application/State/Actions/ActionGet.h>

namespace Seg3D {

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
SCI_REGISTER_ACTION(Get);

bool 
ActionGet::validate(ActionContextHandle& context)
{
  // Check whether the state exists
  StateBaseHandle state;
  if(!(StateManager::instance()->get_state(stateid_.value(),state)))
  {
    context->report_error(
      std::string("Unknown state variable '")+stateid_.value()+"'");
    return (false);
  }
  
  // The action should be able to run 
  return (true);
}

bool 
ActionGet::run(ActionContextHandle& context)
{
  // Get the state
  StateBaseHandle state;
  if(!(StateManager::instance()->get_state(stateid_.value(),state)))
  {
    return (false);
  }

  // Retrieve the current state
  state->export_to_variant(stateresult_);
  
  // Signal action is done
  return (true);
}

} // end namespace Seg3D
