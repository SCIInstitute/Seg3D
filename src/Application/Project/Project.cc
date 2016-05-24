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

// STL includes
#include <queue>
#include <set>
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/FilesystemUtil.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>
#include <Core/DataBlock/DataBlockManager.h>

// Application includes
#include <Application/Project/Project.h>
#include <Application/Provenance/Provenance.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/DatabaseManager/DatabaseManager.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>

// NOTE: This include is needed for APPLE to deal with directories that are bundled as a single file.
#if defined( __APPLE__ )  
#  include <CoreServices/CoreServices.h>
#endif

// Include CMake generated files
#include "ApplicationConfiguration.h"

namespace Seg3D
{

static const boost::filesystem::path SESSION_DIR_C( "sessions" );
static const boost::filesystem::path SESSION_DATABASE_C( "sessions.sqlite" );
static const boost::filesystem::path PROVENANCE_DATABASE_C( "provenance.sqlite" );
static const boost::filesystem::path DATA_DIR_C( "data" );
static const boost::filesystem::path INPUTFILES_DIR_C( "inputfiles" );
static const boost::filesystem::path DATABASE_DIR_C( "database" );
static const boost::filesystem::path NOTE_DATABASE_C( "notes.sqlite" );

static const std::string AUTO_SESSION_NAME_C( "Auto Save" );

class ProjectPrivate
{
  // -- constructor/destructor --
public:
  ProjectPrivate() :
    project_( 0 ),
    changed_( false ),
    last_saved_session_time_stamp_( boost::posix_time::second_clock::local_time() ),
    need_anonymize_( false ) 
  { 
  }

public:
  // UPDATE_PROJECT_SIZE
  // Sum the size of all the files in the directory
  void update_project_size();

  // COMPUTE_DIRECTORY_SIZE
  // Compute the size in bytes used by a directory ( used by update_directory_size() )
  long long compute_directory_size( const boost::filesystem::path& directory );

  // CLEAN_UP_SESSION_DATABASE:
  // this function cleans up sessions in the session list that have been deleted by the user
  bool clean_up_session_database(); 
  
  // CLEAN_UP_DATA_FILES:
  // Clean up files that are not used by any session.
  void clean_up_data_files(); 
  
  // SAVE_STATE:
  // Save the state of the current project into the xml file and save the database
  // in the database file
  bool save_state( const boost::filesystem::path& project_directory );  
      
  // SET_LAST_SAVED_SESSION_TIME_STAMP
  // this function updates the time of when the last session was saved
  void set_last_saved_session_time_stamp();
    
  // INITIALIZE_SESSION_DATABASE:
  // Create tables for the session database.
  bool initialize_session_database();

  // INITIALIZE_PROVENANCE_DATABASE:
  // Create tables for the provenance database.
  bool initialize_provenance_database();

  // CLEAR_PROVENANCE_DATABASE:
  // Delete records from all tables in the provenance database.
  bool clear_provenance_database();

  // INITIALIZE_NOTE_DATABASE:
  // Create tables for the note database.
  bool initialize_note_database();
  
  // INSERT_SESSION_INTO_DATABASE
  // Inset a session into the database
  SessionID insert_session_into_database( const std::string& session_name, 
    const std::string& user_id, const std::string& timestamp = "" );
  
  // DELETE_SESSION_FROM_DATABASE:
  // this deletes a session from the database
  bool delete_session_from_database( SessionID session_id );
  
  // SET_SESSION_DATA:
  // Add the data generation numbers used by the session into the database.
  bool set_session_data( SessionID id, const std::set< long long >& generations );

  // GET_LAST_SESSION:
  // Get the session ID of the most recent session.
  SessionID get_last_session();

  // QUERY_PROVENANCE_TRAIL:
  // Get all the provenance steps required to reproduce the given provenance ID.
  bool query_provenance_trail( const std::vector< ProvenanceID >& prov_ids,
    ProvenanceTrail& provenance_trail );

  // GET_PROVENANCE_STEPS:
  // Get all the provenance steps that lead to the given provenance ID.
  void get_provenance_steps( const std::vector< ProvenanceID >& prov_ids, 
    std::set< ProvenanceStepID >& prov_steps );
  
  // EXTRACT_SESSION_INFO:
  // Extract session information from the database query result.
  void extract_session_info( ResultSet& result_set, std::vector< SessionInfo >& sessions );
  
  // GET_ALL_SESSIONS:
  // Get a list of all sessions.
  bool get_all_sessions( std::vector< SessionInfo >& sessions );

  // CONVERT_VERSION1_PROJECT:
  // Convert version 1 project data to the new format.
  void convert_version1_project();

  // RENAME_VERSION1_SESSION_FILES:
  // Rename the session files to the new format.
  bool rename_version1_session_files();

  // PARSE_SESSION_DATA:
  // Parse the session XML file and get the data generation numbers used by it.
  bool parse_session_data( const boost::filesystem::path& file_path, std::set< long long >& generations );

  // PARSE_SESSION_PROVENANCE_IDS:
  // Parse the session XML file and get the provenance IDs referenced by the session.
  bool parse_session_provenance_ids( const boost::filesystem::path& file_path, 
    std::vector< long long >& prov_ids );

  // SET_SESSION_FILE:
  // Set the session XML file if it's not named after the session ID and stores it in the database.
  // NOTE: This is only for backwards compatibility purpose when the old session XML files
  // could not be renamed.
  void set_session_file( SessionID id, const std::string& file_name );

  // GET_SESSION_FILE:
  // Get the path of the session XML file.
  bool get_session_file( SessionID id, boost::filesystem::path& file_path, std::string& error );
  
  // PROCESS_INPUTFILE_IMPROTERS
  // try to copy all the files into the project
  bool process_inputfile_importers(); 

  // INSERT_NOTE_INTO_DATABASE:
  // Add a note into the note database. If the timestamp is empty, the database will use the
  // current timestamp.
  long long insert_note_into_database( const std::string& note, 
    const std::string& user_id, const std::string& timestamp = "" );

  // GET_ALL_NOTES:
  // Get a list of all project notes.
  bool get_all_notes( ProjectNoteList& notes );

  // SET_PROJECT_CHANGED:
  // Set that the session has been modified
  // NOTE: It needs the same signature of the ActionDispatcher
  // NOTE: This function can only can called from the application thread.
  void set_project_changed( Core::ActionHandle action, Core::ActionResultHandle result );

  // GET_USER_ID:
  // Get the user_id of the given user_name in the provenance database.
  // If the user_name doesn't exist yet, it will be inserted into the database.
  long long get_user_id( const std::string& user_name );

  // GET_ACTION_ID:
  // Get the action_id of the given action_name in the provenance database.
  // If the action_name doesn't exist yet, it will be inserted into the database.
  long long get_action_id( const std::string& action_name );

  // GET_USER_NAME:
  // Get the user name identified by the given ID.
  bool get_user_name( long long user_id, std::string& user_name );

  // GET_ACTION_NAME:
  // Get the action name identified by the given ID.
  bool get_action_name( long long action_id, std::string& action_name );

  // -- internal variables --
public:
  // Pointer back to the project
  Project* project_;

  // Whether the project has changed
  bool changed_;

  // Time when last session was saved for auto save functionality.
  boost::posix_time::ptime last_saved_session_time_stamp_;

  // List of importers that need to be run
  std::vector<InputFilesImporterHandle> input_file_importers_;

  // The database that contains the session list
  DatabaseManager session_database_;
  
  // NOTE: This variable is preventing a circular dependency
  // Layer session saving will add this number to this list so it can be included
  // into the final session database.
  std::set<long long> session_generation_numbers_;
  
  // The database that contains the provenance information
  DatabaseManager provenance_database_;

  // The database that contains  the project notes.
  DatabaseManager note_database_;

  // Whether we need to convert the project when saving
  bool conversion_needed_;

  // Cached user_id to user_name map
  std::map< long long, std::string > user_name_map_;

  // Cached action_id to action_name map
  std::map< long long, std::string > action_name_map_;
  
  // Whether data needs to be anonymized on the next save
  bool need_anonymize_;

