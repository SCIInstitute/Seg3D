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

#ifndef APPLICATION_PROJECTMANAGER_PROJECTMANAGER_H
#define APPLICATION_PROJECTMANAGER_PROJECTMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 



// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>

// Application includes
#include <Core/State/StateHandler.h>
#include <Application/Project/Project.h>

namespace Seg3D
{

// Forward declaration
class ProjectManager;
  

// Class definition
class ProjectManager : public Core::StateHandler
{
  CORE_SINGLETON( ProjectManager );

  // -- Constructor/Destructor --
private:
  ProjectManager();
  virtual ~ProjectManager();
  
public:
  
  // NEW_PROJECT:
  // this function sets the state values of the current project to reflect the desired values
  void new_project( const std::string& project_name, bool consolidate );
  
  // OPEN_PROJECT:
  // this function takes the path to the desired project and loads the values from the file 
  // located at that location
  void open_project( const std::string& project_path, const std::string& project_name );
  
  // SAVE_PROJECT:
  // this function saves the values in current_project_ to the current save location
  void save_project();
  
  // SAVE_PROJECT_AS:
  // this function saves the valuse in current_project_ to the desired save location
  void save_project_as();
  
  // SAVE_PROJECT_MANAGER_STATE:
  // this function calls save_states that writes the state values of ProjectManager to file
  void save_projectmanager_state();
  
  // SAVE_PROJECT_SESSION:
  // this function saves the current session to disk
  bool save_project_session(); 
  
  // LOAD_PROJECT_SESSION:
  // this function saves the current session to disk
  bool load_project_session( int session_index ); 
  
public:
  Core::StateStringVectorHandle recent_projects_state_;
  Core::StateStringHandle current_project_path_state_;
  ProjectHandle current_project_;
  
private:
  // INITIALIZE:
  // this function loads the values for ProjectManager from file
  void initialize();
  
  // ADD_TO_RECENT_PROJECTS:
  // this function adds the latest project to the list of recent projects
  void add_to_recent_projects( const std::string& project_path, const std::string& project_name );
  
  // CREATE_PROJECT_FOLDERS:
  // this will try and create the project folders and if is successfull return true 
  bool create_project_folders( const std::string& project_name );
  
  // RENAME_PROJECT_FOLDER
  // this function is triggered when a user changes the folder name
  void rename_project_folder( const std::string& new_name, Core::ActionSource source );
  
private:
  
  std::vector< Core::Color > project_colors_;
  boost::filesystem::path local_projectmanager_path_;

};

} // end namespace seg3D

#endif

