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

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Layer/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/ProjectManager/Actions/ActionNewProject.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, NewProject )

namespace Seg3D
{

bool ActionNewProject::validate( Core::ActionContextHandle& context )
{
  // Check whether the project has a name.
  if ( this->project_name_.empty() )
  {
    context->report_error( "Project needs to have a name." );
    return false;
  }

  // Need to check whether the location exists
  if ( ! this->project_location_.empty() )
  {
    boost::filesystem::path project_location( this->project_location_ );

    // Complete the project location
    try 
    {
      project_location = boost::filesystem::absolute( project_location );
    }
    catch ( ... )
    {
      std::string error = std::string( "Directory '" ) + project_location.string() +
        "' does not exist.";
      context->report_error( error );
      return false;
    }
    
    // Check whether the new location actually exists
    if ( !boost::filesystem::exists( project_location ) )
    {
      std::string error = std::string( "Directory '" ) + project_location.string() +
        "' does not exist.";
      context->report_error( error );
      return false;
    }
    
    // Check whether it is actually a directory
    if ( !boost::filesystem::is_directory( project_location ) )
    {
      std::string error = std::string( "File location '" ) + project_location.string() +
        "' is not a directory.";
      context->report_error( error );
      return false;
    }

    // Generate the new name of the project folder, to check if it is available
    boost::filesystem::path project_folder = 
      boost::filesystem::path( this->project_location_ ) / 
        ( this->project_name_ + Project::GetDefaultProjectPathExtension() );
    
    // Check if folder already exists
    if ( boost::filesystem::exists( project_folder ) )
    {
      std::string error = std::string( "Directory '" ) + project_folder.string() +
        "' already exists.";    
      context->report_error( error );
      return false;
    }
  }
  
  return true;
}

bool ActionNewProject::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Please wait while project: '" ) + 
    this->project_name_ + std::string( "' is being created ..." );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  // NOTE: This will block input from the GUI.
  // NOTE: Currently it sends a message to the GUI to block the GUI, so it is not yet completely
  // thread-safe.
  progress->begin_progress_reporting();

  bool success = ProjectManager::Instance()->new_project( this->project_location_,
    this->project_name_ );
  // Generate the new project

  ProjectManager::Instance()->get_current_project()->reset_project_changed();
  
  // Release the GUI, again using message passing
  progress->end_progress_reporting();
  
  if ( !success )
  {
    context->report_error( "Could not create new project" );
    return false;
  }

  std::string success_message = std::string( "Successfully created new project '" ) +
    this->project_name_ + "'.";
  CORE_LOG_SUCCESS( success_message );

  return true;
}

void ActionNewProject::Dispatch( Core::ActionContextHandle context,
  const std::string& project_location, const std::string& project_name )
{
  ActionNewProject* action = new ActionNewProject;
  
  action->project_location_ = project_location;
  action->project_name_ = project_name;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
