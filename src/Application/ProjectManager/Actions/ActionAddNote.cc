/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>

#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionAddNote.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, AddNote )

namespace Seg3D
{

bool ActionAddNote::validate( Core::ActionContextHandle& context )
{
  // NOTE: Adding note doesn't require the current project to exist on disk,
  // so validating always succeeds.
  
  return true; // validated
}

bool ActionAddNote::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  bool success = ProjectManager::Instance()->get_current_project()->add_note( this->note_ );
  if( !success )
  {
    context->report_error( "Failed to add note to the project." );    
  }

  return success;   
}

void ActionAddNote::Dispatch( Core::ActionContextHandle context, const std::string& note )
{
  ActionAddNote* action = new ActionAddNote;
  action->note_ = note;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
