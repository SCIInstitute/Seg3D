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

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionDeleteSession.h>

CORE_REGISTER_ACTION( Seg3D, DeleteSession )

namespace Seg3D
{

bool ActionDeleteSession::validate( Core::ActionContextHandle& context )
{
  // Check whether files still exist, if not an error is being generated.
  // We are dealing with file I/O, hence there is no guarantee that files still exist.
  // The user may have accidentally deleted the files, or a network connection may be lost, etc.
  if ( ! ProjectManager::Instance()->get_current_project()->check_project_files() )
  {
    // In this case just report to the user using the normal mechanism.
    // The user is deleting things, which may already not exist anymore, so this is not a 
    // critical error at this point. 
    context->report_error( "The current project directory cannot be found." );
    return false;
  }

  // Ensure the session exists
  if( !ProjectManager::Instance()->get_current_project()->
    is_session( this->session_id_ ) )
  {
    std::string error = Core::ExportToString( this->session_id_ ) + " is not a valid session ID.";
    context->report_error( error );
    return false;
  }
  
  return true;
}

bool ActionDeleteSession::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = "Deleting session: " + Core::ExportToString( this->session_id_ ) + "...";

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  // NOTE: This will send a message to GUI to lock it and report what is going on.
  progress->begin_progress_reporting();

  // Actual work is done here
  bool success = ProjectManager::Instance()->delete_project_session( this->session_id_ );

  // Allow the user to interact with the GUI once more.
  progress->end_progress_reporting();

  return success;
}

void ActionDeleteSession::Dispatch( Core::ActionContextHandle context, long long session_id )
{
  // Create action
  ActionDeleteSession* action = new ActionDeleteSession;
  action->session_id_ = session_id;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
