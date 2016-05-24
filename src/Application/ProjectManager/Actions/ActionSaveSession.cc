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
#include <Application/ProjectManager/Actions/ActionSaveSession.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, SaveSession )

namespace Seg3D
{

bool ActionSaveSession::validate( Core::ActionContextHandle& context )
{
  // Check whether files still exist, if not an error is being generated
  // We are dealing with file I/O, hence there is no guarantee that files still exist.
  // The user may have accidentally deleted the files, or a network connection may be lost, etc.
  if ( ! ProjectManager::Instance()->get_current_project()->check_project_files() )
  {
    context->report_error( "The project directory cannot be found." );
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "Save FAILED as the project directory cannot be found. "
      "Please perform a 'Save Project As' as soon as possible to preserve your data." );        
    return false;
  }
  
  return true; // validated
}

bool ActionSaveSession::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Please wait, while your session is being saved..." );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  // A save can still fail, we have no control over whether disk actions will succeed, hence
  // we need to keep checking the integrity of the file save.
  bool success = ProjectManager::Instance()->save_project_session( this->session_name_ );

  progress->end_progress_reporting();

  if( !success )
  {
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "Save FAILED for project: '" 
      + ProjectManager::Instance()->get_current_project()->project_name_state_->get() 
      + "'. Please perform a 'Save Project As' as soon as possible to preserve your data." );       
  }

  std::string success_message = std::string( "Successfully saved session for project: '" ) + 
    ProjectManager::Instance()->get_current_project()->project_name_state_->get() + "'.";
  CORE_LOG_SUCCESS( success_message );

  return success;   
}

void ActionSaveSession::Dispatch( Core::ActionContextHandle context, const std::string& session_name )
{
  ActionSaveSession* action = new ActionSaveSession;
  action->session_name_ = session_name;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
