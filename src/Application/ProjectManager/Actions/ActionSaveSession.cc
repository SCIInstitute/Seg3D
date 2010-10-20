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
  if( this->is_autosave_.value() )
  {
    boost::posix_time::ptime time_stamp = ProjectManager::Instance()->get_last_saved_session_time_stamp();
    if( ProjectManager::Instance()->get_last_saved_session_time_stamp() > this->time_stamp_ )
    {
      return false;
    }
    
    if( ProjectManager::Instance()->is_saving() )
    {
      return false;
    }
  }

  
  return true; // validated
}

bool ActionSaveSession::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string("Please wait, while your session is being saved...");

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  ProjectManager::Instance()->save_project( this->is_autosave_.value(), this->session_name_.value() );

  progress->end_progress_reporting();

  ProjectManager::Instance()->get_current_project()->reset_project_changed();

  return true;
}

Core::ActionHandle ActionSaveSession::Create( bool is_autosave, const std::string& session_name )
{
  ActionSaveSession* action = new ActionSaveSession;
  
  action->is_autosave_.value() = is_autosave;
  action->session_name_.value() = session_name;
  
  action->time_stamp_ = boost::posix_time::second_clock::local_time();
  return Core::ActionHandle( action );
}

void ActionSaveSession::Dispatch( Core::ActionContextHandle context, bool is_autosave, 
  std::string session_name )
{
  Core::ActionDispatcher::PostAction( Create( is_autosave, session_name ), context );
}

} // end namespace Seg3D
