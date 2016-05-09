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
  // Get the full filename of the project
  boost::filesystem::path full_filename( this->project_file_ );

  // Complete the file name
  try 
  {
    full_filename = boost::filesystem::absolute( full_filename );
  }
  catch( ... )
  {
    std::string error = std::string( "File '" ) + full_filename.string() +
      "' does not exist.";
    context->report_error( error );
    return false;
  }
  
  // Check whether file exists
  if ( !boost::filesystem::exists( full_filename ) )
  {
    context->report_error( std::string( "File '" ) + full_filename.string() +
      "' does not exist." );
    return false;
  }

  // Check if it is a .s3d file, if not we look inside the directory to find one.
  bool is_s3d_file = false;
  
  std::vector<std::string> file_extensions = Project::GetProjectFileExtensions();
  for ( size_t j = 0; j < file_extensions.size(); j++ ) 
  {
    if ( boost::filesystem::extension( full_filename ) == file_extensions[ j ] )
    {
      is_s3d_file = true;
      break;
    } 
  }
  

  
  if ( is_s3d_file == false )
  {
    bool found_s3d_file = false;
    
    if ( boost::filesystem::is_directory( full_filename ) )
    {
      boost::filesystem::directory_iterator dir_end;
      
      for( boost::filesystem::directory_iterator dir_itr( full_filename ); 
        dir_itr != dir_end; ++dir_itr )
      {
        std::string filename = dir_itr->path().filename().string();
        boost::filesystem::path dir_file = full_filename / filename;
        for ( size_t j = 0; j < file_extensions.size(); j++ ) 
        {
          if ( boost::filesystem::extension( dir_file ) == file_extensions[ j ] )
          {
            full_filename = dir_file;
            found_s3d_file = true;
            break;
          }
        }
        if ( found_s3d_file ) break;
      }
    }
    
    if ( !found_s3d_file )
    {
      context->report_error( std::string( "File '" ) + full_filename.string() +
        "' is not a project file." );
      return false;     
    }
  }
  
  // validated  
  return true;
}


bool ActionLoadProject::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  boost::filesystem::path full_filename( this->project_file_ );
  
  std::string message = std::string("Please wait, while loading project '") + 
    full_filename.stem().string() + std::string("' ...");

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  // NOTE: This will block input from the GUI.
  // NOTE: Currently it sends a message to the GUI to block the GUI, so it is not yet completely
  // thread-safe.
  progress->begin_progress_reporting();

  // Open the project
  bool success = ProjectManager::Instance()->open_project( this->project_file_ );
  
  if ( success )
  {
    // TODO: This works around a problem in the current code, that makes a change because a session
    // is loaded. This adds a reset on the current stack so only changes after this point will count.
    // At some point in the near future we should check this logic.
    if ( ProjectManager::Instance()->get_current_project() )
    {
      ActionResetChangesMade::Dispatch( Core::Interface::GetWidgetActionContext() );
    }

    CORE_LOG_SUCCESS( "Successfully loaded project '" + this->project_file_ + "'." );
  }
  else
  {
    std::string error = std::string( "Failed to load project '" ) + this->project_file_ + "'.";
    context->report_error( error );
  }
  
  // Release the GUI, again using message passing
  progress->end_progress_reporting();

  // Let the user know that everything went OK.

  return success;
}

void ActionLoadProject::Dispatch( Core::ActionContextHandle context, 
  const std::string& project_file )
{
  ActionLoadProject* action = new ActionLoadProject;  
  action->project_file_ = project_file;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
