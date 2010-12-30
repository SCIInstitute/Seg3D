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

// Boost includes
#include <boost/filesystem.hpp>

// Application includes
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>
#include <Application/ProjectManager/Actions/ActionResetChangesMade.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, LoadProject )

namespace Seg3D
{

bool ActionLoadProject::validate( Core::ActionContextHandle& context )
{
  std::string extension = "";
  boost::filesystem::path full_filename( this->project_path_.value() );
  extension = boost::filesystem::extension( full_filename );
  if( extension == ".s3d" )
  {
    this->project_path_.value() = full_filename.parent_path().string();
  }
  else
  {
    full_filename = full_filename / ( full_filename.leaf() + ".s3d" );
  }
  
  if ( !( boost::filesystem::exists ( full_filename ) ) )
  {
    context->report_error( std::string( "File '" ) + this->project_path_.value() +
      "' does not exist." );
    return false;
  }
  return true;

}

bool ActionLoadProject::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  boost::filesystem::path full_filename( project_path_.value() );
  std::string message = std::string("Please wait, while loading project '") + 
    full_filename.leaf() + std::string("' ...");

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  ProjectManager::Instance()->open_project( this->project_path_.value() );
  
  progress->end_progress_reporting();

  if ( ProjectManager::Instance()->get_current_project() )
  {
    ActionResetChangesMade::Dispatch( Core::Interface::GetWidgetActionContext() );
  }

  // Clear undo buffer
  UndoBuffer::Instance()->reset_undo_buffer();

  return true;
}

Core::ActionHandle ActionLoadProject::Create( const std::string& project_path )
{
  ActionLoadProject* action = new ActionLoadProject;
  
  action->project_path_.value() = project_path;
  
  return Core::ActionHandle( action );
}

void ActionLoadProject::Dispatch( Core::ActionContextHandle context, 
  const std::string& project_path )
{
  Core::ActionDispatcher::PostAction( Create( project_path ), context );
}

} // end namespace Seg3D
