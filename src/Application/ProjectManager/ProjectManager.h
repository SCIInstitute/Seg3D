/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_PROJECTMANAGER_PROJECTMANAGER_H
#define APPLICATION_PROJECTMANAGER_PROJECTMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>
#include <Core/State/StateHandler.h>

// Application includes
#include <Application/Project/Project.h>
#include <Application/ProjectManager/ProjectInfo.h>

namespace Seg3D
{

// Forward declarations
class ProjectManager;
class ProjectManagerPrivate;
typedef boost::shared_ptr<ProjectManagerPrivate> ProjectManagerPrivateHandle;

// Class definition

class ProjectManager : public Core::StateHandler, public Core::RecursiveLockable
{
  CORE_SINGLETON( ProjectManager );

  // -- Constructor/Destructor --
private:
  ProjectManager();
  virtual ~ProjectManager();
  
  /// GET_VERSION:
  /// Retrieve the version of this class
  virtual int get_version() { return 2; }
  
public:
  /// NEW_PROJECT:
  /// this function sets the state values of the current project to reflect the desired values
  /// NOTE: If no location is given, the project is not saved to disk
  bool new_project( const std::string& project_location, const std::string& project_name );
  
  /// OPEN_PROJECT:
  /// this function takes the path to the desired project and loads the values from the file 
  /// located at that location
  bool open_project( const std::string& project_path );
  
  /// SAVE_PROJECT_AS:
  /// this function saves the values in current_project_ to the current save location.
  /// If anonymize is true then patient-specific data will be removed when saving the project.
  bool save_project_as( const std::string& project_location, const std::string& project_name,
    bool anonymize );
  
  /// EXPORT_PROJECT:
  /// this function saves the value in current_project_, and the selected session to the desired
  /// save location 
  bool export_project( const std::string& export_path, const std::string& project_name, 
    long long session_id );
      
  /// SAVE_PROJECT_SESSION:
  /// this function saves the current session to disk
  bool save_project_session( const std::string& session_name ); 
  
  /// LOAD_PROJECT_SESSION:
  /// this function saves the current session to disk
  bool load_project_session( long long session_id );
  
  /// DELETE_PROJECT_SESSION:
  /// this function deletes the current session from disk
  bool delete_project_session( long long session_id );

  /// CHECKPOINT_PROJECTMANAGER:
  /// This function calls save_states that writes the state values of ProjectManager to file
  void checkpoint_projectmanager();
  
  /// GET_CURRENT_PROJECT:
  /// Get the current project
  ProjectHandle get_current_project() const;

  /// GET_RECENT_PROJECTS:
  /// Get a vector of recent projects
  bool get_recent_projects( ProjectInfoList& recent_projects );
  
  /// GET_CURRENT_PROJECT_FOLDER:
  /// Get a current_project_folder that is actually available
  boost::filesystem::path get_current_project_folder();
  
  /// GET_CURRENT_FILE_FOLDER:
  /// Get a current_file_folder that is actually available
  boost::filesystem::path get_current_file_folder();
  
public:
  // Here is the signal we need to let everyone know that the recent projects database has changed
  typedef boost::signals2::signal< void() > recent_project_signal_type;
  recent_project_signal_type recent_projects_changed_signal_;
  
  typedef boost::signals2::signal< void() > current_project_changed_signal_type;
  current_project_changed_signal_type current_project_changed_signal_;
  
public:
  /// Counter for making new project names
  Core::StateIntHandle default_project_name_counter_state_;
  
  /// Path to the directory from which projects are loaded
  Core::StateStringHandle current_project_folder_state_; 

  /// Path to the directory from which files are loaded
  Core::StateStringHandle current_file_folder_state_; 

  // -- internals --
private:
  ProjectManagerPrivateHandle private_;
  
  // -- static functions --
public:
  
  /// CHECKPROJECTFILE:
  /// Check if a file is a valid project
  static bool CheckProjectFile( const boost::filesystem::path& path, 
        std::string& error ); 
};

} // end namespace seg3d

#endif

