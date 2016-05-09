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
#include <Core/Action/ActionProgress.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionExportProject.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportProject )

namespace Seg3D
{

bool ActionExportProject::validate( Core::ActionContextHandle& context )
{
  // Check whether files still exist, if not an error is being generated
  // We are dealing with file I/O, hence there is no guarantee that files still exist.
  // The user may have accidentally deleted the files, or a network connection may be lost, etc.
  if ( ! ProjectManager::Instance()->get_current_project()->check_project_files() )
  {
    context->report_error( "The project directory cannot be found." );
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "Export Project FAILED for project: '" 
        + ProjectManager::Instance()->get_current_project()->project_name_state_->get() 
        + "'. Please perform a 'Save Project As' as soon as possible to preserve your data." );       
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
  
  // Get the path
  boost::filesystem::path export_path = boost::filesystem::path( this->export_path_ ); 
  
  try
  {
    export_path = boost::filesystem::absolute( export_path );
  }
  catch( ... )
  {
    std::string error = std::string( "Directory'" ) + export_path.string() +
      "' does not exist.";
    context->report_error( error );
    return false;
  }
  
  // Ensure the export path exists
  if( !boost::filesystem::exists( export_path ) )
  {
    std::string error = std::string( "Directory '" ) + export_path.string() +
      "' does not exist.";
    context->report_error( error );
    return false;
  }
  
  // Ensure output path is an actual directory
  if ( !boost::filesystem::is_directory( export_path ) )
  {
    std::string error = std::string( "'" ) + export_path.string() + "' is not a directory.";
    context->report_error( error );
    return false;
  }

  // Ensure a project can be written in this directory
  if ( boost::filesystem::exists( export_path / ( this->project_name_ + 
    Project::GetDefaultProjectPathExtension() ) ) )
  {
    std::string error = std::string( "'" ) + export_path.string() + " already contains a "
      "project called '" + this->project_name_  + "'.";
    context->report_error( error );
    return false;
  }

  // We should be able to export the project
  return true;
}

bool ActionExportProject::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Exporting project: '" ) + this->project_name_
    + std::string( "'" );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  bool success = ProjectManager::Instance()->export_project( this->export_path_,
    this->project_name_, this->session_id_ );

  progress->end_progress_reporting();

  std::string success_message = std::string( "Successfully exported project '" ) +
    this->project_name_ + "'.";
  CORE_LOG_SUCCESS( success_message );

  return success;
}

void ActionExportProject::Dispatch( Core::ActionContextHandle context, 
                   const std::string& export_path, 
                   const std::string& project_name, 
                   long long session_id )
{
  ActionExportProject* action = new ActionExportProject;
  
  action->export_path_ = export_path;
  action->project_name_ = project_name;
  action->session_id_ = session_id;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
