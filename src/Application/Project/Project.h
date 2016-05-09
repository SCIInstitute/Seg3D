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

#ifndef APPLICATION_PROJECT_PROJECT_H
#define APPLICATION_PROJECT_PROJECT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <vector>

// Boost includes
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Action/Action.h>
#include <Core/State/StateHandler.h>

// Application includes
#include <Application/Project/ProjectNote.h>
#include <Application/Project/SessionInfo.h>
#include <Application/Project/InputFilesImporter.h>
#include <Application/Provenance/Provenance.h>
#include <Application/Provenance/ProvenanceStep.h>


namespace Seg3D
{

// TODO: Move it to its own file
// Application/Prtoject/SessionPriority.h
CORE_ENUM_CLASS
(
  SessionPriority,
  DEFAULT_PRIORITY_E = -1,
  METADATA_MANAGER_PRIORITY_E = 400,
  LAYER_MANAGER_PRIORITY_E = 300,
  VIEWER_MANAGER_PRIORITY_E = 200,
  TOOL_MANAGER_PRIORITY_E = 100
)

/// CLASS Project
/// This is the main class for collecting state information on a Project.
/// Project is separated from ProjectManager, so it can have its own state manager,
/// that is saved in the project directory. The one from ProjectManager contains global
/// settings that are stored on a per user basis, instead of a per project basis.

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
  /// The name of the project
  Core::StateStringHandle project_name_state_;

  /// The filename that contains the project information
  Core::StateStringHandle project_file_state_;
  
  /// Path of the current project
  Core::StateStringHandle project_path_state_;  
  
  /// Whether the project has been generated on disk
  Core::StateBoolHandle project_files_generated_state_;
        
  // TODO: Need to work on this variable      
  // Keep track of whether files on disk accessible
  Core::StateBoolHandle project_files_accessible_state_;
    
  /// The size of the directory that is used for this project
  Core::StateLongLongHandle project_size_state_;
  
  /// The name that is shown in the widget for saving a session
  /// NOTE: The next session will have this name.
  Core::StateStringHandle current_session_name_state_;

  /// This state variable is used to save the colors of a session
  Core::StateBoolHandle save_custom_colors_state_;

  /// The colors for the project
  // TODO: used???
  std::vector< Core::StateColorHandle > color_states_;
  
  // TODO: Should be moved in the database
  Core::StateStringVectorHandle project_notes_state_;

  /// This state variable keeps track of the unique ids for each input file
  /// NOTE: We need to save this one, to ensure that new additions to the project
  /// will have unique inputfiles number.
  Core::StateLongLongHandle inputfiles_count_state_;

  /// Generation counter state, this one is filled out when the project is saved
  /// NOTE: We need to save this one, to ensure that new additions to the project
  /// will have unique generation number.
  Core::StateLongLongHandle generation_count_state_;
  
  /// Provenance counter state, this one is filled out when the project is saved
  /// NOTE: We need to save this one, to ensure that new additions to the project
  /// will have unique provenance numbers.
  Core::StateLongLongHandle provenance_count_state_;
  
  /// == old state variable *not* used any more ==
  /// NOTE: This one is maintained for backwards compatibility only
  Core::StateStringVectorHandle sessions_state_;

public:
  /// SESSION_LIST_CHANGED_SIGNAL
  /// When a session is saved or deleted this signal is triggered
  /// NOTE: This one is used by the User Interface
  typedef boost::signals2::signal< void( SessionInfoListHandle ) > session_list_signal_type;
  session_list_signal_type session_list_changed_signal_;

  /// NOTE_LIST_CHANGED_SIGNAL:
  /// Signals the current list of notes.
  typedef boost::signals2::signal< void ( ProjectNoteListHandle ) > note_list_signal_type;
  note_list_signal_type note_list_changed_signal_;
  
  /// PROVENANCE_TRAIL_SIGNAL:
  /// This signal is triggered when a new provenance record is added, or when
  /// the 'request_provenance_trail' function is called.
  typedef boost::signals2::signal< void( ProvenanceTrailHandle ) > provenance_trail_signal_type;
  provenance_trail_signal_type provenance_trail_signal_;

  /// PROJECT_DATA_CHANGED_SIGNAL:
  /// Triggered when the project data has changed.
  boost::signals2::signal< void () > project_data_changed_signal_;

public:
  /// SAVE_PROJECT:
  /// This function will save the current project in the designated path
  /// If anonymize is true then patient-specific data will be removed when saving the project.
  /// NOTE: path already points to the project directory
  /// NOTE: This function can only be called from the application thread.
  bool save_project( const boost::filesystem::path& project_path, const std::string& project_name,
    bool anonymize );

  /// SAVE_STATE:
  /// Save the current state into the xml file
  bool save_state();

  /// LOAD_SESSION:
  /// This function will be called to load a specific session
  /// NOTE: This function can only can called from the application thread.
  bool load_session( SessionID session_id );

  /// LOAD_LAST_SESSION:
  /// Load the last saved session.
  bool load_last_session();
  
  /// SAVE_SESSION:
  /// This function will be called from the project manager to save a session
  /// NOTE: This function can only can called from the application thread.
  bool save_session( const std::string& name );
  
  /// DELETE_SESSION:
  /// This function will be called by the project manager to delete a session
  /// NOTE: This function can only can called from the application thread.
  bool delete_session( SessionID session_id );

  /// IS_SESSION:
  /// Function for validating that a session name exists
  /// NOTE: This function can only can called from the application thread.
  bool is_session( SessionID session_id );

  /// GET_SESSION_INFO:
  /// Query the information of a given session.
  /// Returns true on success, otherwise false.
  bool get_session_info( SessionID session_id, SessionInfo& session_info );

