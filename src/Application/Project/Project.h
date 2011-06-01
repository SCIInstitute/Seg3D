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

// Core includes
#include <Core/Action/Action.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Volume/Volume.h>
#include <Core/State/State.h>

// Application includes
#include <Application/Project/SessionInfo.h>
#include <Application/Provenance/Provenance.h>
#include <Application/Provenance/ProvenanceStep.h>


namespace Seg3D
{

CORE_ENUM_CLASS
(
  SessionPriority,
  DEFAULT_PRIORITY_E = -1,
  METADATA_MANAGER_PRIORITY_E = 400,
  LAYER_MANAGER_PRIORITY_E = 300,
  VIEWER_MANAGER_PRIORITY_E = 200,
  TOOL_MANAGER_PRIORITY_E = 100
)

typedef std::vector< std::pair< std::string, std::string > > ProvenanceTrail;
typedef boost::shared_ptr< ProvenanceTrail > ProvenanceTrailHandle;

// CLASS Project
// This is the main class for collecting state information on a Project.
// Project is separated from ProjectManager, so it can have its own state manager,
// that is saved in the project directory. The one from ProjectManager contains global
// settings that are stored on a per user basis, instead of a per project basis.

class Project;
class ProjectPrivate;
typedef boost::shared_ptr< Project > ProjectHandle;
typedef boost::shared_ptr< ProjectPrivate > ProjectPrivateHandle;

// Class definition
class Project : public Core::StateHandler, public Core::RecursiveLockable
{

  // -- constructor/destructor --
public:
  explicit Project( const std::string& project_name = "New Project" );
  explicit Project( const boost::filesystem::path& project_file );
  virtual ~Project();
  
public:
  // The name of the project
  Core::StateStringHandle project_name_state_;

  // The filename that contains the project information
  Core::StateStringHandle project_file_state_;
  
  // Path of the current project
  Core::StateStringHandle project_path_state_;  
  
  // Whether the project has been generated on disk
  Core::StateBoolHandle project_files_generated_state_;
        
  // Keep track of whether files on disk accessible
  Core::StateBoolHandle project_files_accessible_state_;
    
  // The size of the directory that is used for this project
  Core::StateLongLongHandle project_size_state_;
  
  // The name that is shown in the widget for saving a session
  // NOTE: The next session will have this name.
  Core::StateStringHandle current_session_name_state_;

  // This state variable is used to save the colors of a session
  Core::StateBoolHandle save_custom_colors_state_;

  // The colors for the project
  std::vector< Core::StateColorHandle > color_states_;
  
  // This state variable keeps track of the unique ids for each session
  Core::StateIntHandle session_count_state_;

  // TODO: Should be moved in the database
  Core::StateStringVectorHandle project_notes_state_;

  // Generation counter state, this one is filled out when the project is saved
  // NOTE: We need to save this one, to ensure that new additions to the project
  // will have unique generation number.
  Core::StateLongLongHandle generation_count_state_;
  
  // Provenance counter state, this one is filled out when the project is saved
  // NOTE: We need to save this one, to ensure that new additions to the project
  // will have unique provenance numbers.
  Core::StateLongLongHandle provenance_count_state_;
  
  // == old state variable *not* used any more ==
  // NOTE: This one is maintained for backwards compatibility only
  Core::StateStringVectorHandle sessions_state_;
  
public:
  // SESSION_CHANGED_SIGNAL
  // When a session is saved or deleted this signal is triggered
  // NOTE: This one is used by the User Interface
  typedef boost::signals2::signal< void() > sessions_changed_signal_type;
  sessions_changed_signal_type sessions_changed_signal_;
  
  typedef boost::signals2::signal< void( ProvenanceTrailHandle ) > provenance_records_signal_type;
  provenance_records_signal_type provenance_record_signal_;
public:
  // SAVE_PROJECT:
  // This function will save the current project in the designated path
  // NOTE: path already points to the project directory
  // NOTE: This function can only be called from the application thread.
  bool save_project( const boost::filesystem::path& project_path, const std::string& project_name );

  // SAVE_STATE:
  // Save the current state into the xml file
  bool save_state();

  // LOAD_SESSION:
  // This function will be called to load a specific session
  // NOTE: This function can only can called from the application thread.
  bool load_session( SessionID session_id );

  // LOAD_LAST_SESSION:
  // Load the last saved session.
  bool load_last_session();
  
  // SAVE_SESSION:
  // This function will be called from the project manager to save a session
  // NOTE: This function can only can called from the application thread.
  bool save_session( const std::string& name );
  