  // -- static helper functions --
public:
  // UPDATE_PROJECT_DIRECTORY:
  // Generate all the required directories for the project
  static bool UpdateProjectDirectory( const boost::filesystem::path& project_directory );
};


void ProjectPrivate::update_project_size()
{
  // Get the directory that we need scan
  boost::filesystem::path project_path( this->project_->project_path_state_->get() );

  // Iteratively scan through the full directory
  long long size = this->compute_directory_size( project_path );
  
  // Update the project with the new size
  this->project_->project_size_state_->set( size );
}


// NOTE: This function needs to return a value in long long, even in 32 bit mode, as 
// file sizes can exceed 2GB. The total of directory with session can be larger than
// the total size
long long ProjectPrivate::compute_directory_size( const boost::filesystem::path& directory )
{
  // Check if the directory exists
  // If not just return 0, indicating no actual files are there
  if ( ! boost::filesystem::exists( directory ) ) return 0;

  // Reset the counter
  long long size = 0;
  
  // Walk through elements in the directory
  boost::filesystem::directory_iterator dir_end;
  for( boost::filesystem::directory_iterator dir_itr( directory ); dir_itr != dir_end; ++dir_itr )
  {
    // Filename of the next component in the directory
    std::string filename = dir_itr->path().filename().string();
    boost::filesystem::path dir_file = directory / filename;
      
    // We only scan directories and regular files, to avoid circular dependencies from
    // symbolic links.
    if ( boost::filesystem::is_regular_file( dir_file ) )
    {
      // Add the size of the file to our counter
      size += static_cast< long long >( boost::filesystem::file_size( dir_file ) );
    }
    else if ( boost::filesystem::is_directory( dir_file ) )
    {
      // if it is a directory just sum up the full size of the directory
      size += this->compute_directory_size( dir_file );
    }
  }

  return size;
}


bool ProjectPrivate::UpdateProjectDirectory( const boost::filesystem::path& project_directory  )
{
  // If the directory does not exist, make a new one
  if ( !Core::CreateOrIgnoreDirectory( project_directory ) )
  {
    // If something went wrong, log the problem and continue
    // We should not completely fail. The user should have the opportunity to try again/
    std::string error = std::string( "Could not create directory '" ) + 
      project_directory.filename().string() + "'.";
    CORE_LOG_ERROR( error );
    
    // Return false to force the program to try again.
    return false; 
  }

#if defined( __APPLE__ )  
  // Specialty code for bundles
  if ( PreferencesManager::Instance()->generate_osx_project_bundle_state_->get() )
  {   
    // Get a list with possible extensions.
    std::vector<std::string> project_path_extensions = Project::GetProjectPathExtensions();
    
    // Check if it has the special extension, old version do not have the extension and we should
    // leave them the way they were, in order to prevent it from hiding files the user saved
    // in that directory.
    for ( size_t j = 0; j < project_path_extensions.size(); j++ )
    {
      if ( boost::filesystem::extension( project_directory ) == project_path_extensions[ j ] )
      {
        // MAC CODE
        try
        {
          // This code sets the special flags on the mac
          FSRef file_ref;
          Boolean is_directory;
          FSPathMakeRef( reinterpret_cast<const unsigned char*>( 
            project_directory.string().c_str() ), &file_ref, &is_directory );
          FSCatalogInfo info;
          FSGetCatalogInfo( &file_ref, kFSCatInfoFinderInfo, &info, 0, 0, 0 );
          
          FileInfo&  finder_info = *reinterpret_cast<FileInfo*>( &info.finderInfo );
          finder_info.finderFlags |= kHasBundle;
          FSSetCatalogInfo( &file_ref, kFSCatInfoFinderInfo, &info);
        }
        catch( ... )
        {
        }
        break;
      }
    }
  }
#endif

  // Session folder logic
  // All session XMLs are stored here
  if ( !Core::CreateOrIgnoreDirectory( project_directory / SESSION_DIR_C ) )
  {
    std::string error = "Could not create sessions directory in project folder '" + 
      project_directory.filename().string() + "'.";
    CORE_LOG_ERROR( error );
    return false; 
  }
  
  // Data directory logic
  // All the session data files are stored here
  if ( !Core::CreateOrIgnoreDirectory( project_directory / DATA_DIR_C ) )
  { 
    std::string error = "Could not create data directory in project folder '" + 
      project_directory.filename().string() + "'.";
    CORE_LOG_ERROR( error );
    return false; 
  }
  
  // Database directory logic
  // All the database files (session, provenance, and notes) are stored here.
  if ( !Core::CreateOrIgnoreDirectory( project_directory / DATABASE_DIR_C ) )
  {
    std::string error = "Could not create database directory in project folder '" + 
      project_directory.filename().string() + "'.";
    CORE_LOG_ERROR( error );
    return false; 
  }

  // Inputfiles directory logic
  // All input files are stored here
  if ( !Core::CreateOrIgnoreDirectory( project_directory / INPUTFILES_DIR_C ) )
  {           
    std::string error = "Could not create inputfiles directory in project folder '" + 
      project_directory.filename().string() + "'.";
    CORE_LOG_ERROR( error );
    return false; 
  }

  // Everything worked, we should have a valid project directory
  return true;
} 


bool ProjectPrivate::clean_up_session_database()
{
  // If no files have been generated, just ignore the cleanup
  if ( ! this->project_->project_files_generated_state_->get() ) return true;

  std::string error;
  ResultSet result_set;
  std::string sql_str = "SELECT session_id, session_file FROM session;";
  if( !this->session_database_.run_sql_statement( sql_str, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Get the project directory path
  boost::filesystem::path project_path( this->project_->project_path_state_->get() );

  for( size_t i = 0; i < result_set.size(); ++i )
  {
    SessionID session_id = boost::any_cast< long long >( ( result_set[ i ] )[ "session_id" ] );
    
    // Check the existence of the session file
    boost::filesystem::path session_file;
    if ( !this->get_session_file( session_id, session_file, error ) )
    {
      // No session file exists for the session, delete it from database
      this->delete_session_from_database( session_id );
    }
  }

  return true;
}

bool ProjectPrivate::save_state( const boost::filesystem::path& project_directory )
{
  if ( ! boost::filesystem::exists( project_directory ) )
  {
    CORE_LOG_ERROR( "Tried to save state, while no project directory is active." );
    return false;
  } 

  // Save the project file using the name specified
  std::string project_file_name = this->project_->project_file_state_->get();
  boost::filesystem::path project_file = project_directory / ( project_file_name );

  Core::StateIO stateio;
  stateio.initialize();

  if ( !this->project_->save_states( stateio ) )
  {
    std::string error = std::string( "Failed to save the state of the project in file '" ) +
      project_file.string() + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }
  
  if ( ! stateio.export_to_file( project_file ) )
  {
    std::string error = std::string( "Failed to save the state of the project in file '" ) +
      project_file.string() + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }
  
  this->project_->project_files_generated_state_->set( true );
  this->project_->project_files_accessible_state_->set( true );

  std::string error;

  // Save the session database to disk
  boost::filesystem::path session_database = project_directory / 
    DATABASE_DIR_C / SESSION_DATABASE_C;
  if ( !this->session_database_.save_database( session_database, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Save the provenance database to disk
  boost::filesystem::path provenance_database = project_directory /
    DATABASE_DIR_C / PROVENANCE_DATABASE_C;
  if ( !this->provenance_database_.save_database( provenance_database, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Save the note database to disk
  boost::filesystem::path note_database = project_directory /
    DATABASE_DIR_C / NOTE_DATABASE_C;
  if ( !this->note_database_.save_database( note_database, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

void ProjectPrivate::clean_up_data_files()
{
  // Get all the generation numbers referenced by all the existing sessions
  std::string error;
  ResultSet result_set;
  std::string sql_str = "SELECT DISTINCT data_generation FROM session_data"
    " ORDER BY data_generation;";

  if ( !this->session_database_.run_sql_statement( sql_str, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return;
  }

  size_t num_data = result_set.size();
  std::vector< long long > generations( num_data );
  for ( size_t i = 0; i < num_data; ++i )
  {
    generations[ i ] = boost::any_cast< long long >( result_set[ i ][ "data_generation" ] );
  }

  
  boost::filesystem::path project_path( this->project_->project_path_state_->get() );
  boost::filesystem::path data_path = project_path / "data";
  
  boost::filesystem::directory_iterator dir_end;
  for ( boost::filesystem::directory_iterator dir_itr( data_path ); dir_itr != dir_end; ++dir_itr )
  {
    boost::filesystem::path file_path = dir_itr->path();
    
    // Skip directories
    if ( !boost::filesystem::is_regular_file( file_path ) ) continue;
    // Skip non-nrrd files
    if ( Core::StringToLower( file_path.extension().string() ) != ".nrrd" ) continue;

    std::string file_name = file_path.stem().string();
    try
    {
      long long generation_number = boost::lexical_cast< long long >( file_name );
      // Since the results from database query are sorted, we can use binary search on it
      if ( generations.size() == 0 ||
        !std::binary_search( generations.begin(), generations.end(), generation_number ) )
      {
        boost::filesystem::remove( file_path );
      }
    }
    catch ( ... ) { /* Ignore any exceptions */ }
  }
}


void ProjectPrivate::set_last_saved_session_time_stamp()
{
  // We allow other threads to read this variable, hence we need to ensure it is accessed
  // thread-safe.
  // NOTE: As the state engine has a recursive lock when can lock it this sub function.
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  this->last_saved_session_time_stamp_ = boost::posix_time::second_clock::local_time();
}

bool ProjectPrivate::initialize_session_database()
{
  std::string sql_statements;

  // Create table for storing the database version
  sql_statements += "CREATE TABLE database_version "
    "(version INTEGER NOT NULL PRIMARY KEY);";

  // Create table for sessions
  sql_statements += "CREATE TABLE session "
    "(session_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "session_name TEXT NOT NULL, "
    "session_file TEXT, "
    "user_id TEXT NOT NULL, "
    "timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP);";

  // Create table for storing data files (generation numbers) used by each session
  sql_statements += "CREATE TABLE session_data "
    "(session_id INTEGER NOT NULL REFERENCES session(session_id) ON DELETE CASCADE, "
    "data_generation INTEGER NOT NULL, "
    "PRIMARY KEY (session_id, data_generation));";

  // Create indices for the session_data table
  sql_statements += "CREATE INDEX session_index ON session_data(session_id);";
  sql_statements += "CREATE INDEX data_index ON session_data(data_generation);";

  // Set the database version to 1
  sql_statements += "INSERT INTO database_version VALUES (1);";

  std::string error;
  if ( !this->session_database_.run_sql_script( sql_statements, error ) )
  {
    CORE_LOG_ERROR( "Failed to initialize the session database: " + error );
    return false;
  }

  return true;
}

bool ProjectPrivate::initialize_provenance_database()
{
  std::string sql_statements;

  // Create table for storing the database version
  sql_statements += "CREATE TABLE database_version "
    "(version INTEGER NOT NULL PRIMARY KEY);";

  // Create table for user names
  sql_statements += "CREATE TABLE user "
    "(user_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "user_name TEXT NOT NULL UNIQUE);";

  // Create table for action names
  sql_statements += "CREATE TABLE action "
    "(action_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "action_name TEXT NOT NULL UNIQUE);";

  // Create table for storing provenance steps
  sql_statements += "CREATE TABLE provenance_step "
    "(prov_step_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "action_id INTEGER NOT NULL REFERENCES action(action_id) ON DELETE CASCADE, "
    "action_params TEXT NOT NULL, "
    "timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, "
    "user_id INTEGER NOT NULL REFERENCES user(user_id) ON DELETE CASCADE);";

  // Create index on provenance_step(action_id)
  sql_statements += "CREATE INDEX action_id_index ON provenance_step(action_id);";

  // Create index on provenance_step(user_id)
  sql_statements += "CREATE INDEX user_id_index ON provenance_step(user_id);";

  // Create table for storing inputs of each provenance step
  sql_statements += "CREATE TABLE provenance_input "
    "(input_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "prov_id INTEGER NOT NULL);";

  // Create index on prov_step_id column of provenance_input
  sql_statements += "CREATE INDEX prov_input_index ON provenance_input(prov_step_id);";

  // Create table for storing outputs of each provenance step
  sql_statements += "CREATE TABLE provenance_output "
    "(output_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "prov_id INTEGER NOT NULL UNIQUE);";

  // Create index on prov_step_id column of provenance_output
  sql_statements += "CREATE INDEX prov_output_index ON provenance_output(prov_step_id);";

  // Create table for storing deleted provenance IDs of each provenance step
  sql_statements += "CREATE TABLE provenance_replaced "
    "(prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "prov_id INTEGER NOT NULL, "
    "PRIMARY KEY (prov_step_id, prov_id));";

  // Create index on prov_step_id column of provenance_replaced
  sql_statements += "CREATE INDEX prov_replaced_index ON provenance_replaced(prov_step_id);";

  // Create table for storing outputs of each provenance step
  sql_statements += "CREATE TABLE provenance_inputfiles_cache "
    "(prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "inputfiles_cache_id INTEGER NOT NULL PRIMARY KEY);";

  // Create index on prov_step_id column of provenance_output
  sql_statements += "CREATE INDEX prov_inputfiles_cache_index ON provenance_inputfiles_cache(prov_step_id);";

  // Set the database version to 1
  sql_statements += "INSERT INTO database_version VALUES (1);";

  std::string error;
  if ( !this->provenance_database_.run_sql_script( sql_statements, error ) )
  {
    CORE_LOG_ERROR( "Failed to initialize the provenance database: " + error );
    return false;
  }

  return true;
}

bool ProjectPrivate::clear_provenance_database()
{
  std::string sql_statements;

  // Delete records from table for storing the database version
  sql_statements += "DELETE FROM database_version;";

  // Delete records from table for user names
  sql_statements += "DELETE FROM user;";
    
  // Delete records from table for action names
  sql_statements += "DELETE FROM action;";

  // Delete records from table for storing provenance steps
  sql_statements += "DELETE FROM provenance_step;";

  // Delete records from table for storing inputs of each provenance step
  sql_statements += "DELETE FROM provenance_input;";
  
  // Delete records from table for storing outputs of each provenance step
  sql_statements += "DELETE FROM provenance_output;";

  // Delete records from table for storing deleted provenance IDs of each provenance step
  sql_statements += "DELETE FROM provenance_replaced;";

  // Delete records from table for storing outputs of each provenance step
  sql_statements += "DELETE FROM provenance_inputfiles_cache;";
  
  std::string error;
  if ( !this->provenance_database_.run_sql_script( sql_statements, error ) )
  {
    CORE_LOG_ERROR( "Failed to clear the provenance database: " + error );
    return false;
  }

  // Let the GUI know that provenance has changed
  this->project_->provenance_trail_signal_( ProvenanceTrailHandle() );

  return true;
}

bool ProjectPrivate::initialize_note_database()
{
  std::string sql_statements;

  // Create table for the database version
  sql_statements += "CREATE TABLE database_version "
    "(version INTEGER NOT NULL PRIMARY KEY);";

  // Create table for notes
  sql_statements += "CREATE TABLE note "
    "(note_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "note TEXT NOT NULL, "
    "user_id TEXT NOT NULL, "
    "timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP);";

  // Set the database version to 1
  sql_statements += "INSERT INTO database_version VALUES (1);";

  std::string error;
  if ( !this->note_database_.run_sql_script( sql_statements, error ) )
  {
    CORE_LOG_ERROR( "Failed to initialize the note database: " + error );
    return false;
  }

  return true;
}

SessionID ProjectPrivate::insert_session_into_database( const std::string& session_name, 
  const std::string& user_id, const std::string& timestamp )
{
  std::string sql_statement;
  if ( timestamp.empty() )
  {
    sql_statement = "INSERT INTO session (session_name, user_id) VALUES ('" +
      DatabaseManager::EscapeQuotes( session_name ) + "', '" + 
      DatabaseManager::EscapeQuotes( user_id ) + "');";
  }
  else
  {
    sql_statement = "INSERT INTO session (session_name, user_id, timestamp) VALUES ('" +
      DatabaseManager::EscapeQuotes( session_name ) + "', '" + 
      DatabaseManager::EscapeQuotes( user_id ) + "', '" + timestamp + "');";
  }

  std::string error;
  if( !this->session_database_.run_sql_statement( sql_statement, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }

  return this->session_database_.get_last_insert_rowid();
}

bool ProjectPrivate::delete_session_from_database( SessionID session_id )
{
  std::string sql_str = "DELETE FROM session WHERE session_id = " + 
    Core::ExportToString( session_id ) + ";";

  std::string error;
  if( !this->session_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

bool ProjectPrivate::query_provenance_trail( const std::vector< ProvenanceID >& prov_ids, 
                      ProvenanceTrail& provenance_trail )
{
  if ( prov_ids.size() == 0 ) return false;

  // Get all the provenance steps that lead to the provenance ID
  std::set< ProvenanceStepID > prov_steps;
  this->get_provenance_steps( prov_ids, prov_steps );

  // The length of provenance trail is the same as the number of steps
  provenance_trail.resize( prov_steps.size() );

  // String stream for extracting timestamp value
  std::stringstream ss;
  ss.imbue( std::locale( ss.getloc(), new boost::posix_time::time_input_facet( 
    "%Y-%m-%d %H:%M:%S" ) ) );
  ss.exceptions( std::ios_base::failbit );

  // Query detailed information for each provenance step.
  // NOTE: The provenance steps are already sorted in ascending order
  // because of the use of std::set.
  // NOTE: We start from the end of the trail, look for the steps that output
  // the provenance IDs of interest as we proceed.
  std::set< ProvenanceStepID >::const_reverse_iterator it = prov_steps.rbegin();
  std::set< ProvenanceStepID >::const_reverse_iterator it_end = prov_steps.rend();
  std::set< ProvenanceID > poi_set;
  poi_set.insert( prov_ids.begin(), prov_ids.end() );
  size_t index = prov_steps.size();
  while ( it != it_end )
  {
    ProvenanceStepHandle prov_step( new ProvenanceStep );
    ProvenanceStepID prov_step_id = *it++;

    // Query the action string of the provenance step
    std::string sql_str = "SELECT * FROM provenance_step WHERE "
      "prov_step_id = " + Core::ExportToString( prov_step_id ) + ";";
    ResultSet result_set;
    std::string error;
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
    if ( result_set.size() != 1 )
    {
      CORE_LOG_ERROR( "Provenance database is broken." );
      return false;
    }

    long long action_id, user_id;
    std::string action_name, action_params, user_name, timestamp_str;
    try
    {
      action_id = boost::any_cast< long long >( result_set[ 0 ][ "action_id" ] );
      action_params = boost::any_cast< std::string >( result_set[ 0 ][ "action_params" ] );
      user_id = boost::any_cast< long long >( ( result_set[ 0 ] )[ "user_id" ] );
      timestamp_str = boost::any_cast< std::string >( ( result_set[ 0 ] )[ "timestamp" ] );
    }
    catch ( ... )
    {
      CORE_LOG_ERROR( "Invalid provenance database." );
      return false;
    }

    if ( !this->get_action_name( action_id, action_name ) ||
      !this->get_user_name( user_id, user_name ) )
    {
      return false;
    }
    
    ProvenanceStep::timestamp_type timestamp;
    ss.str( timestamp_str );
    try
    {
      ss >> timestamp;
    }
    catch ( ... )
    {
      timestamp = boost::posix_time::second_clock::universal_time();
    }

    prov_step->set_action_name( action_name );
    prov_step->set_action_params( action_params );
    prov_step->set_username( user_name );
    prov_step->set_timestamp( timestamp );

    // Query outputs for the provenance step
    // We also figure out what output IDs we are interested in for this step
    sql_str = "SELECT prov_id FROM provenance_output WHERE prov_step_id = " +
      Core::ExportToString( prov_step_id ) + " ORDER BY output_id ASC;";
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
    ProvenanceIDList output_prov_ids;
    ProvenanceIDList prov_ids_of_interest;
    for ( size_t i = 0; i < result_set.size(); ++i )
    {
      output_prov_ids.push_back( boost::any_cast< ProvenanceID >( result_set[ i ][ "prov_id" ] ) );
      if ( poi_set.find( output_prov_ids[ i ] ) != poi_set.end() )
      {
        prov_ids_of_interest.push_back( output_prov_ids[ i ] );
        poi_set.erase( output_prov_ids[ i ] );
      }
    }
    prov_step->set_output_provenance_ids( output_prov_ids );
    prov_step->set_provenance_ids_of_interest( prov_ids_of_interest );


    // Query inputs for the provenance step.
    // Update the poi_set in the mean time.
    sql_str = "SELECT prov_id FROM provenance_input WHERE prov_step_id = " +
      Core::ExportToString( prov_step_id ) + " ORDER BY input_id ASC;";
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
    ProvenanceIDList input_prov_ids;
    for ( size_t i = 0; i < result_set.size(); ++i )
    {
      input_prov_ids.push_back( boost::any_cast< ProvenanceID >( result_set[ i ][ "prov_id" ] ) );
      poi_set.insert( input_prov_ids[ i ] );
    }
    prov_step->set_input_provenance_ids( input_prov_ids );


    // Query replaced provenance IDs for the provenance step
    sql_str = "SELECT prov_id FROM provenance_replaced WHERE prov_step_id = " +
      Core::ExportToString( prov_step_id ) + " ORDER BY rowid ASC;";
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
    ProvenanceIDList replaced_prov_ids;
    for ( size_t i = 0; i < result_set.size(); ++i )
    {
      replaced_prov_ids.push_back( boost::any_cast< ProvenanceID >( result_set[ i ][ "prov_id" ] ) );
    }
    prov_step->set_replaced_provenance_ids( replaced_prov_ids );

    provenance_trail[ --index ] = prov_step;
  }

  return true;
}

void ProjectPrivate::extract_session_info( ResultSet& result_set, std::vector< SessionInfo >& sessions )
{
  std::stringstream ss;
  ss.imbue( std::locale( ss.getloc(), new boost::posix_time::time_input_facet( 
    "%Y-%m-%d %H:%M:%S" ) ) );
  ss.exceptions( std::ios_base::failbit );

  for( size_t i = 0; i < result_set.size(); ++i )
  {
    SessionID session_id = boost::any_cast< long long >( ( result_set[ i ] )[ "session_id" ] );
    std::string session_name = boost::any_cast< std::string >( ( result_set[ i ] )[ "session_name" ] );
    std::string user_id = boost::any_cast< std::string >( ( result_set[ i ] )[ "user_id" ] );
    std::string timestamp_str = boost::any_cast< std::string >( ( result_set[ i ] )[ "timestamp" ] );
    SessionInfo::timestamp_type timestamp;
    ss.str( timestamp_str );
    try
    {
      ss >> timestamp;
    }
    catch ( ... )
    {
      timestamp = boost::posix_time::second_clock::universal_time();
    }
    sessions.push_back( SessionInfo( session_id, session_name, user_id, timestamp ) );
  }
}

bool ProjectPrivate::get_all_sessions( std::vector< SessionInfo >& sessions )
{
  ASSERT_IS_APPLICATION_THREAD();

  std::string error;

  ResultSet result_set;
  std::string select_statement = "SELECT * FROM session ORDER BY session_id DESC;";
  if( !this->session_database_.run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  this->extract_session_info( result_set, sessions );
  return true;
}

long long ProjectPrivate::insert_note_into_database( const std::string& note, 
  const std::string& user_id, const std::string& timestamp )
{
  std::string sql_statement;
  if ( timestamp.empty() )
  {
    sql_statement = "INSERT INTO note (note, user_id) VALUES ('" +
      DatabaseManager::EscapeQuotes( note ) + "', '" + 
      DatabaseManager::EscapeQuotes( user_id ) + "');";
  }
  else
  {
    sql_statement = "INSERT INTO note (note, user_id, timestamp) VALUES ('" +
      DatabaseManager::EscapeQuotes( note ) + "', '" + 
      DatabaseManager::EscapeQuotes( user_id ) + "', '" + timestamp + "');";
  }

  std::string error;
  if( !this->note_database_.run_sql_statement( sql_statement, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }

  return this->note_database_.get_last_insert_rowid();
}

void ProjectPrivate::convert_version1_project()
{
  this->initialize_session_database();
  this->initialize_provenance_database();
  this->initialize_note_database();

  // ==== Convert sessions ====

  // The session names were stored in the session_state variable.
  // Hence we grab them all
  std::vector< std::string > sessions = this->project_->sessions_state_->get();

  boost::filesystem::path project_path = this->project_->project_path_state_->get();

  // I/O stream for timestamp conversion
  std::stringstream ss;
  ss.imbue( std::locale( std::locale::classic(), new boost::posix_time::time_facet( 
    "%Y-%m-%d %H:%M:%S" ) ) );
  ss.imbue( std::locale( ss.getloc(), new boost::posix_time::time_input_facet( 
    "%d-%b-%Y-%H-%M-%S" ) ) );
  ss.exceptions( std::ios_base::failbit );

  // Time offset between local time and universal time
  // NOTE: This assumes that the time zone and DST setting when the session was saved
  // is the same as the current machine setting, which isn't always true. But this is the
  // best guess I can do since the old timestamp didn't ecode local time information.
  boost::posix_time::time_duration time_diff = boost::posix_time::second_clock::local_time() - 
    boost::posix_time::second_clock::universal_time();

  int number_of_sessions = static_cast< int >( sessions.size() );
  long long generation_count = -1;
  for( int i = ( number_of_sessions - 1 ); i >= 0; --i )
  {
    // Get the old session name
    std::string old_session_name;
    Core::ImportFromString( sessions[ i ], old_session_name );

    std::vector< std::string > session_info_strs = Core::SplitString( old_session_name, " - " );

    // Ensure it is a valid name  
    if ( session_info_strs.size() < 3 ) continue;

    boost::filesystem::path old_path = project_path / SESSION_DIR_C / ( old_session_name + ".xml" );

    // If the old one does not exist, just continue - we should not fail -
    if( ! boost::filesystem::exists( old_path ) ) continue; 

    // Convert the timestamp to universal time
    ss.str( session_info_strs[ 0 ] );
    boost::posix_time::ptime session_time( boost::date_time::not_a_date_time );
    try
    {
      ss >> session_time;
      session_time -= time_diff;
    }
    catch( ... )
    {
      // If the timestamp wasn't correct, use the current time as the timestamp
      session_time = boost::posix_time::second_clock::universal_time();
    }
    ss.str( "" );
    ss << session_time;

    // Parse out data generation numbers from the session file
    std::set< long long > generations;
    if ( !this->parse_session_data( old_path, generations ) ) continue;

    if ( !generations.empty() )
    {
      generation_count = Core::Max( generation_count, 
        *( std::max_element( generations.begin(), generations.end() ) ) );
    }

    SessionID session_id = this->insert_session_into_database( session_info_strs[ 1 ], session_info_strs[ 2 ], ss.str() );
    if ( session_id < 0 )
    {
      CORE_LOG_ERROR( "Error happened when converting old project data." );
      continue;
    }

    this->set_session_file( session_id, old_session_name + ".xml" );

    this->set_session_data( session_id, generations );
  }

  // Finally we set the session list to an empty vector so we aren't tempted to do this again.
  // This variable remains empty and unused from this point onward. 
  this->project_->sessions_state_->clear();

  // Set the generation count
  Core::DataBlockManager::Instance()->set_generation_count( generation_count + 1 );

  // Need to overwrite the generation_count_state, otherwise at the end of session load
  // it will be set back to zero if there was no generation_count_state in the old project
  this->project_->generation_count_state_->set( generation_count + 1);
     
  // ==== Convert notes ====

  const std::vector< std::string >& notes = this->project_->project_notes_state_->get();
  int number_of_notes = static_cast< int >( notes.size() );
  for ( int i = 0 ; i < number_of_notes; ++i )
  {
    std::vector< std::string > note_components = Core::SplitString( notes[ i ], "|" );
    if ( note_components.size() < 2 ) continue;

    std::vector< std::string > note_info = Core::SplitString( note_components[ 0 ], " - " );
    if ( note_info.size() < 2 ) continue;
    
    boost::posix_time::ptime note_time( boost::posix_time::not_a_date_time );
    ss.str( note_info[ 0 ] );
    try
    {
      ss >> note_time;
      note_time -= time_diff;
    }
    catch( ... )
    {
      // If the timestamp wasn't correct, use the current time as the timestamp
      note_time = boost::posix_time::second_clock::universal_time();
    }

    // Convert the adjusted timestamp back to string
    ss.str( "" );
    ss << note_time;

    this->insert_note_into_database( note_components[ 1 ], note_info[ 1 ], ss.str() );
  }

  // Clear the old state variable for notes
  this->project_->project_notes_state_->clear();
}

bool ProjectPrivate::rename_version1_session_files()
{
  std::string sql_str = "SELECT session_id, session_file FROM session;";
  std::string error;
  ResultSet results;
  if ( !this->session_database_.run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  boost::filesystem::path sessions_path = this->project_->get_project_sessions_path();
  
  for ( size_t i = 0; i < results.size(); ++i )
  {
    std::string session_file_name;
    try
    {
      session_file_name = boost::any_cast< std::string >( results[ i ][ "session_file" ] );
    }
    catch ( ... ) 
    {
      continue; 
    }

    SessionID session_id = boost::any_cast< SessionID >( results[ i ][ "session_id" ] );
    boost::filesystem::path old_session_file = sessions_path / session_file_name;
    boost::filesystem::path new_session_file = sessions_path / 
      ( Core::ExportToString( session_id ) + ".xml" );

    try
    {
      boost::filesystem::rename( old_session_file, new_session_file );
    }
    catch ( ... ) 
    {
      continue;
    }

    sql_str = "UPDATE session SET session_file = NULL WHERE session_id = " +
      Core::ExportToString( session_id ) + ";";
    if ( !this->session_database_.run_sql_statement( sql_str, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
  }

  return true;
}

bool ProjectPrivate::parse_session_data( const boost::filesystem::path& file_path, std::set< long long >& generations )
{
  Core::StateIO state_io;
  if ( !state_io.import_from_file( file_path ) ) return false;

  TiXmlElement* root_element = state_io.get_current_element();
  TiXmlElement* lm_element = root_element->FirstChildElement( "layermanager" );
  if ( lm_element == 0 ) return false;

  TiXmlElement* groups_element = lm_element->FirstChildElement( "groups" );
  if ( groups_element == 0 ) return true;

  TiXmlElement* group_element = groups_element->FirstChildElement();
  while ( group_element != 0 )
  {
    TiXmlElement* layers_element = group_element->FirstChildElement( "layers" );
    if ( layers_element == 0 ) return false;

    TiXmlElement* layer_element = layers_element->FirstChildElement();
    while ( layer_element != 0 )
    {
      TiXmlElement* state_element = layer_element->FirstChildElement( "State" );
      while ( state_element != 0 )
      {
        const char* state_id = state_element->Attribute( "id" );
        if ( state_id == 0 ) return false;
        if ( strcmp( state_id, "generation" ) == 0 )
        {
          const char* generation_str = state_element->GetText();
          if ( generation_str == 0 || strlen( generation_str ) == 0 ) return false;
          long long generation;
          Core::ImportFromString( generation_str, generation );
          generations.insert( generation );
          break;
        }
        state_element = state_element->NextSiblingElement( "State" );
      } // end while ( state_element != 0 )

      layer_element = layer_element->NextSiblingElement();
    } // end while ( layer_element != 0 )

    group_element = group_element->NextSiblingElement();
  } // end while ( group_element != 0 )

  return true;
}

bool ProjectPrivate::parse_session_provenance_ids( const boost::filesystem::path& file_path, 
                          std::vector< long long >& prov_ids )
{
  Core::StateIO state_io;
  if ( !state_io.import_from_file( file_path ) ) return false;

  TiXmlElement* root_element = state_io.get_current_element();
  TiXmlElement* lm_element = root_element->FirstChildElement( "layermanager" );
  if ( lm_element == 0 ) return false;

  TiXmlElement* groups_element = lm_element->FirstChildElement( "groups" );
  if ( groups_element == 0 ) return true;

  TiXmlElement* group_element = groups_element->FirstChildElement();
  while ( group_element != 0 )
  {
    TiXmlElement* layers_element = group_element->FirstChildElement( "layers" );
    if ( layers_element == 0 ) return false;

    TiXmlElement* layer_element = layers_element->FirstChildElement();
    while ( layer_element != 0 )
    {
      TiXmlElement* state_element = layer_element->FirstChildElement( "State" );
      while ( state_element != 0 )
      {
        const char* state_id = state_element->Attribute( "id" );
        if ( state_id == 0 ) return false;
        if ( strcmp( state_id, "provenance_id" ) == 0 )
        {
          const char* prov_id_str = state_element->GetText();
          if ( prov_id_str == 0 || strlen( prov_id_str ) == 0 ) return false;
          long long prov_id = -1;
          if ( !Core::ImportFromString( prov_id_str, prov_id ) || prov_id < 0 )
          {
            return false;
          }
          prov_ids.push_back( prov_id );
          break;
        }
        state_element = state_element->NextSiblingElement( "State" );
      } // end while ( state_element != 0 )

      layer_element = layer_element->NextSiblingElement();
    } // end while ( layer_element != 0 )

    group_element = group_element->NextSiblingElement();
  } // end while ( group_element != 0 )

  return true;
}

void ProjectPrivate::set_session_file( SessionID id, const std::string& file_name )
{
  std::string sql_str = "UPDATE session SET session_file = '" + 
    DatabaseManager::EscapeQuotes( file_name ) +
    "' WHERE session_id = " + Core::ExportToString( id ) + ";";
  std::string err;
  if ( !this->session_database_.run_sql_statement( sql_str, err ) )
  {
    CORE_LOG_ERROR( err );
  }
}

bool ProjectPrivate::set_session_data( SessionID id, const std::set< long long >& generations )
{
  std::set< long long >::const_iterator it = generations.begin();
  std::string sql_str;
  while ( it != generations.end() )
  {
    sql_str += "INSERT INTO session_data VALUES (" + Core::ExportToString(  id ) +
      ", " + Core::ExportToString( *it ) + ");";
    ++it;
  }

  std::string err;
  if ( !this->session_database_.run_sql_script( sql_str, err ) )
  {
    CORE_LOG_ERROR( err );
    return false;
  }

  return true;
}

SessionID ProjectPrivate::get_last_session()
{
  std::string sql_str = "SELECT session_id FROM session ORDER BY session_id DESC LIMIT 1;";
  ResultSet result;
  std::string error;
  if ( !this->session_database_.run_sql_statement( sql_str, result, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }

  if ( result.size() == 0 )
  {
    return -1;
  }

  boost::any value = result[ 0 ][ "session_id" ];
  try
  {
    SessionID id = boost::any_cast< long long >( value );
    return id;
  }
  catch ( ... ) {}

  return -1;
}


bool ProjectPrivate::process_inputfile_importers()
{
  if ( this->project_->project_files_generated_state_->get() == false )
  {
    // Project has not yet been created, so we cannot save it
    return false;
  }

  // Get the directory in which we need to store the input files.
  boost::filesystem::path inputfiles_dir = this->project_->get_project_inputfiles_path(); 
  if ( ! boost::filesystem::exists( inputfiles_dir ) )
  {
    // Directory does not exist
    return false;
  } 
  
  // TODO: Keep the records that could not be written
  for ( size_t j = 0; j < this->input_file_importers_.size();  j++ )
  {
    InputFilesImporterHandle importer = this->input_file_importers_[ j ];
    InputFilesID inputfiles_id = importer->get_inputfiles_id();
    
    boost::filesystem::path import_dir = inputfiles_dir / Core::ExportToString( inputfiles_id );
    if ( boost::filesystem::exists( import_dir ) )
    {
      try
      {
        boost::filesystem::remove_all( import_dir );
      }
      catch ( ... )
      {
        CORE_LOG_ERROR( std::string( "Could not remove directory '" ) + import_dir.string() +
          "'." );
      }
    }

    if ( ! boost::filesystem::exists( import_dir ) )
    {
      try
      {
        boost::filesystem::create_directory( import_dir );
      }
      catch ( ... )
      {
        CORE_LOG_ERROR( std::string( "Could not create directory '" ) + import_dir.string() +
          "'." );     
      }
    }
    
    if ( boost::filesystem::exists( import_dir ) )
    {
      if ( !importer->copy_files( import_dir ) )
      {
        CORE_LOG_ERROR( std::string( "Could not import original files into directory '" ) + 
          import_dir.string() + "'." );
      }
    }
  }
  
  // Clear the list with importers
  this->input_file_importers_.clear();
  
  return true;
}

bool ProjectPrivate::get_all_notes( ProjectNoteList& notes )
{
  ASSERT_IS_APPLICATION_THREAD();

  std::string error;

  ResultSet result_set;
  std::string sql_str = "SELECT * FROM note;";
  if( !this->note_database_.run_sql_statement( sql_str, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  std::stringstream ss;
  ss.imbue( std::locale( ss.getloc(), new boost::posix_time::time_input_facet( 
    "%Y-%m-%d %H:%M:%S" ) ) );
  ss.exceptions( std::ios_base::failbit );

  for( size_t i = 0; i < result_set.size(); ++i )
  {
    std::string note_text = boost::any_cast< std::string >( ( result_set[ i ] )[ "note" ] );
    std::string user_id = boost::any_cast< std::string >( ( result_set[ i ] )[ "user_id" ] );
    std::string timestamp_str = boost::any_cast< std::string >( ( result_set[ i ] )[ "timestamp" ] );
    ProjectNote::timestamp_type timestamp;
    ss.str( timestamp_str );
    try
    {
      ss >> timestamp;
    }
    catch ( ... )
    {
      timestamp = boost::posix_time::second_clock::universal_time();
    }
    notes.push_back( ProjectNote( note_text, user_id, timestamp ) );
  }

  return true;
}

bool ProjectPrivate::get_session_file( SessionID session_id, boost::filesystem::path& file_path, std::string& error )
{
  // Query session information from database
  std::string sql_str = "SELECT session_file FROM session WHERE session_id = " + 
    Core::ExportToString( session_id ) + ";";
  ResultSet result_set;
  if ( !this->session_database_.run_sql_statement( sql_str, result_set, error ) ||
    result_set.size() == 0 )
  {
    error = "Session " + Core::ExportToString( session_id ) + " doesn't exist.";
    return false;
  }
  assert( result_set.size() == 1 );

  // Get the project directory path
  boost::filesystem::path project_path( this->project_->project_path_state_->get() );

  // Get the session XML file
  boost::filesystem::path session_file = project_path / SESSION_DIR_C / 
    ( Core::ExportToString( session_id ) + ".xml" );
  if ( !boost::filesystem::exists( session_file ) )
  {
    try
    {
      std::string session_file_str = boost::any_cast< std::string >( result_set[ 0 ][ "session_file" ] );
      session_file = project_path / SESSION_DIR_C / session_file_str;
    }
    catch ( ... ) {}
  }

  if ( !boost::filesystem::exists( session_file ) )
  {
    error = "Missing session file for session " + Core::ExportToString( session_id ) + ".";
    return false;
  }

  file_path = session_file;
  return true;
}

void ProjectPrivate::get_provenance_steps( const std::vector< ProvenanceID >& prov_ids, 
                      std::set< ProvenanceStepID >& prov_steps )
{
  typedef std::queue< ProvenanceID > prov_id_queue_type;
  void ( prov_id_queue_type::*push_queue_func )( const ProvenanceID& ) = &prov_id_queue_type::push;
  prov_id_queue_type provenance_queue;
  std::for_each( prov_ids.begin(), prov_ids.end(), boost::bind( 
    push_queue_func, &provenance_queue, _1 ) );

  while ( !provenance_queue.empty() )
  {
    ProvenanceID output_id = provenance_queue.front();
    provenance_queue.pop();
    if ( output_id == -1 ) continue;

    std::string error;
    std::string sql_str;
    ResultSet result_set;

    // Query the provenance step that generated the output_id
    sql_str = "SELECT prov_step_id FROM provenance_output WHERE prov_id = " +
      Core::ExportToString( output_id ) + ";";
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return;
    }
    if ( result_set.size() == 0 ) continue;

    ProvenanceStepID prov_step_id = boost::any_cast< long long >( result_set[ 0 ][ "prov_step_id" ] );
    prov_steps.insert( prov_step_id );

    // Query the inputs of the provenance step
    sql_str = "SELECT prov_id FROM provenance_input WHERE prov_step_id = " +
      Core::ExportToString( prov_step_id ) + ";";
    result_set.clear();
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return;
    }
    for ( size_t i = 0; i < result_set.size(); ++i )
    {
      ProvenanceID input_id = boost::any_cast< long long >( result_set[ i ][ "prov_id" ] );
      provenance_queue.push( input_id );
    }
  }
}

void ProjectPrivate::set_project_changed( Core::ActionHandle action, Core::ActionResultHandle result )
{
  // NOTE: This is executed on the application thread, hence we do not need a lock to read
  // the variable that is only changed on the same thread

  if ( action->changes_project_data() )
  {
    // Trigger the project_data_changed_signal_ whenever there is a new change
    this->project_->project_data_changed_signal_();

    // Set the changed flag if it's not been set yet
    if ( !this->changed_ )
    {
      // NOTE: Changing the variable
      Core::Application::lock_type lock( Core::Application::GetMutex() );
      this->changed_ = true;
    } 
  }
}

long long ProjectPrivate::get_user_id( const std::string& user_name )
{
  std::string sql_str = "SELECT user_id FROM user WHERE user_name = '" + 
    DatabaseManager::EscapeQuotes( user_name ) + "';";
  std::string error;
  ResultSet results;
  if ( !this->provenance_database_.run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }

  if ( results.size() > 0 )
  {
    return boost::any_cast< long long >( results[ 0 ][ "user_id" ] );
  }
  
  sql_str = "INSERT INTO user (user_name) VALUES('" + 
    DatabaseManager::EscapeQuotes( user_name ) + "');";
  if ( !this->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }
  
  return this->provenance_database_.get_last_insert_rowid();
}

long long ProjectPrivate::get_action_id( const std::string& action_name )
{
  std::string sql_str = "SELECT action_id FROM action WHERE action_name = '" + action_name + "';";
  std::string error;
  ResultSet results;
  if ( !this->provenance_database_.run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }

  if ( results.size() > 0 )
  {
    return boost::any_cast< long long >( results[ 0 ][ "action_id" ] );
  }

  sql_str = "INSERT INTO action (action_name) VALUES('" + action_name + "');";
  if ( !this->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }

  return this->provenance_database_.get_last_insert_rowid();
}

bool ProjectPrivate::get_user_name( long long user_id, std::string& user_name )
{
  std::map< long long, std::string >::iterator it = this->user_name_map_.find( user_id );
  if ( it != this->user_name_map_.end() )
  {
    user_name = it->second;
    return true;
  }
  
  std::string sql_str = "SELECT user_name FROM user WHERE user_id = " +
    Core::ExportToString( user_id ) + ";";
  ResultSet results;
  std::string error;
  if ( !this->provenance_database_.run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  if ( results.size() == 0 )
  {
    CORE_LOG_ERROR( "Invalid provenance database." );
    return false;
  }
  
  try
  {
    user_name = boost::any_cast< std::string >( results[ 0 ][ "user_name" ] );
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "Invalid provenance database." );
    return false;
  }

  return true;
}

bool ProjectPrivate::get_action_name( long long action_id, std::string& action_name )
{
  std::map< long long, std::string >::iterator it = this->action_name_map_.find( action_id );
  if ( it != this->action_name_map_.end() )
  {
    action_name = it->second;
    return true;
  }

  std::string sql_str = "SELECT action_name FROM action WHERE action_id = " +
    Core::ExportToString( action_id ) + ";";
  ResultSet results;
  std::string error;
  if ( !this->provenance_database_.run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( results.size() == 0 )
  {
    CORE_LOG_ERROR( "Invalid provenance database." );
    return false;
  }

  try
  {
    action_name = boost::any_cast< std::string >( results[ 0 ][ "action_name" ] );
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "Invalid provenance database." );
    return false;
  }

  return true;
}

//////////////////////////////////////////////////////////

Project::Project( const std::string& project_name ) :
  StateHandler( "project", true ),
  private_( new ProjectPrivate )
{
  this->initialize();
  this->set_initializing( true );
  this->project_name_state_->set( project_name );
  this->project_file_state_->set( project_name + Project::GetDefaultProjectFileExtension() );
  this->private_->initialize_session_database();
  this->private_->initialize_provenance_database();
  this->private_->initialize_note_database();
  SetProvenanceCount( 0 );
  this->set_initializing( false );
}

Project::Project( const boost::filesystem::path& project_file ) :
  StateHandler( "project", true ),
  private_( new ProjectPrivate )
{
  this->initialize();
  this->set_initializing( true );
  if ( !this->load_project( project_file ) )
  {
    CORE_THROW_INVALIDARGUMENT( "Invalid project file." );
  }
  this->set_initializing( false );
}

Project::~Project()
{
  // Remove all active connections
  this->disconnect_all();
}

void Project::initialize()
{
  // Mark this class so it does not add the state id number into the project file itself
  this->do_not_save_id_number();

  // Keep a pointer to this class in the private class
  this->private_->project_ = this;

  this->private_->conversion_needed_ = false;

  // Name of the project
  this->add_state( "project_name", this->project_name_state_, "" );

  // File that contains project information
  this->add_state( "project_file", this->project_file_state_, "" );

  // Path of the project
  this->add_state( "project_path", this->project_path_state_, "" );

  // Whether project files have been saved to disk, if not the first time a save is called
  // it needs to ask for a location to save the project
  this->add_state( "project_files_generated", this->project_files_generated_state_, false );

  // Whether project files have been saved to disk and are accessible
  this->add_state( "project_files_accessible", this->project_files_accessible_state_, false );

  // Running count of how much the data files consume. It is not exact it is an approximation.
  this->add_state( "project_size", this->project_size_state_, 0 );

  // These three properties do not need to be saved into the file:
  // They are dynamically generated, and hence have no place in the xml file
  this->project_name_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->project_file_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->project_path_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->project_files_generated_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->project_files_accessible_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );
  this->project_size_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  // Whether colors are saved in the project
  this->add_state( "save_custom_colors", this->save_custom_colors_state_, false );

  // State variable with the project notes
  this->add_state( "project_notes", this->project_notes_state_ );

  // Name of the next session in the session menu. For each session it will just be set to the
  // last session name. It is a state variable however so we can handle all the name handling
  // automatically.
  this->add_state( "current_session_name", this->current_session_name_state_, "New Session" );

  // State variable for backwards compatibility, not used anymore
  this->add_state( "sessions", this->sessions_state_ );

  // State of all the 12 colors in the system.
  // Initialize them with the colors from the preference manager
  this->color_states_.resize( 12 );
  for ( size_t j = 0; j < 12; j++ )
  {
    // Initialize the colors with the default colors from the preference manager
    std::string stateid = std::string( "color_" ) + Core::ExportToString( j );
    this->add_state( stateid, this->color_states_[ j ], 
      PreferencesManager::Instance()->get_default_colors()[ j ] );
  }

  // Count that keeps track of the unique ids for each input file
  this->add_state( "inputfiles_count", this->inputfiles_count_state_, 0 );

  // Count that keeps track of datablock generation numbers
  this->add_state( "generation_count", this->generation_count_state_, 0 );

  // Count that keeps track of maximum provenance ID
  this->add_state( "provenance_count", this->provenance_count_state_, 0 );

  // Each time an action is executed, check whether it changes the project data, if so
  // mark this in the project, so the UI can query the user for a save action if the application
  // is closed while there is unsaved data.
  this->add_connection( Core::ActionDispatcher::Instance()->post_action_signal_.connect( 
    boost::bind( &ProjectPrivate::set_project_changed, this->private_, _1, _2 ) ) );
}

bool Project::load_project( const boost::filesystem::path& project_file )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();
  
  // Ensure the filename is an actual file that exists
  boost::filesystem::path full_filename;
  
  try
  {
    full_filename = boost::filesystem::absolute( project_file ); 

    // Ensure the given file actually exists
    if ( ! boost::filesystem::exists( full_filename ) )
    {
      std::string error = std::string( "File '" ) + project_file.string() +
        "' does not exist.";
      CORE_LOG_ERROR( error );      
      return false;
    }
    
    // Ensure it is a regular file and not a directory or a symbolic link
    if ( ! boost::filesystem::is_regular_file( full_filename ) )
    {
      std::string error = std::string( "File '" ) + project_file.string() +
        "' is not a regular file.";
      CORE_LOG_ERROR( error );      
      return false;
    }
  }
  catch( ... )
  {
    std::string error = std::string( "Could not resolve filename '" ) + project_file.string() +
      "'.";
    CORE_LOG_ERROR( error );
    return false;
  }
  
  // Load the information from the file
  Core::StateIO stateio;
  if ( !stateio.import_from_file( project_file ) )
  {
    // We could not load the file, hence send a message to the user
    std::string error = std::string( "Could not open project file '" ) + project_file.string() + "'.";
    CORE_LOG_ERROR( error );
    return false; 
  }
  
  if ( !this->load_states( stateio ) )  
  {
    // We could not load the file, hence send a message to the user
    std::string error = std::string( "Could not read project file '" ) + project_file.string() +  "'.";
    CORE_LOG_ERROR( error );
    return false; 
  }
  // First update the state variables that were not saved, such as
  // project name and project path
  
  bool is_bundle = false;

#ifdef __APPLE__

  // MAC CODE
  try
  {
    FSRef file_ref;
    Boolean is_directory;
    FSPathMakeRef( reinterpret_cast<const unsigned char*>( 
      full_filename.parent_path().string().c_str() ), &file_ref, &is_directory );
    FSCatalogInfo info;
    FSGetCatalogInfo( &file_ref, kFSCatInfoFinderInfo, &info, 0, 0, 0 );
    
    FileInfo&  finder_info = *reinterpret_cast<FileInfo*>( &info.finderInfo );
    if ( finder_info.finderFlags & kHasBundle ) is_bundle = true;
  }
  catch( ... )
  {
  }

#endif  
  
  if ( is_bundle )
  {
    if ( full_filename.parent_path().stem().string() != full_filename.stem().string() )
    {
      boost::filesystem::path new_project_file = full_filename.parent_path() /
        ( full_filename.parent_path().stem().string() + Project::GetDefaultProjectFileExtension() );
    
      try
      {
        boost::filesystem::rename( full_filename, new_project_file );
        full_filename = new_project_file;
      }
      catch ( ... )
      {
      }
    }
  }

  // Grab the name from the .s3d file. This name is currently the only
  // indicator of the project name. Hence if the user renames this file, the
  // project name will change as well.
  this->project_name_state_->set( full_filename.stem().string() );

  // The file in which the project was saved
  this->project_file_state_->set( full_filename.filename().string() );

  // Ensure all directories that we require are present, if not this function
  // will generate them.
  //if ( ! this->private_->update_project_directory( full_filename.parent_path() ) )
  //{
  //  return false;
  //}
  
  this->project_path_state_->set( full_filename.parent_path().string() );

  // Since we load this one from disk, it obviously exists on disk
  this->project_files_generated_state_->set( true );
  this->project_files_accessible_state_->set( true );

  // If the project was created by an old version, convert it
  if ( this->get_loaded_version() == 1 )
  {
    this->private_->conversion_needed_ = true;
    this->private_->convert_version1_project();
  }
  // Otherwise load the database
  else
  {
    boost::filesystem::path session_db_file = full_filename.parent_path() /
      DATABASE_DIR_C / SESSION_DATABASE_C;
    std::string error;
    // If the session database doesn't exist or it's invalid, create an empty one
    if ( !boost::filesystem::exists( session_db_file ) ||
      !this->private_->session_database_.load_database( session_db_file, error ) )
    {
      this->private_->initialize_session_database();
    }

    boost::filesystem::path provenance_db_file = full_filename.parent_path() /
      DATABASE_DIR_C / PROVENANCE_DATABASE_C;
    if ( !boost::filesystem::exists( provenance_db_file ) ||
      !this->private_->provenance_database_.load_database( provenance_db_file, error ) )
    {
      this->private_->initialize_provenance_database();
    }

    boost::filesystem::path note_db_file = full_filename.parent_path() / 
      DATABASE_DIR_C / NOTE_DATABASE_C;
    if ( !boost::filesystem::exists( note_db_file ) || 
      !this->private_->note_database_.load_database( note_db_file, error ) )
    {
      this->private_->initialize_note_database();
    }
  }

  // Delete session records from the database that don't have corresponding session files
  this->private_->clean_up_session_database();

  return true;
}

bool Project::save_project( const boost::filesystem::path& project_path, 
  const std::string& project_name, bool anonymize )
{
  // This function sets state variables directly, hence we need to be on the application thread.
  ASSERT_IS_APPLICATION_THREAD();
  
  // Ensure that we have the full path
  boost::filesystem::path full_path;
  try
  {
    full_path = boost::filesystem::absolute( project_path );
  }
  catch ( ... )
  {
    std::string error = std::string( "Could not resolve directory name '" ) + 
      project_path.string() + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }
  
  // Directory may not exist, but if it exists it needs to be directory
  if ( boost::filesystem::exists( project_path ) && 
     !boost::filesystem::is_directory( project_path ) )
  {
    std::string error = std::string( "'" ) + project_path.string() + "' is not a directory.";
    CORE_LOG_ERROR( error );
    return false;
  }
  
  // Remove patient-specific data if project should be anonymized
  if( anonymize )
  {
    // Remove inputfiles directory from new project in case user overwrites existing project
    boost::filesystem::path inputfiles_dir = project_path / INPUTFILES_DIR_C;
    try
    {
      boost::filesystem::remove_all( inputfiles_dir );
    }
    catch ( ... ) 
    {
      CORE_LOG_ERROR( "Couldn't remove directory '" + inputfiles_dir.string() + "' for anonymization." );
      return false;
    }

    // Clear provenance database in memory
    if( !this->private_->clear_provenance_database() )
    {
      CORE_LOG_ERROR( "Failed to clear provenance database for anonymization." );
      return false;
    }
  }

  // NOTE: This function has several modes:
  // (1) If the project has not been saved yet, the project will be saved to the location 
  // indicated and a session will be saved, as a session is needed to load a project
  // (2) If the project exists and the name is the same, we only save a session
  // (3) If the project is in the same directory but the project name is changed, we save a new
  // session and rename the project file
  // (4) If the project is directed towards a new directory all files are copied and the project
  // is saved in the new location. Afterwards the project refers to that new project.
  
  if ( this->project_files_generated_state_->get() == false ||
    this->project_files_accessible_state_->get() == false )
  {   
    // Set the new project name.
    this->project_name_state_->set( project_name );
    this->project_file_state_->set( project_name + Project::GetDefaultProjectFileExtension() );

    // Set the new directory in which this project is saved.
    this->project_path_state_->set( project_path.string() );

    // Make sure that the program knows that the current project now lives on disk. 
    this->project_files_generated_state_->set( true );

    // Save the current session so there is something to load when the project is opened.
    return this->save_session( AUTO_SESSION_NAME_C );
  }
  else 
  {
    boost::filesystem::path current_project_path( this->project_path_state_->get() );
    current_project_path = boost::filesystem::absolute( current_project_path );
    std::string current_project_name = this->project_name_state_->get();
    std::string current_project_file = this->project_file_state_->get();
    
    if ( current_project_path == project_path && current_project_name == project_name )
    {
      // All the user wanted is to save a session, as it is the same name and directory
      return this->save_session( AUTO_SESSION_NAME_C );
    }

    if ( current_project_path == project_path )
    {
      // OK, the user gave us a new name, but directory is the same
      // Update the project information
      this->project_name_state_->set( project_name ); 
      
      // Save out a session with the current information
      bool succeeded = this->save_session( this->current_session_name_state_->get() );

      if ( succeeded )
      {
        boost::filesystem::path current_xml_file = current_project_path / current_project_file;
        // Remove the old project file
        try 
        {
          boost::filesystem::remove( current_xml_file );
        }
        catch( ... )
        {
          std::string error = std::string( "Could not delete file '" ) + 
            current_xml_file.string() + "'.";
          CORE_LOG_ERROR( error );
        }
      }
      
      return succeeded;
    }
    else
    {
      // OK copy the full project to the new directory
      // Some users have requested that all files/folders be copied, even ones they put in the 
      // project folder.  This is why we need to do a recursive copy instead of explicity
      // copying certain files and folders.
      if ( !Core::RecursiveCopyDirectory( current_project_path, project_path ) )
      {
        CORE_LOG_ERROR( "Couldn't copy the project directory to the new location." );
        return false;
      }

      // Remove patient-specific data if project should be anonymized
      if( anonymize )
      {
        // Remove inputfiles directory from new project
        boost::filesystem::path inputfiles_dir = project_path / INPUTFILES_DIR_C;
        try
        {
          boost::filesystem::remove_all( inputfiles_dir );
        }
        catch ( ... ) 
        {
          CORE_LOG_ERROR( "Couldn't remove directory '" + 
            inputfiles_dir.string() + "' for anonymization." );
          return false;
        }
        
        // Remove meta data from session files
        boost::filesystem::path session_dir = project_path / SESSION_DIR_C;
        
        // Iterate through directory
        boost::filesystem::directory_iterator it = 
          boost::filesystem::directory_iterator( session_dir );
        boost::filesystem::directory_iterator it_end;
                
        while ( it != it_end )
        {
          boost::filesystem::path filename = (*it).path();
          if ( filename.extension().string() == ".xml" )
          {         
            Core::StateIO state_io;
            if ( !state_io.import_from_file( filename ) ) 
            {
              // NOTE: Rather remove a file than let patient data continue to persist
              
              try
              {
                boost::filesystem::remove( filename );
                continue;
              }
              catch( ... )
              {
                CORE_LOG_ERROR( "Couldn't remove file '" + filename.string() 
                  + "' for anonymization." );
                return false;             
              }
            }
            
            // Find the layer manager element
            TiXmlElement* root_element = state_io.get_current_element();
            TiXmlElement* lm_element = root_element->FirstChildElement( "layermanager" );
            if ( lm_element == 0 ) 
            {
              // does not contain layer manager so continue
              continue;
            }
            
            // Finr the groups element
            TiXmlElement* groups_element = lm_element->FirstChildElement( "groups" );
            if ( groups_element == 0 ) 
            {
              // does not contain layer groups so continue
              continue;
            }
            
            TiXmlElement* group_element = groups_element->FirstChildElement();
            while ( group_element != 0 )
            {
              TiXmlElement* state_element = group_element->FirstChildElement( "State" );
              while ( state_element != 0 )
              {
                const char* state_id = state_element->Attribute( "id" );
                if ( state_id != 0 )
                {
                  if ( strcmp( state_id, "meta_data" ) == 0 ||
                     strcmp( state_id, "meta_data_info" ) == 0 )
                  {
                    if ( state_element->FirstChild() )
                    {
                      state_element->FirstChild()->SetValue( "" );
                    }
                  }                   
                }
                
                state_element = state_element->NextSiblingElement( "State" );
              }

              group_element = group_element->NextSiblingElement();
            }
            
            group_element = groups_element->FirstChildElement();
            while ( group_element != 0 )
            {
              TiXmlElement* layers_element = group_element->FirstChildElement( "layers" );
              if ( layers_element == 0 )
              {
                // does not contain layer groups so continue
                continue;
              }

              TiXmlElement* layer_element = layers_element->FirstChildElement();
              while ( layer_element != 0 )
              {
                TiXmlElement* state_element = layer_element->FirstChildElement( "State" );
                while ( state_element != 0 )
                {
                  const char* state_id = state_element->Attribute( "id" );
                  if ( state_id != 0 )
                  {
                    if ( strcmp( state_id, "metadata" ) == 0 ||
                       strcmp( state_id, "metadata_info" ) == 0 )
                    {
                      if ( state_element->FirstChild() )
                      {
                        state_element->FirstChild()->SetValue( "" );
                      }
                    }                   
                  }
                  
                  state_element = state_element->NextSiblingElement( "State" );
                }

                layer_element = layer_element->NextSiblingElement();
              }

              group_element = group_element->NextSiblingElement();
            }
            
            if ( !state_io.export_to_file( filename ) )
            {
              // NOTE: Rather remove a file than let patient data continue to persist
              
              try
              {
                boost::filesystem::remove( filename );
                continue;
              }
              catch( ... )
              {
                CORE_LOG_ERROR( "Couldn't remove file '" + filename.string() 
                  + "' for anonymization." );
                return false;             
              }
            }
          }
          ++it;
        }
        
        // This will instruct the save session to strip the meta data out, when it is
        // saved at the end of this function.
        
        this->set_need_anonymize( true );
        // Done anonymizing
      }

      // Delete the project file from the new directory because a new one will be generated
      boost::filesystem::path old_project_file = project_path / current_project_file;
      try
      {
        boost::filesystem::remove( old_project_file );
      }
      catch ( ... ) 
      {
        CORE_LOG_ERROR( "Couldn't remove file '" + old_project_file.string() + "'." );
      }

      // Update the project
      this->project_name_state_->set( project_name );
      this->project_file_state_->set( project_name + 
        Project::GetDefaultProjectFileExtension() );
      this->project_path_state_->set( project_path.string() );
      
      // This will save a session and update the project file and 
      // provenance database
      return this->save_session( AUTO_SESSION_NAME_C );
    }
  }
}

boost::filesystem::path Project::get_project_data_path() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  boost::filesystem::path project_path( this->project_path_state_->get() );
  return project_path / DATA_DIR_C;
}

boost::filesystem::path Project::get_project_sessions_path() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  boost::filesystem::path project_path( this->project_path_state_->get() );
  return project_path / SESSION_DIR_C;
}

boost::filesystem::path Project::get_project_inputfiles_path() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  boost::filesystem::path project_path( this->project_path_state_->get() );
  return project_path / INPUTFILES_DIR_C;
}

bool Project::find_cached_file( const boost::filesystem::path& filename, InputFilesID inputfiles_id,
    boost::filesystem::path& cached_filename ) const
{
  cached_filename = this->get_project_inputfiles_path() / Core::ExportToString( inputfiles_id ) /
    filename.filename();
  if ( boost::filesystem::exists( cached_filename ) ) return true;
  else return false;
}

bool Project::save_state()
{
  // This function sets state variables directly, hence we need to be on the application thread.
  ASSERT_IS_APPLICATION_THREAD();

  if ( !this->project_files_generated_state_->get() )
  {
    CORE_LOG_ERROR( "Project has not yet been saved to disk." );
    return false;
  }

  boost::filesystem::path project_directory( this->project_path_state_->get() );
  return this->private_->save_state( project_directory );
}


bool Project::export_project( const boost::filesystem::path& export_path, 
  const std::string& project_name, long long session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread.
  ASSERT_IS_APPLICATION_THREAD();

  if ( !this->is_session( session_id ) )
  {
    CORE_LOG_ERROR( Core::ExportToString( session_id ) + "is not a valid session ID." );
    return false;
  }

  std::string error;
  if ( boost::filesystem::exists( export_path ) && 
    !boost::filesystem::is_directory( export_path ) )
  {
    error = std::string( "Could not export project, because '"  ) +
      export_path.string() + "' is not a directory.";
    CORE_LOG_ERROR( error );
    return false;
  }

  // Create necessary directories for exporting the project
  if ( !ProjectPrivate::UpdateProjectDirectory( export_path ) )
  {
    return false;
  }

  boost::filesystem::path project_path( this->project_path_state_->get() );

  // Make copies of the session and provenance databases
  DatabaseManager export_session_db( this->private_->session_database_ );
  DatabaseManager export_prov_db( this->private_->provenance_database_ );

  // Delete all the session entries except the one to be exported
  std::string sql_str = "DELETE FROM session WHERE session_id != " +
    Core::ExportToString( session_id ) + ";";
  if ( !export_session_db.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Get the session XML file
  boost::filesystem::path session_file;
  if ( !this->private_->get_session_file( session_id, session_file, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Copy the session XML file
  try
  {
    boost::filesystem::copy_file( session_file, export_path / SESSION_DIR_C /
      ( Core::ExportToString( session_id ) + ".xml" ) );
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "Couldn't copy file '" + session_file.string() + "'." );
    return false;
  }

  // Save out the session database
  boost::filesystem::path export_session_db_path = 
    export_path / DATABASE_DIR_C / SESSION_DATABASE_C;
  if ( !export_session_db.save_database( export_session_db_path, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Get data generations used by the session
  sql_str = "SELECT data_generation FROM session_data;";
  ResultSet result_set;
  if ( !export_session_db.run_sql_statement( sql_str, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  std::set< Core::DataBlock::generation_type > generations;
  for ( size_t i = 0; i < result_set.size(); ++i )
  {
    generations.insert( boost::any_cast< Core::DataBlock::generation_type >(
      result_set[ i ][ "data_generation" ] ) );
  }

  // Copy those data files
  BOOST_FOREACH( Core::DataBlock::generation_type generation, generations )
  {
    boost::filesystem::path src_file = project_path / DATA_DIR_C / 
      ( Core::ExportToString( generation ) + ".nrrd" );
    boost::filesystem::path dst_file = export_path / DATA_DIR_C /
      ( Core::ExportToString( generation ) + ".nrrd" );
    if ( !boost::filesystem::exists( src_file ) )
    {
      CORE_LOG_ERROR( "Missing data file '" + src_file.string() + "'." );
      return false;
    }
    try
    {
      boost::filesystem::copy_file( src_file, dst_file );
    }
    catch ( ... )
    {
      CORE_LOG_ERROR( "Failed to copy file '" + src_file.string() + "'." );
      return false;
    }
  }

  // Parse out the provenance IDs referenced by the session
  std::vector< ProvenanceID > prov_ids;
  this->private_->parse_session_provenance_ids( session_file, prov_ids );

  // Get all the provenance steps relevant to prov_ids
  std::set< ProvenanceStepID > prov_steps;
  this->private_->get_provenance_steps( prov_ids, prov_steps );

  // Delete irrelevant provenance records from the provenance database
  if ( !prov_steps.empty() )
  {
    sql_str = "DELETE FROM provenance_step WHERE prov_step_id NOT IN (";
    BOOST_FOREACH( ProvenanceStepID step_id, prov_steps )
    {
      sql_str += ( Core::ExportToString( step_id ) + "," );
    }
    sql_str[ sql_str.size() - 1 ] = ')';
    sql_str += ";";
  }
  else
  {
    sql_str = "DELETE FROM provenance_step;";
  }
  if ( !export_prov_db.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  // Save out the provenance database
  boost::filesystem::path export_provenance_db_path = 
    export_path / DATABASE_DIR_C / PROVENANCE_DATABASE_C;
  if ( !export_prov_db.save_database( export_provenance_db_path, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Copy inputfiles
  sql_str = "SELECT inputfiles_cache_id FROM provenance_inputfiles_cache;";
  if ( !export_prov_db.run_sql_statement( sql_str, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  BOOST_FOREACH( ResultSet::value_type result, result_set )
  {
    long long inputfiles_cache_id = boost::any_cast< long long >( result[ "inputfiles_cache_id" ] );
    boost::filesystem::path src_dir = project_path / INPUTFILES_DIR_C / 
      Core::ExportToString( inputfiles_cache_id );
    boost::filesystem::path dst_dir = export_path / INPUTFILES_DIR_C /
      Core::ExportToString( inputfiles_cache_id );
    if ( !Core::RecursiveCopyDirectory( src_dir, dst_dir ) )
    {
      CORE_LOG_ERROR( "Failed to copy directory '" + src_dir.string() + "'." );
      return false;
    }
  }

  // Save out the notes database
  boost::filesystem::path export_note_db_path =
    export_path / DATABASE_DIR_C / NOTE_DATABASE_C;
  if ( !this->private_->note_database_.save_database( export_note_db_path, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Write out the project file
  boost::filesystem::path export_project_file = export_path / 
    ( project_name + GetDefaultProjectFileExtension() );
  Core::StateIO stateio;
  stateio.initialize();
  if ( !this->save_states( stateio ) )
  {
    error = std::string( "Failed to save out the state of the project to file" );
    CORE_LOG_ERROR( error );
    return false;
  }
  if ( !stateio.export_to_file( export_project_file ) )
  {
    std::string error = std::string( "Failed to write project file '" ) +
      export_project_file.string() + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }

  return true;
}


bool Project::check_project_files()
{
  // If no files have been generated just return
  if ( this->project_files_generated_state_->get() == false ) return false;

  // Get the directory where the project is stored
  boost::filesystem::path project_path( this->project_path_state_->get() );
  if ( !boost::filesystem::exists( project_path ) )
  {
    this->project_files_accessible_state_->set( false );
    return false;
  }

  if ( !boost::filesystem::exists( project_path / DATA_DIR_C ) )
  {
    this->project_files_accessible_state_->set( false );
    return false;
  }

  if ( !boost::filesystem::exists( project_path / SESSION_DIR_C ) )
  {
    this->project_files_accessible_state_->set( false );
    return false;
  }
  
  

  //// Check if s3d file still exists
  //if ( !boost::filesystem::exists( project_path / ( this->project_file_state_->get() ) ) )
  //{
  //  //this->project_files_generated_state_->set( false );
  //  this->project_files_accessible_state_->set( false );
  //  return false;   
  //}

  //// Check if session database file still exists
  //if ( !boost::filesystem::exists( project_path / DATABASE_DIR_C / SESSION_DATABASE_C ) )
  //{
  //  //this->project_files_generated_state_->set( false );
  //  this->project_files_accessible_state_->set( false );
  //  return false;   
  //}

  //// Check if provenance database file still exists
  //if ( !boost::filesystem::exists( project_path / DATABASE_DIR_C / PROVENANCE_DATABASE_C ) )
  //{
  //  //this->project_files_generated_state_->set( false );
  //  this->project_files_accessible_state_->set( false );
  //  return false;   
  //}

  this->project_files_accessible_state_->set( true );
  return true;
}


bool Project::load_session( SessionID session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Get the session XML file
  std::string error;
  boost::filesystem::path session_file;
  if ( !this->private_->get_session_file( session_id, session_file, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  // Tell program that project does not yet need to be saved.
  this->reset_project_changed();
  
  Core::StateIO state_io;
  if ( !state_io.import_from_file( session_file ) )
  {
    std::string error = std::string( "Failed to read session file '" ) + session_file.string() + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( ! Core::StateEngine::Instance()->load_states( state_io ) )
  {
    std::string error = std::string( "Failed to apply session data." );
    CORE_LOG_ERROR( error );
    return false;
  }
  
  this->project_files_generated_state_->set( true );
  this->project_files_accessible_state_->set( true );

  // We need to restore provenance and generation count from the project
  Core::DataBlock::generation_type generation = this->generation_count_state_->get(); 
  Core::DataBlockManager::Instance()->set_generation_count( generation );
  ProvenanceID provenance_id = this->provenance_count_state_->get();
  SetProvenanceCount( provenance_id );

  return true;    
}

bool Project::load_last_session()
{
  std::vector< SessionInfo > sessions;
  this->private_->get_all_sessions( sessions );
  // Starting from the most recent session, try to load the sessions one by one
  // until one can be successfully loaded.
  for ( size_t i = 0; i < sessions.size(); ++i )
  {
    if ( this->load_session( sessions[ i ].session_id() ) )
    {
      this->private_->update_project_size();
      return true;
    }
    // Remove the session record since it can't be loaded
    CORE_LOG_ERROR( "Session '" + sessions[ i ].session_name() + "' is invalid." );
    this->private_->delete_session_from_database( sessions[ i ].session_id() );
    Core::Application::Reset();
  }

  // We could not load the file, hence send a message to the user
  CORE_LOG_ERROR( this->project_name_state_->get() + " doesn't contain a valid session." );
  return false;
}

bool Project::save_session( const std::string& name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  std::string session_name = name;
  // Update the session name if needed
  if ( session_name.empty() ) 
  {
    session_name = this->current_session_name_state_->get();
  }

  // NOTE: Need to clear this variable, as the save_states system will fill it out again.
  // The reason for this is to avoid circular dependencies.
  this->private_->session_generation_numbers_.clear();

  boost::filesystem::path project_path( this->project_path_state_->get() );

  // Generate the project directory if it does not exist and generate all the sub directories.
  if ( !ProjectPrivate::UpdateProjectDirectory( project_path ) )
  {
    return false;
  } 

  // Copy all the input files into the project directory
  this->private_->process_inputfile_importers();

  // NOTE: We need to save first before making an entry into the database to be sure it will succeed.
  Core::StateIO state_io;
  state_io.initialize();
  if ( !Core::StateEngine::Instance()->save_states( state_io ) )
  {
    std::string error = "Could not extract all the session information from the project.";
    CORE_LOG_ERROR( error );
    return false;
  }

  // Get the user name, as session information contains the name of the user that saved the session.
  std::string user_id;
  if ( !Core::Application::Instance()->get_user_name( user_id ) )
  {
    user_id = "unknown";
  }
  
  // Convert the old session files if necessary
  if ( this->private_->conversion_needed_ )
  {
    this->private_->rename_version1_session_files();
    this->private_->conversion_needed_ = false;
  }

  // Add the entry to the session database
  SessionID session_id = this->private_->insert_session_into_database( session_name, user_id );
  if ( session_id < 0 )
  {
    CORE_LOG_ERROR( "Failed to added a new session record to the database." );
    return false;
  }

  // Write the XML file in the session directory
  boost::filesystem::path session_path = project_path / SESSION_DIR_C / 
    ( Core::ExportToString( session_id ) + ".xml" );
  if ( !state_io.export_to_file( session_path ) )
  {
    std::string error = std::string( "Could not save session file '" ) + 
      session_path.string() + "'.";
    CORE_LOG_ERROR( error );
    
    // NOTE: We need to delete it when saving fails
    this->private_->delete_session_from_database( session_id );
    return false;
  }

  // NOTE: This variable was filled out by the saving function of each layer
  if ( !this->private_->set_session_data( session_id, this->private_->session_generation_numbers_ ) )
  {
    this->private_->delete_session_from_database( session_id );
    return false;
  }

  // Save the state of the project to disk
  this->save_state();

  // Update the size of the project
  this->private_->update_project_size();
  
  // Add a timestamp of when the last session was saved for auto save functionality
  this->private_->set_last_saved_session_time_stamp();

  // Tell program that project does not yet need to be saved.
  this->reset_project_changed();
  
  // Signal the user interface the new session list
  SessionInfoListHandle session_list( new SessionInfoList );
  this->private_->get_all_sessions( *session_list );
  this->session_list_changed_signal_( session_list );

  return true;
}


bool Project::delete_session( SessionID session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  std::string error;
  boost::filesystem::path session_file;
  if ( this->private_->get_session_file( session_id, session_file, error ) )
  {
    try
    {
      boost::filesystem::remove( session_file );
    }
    catch ( ... ) 
    {
      CORE_LOG_ERROR( "Couldn't delete file '" + session_file.string() + "'." );
    }
  }
  
  // Delete the file from the database
  this->private_->delete_session_from_database( session_id );
  
  // Remove redundant files from the project
  this->private_->clean_up_data_files();

  // Save the state of the project to disk
  this->save_state();

  // Update the size of the project
  this->private_->update_project_size();

  // Signal the user interface the new session list
  SessionInfoListHandle session_list( new SessionInfoList );
  this->private_->get_all_sessions( *session_list );
  this->session_list_changed_signal_( session_list );

  return true;
} 

bool Project::get_session_info( SessionID session_id, SessionInfo& session_info )
{
  // We should only access the database in the application thread
  ASSERT_IS_APPLICATION_THREAD();

  std::string error;
  ResultSet result_set;
  std::string sql_str = "SELECT * FROM session WHERE session_id = " +
    Core::ExportToString( session_id ) + ";";
  if( !this->private_->session_database_.run_sql_statement( sql_str, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( result_set.size() == 0 )
  {
    return false;
  }

  std::vector< SessionInfo > sessions;
  this->private_->extract_session_info( result_set, sessions );
  session_info = sessions[ 0 ];

  return true;
}

void Project::reset_project_changed()
{
  // This function can be called from every where, hence we need to lock
  Core::Application::lock_type lock( Core::Application::GetMutex() );
  this->private_->changed_ = false;
}

bool Project::check_project_changed()
{
  // This function can be called from every where, hence we need to lock
  Core::Application::lock_type lock( Core::Application::GetMutex() );
  return this->private_->changed_;
}

bool Project::pre_save_states( Core::StateIO& state_io )
{
  // TODO: This logic is not quite right -> it over writes the preferences when restoring colors
  // The logic should allow for the right state variables to be taken when selecting a color
  // It comes from the preference manager OR the current_project

  // Grab the colors from the preferences manager
  if( this->save_custom_colors_state_->get() )
  {
    for ( size_t j = 0; j < this->color_states_.size(); j++ )
    {
      // NOTE: color_states_ is only used locally for storing and retrieving colors
      this->color_states_[ j ]->set( 
        PreferencesManager::Instance()->color_states_[ j ]->get() );
    }
  }
  
  // We need to store the generation count
  // NOTE: Again this is an internally used variable, and it is only synchronized so generation
  // numbers are unique project wide.
  this->generation_count_state_->set( Core::DataBlockManager::Instance()->get_generation_count() );

  // We need to store the provenance count as well.
  // NOTE: This will ensure that everything will get unique provenance numbers
  this->provenance_count_state_->set( GetProvenanceCount() );

  return true;
}

bool Project::post_load_states( const Core::StateIO& state_io )
{
  // If the user has chosen to save their custom colors as part of the project, we load them into
  // the preferences manager from the project's state variables
  if( this->save_custom_colors_state_->get() )
  {
    for ( size_t j = 0; j < this->color_states_.size(); j++ )
    {
      PreferencesManager::Instance()->color_states_[ j ]->set( 
        this->color_states_[ j ]->get() );
    }
  }

  // TODO: Need to move this out of this function
  boost::filesystem::path project_path( this->project_path_state_->get() );

  // We need to restore provenance and generation count for the project

  Core::DataBlock::generation_type generation = this->generation_count_state_->get(); 
  Core::DataBlockManager::Instance()->set_generation_count( generation );

  ProvenanceID provenance_id = this->provenance_count_state_->get();

  // Read the provenance count from file
  SetProvenanceCount( provenance_id );

  return true;
}

bool Project::is_session( SessionID session_id )
{
  // We should only access the database in the application thread
  ASSERT_IS_APPLICATION_THREAD();

  std::string sql_str = "SELECT * FROM session WHERE session_id = " +
    Core::ExportToString( session_id ) + ";";
  std::string error;
  ResultSet results;
  if ( !this->private_->session_database_.run_sql_statement( sql_str, results, error ) ||
    results.size() == 0 )
  {
    return false;
  }

  return true;
}

ProvenanceStepID Project::add_provenance_record( const ProvenanceStepHandle& step )
{
  long long user_id = this->private_->get_user_id( step->get_username() );
  long long action_id = this->private_->get_action_id( step->get_action_name() );
  if ( user_id == -1 || action_id == -1 ) return -1;

  // Make sure action_params is not empty.
  // NOTE: A non-empty parameter string simplifies the query process 
  std::string action_params = step->get_action_params();
  if ( action_params.empty() )
  {
    action_params = " ";
  }
    
  ProvenanceIDList output_list = step->get_output_provenance_ids();
  ProvenanceIDList input_list = step->get_input_provenance_ids();
  ProvenanceIDList deleted_list = step->get_replaced_provenance_ids();
  InputFilesID inputfiles_id = step->get_inputfiles_id();

  std::string sql_str = "INSERT INTO provenance_step (action_id, action_params, user_id)"
    " VALUES(" + Core::ExportToString( action_id ) + ", '" + 
    DatabaseManager::EscapeQuotes( action_params ) + 
    "', " + Core::ExportToString( user_id ) + ");";
  std::string error;
  if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }
  ProvenanceStepID step_id = this->private_->provenance_database_.get_last_insert_rowid();

  for ( size_t i = 0; i < input_list.size(); ++i )
  {
    sql_str = "INSERT INTO provenance_input (prov_step_id,prov_id) VALUES(" + Core::ExportToString( step_id ) + ", " +
      Core::ExportToString( input_list[ i ] ) + ");";
    if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
    {
      CORE_LOG_ERROR( error );
      this->delete_provenance_record( step_id );
      return -1;
    }
  }

  for ( size_t i = 0; i < output_list.size(); ++i )
  {
    sql_str = "INSERT INTO provenance_output (prov_step_id,prov_id) VALUES(" + Core::ExportToString( step_id ) + ", " +
      Core::ExportToString( output_list[ i ] ) + ");";
    if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
    {
      CORE_LOG_ERROR( error );
      this->delete_provenance_record( step_id );
      return -1;
    }
  }
  
  for ( size_t i = 0; i < deleted_list.size(); ++i )
  {
    sql_str = "INSERT INTO provenance_replaced (prov_step_id,prov_id) VALUES(" + Core::ExportToString( step_id ) + ", " +
      Core::ExportToString( deleted_list[ i ] ) + ");";
    if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
    {
      CORE_LOG_ERROR( error );
      this->delete_provenance_record( step_id );
      return -1;
    }
  }
  
  if ( inputfiles_id > -1 )
  { // If it is a valid ID add it to the table
    sql_str = "INSERT INTO provenance_inputfiles_cache VALUES (" + 
      Core::ExportToString( step_id ) + ", " + Core::ExportToString( inputfiles_id ) + ");";
      
    if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
    {
      CORE_LOG_ERROR( error );
      this->delete_provenance_record( step_id );
      return -1;
    }
  }
  
  return step_id;
}

bool Project::delete_provenance_record( ProvenanceStepID record_id )
{
  std::string sql_str = "DELETE FROM provenance_step WHERE prov_step_id = " +
    Core::ExportToString( record_id ) + ";";
  std::string error;
  if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  return true;
}

void Project::update_provenance_record( ProvenanceStepID record_id, const ProvenanceStepHandle& prov_step )
{
  std::string action_params = prov_step->get_action_params();

  // Make sure action_params is not empty.
  // NOTE: A non-empty parameter string simplifies the query process 
  if ( action_params.empty() )
  {
    action_params = " ";
  }

  std::string sql_str = "UPDATE provenance_step SET action_params = '" + 
    DatabaseManager::EscapeQuotes( action_params ) +
    "' WHERE prov_step_id = " + Core::ExportToString( record_id ) + ";";
  std::string error;
  if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
  }
}

boost::posix_time::ptime Project::get_last_saved_session_time_stamp() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->last_saved_session_time_stamp_;
}

void Project::request_provenance_trail( ProvenanceID prov_id )
{
  // Run SQL queries in application thread
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Project::request_provenance_trail, this, prov_id ) );
    return;
  }

  ProvenanceTrailHandle provenance_trail( new ProvenanceTrail );
  std::vector< ProvenanceID > prov_ids( 1, prov_id );
  if ( !this->private_->query_provenance_trail( prov_ids, *provenance_trail ) )
  {
    provenance_trail->clear();
  }

  this->provenance_trail_signal_( provenance_trail );
}

ProvenanceTrailHandle Project::get_provenance_trail( const std::vector< ProvenanceID >& prov_ids )
{
  ASSERT_IS_APPLICATION_THREAD();

  ProvenanceTrailHandle provenance_trail( new ProvenanceTrail );
  if ( !this->private_->query_provenance_trail( prov_ids, *provenance_trail ) )
  {
    provenance_trail.reset();
  }

  return provenance_trail;
}

void Project::request_session_list()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Project::request_session_list, this ) );
    return;
  }
  
  SessionInfoListHandle session_list( new SessionInfoList );
  this->private_->get_all_sessions( *session_list );
  this->session_list_changed_signal_( session_list );
}

std::string Project::GetDefaultProjectPathExtension()
{
  std::vector<std::string> path_extensions = 
    Core::SplitString( CORE_APPLICATION_PROJECT_FOLDER_EXTENSION, ";");
    
  if ( path_extensions.size() > 0 )
  {
    return path_extensions[ 0 ];
  }
  else
  {
    return ".seg3dproj";
  }
}
  
std::string Project::GetDefaultProjectFileExtension()
{
  std::vector<std::string> file_extensions = 
    Core::SplitString( CORE_APPLICATION_PROJECT_EXTENSION, ";");
    
  if ( file_extensions.size() > 0 )
  {
    return file_extensions[ 0 ];
  }
  else
  {
    return ".s3d";
  }
}

std::vector<std::string> Project::GetProjectPathExtensions()
{
  std::vector<std::string> path_extensions = 
    Core::SplitString( CORE_APPLICATION_PROJECT_FOLDER_EXTENSION, ";");
    
  if ( path_extensions.size() > 0 )
  {
    return path_extensions;
  }
  else
  {
    std::vector<std::string> default_vector( 1, ".seg3dproj" );
    return default_vector;
  }
} 
  
std::vector<std::string> Project::GetProjectFileExtensions()    
{
  std::vector<std::string> file_extensions = 
    Core::SplitString( CORE_APPLICATION_PROJECT_EXTENSION, ";");
    
  if ( file_extensions.size() )
  {
    return file_extensions;
  }
  else
  {
    std::vector<std::string> default_vector( 1, ".s3d" );
    return default_vector;
  }
}

void Project::add_generation_number( const long long generation_number )
{
  this->private_->session_generation_numbers_.insert( generation_number );
}

bool Project::execute_or_add_inputfiles_importer( const InputFilesImporterHandle& importer )
{
  // Add the importer to the list
  this->private_->input_file_importers_.push_back( importer );

  // If the project exists on disk copy all the files over
  return this->private_->process_inputfile_importers(); 
}

int Project::get_version()
{
  return 2;
}

bool Project::add_note( const std::string& note )
{
  ASSERT_IS_APPLICATION_THREAD();

  // Get the user name
  std::string user_id;
  if ( !Core::Application::Instance()->get_user_name( user_id ) )
  {
    user_id = "unknown";
  }

  bool succeeded = this->private_->insert_note_into_database( note, user_id ) > 0;
  if ( succeeded )
  {
    this->request_note_list();
  }
  
  return succeeded;
}

void Project::request_note_list()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Project::request_note_list, this ) );
    return;
  }
  
  ProjectNoteListHandle note_list( new ProjectNoteList );
  this->private_->get_all_notes( *note_list );
  this->note_list_changed_signal_( note_list );
}


bool Project::get_need_anonymize()
{
  return this->private_->need_anonymize_;
}

void Project::set_need_anonymize( bool anonymize )
{
  this->private_->need_anonymize_ = anonymize;
}


} // end namespace Seg3D
