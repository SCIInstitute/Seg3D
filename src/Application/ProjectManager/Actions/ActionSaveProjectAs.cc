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
#include <Application/ProjectManager/Actions/ActionSaveProjectAs.h>

CORE_REGISTER_ACTION( Seg3D, SaveProjectAs )

namespace Seg3D
{

bool ActionSaveProjectAs::validate( Core::ActionContextHandle& context )
{
  // Check whether files still exist, if not an error is being generated.
  // We are dealing with file I/O, hence there is no guarantee that files still exist.
  // The user may have accidentally deleted the files, or a network connection may be lost, etc.
  ProjectManager::Instance()->get_current_project()->check_project_files();

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

bool ActionSaveProjectAs::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Saving project as: '" ) + this->project_name_
    + std::string( "'" );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();
  
  bool success = 
    ProjectManager::Instance()->save_project_as( 
      this->project_location_, this->project_name_, this->anonymize_ );

  progress->end_progress_reporting();

  if( !success )
  {
    // Draw the users attention to this problem.
    CORE_LOG_CRITICAL_ERROR( "Save Project As FAILED for project: '" 
      + ProjectManager::Instance()->get_current_project()->project_name_state_->get() 
      + "'. Please perform a 'Save Project As' as soon as possible to preserve your data." );
  }
  else
  {   
    std::string success_message = std::string( "Successfully saved project as '" ) + 
      ProjectManager::Instance()->get_current_project()->project_name_state_->get() + "'.";
    CORE_LOG_SUCCESS( success_message );
  }
  
  return success;
}

void ActionSaveProjectAs::Dispatch( Core::ActionContextHandle context, 
  const std::string& project_location, const std::string& project_name, bool anonymize )
{
  ActionSaveProjectAs* action = new ActionSaveProjectAs;
  
  action->project_location_ = project_location;
  action->project_name_ = project_name;
  action->anonymize_ = anonymize;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