  // DELETE_SESSION:
  // This function will be called by the project manager to delete a session
  // NOTE: This function can only can called from the application thread.
  bool delete_session( SessionID session_id );

  // IS_SESSION:
  // Function for validating that a session name exists
  // NOTE: This function can only can called from the application thread.
  bool is_session( SessionID session_id );

  // EXPORT_PROJECT:
  // This function will export the current project and the passed vector of session names to file
  // NOTE: This function can only can called from the application thread.
  bool export_project( const boost::filesystem::path& project_path, 
    const std::string& project_name, long long session_id );
  
  // CHECK_PROJECT_FILES:
  // Check whether the project files still exist
  // NOTE: This function can only be called from the application thread. It is intended for
  // the validate state of the project actions.
  bool check_project_files();
    
  // GET_PROJECT_DATA_PATH:
  // Get the data path of this project
  boost::filesystem::path get_project_data_path() const;
  
  // GET_PROJECT_SESSION_PATH:
  // Get the session path of this project
  boost::filesystem::path get_project_sessions_path() const;

  // GET_PROJECT_INPUTFILES_PATH:
  // Get the input files path of this project
  boost::filesystem::path get_project_inputfiles_path() const;
  
protected:
  // PRE_SAVE_STATES:
  // this function synchronizes the colors if they are set to be saved with the project
  virtual bool pre_save_states( Core::StateIO& state_io );
  
  // POST_LOAD_STATES:
  // this function sets Seg3d's mask colors if they are set to be saved with the project
  virtual bool post_load_states( const Core::StateIO& state_io );

  // GET_VERSION:
  // Get the version number of the project file.
  virtual int get_version();
  
  // -- functions for keeping track of whether project still needs to be saved --
public:
  // SET_PROJECT_CHANGED:
  // Set that the session has been modified
  // NOTE: It needs the same signature of the ActionDispatcher
  // NOTE: This function can only can called from the application thread.
  void set_project_changed( Core::ActionHandle action, Core::ActionResultHandle result );

  // RESET_PROJECT_CHANGED:
  // Reset the flag that remembers that a session has changed
  // NOTE: This is called by saving a session.
  void reset_project_changed();
  
  // CHECK_PROJECT_CHANGED:
  // Check whether the project was changed
  // NOTE: This function can be called from any thread
  bool check_project_changed();

  // GET_LAST_SAVED_SESSION_TIME_STAMP:
  // When was the last session saved
  boost::posix_time::ptime get_last_saved_session_time_stamp() const;

  // -- provenance support --
public: 
  // ADD_PROVENANCE_RECORD:
  // Add the provenance step to the database and return the ID of the new record.
  ProvenanceStepID add_provenance_record( const ProvenanceStepHandle& step );

  // DELETE_PROVENANCE_RECORD:
  // Delete the specified provenance record.
  bool delete_provenance_record( ProvenanceStepID record_id );

  // UPDATE_PROVENANCE_RECORD:
  // Update the provenance record with new information.
  void update_provenance_record( ProvenanceStepID record_id, const ProvenanceStepHandle& prov_step );
  
  // GET_PROVENANCE_RECORD:
  // returns a vector that is the provenance record for a particular ProvenanceID
  void request_provenance_record( ProvenanceID prov_id );
  
  // -- session support --
public:     
  // GET_ALL_SESSIONS:
  // Get the names of all the sessions in this project
  bool get_all_sessions( std::vector< SessionInfo >& sessions );
    
private:
  // INITIALIZE_STATES:
  // Called by constructors to initialize state variables
  void initialize_states();

  // LOAD_PROJECT:
  // This function is called by the constructor to load the project into memory.
  // NOTE: This function can only be called from the application thread.
  // NOTE: project_file is the .s3d file
  bool load_project( const boost::filesystem::path& project_file );

private:
  ProjectPrivateHandle private_;
  
  // -- File extension information --
public:
  // GETDEFAULTPROJECTPATHEXTENSION
  // Get the extension that the project directories get
  static std::string GetDefaultProjectPathExtension();
  
  // GETDEFAUTLPROEJCTFILEEXTENSION
  // Get the extension of the project file itself
  static std::string GetDefaultProjectFileExtension();
  
  // GETPROJECTPATHEXTENSION
  // Get all the possible project extensions that we read
  static std::vector<std::string> GetProjectPathExtensions();
  
  // GETPROJECTFILEEXTENSION
  // Get all the possible project file extensions that we read
  static std::vector<std::string> GetProjectFileExtensions();   
};

} // end namespace Seg3D

#endif