  /// REQUEST_SESSION_LIST:
  /// Request a list of all the sessions.
  /// This would trigger the session_list_changed_signal_ in the application thread.
  void request_session_list();

  /// EXPORT_PROJECT:
  /// This function will export the current project and the passed vector of session names to file
  /// NOTE: This function can only can called from the application thread.
  bool export_project( const boost::filesystem::path& project_path, 
    const std::string& project_name, long long session_id );
  
  /// CHECK_PROJECT_FILES:
  /// Check whether the project files still exist
  /// NOTE: This function can only be called from the application thread. It is intended for
  /// the validate state of the project actions.
  bool check_project_files();
    
  /// GET_PROJECT_DATA_PATH:
  /// Get the data path of this project
  boost::filesystem::path get_project_data_path() const;
  
  /// GET_PROJECT_SESSION_PATH:
  /// Get the session path of this project
  boost::filesystem::path get_project_sessions_path() const;

  /// GET_PROJECT_INPUTFILES_PATH:
  /// Get the input files path of this project
  boost::filesystem::path get_project_inputfiles_path() const;
  
  /// FIND_CACHED_FILE
  /// Find a cached file in the project
  bool find_cached_file( const boost::filesystem::path& filename, InputFilesID inputfiles_id,
    boost::filesystem::path& cached_filename ) const;

  // -- Notes --
public:
  /// ADD_NOTE:
  /// Add a new note to project.
  bool add_note( const std::string& note );

  /// REQUEST_NOTE_LIST:
  /// Request a list of all the notes.
  /// This would trigger the note_list_changed_signal_ in the application thread.
  void request_note_list();
  
protected:
  /// PRE_SAVE_STATES:
  /// this function synchronizes the colors if they are set to be saved with the project
  virtual bool pre_save_states( Core::StateIO& state_io );
  
  /// POST_LOAD_STATES:
  /// this function sets Seg3d's mask colors if they are set to be saved with the project
  virtual bool post_load_states( const Core::StateIO& state_io );

  /// GET_VERSION:
  /// Get the version number of the project file.
  virtual int get_version();
  
  // -- anonymize --
public:

  /// GET_NEED_ANONYMIZE:
  /// Whether data needs to be anonymized on the next save
  bool get_need_anonymize();
  
  /// SET_NEED_ANONYMIZE:
  /// Whether data needs to be anonymized on the next save
  void set_need_anonymize( bool enable );
  
  // -- functions for keeping track of whether project still needs to be saved --
public:
  /// RESET_PROJECT_CHANGED:
  /// Reset the flag that remembers that a session has changed
  /// NOTE: This is called by saving a session.
  void reset_project_changed();
  
  /// CHECK_PROJECT_CHANGED:
  /// Check whether the project was changed
  /// NOTE: This function can be called from any thread
  bool check_project_changed();

  /// GET_LAST_SAVED_SESSION_TIME_STAMP:
  /// When was the last session saved
  boost::posix_time::ptime get_last_saved_session_time_stamp() const;

  // -- provenance support --
public: 
  /// ADD_PROVENANCE_RECORD:
  /// Add the provenance step to the database and return the ID of the new record.
  ProvenanceStepID add_provenance_record( const ProvenanceStepHandle& step );

  /// DELETE_PROVENANCE_RECORD:
  /// Delete the specified provenance record.
  bool delete_provenance_record( ProvenanceStepID record_id );

  /// UPDATE_PROVENANCE_RECORD:
  /// Update the provenance record with new information.
  void update_provenance_record( ProvenanceStepID record_id, const ProvenanceStepHandle& prov_step );
  
  /// REQUEST_PROVENANCE_RECORD:
  /// Request the provenance_trail_signal_ to be triggered with the 
  /// provenance trail of the given provenance ID.
  void request_provenance_trail( ProvenanceID prov_id );

  /// GET_PROVENANCE_TRAIL:
  /// Get the provenance trail of the given provenance ID.
  /// NOTE: This function can only be called on the application thread.
  ProvenanceTrailHandle get_provenance_trail( const std::vector< ProvenanceID >& prov_ids );
  
  // -- function called by layers --
public:
  /// ADD_GENERATION_NUMBER:
  /// Tell the project which generation numbers are part of the project
  void add_generation_number( const long long generation_number );

  //-- input file directory handling --
public:
  /// Add a file list of files to import to the project and execute if it already resides on
  /// disk. If the project is not saved yet, copying will be delayed until the project will be
  /// saved to disk.
  bool execute_or_add_inputfiles_importer( const InputFilesImporterHandle& importer );

private:
  /// INITIALIZE_STATES:
  /// Called by constructors to initialize state variables
  void initialize();

  /// LOAD_PROJECT:
  /// This function is called by the constructor to load the project into memory.
  /// NOTE: This function can only be called from the application thread.
  /// NOTE: project_file is the .s3d file
  bool load_project( const boost::filesystem::path& project_file );

private:
  ProjectPrivateHandle private_;
  
  // -- File extension information --
public:
  /// GETDEFAULTPROJECTPATHEXTENSION
  /// Get the extension that the project directories get
  static std::string GetDefaultProjectPathExtension();
  
  /// GETDEFAUTLPROEJCTFILEEXTENSION
  /// Get the extension of the project file itself
  static std::string GetDefaultProjectFileExtension();
  
  /// GETPROJECTPATHEXTENSION
  /// Get all the possible project extensions that we read
  static std::vector<std::string> GetProjectPathExtensions();
  
  /// GETPROJECTFILEEXTENSION
  /// Get all the possible project file extensions that we read
  static std::vector<std::string> GetProjectFileExtensions();   
};

} // end namespace Seg3D

#endif
