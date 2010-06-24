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
#include <Application/ProjectManager/Actions/ActionLoadSession.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, LoadSession )

namespace Seg3D
{

bool ActionLoadSession::validate( Core::ActionContextHandle& context )
{
  if( ProjectManager::Instance()->current_project_->
    get_session_name( this->session_index_.value(), this->session_name_.value() ) )
  {
    return true;
  }
  return false;
}

bool ActionLoadSession::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  bool success = false;

  std::string message = std::string("Loading session: '") + 
    this->session_name_.value() + std::string("'");

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  if( ProjectManager::Instance()->load_project_session( this->session_index_.value() ) )
  {
    success = true;
  }

  progress->end_progress_reporting();


  return success;
}

Core::ActionHandle ActionLoadSession::Create( int session_index )
{
  ActionLoadSession* action = new ActionLoadSession;
  
  action->session_index_.value() = session_index;
  
  return Core::ActionHandle( action );
}

void ActionLoadSession::Dispatch( int session_index )
{
  Core::Interface::PostAction( Create( session_index ) );
}

} // end namespace Seg3D
