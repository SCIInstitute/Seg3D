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

#if defined( __APPLE__ )  
#include <CoreServices/CoreServices.h>
#endif

// STL includes
#include <queue>
#include <set>
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>
#include <Core/DataBlock/DataBlockManager.h>

// Application includes
#include <Application/Project/Project.h>
#include <Application/Provenance/Provenance.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/DatabaseManager/DatabaseManager.h>
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>

// Include CMake generated files
#include "ApplicationConfiguration.h"

namespace Seg3D
{

static const boost::filesystem::path SESSION_DIR_C( "sessions" );
static const boost::filesystem::path SESSION_DATABASE_C( "sessions.sqlite" );
static const boost::filesystem::path PROVENANCE_DIR_C( "provenance" );
static const boost::filesystem::path PROVENANCE_DATABASE_C( "provenance.sqlite" );

class ProjectPrivate
{
  // -- constructor/destructor --
public:
  ProjectPrivate() :
    project_( 0 ),
    changed_( false ),
    last_saved_session_time_stamp_( boost::posix_time::second_clock::local_time() )
  { 
  }

public:
  // UPDATE_PROJECT_SIZE
  // Sum the size of all the files in the directory
  void update_project_size();

  // COMPUTE_DIRECTORY_SIZE
  // Compute the size in bytes used by a directory
  long long compute_directory_size( const boost::filesystem::path& directory );

  // UPDATE_PROJECT_DIRECTORY:
  // Generate all the required directories for this project
  bool update_project_directory( const boost::filesystem::path& project_directory );

  // CLEANUP_SESSION_DATABASE:
  // this function cleans up sessions in the session list that have been deleted by the user
  bool clean_up_session_database(); 
  
  // CLEAN_UP_DATA_FILES:
  // Clean up files that are not used by any session.
  void clean_up_data_files(); 
  
  // SAVE_STATE:
  // Save the state of the current project into the xml file and save the database
  // in the database file
  bool save_state( const boost::filesystem::path& project_directory );  
  
  // COPY_ALL_FILES:
  // Copy all files from one directory into another directory
  bool copy_all_files( const boost::filesystem::path& src_path, 
    const boost::filesystem::path& dst_path, bool copy_s3d_file = false );
    
  // SET_LAST_SAVED_SESSION_TIME_STAMP
  // this function updates the time of when the last session was saved
  void set_last_saved_session_time_stamp();
    
  // INITIALIZE_SESSION_DATABASE:
  // Create tables for the session database.
  bool initialize_session_database();

  // INITIALIZE_PROVENANCE_DATABASE:
  // Create tables for the provenance database.
  bool initialize_provenance_database();

  SessionID insert_session_into_database( const std::string& session_name, 
    const std::string& user_id, const std::string& timestamp = "" );
  
  // DELETE_SESSION_FROM_DATABASE:
  // this deletes a session from the database
  bool delete_session_from_database( SessionID session_id );
  
  bool set_session_data( SessionID id, const std::set< long long >& generations );

  SessionID get_last_session();

  void query_provenance_trail( ProvenanceID prov_id, ProvenanceTrail& provenance_trail );
  
  bool get_all_sessions( std::vector< SessionInfo >& sessions );

  void convert_version1_project();

  bool parse_session_data( const boost::filesystem::path& file_path, std::set< long long >& generations );

  void set_session_file( SessionID id, const std::string& file_name );

  // -- internal variables --
public:
  // Pointer back to the project
  Project* project_;

  // Whether the project has changed
  bool changed_;

  // Time when last session was saved for auto save functionality.
  boost::posix_time::ptime last_saved_session_time_stamp_;

  DatabaseManager session_database_;
  DatabaseManager provenance_database_;

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
// file sizes can exceed 2GB
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
    std::string filename = dir_itr->path().filename().string();
    boost::filesystem::path dir_file = directory / filename;
      
    // We only scan directories and regular files, to avoid circular dependencies from
    // symbolic links.
    if ( boost::filesystem::is_regular_file( dir_file ) )
    {
      size += static_cast< long long >( boost::filesystem::file_size( dir_file ) );
    }
    else if ( boost::filesystem::is_directory( dir_file ) )
    {
      size += this->compute_directory_size( dir_file );
    }
  }

