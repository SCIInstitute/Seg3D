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

#include <Core/ActionManager/Actions/ActionRedo.h>
#include <Core/Interface/Interface.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Core, Redo )

namespace Core
{

bool ActionRedo::validate( ActionContextHandle& context )
{
  if ( !( ActionUndoBuffer::Instance()->has_redo_action() ) )
  {
    context->report_error( std::string( "No actions to redo" ) );
    return false;
  }
  return true; // validated
}

bool ActionRedo::run( ActionContextHandle& context, ActionResultHandle& result )
{
  ActionUndoBuffer::Instance()->redo_action( context );
  return true; // success
}

void ActionRedo::Dispatch( ActionContextHandle context )
{
  // Post the new action
  ActionDispatcher::PostAction( Create(), context );
}

ActionHandle ActionRedo::Create()
{
  // Create new action
  ActionRedo* action = new ActionRedo;
  return ActionHandle( action );
}

} // end namespace Core
