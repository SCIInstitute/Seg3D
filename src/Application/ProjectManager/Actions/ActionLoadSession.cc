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
#include <Core/Action/ActionProgress.h>

#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionLoadSession.h>
#include <Application/ProjectManager/Actions/ActionResetChangesMade.h>
#include <Application/UndoBuffer/UndoBuffer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, LoadSession )

namespace Seg3D
{

bool ActionLoadSession::validate( Core::ActionContextHandle& context )
{
  // Check whether files still exist, if not an error is being generated.
  // We are dealing with file I/O, hence there is no guarantee that files still exist.
  // The user may have accidentally deleted the files, or a network connection may be lost, etc.
  if ( ! ProjectManager::Instance()->get_current_project()->check_project_files() )
  {
    // In this case just report to the user using the normal mechanism.
    // The user is trying to load a new session, it will be apparent that the operation
    // failed, when no new session is loaded. However we will still issue a critical error.
    context->report_error( "The current project directory cannot be found." );
    
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "Could not load session, as the current project directory cannot be found." );
    return false;
  }

  // Ensure the session exists
  SessionInfo session_info;
  if( !ProjectManager::Instance()->get_current_project()->
    get_session_info( this->session_id_, session_info ) )
  {
    std::string error = Core::ExportToString( this->session_id_ ) + " is not a valid session ID.";
    context->report_error( error );
    return false;
  }
  this->session_name_ = session_info.session_name();
  
  return true;
}

bool ActionLoadSession::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Loading session '" ) + 
    this->session_name_ + std::string( "' ..." );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );
  
  // NOTE: This will send a message to GUI to lock it and report what is going on.
  progress->begin_progress_reporting();

  // -- For now add logging of exceptions if session cannot be loaded correctly --

  bool success = false;
  try
  {
    success = ProjectManager::Instance()->load_project_session( this->session_id_ );
  }
  catch( ... )
  {
    std::string error = std::string( "Failed to load session '" ) + 
      this->session_name_ + "'.";
    context->report_error( error );
    success = false;  
  }

  // TODO: This works around a problem in the current code, that makes a change when a session
  // is loaded. This adds a reset on the current application callback stack so only changes 
  // after this point will count. At some point in the near future we should check this logic.
  if ( ProjectManager::Instance()->get_current_project() )
  {
    ActionResetChangesMade::Dispatch( Core::Interface::GetWidgetActionContext() );
  }

  // Allow the user to interact with the GUI once more.
  progress->end_progress_reporting();
  
  if ( success )
  {
    std::string success_message = std::string( "Successfully loaded session '" ) + 
      this->session_name_ + "'.";
    CORE_LOG_SUCCESS( success_message );
  }
  else
  {
    std::string error = "Failed to load session '" + this->session_name_ + "'.";
    context->report_error( error );
  }
  
  return success;
}

void ActionLoadSession::Dispatch( Core::ActionContextHandle context, 
   long long session_id )
{
  ActionLoadSession* action = new ActionLoadSession;
  action->session_id_ = session_id;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