  return size;
}


bool ProjectPrivate::update_project_directory( const boost::filesystem::path& project_directory  )
{ 

  // If the directory does not exist, make a new one
  if ( ! boost::filesystem::exists( project_directory ) )
  {
    try // try to create a project folder
    {
      boost::filesystem::create_directory( project_directory );
    }
    catch ( ... )
    {
      std::string error = std::string( "Could not create '" ) + project_directory.filename().string()
        + "'.";
      CORE_LOG_ERROR( error );
      return false; 
    }
  }

#if defined( __APPLE__ )  
  if ( PreferencesManager::Instance()->generate_osx_project_bundle_state_->get() )
  {   
    std::vector<std::string> project_path_extensions = Project::GetProjectPathExtensions();
    
    for ( size_t j = 0; j < project_path_extensions.size(); j++ )
    {
      if ( boost::filesystem::extension( project_directory ) == project_path_extensions[ j ] )
      {
        // MAC CODE
        try
        {
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
        //std::string command = std::string( "SetFile -a B \"" ) + project_directory.string() +"\"";
        //system( command.c_str() );
        break;
      }
    }
  }
#endif

  
  if ( ! boost::filesystem::exists( project_directory / SESSION_DIR_C ) )
  {
    try // to create a project sessions folder
    {
      boost::filesystem::create_directory( project_directory / SESSION_DIR_C );
    }
    catch ( ... )
    {
      std::string error = 
        std::string( "Could not create sessions directory in project folder '" ) + 
        project_directory.filename().string() + "'.";
      CORE_LOG_ERROR( error );
      return false; 
    }   
  }
  
  if ( ! boost::filesystem::exists( project_directory / "data" ) )
  { 
    try // to create a project data folder
    {
      boost::filesystem::create_directory( project_directory / "data");
    }
    catch ( ... )
    {
      std::string error = 
        std::string( "Could not create data directory in project folder '" ) + 
        project_directory.filename().string() + "'.";
      CORE_LOG_ERROR( error );
      return false; 
    }
  }
  
  if ( ! boost::filesystem::exists( project_directory / PROVENANCE_DIR_C ) )
  {         
    try // to create a folder with the provenance data base
    {
      boost::filesystem::create_directory( project_directory / PROVENANCE_DIR_C );
    }
    catch ( ... )
    {
      std::string error = 
        std::string( "Could not create provenance directory in project folder '" ) + 
        project_directory.filename().string() + "'.";
      CORE_LOG_ERROR( error );
      return false; 
    }
  }

  if ( ! boost::filesystem::exists( project_directory / "inputfiles" ) )
  {           
    try // to create a folder to store the original input files
    {
      boost::filesystem::create_directory( project_directory / "inputfiles" );
    }
    catch ( ... )
    {
      std::string error = 
        std::string( "Could not create inputfiles directory in project folder '" ) + 
        project_directory.filename().string() + "'.";
      CORE_LOG_ERROR( error );
      return false; 
    }   
  }
  
  return true;
} 

bool ProjectPrivate::clean_up_session_database()
{
  // If no files have been generated, just ignore the cleanup
  if ( ! this->project_->project_files_generated_state_->get() ) return true;

  std::string error;
  ResultSet result_set;
  std::string sql_str = "SELECT session_id, session_file FROM sessions;";
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
    
    // First, try session ID as the session file name
    boost::filesystem::path session_file = project_path / SESSION_DIR_C / 
      ( Core::ExportToString( session_id ) + ".xml" );
    if ( boost::filesystem::exists( session_file ) ) continue;

    // Otherwise, if there is 'session_file' explicitly stored in the database, try it also
    ResultSet::value_type::iterator it = result_set[ i ].find( "session_file" );
    if ( it != result_set[ i ].end() )
    {
      std::string session_file_str = boost::any_cast< std::string >( ( *it ).second );
      if ( !session_file_str.empty() )
      {
        session_file = project_path / SESSION_DIR_C / session_file_str;
        if ( boost::filesystem::exists( session_file ) ) continue;
      }
    }

    // No session file exists for the session, delete it from database
    this->delete_session_from_database( session_id );
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

  // Save the project database
  boost::filesystem::path session_database = project_directory / 
    SESSION_DIR_C / SESSION_DATABASE_C;
  boost::filesystem::path provenance_database = project_directory /
    PROVENANCE_DIR_C / PROVENANCE_DATABASE_C;
  std::string error;
  if ( !this->session_database_.save_database( session_database, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( !this->provenance_database_.save_database( provenance_database, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  return true;
}


bool ProjectPrivate::copy_all_files( const boost::filesystem::path& src_path, 
  const boost::filesystem::path& dst_path, bool copy_s3d_file )
{
  if ( ! boost::filesystem::exists( dst_path ) )
  {
    try // to create a project sessions folder
    {
      boost::filesystem::create_directory( dst_path );
    }
    catch ( ... )
    {
      std::string error = std::string( "Could not create directory '" ) +
        dst_path.string() + "'.";
      CORE_LOG_ERROR( error );
      return false;
    }
  }

  std::string extension = Project::GetDefaultProjectFileExtension();
  
  boost::filesystem::directory_iterator dir_end;
  for( boost::filesystem::directory_iterator dir_itr( src_path ); 
    dir_itr != dir_end; ++dir_itr )
  {
    // Only copy regular files and directories
    // Do not touch symbolic links, as they can generate circular paths
    boost::filesystem::path dir_file( src_path / dir_itr->path().filename().string() );
    
    if ( boost::filesystem::is_regular_file( dir_file ) )
    {
      if ( !copy_s3d_file && dir_file.extension() == extension ) continue;
      try
      {
        boost::filesystem::copy_file( dir_file, ( dst_path / dir_itr->path().filename().string() ) );
      }
      catch ( ... )
      {
        std::string error = std::string( "Could not copy file '" ) +
          dir_itr->path().filename().string() + "'.";
        CORE_LOG_ERROR( error );
        return false;
      }
    }
    else if ( boost::filesystem::is_directory( dir_file ) )
    {
      // Iterative copy of all files
      if ( ! this->copy_all_files( dir_file, ( dst_path / dir_itr->path().filename().string() ), true ) )
      {
        return false;
      }
    }
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
  sql_statements += "CREATE TABLE sessions "
    "(session_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "session_name TEXT NOT NULL, "
    "session_file TEXT, "
    "user_id TEXT NOT NULL, "
    "timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP);";

  // Create table for storing data files (generation numbers) used by each session
  sql_statements += "CREATE TABLE session_data "
    "(session_id INTEGER NOT NULL REFERENCES sessions(session_id) ON DELETE CASCADE, "
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

  // Create table for storing provenance steps
  sql_statements += "CREATE TABLE provenance_step "
    "(prov_step_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
    "action_name TEXT NOT NULL, "
    "action_params TEXT NOT NULL, "
    "timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, "
    "user_id TEXT NOT NULL);";

  // Create table for storing inputs of each provenance step
  sql_statements += "CREATE TABLE provenance_input "
    "(prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "prov_id INTEGER NOT NULL, "
    "PRIMARY KEY (prov_step_id, prov_id));";

  // Create index on prov_step_id column of provenance_input
  sql_statements += "CREATE INDEX prov_input_index ON provenance_input(prov_step_id);";

  // Create table for storing outputs of each provenance step
  sql_statements += "CREATE TABLE provenance_output "
    "(prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "prov_id INTEGER NOT NULL PRIMARY KEY);";

  // Create index on prov_step_id column of provenance_output
  sql_statements += "CREATE INDEX prov_output_index ON provenance_output(prov_step_id);";

  // Create table for storing deleted provenance IDs of each provenance step
  sql_statements += "CREATE TABLE provenance_deleted "
    "(prov_step_id INTEGER NOT NULL REFERENCES provenance_step(prov_step_id) ON DELETE CASCADE, "
    "prov_id INTEGER NOT NULL, "
    "PRIMARY KEY (prov_step_id, prov_id));";

  // Create index on prov_step_id column of provenance_deleted
  sql_statements += "CREATE INDEX prov_deleted_index ON provenance_deleted(prov_step_id);";

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

SessionID ProjectPrivate::insert_session_into_database( const std::string& session_name, 
  const std::string& user_id, const std::string& timestamp )
{
  std::string sql_statement;
  if ( timestamp.empty() )
  {
    sql_statement = "INSERT INTO sessions (session_name, user_id) VALUES ('" +
      session_name + "', '" + user_id + "');";
  }
  else
  {
    sql_statement = "INSERT INTO sessions (session_name, user_id, timestamp) VALUES ('" +
      session_name + "', '" + user_id + "', '" + timestamp + "');";
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
  std::string sql_str = "DELETE FROM sessions WHERE session_id = " + 
    Core::ExportToString( session_id ) + ";";

  std::string error;
  if( !this->session_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

void ProjectPrivate::query_provenance_trail( ProvenanceID prov_id, ProvenanceTrail& provenance_trail )
{
  if ( prov_id < 0 ) return;

  std::queue< ProvenanceID > provenance_queue;
  provenance_queue.push( prov_id );
  while ( !provenance_queue.empty() )
  {
    ProvenanceID output_id = provenance_queue.front();
    provenance_queue.pop();

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

    // Query the action string of the provenance step
    sql_str = "SELECT action_name, action_params FROM provenance_step WHERE "
      "prov_step_id = " + Core::ExportToString( prov_step_id ) + ";";
    result_set.clear();
    if ( !this->provenance_database_.run_sql_statement( sql_str, result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return;
    }
    if ( result_set.size() != 1 )
    {
      CORE_LOG_ERROR( "Provenance database is broken." );
      return;
    }
    std::string action_name = boost::any_cast< std::string >( result_set[ 0 ][ "action_name" ] );
    std::string action_params = boost::any_cast< std::string >( result_set[ 0 ][ "action_params" ] );
    provenance_trail.push_back( std::make_pair( output_id, action_name + " " + action_params ) );

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

bool ProjectPrivate::get_all_sessions( std::vector< SessionInfo >& sessions )
{
  std::string error;

  ResultSet result_set;
  std::string select_statement = "SELECT * FROM sessions ORDER BY session_id DESC;";
  if( !this->session_database_.run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  typedef boost::date_time::c_local_adjustor< boost::posix_time::ptime > local_adjustor;
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

  return true;
}

void ProjectPrivate::convert_version1_project()
{
  this->initialize_session_database();
  this->initialize_provenance_database();

  // Convert sessions

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

    generation_count = Core::Max( generation_count, 
      *( std::max_element( generations.begin(), generations.end() ) ) );

    SessionID session_id = this->insert_session_into_database( session_info_strs[ 1 ], session_info_strs[ 2 ], ss.str() );
    if ( session_id < 0 )
    {
      CORE_LOG_ERROR( "Error happened when converting old project data." );
      continue;
    }

    // Generate the new path of where the session is located
    boost::filesystem::path new_path = project_path / SESSION_DIR_C / ( Core::ExportToString( session_id ) + ".xml" );

    try 
    {
      // Try to rename the session
      boost::filesystem::rename( old_path, new_path );
    }
    catch ( ... )
    {
      // If the file can't be renamed, store the original file name in the database
      this->set_session_file( session_id, old_session_name + ".xml" );
    }

    this->set_session_data( session_id, generations );
  }

  // Finally we set the session list to an empty vector so we aren't tempted to do this again.
  // This variable remains empty and unused from this point onward. 
  this->project_->sessions_state_->clear();

  // Set the generation count
  Core::DataBlockManager::Instance()->set_generation_count( generation_count + 1 );

  this->project_->save_state();
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

void ProjectPrivate::set_session_file( SessionID id, const std::string& file_name )
{
  std::string sql_str = "UPDATE sessions SET session_file = '" + file_name +
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
  std::string sql_str = "SELECT session_id FROM sessions ORDER BY session_id DESC LIMIT 1;";
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


//////////////////////////////////////////////////////////

Project::Project( const std::string& project_name ) :
  StateHandler( "project", true ),
  private_( new ProjectPrivate )
{
  this->initialize_states();
  this->set_initializing( true );
  this->project_name_state_->set( project_name );
  this->project_file_state_->set( project_name + Project::GetDefaultProjectFileExtension() );
  this->private_->initialize_session_database();
  this->private_->initialize_provenance_database();
  this->set_initializing( false );
}

Project::Project( const boost::filesystem::path& project_file ) :
  StateHandler( "project", true ),
  private_( new ProjectPrivate )
{
  this->initialize_states();
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

void Project::initialize_states()
{
  // Mark this class so it does not add the state id number into the project file itself
  this->do_not_save_id_number();

  // Keep a pointer to this class in the private class
  this->private_->project_ = this;

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

  // This keeps track of the count for the sessions. Currently the maximum number of sessions 
  // that we handle is 9999. 
  this->add_state( "session_count", this->session_count_state_, 0 );

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

  // Count that keeps track of datablock generation numbers
  this->add_state( "generation_count", this->generation_count_state_, -1 );

  // Count that keeps track of maximum provenance ID
  this->add_state( "provenance_count", this->provenance_count_state_, -1 );

  // Each time an action is executed, check whether it changes the project data, if so
  // mark this in the project, so the UI can query the user for a save action if the application
  // is closed while there is unsaved data.
  this->add_connection( Core::ActionDispatcher::Instance()->post_action_signal_.connect( 
    boost::bind( &Project::set_project_changed, this, _1, _2 ) ) );
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
  if ( ! this->private_->update_project_directory( full_filename.parent_path() ) )
  {
    return false;
  }
  
  this->project_path_state_->set( full_filename.parent_path().string() );

  // Since we load this one from disk, it obviously exists on disk
  this->project_files_generated_state_->set( true );
  this->project_files_accessible_state_->set( true );

  // If the project was created by an old version, convert it
  if ( this->get_loaded_version() == 1 )
  {
    this->private_->convert_version1_project();
  }
  // Otherwise load the database
  else
  {
    boost::filesystem::path session_db_file = full_filename.parent_path() /
      SESSION_DIR_C / SESSION_DATABASE_C;
    std::string error;
    // If the session database doesn't exist or it's invalid, create an empty one
    if ( !boost::filesystem::exists( session_db_file ) ||
      !this->private_->session_database_.load_database( session_db_file, error ) )
    {
      this->private_->initialize_session_database();
    }

    boost::filesystem::path provenance_db_file = full_filename.parent_path() /
      PROVENANCE_DIR_C / PROVENANCE_DATABASE_C;
    if ( !boost::filesystem::exists( provenance_db_file ) ||
      !this->private_->provenance_database_.load_database( provenance_db_file, error ) )
    {
      this->private_->initialize_provenance_database();
    }
  }

  // Delete session records from the database that don't have corresponding session files
  this->private_->clean_up_session_database();
//  this->private_->clean_up_data_files();
  return true;
}

bool Project::save_project( const boost::filesystem::path& project_path, 
  const std::string& project_name )
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
    std::string project_file_name = project_name + Project::GetDefaultProjectFileExtension();

    // Generate the project directory if it does not exist and generate all the sub directories.
    if (! this->private_->update_project_directory( project_path ) )
    {
      return false;
    } 

    // Set the new directory in which this project is saved.
    this->project_path_state_->set( project_path.string() );

    // Make sure that the program knows that the current project now lives on disk. 
    this->project_files_generated_state_->set( true );

    // Save the current session so there is something to load when the project is opened.
    return this->save_session( this->current_session_name_state_->get() );
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
      return this->save_session( this->current_session_name_state_->get() );
    }

    if ( current_project_path == project_path )
    {
      // OK, the user gave us a new name, but directory is the same
          
      // Generate the new .s3d file and delete the old one  
      boost::filesystem::path current_xml_file = current_project_path / current_project_file;

      boost::filesystem::path new_xml_file = current_project_path / 
        ( project_name + Project::GetDefaultProjectFileExtension() );     

      // First save the new file, with all the new state information      
      this->private_->save_state( new_xml_file );
      
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
      
      // Update the project information
      this->project_name_state_->set( project_name ); 
      
      // Save out a session with the current information
      return this->save_session( this->current_session_name_state_->get() );
    }
    else
    {
      // OK copy the full project to the new directory, except the .s3d file
      this->private_->copy_all_files( current_project_path, project_path, false );
      
      // Update the project
      this->project_name_state_->set( project_name );
      this->project_file_state_->set( project_name + 
        Project::GetDefaultProjectFileExtension() );
      this->project_path_state_->set( project_path.string() );
      
      // This will save a session and update the project file and 
      // provenance database
      return this->save_session( this->current_session_name_state_->get() );
    }
  }
}

boost::filesystem::path Project::get_project_data_path() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  boost::filesystem::path project_path( this->project_path_state_->get() );
  return project_path / "data";
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
  return project_path / "inputfiles";
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

  // Check whether the directory in which we are exporting the data exists
  if ( ! boost::filesystem::exists( export_path ) )
  {
    try 
    {
      boost::filesystem::create_directory( export_path );
    }
    catch ( ... )
    {
      std::string error = std::string( "Could not create directory '" ) +
        export_path.string() + "'.";
      CORE_LOG_ERROR( error );
      return false;
    }
  }

  if ( ! boost::filesystem::is_directory( export_path ) )
  {
    std::string error = std::string( "Could not export project, because '"  ) +
      export_path.string() + "' is not a directory.";
    CORE_LOG_ERROR( error );
    return false;
  }


  // TODO: This code needs to be redone, as it currently copies all the files
  // TODO: This code does not export provenance currently
  
  boost::filesystem::path project_path( this->project_path_state_->get() );
  
  boost::filesystem::path data_path = project_path / "data";
  boost::filesystem::path export_data_path = export_path / "data";
  
  boost::filesystem::directory_iterator data_dir_itr( data_path );
  boost::filesystem::directory_iterator data_dir_end;
  for( ; data_dir_itr != data_dir_end; ++data_dir_itr )
  {
    try
    {
      boost::filesystem::copy_file( ( data_path / data_dir_itr->path().filename().string() ),
        ( export_data_path / data_dir_itr->path().filename().string() ) );
    }
    catch ( ... ) 
    {
      std::string error = std::string( "Could not copy file '" ) +
        ( data_path / data_dir_itr->path().filename() ).string() + "'.";
      CORE_LOG_ERROR( error );
      return false;
    }
  }
  
  try
  {
    boost::filesystem::copy_file( ( project_path / SESSION_DIR_C / 
      ( Core::ExportToString( session_id ) + ".xml" ) ),
      ( export_path / SESSION_DIR_C / ( Core::ExportToString( session_id ) + ".xml" ) ) );
  }
  catch ( ... ) // any errors that we might get thrown
  {
    std::string error = std::string( "Could not copy file '" ) +
      ( project_path / SESSION_DIR_C / ( Core::ExportToString( session_id ) + ".xml" ) ).string() + "'.";
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
    //this->project_files_generated_state_->set( false );
    this->project_files_accessible_state_->set( false );
    return false;
  }

  // Check if s3d file still exists
  if ( !boost::filesystem::exists( project_path / ( this->project_file_state_->get() ) ) )
  {
    //this->project_files_generated_state_->set( false );
    this->project_files_accessible_state_->set( false );
    return false;   
  }

  // Check if session database file still exists
  if ( !boost::filesystem::exists( project_path / SESSION_DIR_C / SESSION_DATABASE_C ) )
  {
    //this->project_files_generated_state_->set( false );
    this->project_files_accessible_state_->set( false );
    return false;   
  }

  // Check if provenance database file still exists
  if ( !boost::filesystem::exists( project_path / PROVENANCE_DIR_C / PROVENANCE_DATABASE_C ) )
  {
    //this->project_files_generated_state_->set( false );
    this->project_files_accessible_state_->set( false );
    return false;   
  }

  this->project_files_accessible_state_->set( true );
  return true;
}


bool Project::load_session( SessionID session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Query session information from database
  std::string sql_str = "SELECT * FROM sessions WHERE session_id = " + 
    Core::ExportToString( session_id ) + ";";
  ResultSet result_set;
  std::string error;
  if ( !this->private_->session_database_.run_sql_statement( sql_str, result_set, error ) ||
    result_set.size() == 0 )
  {
    CORE_LOG_ERROR( "Session " + Core::ExportToString( session_id ) + " doesn't exist." );
    return false;
  }
  assert( result_set.size() == 1 );

  // Get the project directory path
  boost::filesystem::path project_path( this->project_path_state_->get() );

  // Get the session XML file
  boost::filesystem::path session_file = project_path / SESSION_DIR_C / 
    ( Core::ExportToString( session_id ) + ".xml" );
  if ( !boost::filesystem::exists( session_file ) )
  {
    ResultSet::value_type::iterator it = result_set[ 0 ].find( "session_file" );
    if ( it != result_set[ 0 ].end() )
    {
      std::string session_file_str = boost::any_cast< std::string >( ( *it ).second );
      if ( !session_file_str.empty() )
      {
        session_file = project_path / SESSION_DIR_C / session_file_str;
      }
    }
  }

  if ( !boost::filesystem::exists( session_file ) )
  {
    CORE_LOG_ERROR( "Missing session file for session " + Core::ExportToString( session_id ) );
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
    this->private_->delete_session_from_database( sessions[ i ].session_id() );
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

  Core::StateIO state_io;
  state_io.initialize();
  if ( !Core::StateEngine::Instance()->save_states( state_io ) )
  {
    std::string error = "Could not extract all the session information from the project.";
    CORE_LOG_ERROR( error );
    return false;
  }

  std::string user_id;
  if ( !Core::Application::Instance()->get_user_name( user_id ) )
  {
    user_id = "unknown";
  }
  SessionID session_id = this->private_->insert_session_into_database( session_name, user_id );
  assert( session_id >= 0 );

  // Figure out where to save the data
  boost::filesystem::path project_path( this->project_path_state_->get() );
  boost::filesystem::path session_path = project_path / SESSION_DIR_C / 
    ( Core::ExportToString( session_id ) + ".xml" );
  if ( !state_io.export_to_file( session_path ) )
  {
    std::string error = std::string( "Could not save session file '" ) + 
      session_path.string() + "'.";
    CORE_LOG_ERROR( error );
    this->private_->delete_session_from_database( session_id );
    return false;
  }
    
  std::vector< LayerHandle > layers;
  LayerManager::Instance()->get_layers( layers );
  std::set< long long > generation_numbers;
  for ( size_t i = 0; i < layers.size(); ++i )
  {
    long long generation = layers[ i ]->get_generation();
    if ( generation >= 0 )
    {
      generation_numbers.insert( generation );
    }
  }
  
  if ( !this->private_->set_session_data( session_id, generation_numbers ) )
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
  
  // Signal the user interface that a new session is available
  this->sessions_changed_signal_();

  return true;
}


bool Project::delete_session( SessionID session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  std::string error;
  ResultSet result_set;
  std::string sql_str = "SELECT session_id, session_file FROM sessions WHERE session_id = "
    + Core::ExportToString( session_id ) + ";";
  if ( !this->private_->session_database_.run_sql_statement( sql_str, result_set, error ) ||
    result_set.size() == 0 )
  {
    CORE_LOG_ERROR( "Session " + Core::ExportToString( session_id ) + " doesn't exist." );
    return false;
  }

  // Get the project directory path
  boost::filesystem::path project_path( this->project_path_state_->get() );

  // First, try session ID as the session file name
  boost::filesystem::path session_file = project_path / SESSION_DIR_C / 
    ( Core::ExportToString( session_id ) + ".xml" );
  if ( boost::filesystem::exists( session_file ) )
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
  else
  {
    // Otherwise, if there is 'session_file' explicitly stored in the database, try it also
    ResultSet::value_type::iterator it = result_set[ 0 ].find( "session_file" );
    if ( it != result_set[ 0 ].end() )
    {
      std::string session_file_str = boost::any_cast< std::string >( ( *it ).second );
      if ( !session_file_str.empty() )
      {
        session_file = project_path / SESSION_DIR_C / session_file_str;
        if ( boost::filesystem::exists( session_file ) ) 
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
      }
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

  // Signal the user interface that session list has changed
  this->sessions_changed_signal_();

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

void Project::set_project_changed( Core::ActionHandle action, Core::ActionResultHandle result )
{
  // NOTE: This is executed on the application thread, hence we do not need a lock to read
  // the variable that is only changed on the same thread
  
  if ( this->private_->changed_ == false  && action->changes_project_data() )
  {
    // NOTE: Changing the variable
    Core::Application::lock_type lock( Core::Application::GetMutex() );
    this->private_->changed_ = true;
    return;
  }
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
  std::string sql_str = "SELECT * FROM sessions WHERE session_id = " +
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Provenance Database Functionality //////////////////////////////////

// This function is mostly just a placeholder.  Currently it just registers the actions.  We will probably want to 
// create a Providence Object and then add it to the db.
ProvenanceStepID Project::add_provenance_record( const ProvenanceStepHandle& step )
{
  // Print diagnostics
  //step->print();
  
  std::string action_desc = step->get_action();
  std::string user_name = step->get_username();

  // Separate out action name and parameter strings
  std::string action_name, action_params;
  std::string::size_type pos = action_desc.find( ' ' );
  if ( pos != std::string::npos )
  {
    action_name = action_desc.substr( 0, pos );
    action_params = action_desc.substr( pos + 1 );
  }
  else
  {
    action_name = action_desc;
  }

  // Make sure action_params is not empty.
  // NOTE: A non-empty parameter string simplifies the query process 
  if ( action_params.empty() )
  {
    action_params = " ";
  }
    
  ProvenanceIDList output_list = step->get_output_provenance_ids();
  ProvenanceIDList input_list = step->get_input_provenance_ids();
  ProvenanceIDList deleted_list = step->get_deleted_provenance_ids();

  std::string sql_str = "INSERT INTO provenance_step (action_name, action_params, user_id)"
    " VALUES('" + action_name + "', '" + action_params + "', '" + user_name + "');";
  std::string error;
  if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
    return -1;
  }
  ProvenanceStepID step_id = this->private_->provenance_database_.get_last_insert_rowid();

  for ( size_t i = 0; i < input_list.size(); ++i )
  {
    sql_str = "INSERT INTO provenance_input VALUES(" + Core::ExportToString( step_id ) + ", " +
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
    sql_str = "INSERT INTO provenance_output VALUES(" + Core::ExportToString( step_id ) + ", " +
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
    sql_str = "INSERT INTO provenance_deleted VALUES(" + Core::ExportToString( step_id ) + ", " +
      Core::ExportToString( deleted_list[ i ] ) + ");";
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
  // Separate out action name and parameter strings
  std::string action_desc = prov_step->get_action();
  std::string action_params;
  std::string::size_type pos = action_desc.find( ' ' );
  if ( pos != std::string::npos )
  {
    action_params = action_desc.substr( pos + 1 );
  }

  // Make sure action_params is not empty.
  // NOTE: A non-empty parameter string simplifies the query process 
  if ( action_params.empty() )
  {
    action_params = " ";
  }

  std::string sql_str = "UPDATE provenance_step SET action_params = '" + action_params +
    "' WHERE prov_step_id = " + Core::ExportToString( record_id ) + ";";
  std::string error;
  if ( !this->private_->provenance_database_.run_sql_statement( sql_str, error ) )
  {
    CORE_LOG_ERROR( error );
  }
}

bool Project::get_all_sessions( std::vector< SessionInfo >& sessions )
{
  return this->private_->get_all_sessions( sessions );
}

boost::posix_time::ptime Project::get_last_saved_session_time_stamp() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->last_saved_session_time_stamp_;
}

void Project::request_provenance_record( ProvenanceID prov_id )
{
  // Run SQL queries in application thread
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &Project::request_provenance_record, this, prov_id ) );
    return;
  }

  ProvenanceTrail provenance_trail;
  this->private_->query_provenance_trail( prov_id, provenance_trail );

  // Shouldn't have to sort the provenance trail if the record is correct
  //std::sort( provenance_trail.begin(), provenance_trail.end() );

  // TODO: Need to change this to just send a handle

  this->provenance_record_signal_( provenance_trail );
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

int Project::get_version()
{
  return 2;
}

} // end namespace Seg3D

