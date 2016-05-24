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

// Core includes
#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/Action/ActionProgress.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionAutoSave.h>

CORE_REGISTER_ACTION( Seg3D, AutoSave )

namespace Seg3D
{

bool ActionAutoSave::validate( Core::ActionContextHandle& context )
{
  // TODO: We need to upgrade the system, so that when access is restored files can be used 
  // again.

  // Check whether files still exist, if not an error is being generated
  // We are dealing with file I/O, hence there is no guarantee that files still exist.
  // The user may have accidentally deleted the files, or a network connection may be lost, etc.
  if ( ! ProjectManager::Instance()->get_current_project()->check_project_files() )
  {
    context->report_error( "The project directory cannot be found." );
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "AutoSave FAILED as the project directory cannot be found. "
      "Please perform a 'Save Project As' as soon as possible to preserve your data." );        
    return false;
  }
  
  // Cancel auto save if project has been save between issuing autosave and now
  if ( this->time_stamp_ < ProjectManager::Instance()->get_current_project()->
    get_last_saved_session_time_stamp() )
  {
    // User already saved the session, hence the action is no longer needed
    return false;
  }

  return true; // validated
}

bool ActionAutoSave::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Please wait, while auto saving..." );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  // A save can still fail, we have no control over whether disk actions will succeed, hence
  // we need to keep checking the integrity of the file save.
  bool success = ProjectManager::Instance()->save_project_session( "Auto Save" );

  progress->end_progress_reporting();

  if( success )
  {
    // TODO: Check whether this call is still needed here
    ProjectManager::Instance()->get_current_project()->reset_project_changed();
    std::string message = std::string( "Successfully autosaved session for project: '" ) + 
      ProjectManager::Instance()->get_current_project()->project_name_state_->get() + "'.";
    CORE_LOG_SUCCESS( message );
    return true;
  }
  else 
  {
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "AutoSave FAILED for project: '" 
      + ProjectManager::Instance()->get_current_project()->project_name_state_->get() 
      + "'. Please perform a 'Save Project As' as soon as possible to preserve your data." );       
    return false;   
  }
}

void ActionAutoSave::Dispatch( Core::ActionContextHandle context )
{
  ActionAutoSave* action = new ActionAutoSave;
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
