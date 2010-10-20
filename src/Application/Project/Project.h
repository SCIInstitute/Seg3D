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
#include <Application/Session/Session.h>
#include <Application/Project/DataManager.h>


// Core includes
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
  Core::StateStringVectorHandle sessions_state_;
  Core::StateStringVectorHandle project_notes_state_;
  Core::StateLongLongHandle project_file_size_state_;
  Core::StateStringHandle current_session_name_state_;
  std::vector< Core::StateColorHandle > color_states_;
  
  
public:
  typedef boost::signals2::signal< void( std::string ) > session_deleted_signal_type;
  session_deleted_signal_type session_deleted_signal_;
  
public:
  // INITIALIZE_FROM_FILE:
  // this file initializes the state values for project from the file at the path specified
  bool initialize_from_file( const std::string& project_name );

  // LOAD_SESSION:
  // this function will be called to load a specific session
  bool load_session( const std::string& session_name );
  
  // SAVE_SESSION:
  // this function will be called from the project manager to save a session
  bool save_session( const std::string& session_name );
  
  // DELETE_SESSION:
  // this function will be called by the project manager to delete a session
  bool delete_session( const std::string& session_name );

  // EXPORT:
  // this function will export the current project and the passed vector of session names to file
  bool project_export( boost::filesystem::path path, const std::string& project_name, const std::string& session_name );
  
  // NAME_IS_SET:
  // this function is set called to set the name_set_ toggle in the project so it knows if the name
  // has actually been set.
  void set_valid( bool set )
  { 
    this->valid_ = set; 
  }
  
  // NAME_STATUS:
  // this function is called to check the status of the project name.  This is because we get a 
  // signal that the project name has changed the first time it gets set.  This is a temporary 
  // stopgap until we can implement signal blocking
  bool is_valid()
  {
    return this->valid_; 
  }

  // GET_SESSION_NAME:
  // this function gets the name of a session at an index of the projects session list, this is 
  // used for display what session you are loading when you load a session.
  std::string get_session_name( int index );

  // VALIDATE_SESSION_NAME:
  // function for validating that a session name exists
  bool validate_session_name( std::string& session_name );

  // INVALIDATE_CURRENT_SESSION: // NOT CURRENTLY USED //
  // this is a public function that enables the ProjectManager to call invalidate on the current
  // session
  void invalidate_current_session(){ this->current_session_->invalidate(); }

  // CLEAR_DATAMANAGER_LIST:
  // function for clearing out the datamanager list
  void clear_datamanager_list(){ this->data_manager_->clear_data_file_list(); }

  // SET_PROJECT_PATH:
  // function that lets the project manager set the project path for the project
  void set_project_path( const boost::filesystem::path& project_path );

  // SET_SIGNAL_BLOCK:
  // this function is a public function that enables the project manager to disable the signals 
  // that the project emits when it's state variables are changed
  void set_signal_block( bool on_off );
  
  // CHECK_PROJECT_CHANGED:
  // Check whether the project was changed
  bool check_project_changed();

protected:
  // PRE_SAVE_STATES:
  // this function synchronizes the colors if they are set to be saved with the project
  virtual bool pre_save_states( Core::StateIO& state_io );
  
  // POST_LOAD_STATES:
  // this function sets Seg3d's mask colors if they are set to be saved with the project
  virtual bool post_load_states( const Core::StateIO& state_io );
  
private:
  // ADD_SESSION_TO_LIST
  // this function adds sessions to the list of sessions that is stored in the projects state
  void add_session_to_list( const std::string& session_path_and_name );

  // CLEANUP_SESSION_LIST:
  // this function cleans up sessions in the session list that have been deleted by the user
  void cleanup_session_list();

public:
  // SET_PROJECT_CHANGED:
  // Set that the session has been modified
  void set_project_changed( Core::ActionHandle action, Core::ActionResultHandle result );

  // RESET_PROJECT_CHANGED:
  // Reset the flag that remembers that a session has changed
  void reset_project_changed();

private:
  // Session current using
  SessionHandle current_session_;

  // whether a name has been assigned
  bool valid_;
  
  // Where to save the project
  boost::filesystem::path project_path_;

  // Where the data is being managed
  DataManagerHandle data_manager_;
  
  // Whether the project has changed
  bool changed_;
  
};

} // end namespace Seg3D

#endif
