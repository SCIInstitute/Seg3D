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

#ifndef APPLICATION_PROJECT_PROJECT_H
#define APPLICATION_PROJECT_PROJECT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// Application indludes
#include <Application/Project/Session.h>


// Volume includes
#include <Core/Action/Action.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Volume/Volume.h>
#include <Core/State/State.h>

namespace Seg3D
{

// CLASS Project
// This is the main class for collecting state information on a Project
class Project;
  
typedef boost::shared_ptr< Project > ProjectHandle;

// Class definition
class Project : public Core::StateHandler
{

  // -- constructor/destructor --
public:
  Project( const std::string& project_name );
  virtual ~Project();
  
public:
  Core::StateStringHandle project_name_state_;
  Core::StateBoolHandle save_custom_colors_state_;
  Core::StateBoolHandle auto_consolidate_files_state_;
  Core::StateStringVectorHandle sessions_state_;
  Core::StateStringVectorHandle project_notes_state_;
  
  
  
public:
  // INITIALIZE_FROM_FILE:
  // this file initializes the state values for project from the file at the path specified
  bool initialize_from_file( boost::filesystem::path project_path, const std::string& project_name );
  
  // LOAD_SESSION:
  // this function will be called to load a specific session
  bool load_session( boost::filesystem::path project_path, int state_index );
  
  // SAVE_SESSION:
  // this function will be called from the project manager to save a session
  bool save_session( boost::filesystem::path project_path, const std::string& session_name );
  
  // DELETE_SESSION:
  // this function will be called by the project manager to delete a session
  bool delete_session( boost::filesystem::path project_path, int state_index );
  
  // NAME_IS_SET:
  // this function is set called to set the name_set_ toggle in the project so it knows if the name
  // has actually been set.
  void name_is_set( bool set ){ this->name_set_ = set; }
  
  // NAME_STATUS:
  // this function is called to check the status of the project name.  This is because we get a 
  // signal that the project name has changed the first time it gets set.  This is a temporary 
  // stopgap until we can implement signal blocking
  bool name_status(){ return this->name_set_; }
  
  
private:
  // ADD_SESSION_TO_LIST
  // this function adds sessions to the list of sessions that is stored in the projects state
  void add_session_to_list( const std::string& session_path_and_name );
  
private:
  SessionHandle current_session_;
  bool name_set_;
  

};

} // end namespace Seg3D

#endif
