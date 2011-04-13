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
#include <vector>
#include <set>

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>
#include <Core/DataBlock/DataBlockManager.h>

// Application includes
#include <Application/Project/Project.h>
#include <Application/Provenance/Provenance.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/DatabaseManager/DatabaseManager.h>

// Include CMake generated files
#include "ApplicationConfiguration.h"

namespace Seg3D
{

class ProjectPrivate : public DatabaseManager 
{
  // -- constructors/destructors --
public:
  ProjectPrivate() :
    project_( 0 ),
    changed_( false ),
    database_initialized_( false ),
    last_saved_session_time_stamp_( boost::posix_time::second_clock::local_time() )
  { 
    // This will create the default tables
    this->setup_database();
  }

  // -- internal variables --
public:
  // Pointer back to the project
  Project* project_;

  // Where the data is being managed
  DataManagerHandle data_manager_;
  
  // Whether the project has changed
  bool changed_;
  
  // Whether the database was constructed
  bool database_initialized_;
  
  // Time when last session was saved for auto save functionality.
  boost::posix_time::ptime last_saved_session_time_stamp_;
  
  // Data files, this one is filled while saving a session by the various components
  std::vector<std::string> data_files_;

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
  
  // IMPORT_OLD_SESSION_INFO_INTO_DATABASE:
  // this function is for backwards compatibility with older versions of seg3d that store the
  // list of sessions in the project xml file rather than the database
  void import_old_session_info_into_database();

  // CLEANUP_SESSION_DATABASE:
  // this function cleans up sessions in the session list that have been deleted by the user
  void cleanup_session_database();  
  
  // SAVE_STATE:
  // Save the state of the current project into the xml file and save the database
  // in the database file
  bool save_state( const boost::filesystem::path& project_directory );  
  
  // COPY_ALL_FILES:
  // Copy all files from one directory into another directory
  bool copy_all_files( const boost::filesystem::path& src_path, 
    const boost::filesystem::path& dst_path, bool copy_s3d_file = false );
    
  // DELETE_UNUSED_DATA_FILES
  // Clean up files that it knows it does not need anymore
  void delete_unused_data_files();  
  
  // SET_LAST_SAVED_SESSION_TIME_STAMP
  // this function updates the time of when the last session was saved
  void set_last_saved_session_time_stamp();
  
  // GET_MOST_RECENT_SESSION:
  // this function true or false based on whether it was able to find the most recent
  // session name
  bool get_most_recent_session_name( std::string& session_name );
  
  // SETUP_DATABASE:
  // Setup the tables in the database
  bool setup_database(); 
  
  // INSERT_SESSION_INTO_DATABASE:
  // this inserts a session into the database
  bool insert_session_into_database( const std::string& timestamp, 
    const std::string& session_name, const std::string& user_name );
  
  // INSERT_SESSION_DATA_FILES_INTO_DATABASE:
  // this inserts the datafiles associated with a session into the database
  bool insert_session_data_files_into_database( const std::string& session_name, 
    const std::vector<std::string>& data_files );
  
  // DELETE_SESSION_FROM_DATABASE:
  // this deletes a session from the database
  bool delete_session_from_database( const std::string& session_name );
  
  // PROVENANCE_RECURSIVE_HELPER
  // this is a recursive function that gets the provenance trail out of the database for a particular provenance id 
  void provenance_recursive_helper( 
    std::vector< std::pair< ProvenanceID, std::string > >& provenance_trail, 
    std::set<ProvenanceID>& provenance_ids, ProvenanceID prov_id );
  
  bool get_all_sessions( std::vector< SessionInfo >& sessions );

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
          
          FolderInfo&  finder_info = *reinterpret_cast<FolderInfo*>( &info.finderInfo );
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

  
  if ( ! boost::filesystem::exists( project_directory / "sessions" ) )
  {
    try // to create a project sessions folder
    {
      boost::filesystem::create_directory( project_directory / "sessions" );
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
  
  if ( ! boost::filesystem::exists( project_directory / "database" ) )
  {         
    try // to create a folder with the provenance data base
    {
      boost::filesystem::create_directory( project_directory / "database" );
    }
    catch ( ... )
    {
      std::string error = 
        std::string( "Could not create database directory in project folder '" ) + 
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


void ProjectPrivate::import_old_session_info_into_database()
{
  // The session names were stored in the session_state variable.
  // Hence we grab them all
  std::vector< std::string > session_vector = this->project_->sessions_state_->get();

  boost::filesystem::path project_path = this->project_->project_path_state_->get();

  int number_of_sessions_to_import = static_cast< int >( session_vector.size() );
  for( int i = ( number_of_sessions_to_import - 1 ); i >= 0; i-- )
  {
    // Get the old session name
    std::string old_session_name;
    Core::ImportFromString( session_vector[ i ], old_session_name );
    
    boost::filesystem::path old_path = project_path / "sessions" / 
      ( old_session_name + ".xml" );
    
    std::vector< std::string > old_session_name_vector = 
      Core::SplitString( old_session_name, " - " );
      
    // Ensure it is a valid name  
    if ( old_session_name_vector.size() < 3 ) continue;
    
    
    // Generate the new name, e.g. 0001-MyNewSession.xml
    std::string session_count = Core::ExportToString( 
      static_cast<unsigned int>( this->project_->session_count_state_->get() ), 4 );    
    std::string new_session_name = session_count + "-" + old_session_name_vector[ 1 ];
  
    // Generate the new path of where the session is located
    boost::filesystem::path new_path = project_path / "sessions" / ( new_session_name + ".xml" );
      
    // If the old one does not exist, just continue - we should not fail -
    if( ! boost::filesystem::exists( old_path ) ) continue; 
    try 
    {
      // Try to rename the session
      boost::filesystem::rename( old_path, new_path );
    }
    catch ( ... )
    {
      std::string error = std::string( "Could not rename file '"  ) + old_path.string() +
        "'.";
      CORE_LOG_ERROR( error );
      // NOTE: We will continue as more of the project could be salved potentially
      continue;
    }
    
    // Parse the time stamp out
    std::string day = ( Core::SplitString( old_session_name_vector[ 0 ], "-" ) )[ 0 ];
    std::string month = ( Core::SplitString( old_session_name_vector[ 0 ], "-" ) )[ 1 ];
    std::string year = ( Core::SplitString( old_session_name_vector[ 0 ], "-" ) )[ 2 ];
    
    std::string hour = ( Core::SplitString( old_session_name_vector[ 0 ], "-" ) )[ 3 ];
    std::string minute = ( Core::SplitString( old_session_name_vector[ 0 ], "-" ) )[ 4 ];
    std::string second = ( Core::SplitString( old_session_name_vector[ 0 ], "-" ) )[ 5 ];
    
    std::string timestamp = year + "-" + month + "-" + day + " " + hour + ":" + 
      minute + ":" + second;
    
    // Here we insert the converted session name into the database
    this->insert_session_into_database( timestamp, new_session_name, 
      old_session_name_vector[ 2 ] );
    
    // Now if it had data associated with it, put it in the database too.
    std::vector< std::string > files;
    if( this->data_manager_->get_session_files_vector( old_session_name, files ) )
    {
      for( size_t j = 0; j < files.size(); ++j )
      {
        std::string insert_statement = "INSERT OR IGNORE INTO data_relations "
          "(session_name, data_file) VALUES('" + new_session_name + 
          "', '" + files[ j ] + "');";

        std::string error;
        if( !this->run_sql_statement( insert_statement, error ) )
        {
          CORE_LOG_ERROR( error );
          // NOTE: Either way continuing or giving up is not great.
          // NOTE: We will continue as more of the project could potentially be salvaged 
          continue;
        }
      }
    }
  }
  
  // Finally we set the session list to an empty vector so we aren't tempted to do this again.
  // This variable remains empty and unused from this point onward. 
  this->project_->sessions_state_->clear();
  
  this->project_->save_state();
}

void ProjectPrivate::cleanup_session_database()
{
  // If no files have been generated, just ignore the cleanup
  if ( ! this->project_->project_files_generated_state_->get() ) return;

  // We need to cleanup the database once a while, to remove none existing sessions
  std::vector< SessionInfo > sessions;
  if( this->get_all_sessions( sessions ) )
  {
    for( size_t i = 0; i < sessions.size(); ++i )
    {
      boost::filesystem::path session_file = 
        boost::filesystem::path ( this->project_->project_path_state_->get() ) / "sessions" 
        / ( sessions[ i ].session_name_ + ".xml" );
        
      // Ensure the file exists 
      if( !boost::filesystem::exists( session_file ) )
      {
        // File no longer exists, hence take it out of the database
        this->delete_session_from_database( sessions[ i ].session_name_ );
      }
    }
  }
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
  boost::filesystem::path project_database = project_directory / 
    "database" / "project_database.sqlite";
  
  std::string error;
  if ( ! this->save_database( project_database, error ) )
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


void ProjectPrivate::delete_unused_data_files()
{
  std::string error;
  ResultSet result_set;
  std::string select_statement = "SELECT DISTINCT data_file FROM data_relations;";

  if( !this->run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return;
  }

  if ( result_set.size() == 0 )
  {
    return;
  }
  
  boost::filesystem::path project_path( this->project_->project_path_state_->get() );
  boost::filesystem::path data_path = project_path / "data";
  
  std::vector< std::string > existing_data_files;
  
  boost::filesystem::directory_iterator dir_end;
  for( boost::filesystem::directory_iterator dir_itr( data_path ); dir_itr != dir_end; ++dir_itr )
  {
    existing_data_files.push_back( dir_itr->path().filename().string() );
  }
  
  for( size_t j = 0; j < existing_data_files.size(); j++ )
  {
    bool found = false;
    for( size_t i = 0; i < result_set.size(); ++i )
    {
      if( boost::any_cast< std::string >( ( result_set[ i ] )[ "data_file" ] ) == 
        existing_data_files[ j ] ) 
      {
        found = true;
        break;  
      }
    }
    if( !found )
    {
      try 
      {
        boost::filesystem::remove( data_path / existing_data_files[ j ] );
      }
      catch ( ... )
      {
        std::string error = std::string( "Could delete file '" ) +
          existing_data_files[ j ] + "'.";
        CORE_LOG_ERROR( error );
        return;
      }
    }
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


bool ProjectPrivate::get_most_recent_session_name( std::string& session_name )
{
  std::string error;
  ResultSet result_set;
  std::string select_statement = "SELECT * FROM sessions ORDER BY session_id DESC;";
  if( !this->run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( result_set.size() )
  {
    session_name = boost::any_cast< std::string >( ( result_set[ 0 ] )[ "session_name" ] );
    return true;
  }

  return false;
}


bool ProjectPrivate::setup_database()
{
  std::vector< std::string > create_table_commands;
  create_table_commands.push_back(
    // this table will store the actual provenance step
    "CREATE TABLE provenance_step "
    "(provenance_id INTEGER NOT NULL, "
    "action TEXT NOT NULL, "
    "timestamp TEXT NOT NULL, "
    "user TEXT NOT NULL, "
    "output_number INTEGER NOT NULL, "
    "PRIMARY KEY (provenance_id));" );
    
  create_table_commands.push_back(
    // this is a table relating the input provenance_ids to the output
    "CREATE TABLE provenance_input_relations "
    "(id INTEGER NOT NULL, "
    "provenance_id INTEGER NOT NULL, "
    "input_provenance_id INTEGER NOT NULL, "
    "PRIMARY KEY (id));" );
    
  create_table_commands.push_back(
    // this is a table relating the input provenance_ids to the output
    "CREATE TABLE provenance_output_relations "
    "(id INTEGER NOT NULL, "
    "provenance_id INTEGER NOT NULL, "
    "output_provenance_id INTEGER NOT NULL, "
    "PRIMARY KEY (id));" );
    
  create_table_commands.push_back(
    // this is a table relating the input provenance_ids to the output
    "CREATE TABLE provenance_deleted_relations "
    "(id INTEGER NOT NULL, "
    "provenance_id INTEGER NOT NULL, "
    "deleted_provenance_id INTEGER NOT NULL, "
    "PRIMARY KEY (id));" );
    
  create_table_commands.push_back(
    // this table represents the actions that cause new provenance_id's
    "CREATE TABLE actions "
    "(id INTEGER NOT NULL, "
    "action_id TEXT NOT NULL, "
    "number_of_inputs INTEGER NOT NULL, "
    "number_of_outputs INTEGER NOT NULL, "
    "PRIMARY KEY (id), "
    "UNIQUE (action_id));" );
    
  create_table_commands.push_back(
    // this is a table relating settings to actions
    "CREATE TABLE action_settings "
    "(setting_id INTEGER NOT NULL, "
    "action_id TEXT NOT NULL, "
    "setting_name TEXT NOT NULL, "
    "setting_value TEXT NOT NULL, "
    "PRIMARY KEY (setting_id));" );
    
  create_table_commands.push_back(
    // this table represents the actions that cause new provenance_id's
    "CREATE TABLE sessions "
    "(session_id INTEGER NOT NULL, "
    "session_name TEXT NOT NULL, "
    "username TEXT NOT NULL, "
    "timestamp TEXT NOT NULL, "
    "UNIQUE (session_name),"
    "UNIQUE (timestamp),"
    "PRIMARY KEY (session_id));" );

  create_table_commands.push_back(
    // this is a table relating the input provenance_ids to the output
    "CREATE TABLE data_relations "
    "(id INTEGER NOT NULL, "
    "session_name TEXT NOT NULL, "
    "data_file TEXT NOT NULL, "
    "UNIQUE(session_name, data_file),"
    "PRIMARY KEY (id));" );
    
  std::string error;
  if( this->create_database( create_table_commands, error ) )
  {
    this->database_initialized_ = true;
    return true;
  }
  
  CORE_LOG_ERROR( error );
  return false;
}


bool ProjectPrivate::insert_session_into_database( const std::string& timestamp, 
  const std::string& session_name, const std::string& user_name  )
{
  std::string insert_statement = "INSERT OR IGNORE INTO sessions "
    "(session_name, username, timestamp) VALUES('" + 
    session_name+ "', '" + user_name + "', '" + timestamp + "');";

  std::string error;
  if( !this->run_sql_statement( insert_statement, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  this->project_->session_count_state_->set( this->project_->session_count_state_->get() + 1 );
  return true;
}


bool ProjectPrivate::insert_session_data_files_into_database( const std::string& session_name, 
  const std::vector<std::string>& data_files  )
{
  for( size_t i = 0; i < data_files.size(); i++ )
  {
    std::string insert_statement = "INSERT OR IGNORE INTO data_relations "
      "(session_name, data_file) VALUES('" + session_name + 
      "', '" + data_files[ i ] +  "');";

    std::string error;
    this->run_sql_statement( insert_statement, error );
  }
  
  return true;
}



bool ProjectPrivate::delete_session_from_database( const std::string& session_name )
{
  std::string delete_statement = "DELETE FROM sessions WHERE (session_name = '" + 
    session_name + "');";

  std::string error;
  if( !this->run_sql_statement( delete_statement, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  delete_statement = "DELETE FROM data_relations WHERE (session_name = '" + 
    session_name + "');";

  if( !this->run_sql_statement( delete_statement, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

void ProjectPrivate::provenance_recursive_helper( 
    std::vector< std::pair< ProvenanceID, std::string > >& provenance_trail, 
    std::set<ProvenanceID>& provenance_ids, ProvenanceID prov_id )
  
{
  if( prov_id == -1 )
  {
    return;
  }
  else
  {
    ResultSet step_result_set;
    std::string error;
    std::string select_statement = "SELECT action FROM provenance_step WHERE provenance_id =" + 
      Core::ExportToString( prov_id ) + " ;";
    if( !this->run_sql_statement( select_statement, step_result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return;
    }

    std::string action = boost::any_cast< std::string >( ( step_result_set[ 0 ] )[ "action" ] );
    provenance_trail.push_back( std::make_pair( prov_id, action ) );
    provenance_ids.insert( prov_id );

    ResultSet source_result_set;
    select_statement = 
      "SELECT input_provenance_id FROM provenance_input_relations WHERE provenance_id =" + 
      Core::ExportToString( prov_id ) + " ;";
    if( !this->run_sql_statement( select_statement, source_result_set, error ) )
    {
      CORE_LOG_ERROR( error );
      return;
    }

    for( size_t i = 0; i < source_result_set.size(); ++i )
    {
      ProvenanceID new_prov_id = 
        boost::any_cast< long long >( ( source_result_set[ i ] )[ "input_provenance_id" ] );
      
      if ( provenance_ids.find( new_prov_id ) == provenance_ids.end() )
      {
        this->provenance_recursive_helper( provenance_trail, provenance_ids, new_prov_id );
      }
    }
  }
}

bool ProjectPrivate::get_all_sessions( std::vector< SessionInfo >& sessions )
{
  std::string error;

  ResultSet result_set;
  std::string select_statement = "SELECT * FROM sessions ORDER BY session_id DESC;";
  if( !this->run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  for( size_t i = 0; i < result_set.size(); ++i )
  {
    sessions.push_back( SessionInfo( 
      boost::any_cast< std::string >( ( result_set[ i ] )[ "session_name" ] ),
      boost::any_cast< std::string >( ( result_set[ i ] )[ "username" ] ),
      boost::any_cast< std::string >( ( result_set[ i ] )[ "timestamp" ] ) ) );
  }

  return true;
}


//////////////////////////////////////////////////////////

Project::Project( const std::string& project_name ) :
  StateHandler( "project", true ),
  private_( new ProjectPrivate )
{ 
  // Mark this class so it does not add the state id number into the project file itself
  this->do_not_save_id_number();
  
  // Keep a pointer to this class in the private class
  this->private_->project_ = this;

  // Name of the project
  this->add_state( "project_name", this->project_name_state_, project_name );

  // File that contains project information
  std::string project_file = project_name + Project::GetDefaultProjectFileExtension();
  this->add_state( "project_file", this->project_file_state_, project_file );

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
  this->add_state( "current_session_name", this->current_session_name_state_, "NewSession" );

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

  // Count that keeps track of datablock generation numbers
  this->add_state( "provenance_count", this->provenance_count_state_, -1 );
  
  this->private_->data_manager_ = DataManagerHandle( new DataManager() );

  // Each time an action is execute, check whether it changes the project data, if so
  // mark this in the project, so the UI can query the user for a save action if the application
  // is closed while there is unsaved data.
  this->add_connection( Core::ActionDispatcher::Instance()->post_action_signal_.connect( 
    boost::bind( &Project::set_project_changed, this, _1, _2 ) ) );
}
  
Project::~Project()
{
  // Remove all active connections
  this->disconnect_all();
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
    std::string error = std::string( "Could not open project file '" ) + project_file.string() + 
      "'.";
    CORE_LOG_ERROR( error );

    return false; 
  }
  
  if ( !this->load_states( stateio ) )  
  {
    // We could not load the file, hence send a message to the user
    std::string error = std::string( "Could not read project file '" ) + project_file.string() + 
      "'.";
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
    
    FolderInfo&  finder_info = *reinterpret_cast<FolderInfo*>( &info.finderInfo );
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

  // The name of the project
  this->project_name_state_->set( full_filename.stem().string() );


  // The file in which the project was saved
  this->project_file_state_->set( full_filename.filename().string() );


  // Ensure all directories that we require are present, if not this function
  // will generate them.
  if ( ! this->private_->update_project_directory( full_filename.parent_path() ) )
  {
    return false;
  }
  
  // Grab the name from the .s3d file. This name is currently the only
  // indicator of the project name. Hence if the user renames this file, the
  // project name will change as well.
  this->project_path_state_->set( full_filename.parent_path().string() );

  // Since we load this one from disk, it obviously exists on disk
  this->project_files_generated_state_->set( true );
  this->project_files_accessible_state_->set( true );

  // Update the information in the database
  boost::filesystem::path database_file = full_filename.parent_path() /
    "database" / "project_database.sqlite";

  if ( boost::filesystem::exists( database_file ) )
  {
    std::string error;
    if ( ! this->private_->load_database( database_file, error ) )
    {
      CORE_LOG_ERROR( error );
    }
  }
  
  //////////////////// Backwards compatibility //////////////////////
  // NOTE: We need this code for backwards compatibility
  if( !this->sessions_state_->get().empty() )
  {
    // Import the old session information
    this->session_count_state_->set( 0 );
    this->private_->import_old_session_info_into_database();
  }
  //////////////////////////////////////////////////////////////////


  // Once we have loaded the state of the sessions_list, 
  // we need to validate that the files exist. As the user
  // may have editted files and may have removed them, we
  // check all the files
  this->private_->cleanup_session_database();

  std::string most_recent_session_name;
  if( this->private_->get_most_recent_session_name( most_recent_session_name ) && 
    this->load_session( most_recent_session_name ) )
  {
    this->private_->update_project_size();
    this->sessions_changed_signal_();
    return true;
  }
  
  
  
  // We could not load the file, hence send a message to the user
  std::string error = std::string( "Could not open session '" ) +
    most_recent_session_name + "'.";
  CORE_LOG_ERROR( error );

  return false;
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
  return project_path / "sessions";
}


boost::filesystem::path Project::get_project_database_path() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  boost::filesystem::path project_path( this->project_path_state_->get() );
  return project_path / "database";
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
  const std::string& project_name, const std::string& session_name )
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


  // TODO: This code neeeds to be redone, as it currently copies all the files
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
    boost::filesystem::copy_file( ( project_path / "sessions" / ( session_name + ".xml" ) ),
      ( export_path / "sessions" / ( session_name + ".xml" ) ) );
  }
  catch ( ... ) // any errors that we might get thrown
  {
    std::string error = std::string( "Could not copy file '" ) +
      ( project_path / "sessions" / ( session_name + ".xml" ) ).string() + "'.";
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
  if ( !boost::filesystem::exists( project_path / ( this->project_file_state_->get()) ) )
  {
    //this->project_files_generated_state_->set( false );
    this->project_files_accessible_state_->set( false );
    return false;   
  }

  // Check if s3d file still exists
  if ( !boost::filesystem::exists( project_path / "database" / "project_database.sqlite" ) )
  {
    //this->project_files_generated_state_->set( false );
    this->project_files_accessible_state_->set( false );
    return false;   
  }

  this->project_files_accessible_state_->set( true );
  return true;
}


bool Project::load_session( const std::string& session_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Get the project directory path
  boost::filesystem::path project_path( this->project_path_state_->get() );

  // Tell program that project does not yet need to be saved.
  this->reset_project_changed();
  
  Core::StateIO state_io;
  boost::filesystem::path session_file = project_path / "sessions" / ( session_name + ".xml" );
  if ( !state_io.import_from_file( session_file ) )
  {
    std::string error = std::string( "Failed to read session file '" ) + session_file.string() +
      "'.";
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


bool Project::save_session( std::string session_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Update the session name if needed
  if ( session_name.empty() ) session_name = this->current_session_name_state_->get();

  // Figure out where to save the data
  boost::filesystem::path project_path( this->project_path_state_->get() );

  // Generate the session name as it is written to disk
  std::string session_count = Core::ExportToString( 
    static_cast<unsigned int>( this->session_count_state_->get() ), 4 );
  std::string full_session_name = session_count + "-" + session_name; 
    
  
  // This vector will be filled by the session saving algorithm
  this->private_->data_files_.clear();
      
  Core::StateIO state_io;
  state_io.initialize();
  boost::filesystem::path session_path = project_path / "sessions";
  if ( ! Core::StateEngine::Instance()->save_states( state_io ) )
  {
    std::string error = "Could not extract all the session information from the project.";
    CORE_LOG_ERROR( error );
    return false;
  }   
    
  if ( ! state_io.export_to_file( session_path / ( full_session_name + ".xml" ) ) )
  {
    std::string error = std::string( "Could not save session file '" ) + 
      full_session_name + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }
    
  // Get the current time
  std::string time_stamp = boost::posix_time::to_simple_string( 
    boost::posix_time::second_clock::local_time() );
  
  std::string user_name;
  Core::Application::Instance()->get_user_name( user_name );  
  
  if( !this->private_->insert_session_into_database( time_stamp, full_session_name, user_name ) )
  {
    // The insert function should report the error encountered
    return false;
  }

  if( !this->private_->insert_session_data_files_into_database( full_session_name, 
    this->private_->data_files_ ) )
  {
    // The insert function should report the error encountered
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


bool Project::delete_session( const std::string& session_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Get the project directory path
  boost::filesystem::path project_path( this->project_path_state_->get() );

  // Get the name of the session
  boost::filesystem::path session_path = project_path / "sessions" / ( session_name + ".xml" );

  try 
  {
    boost::filesystem::remove( session_path );
  }
  catch(  ... ) 
  {
    std::string error = std::string( "Could not remove file '" ) + session_path.string() +
      "'.";
    CORE_LOG_ERROR( error );
    return false;
  }

  // Delete the file from the database
  this->private_->delete_session_from_database( session_name );
  
  // Remove redundand files from the project
  this->private_->delete_unused_data_files();

  // Save the state of the project to disk
  this->save_state();

  // Update the size of the project
  this->private_->update_project_size();

  // Signal the user interface that a new session is available
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
  this->private_->data_manager_->initialize( project_path );

  // We need to restore provenance and generation count for the project

  Core::DataBlock::generation_type generation = this->generation_count_state_->get();

  //////////////////// BACKWARDS COMPATIBILITY /////////////////////////////
  // NOTE: Because an earlier version mistakenly did not save this number, it may be initialized
  // to -1, in that case we need to derive it from the actual generation numbers stored in
  // the data directory.  
  boost::filesystem::path data_path = project_path / "data";
  if( boost::filesystem::exists( data_path ) )
  {
    boost::filesystem::directory_iterator dir_end;
    for( boost::filesystem::directory_iterator dir_itr( data_path ); 
      dir_itr != dir_end; ++dir_itr )
    {
      if ( boost::filesystem::extension( *dir_itr ) == ".nrrd" )
      {
        Core::DataBlock::generation_type file_generation = -1;
        if ( Core::ImportFromString( boost::filesystem::basename( dir_itr->path().filename().string() ),
          file_generation ) )
        {
          if ( file_generation > generation ) generation = file_generation;
        }
      }
    }

    // Ensure the counter will have at least 0
    if ( generation == -1 ) generation = 0;
    generation++;
  }
  /////////////////////////////////////////////////////////////////////////
  
  // Read the generation count from file
  Core::DataBlockManager::Instance()->set_generation_count( generation );

  ProvenanceID provenance_id = this->provenance_count_state_->get();

  // Read the provenance count from file
  SetProvenanceCount( provenance_id );

  return true;
}


bool Project::is_session( const std::string& session_name )
{
  SessionInfo session;
  if( this->get_session( session, session_name ) )
  {
    if( session.session_name_ != "" )
    {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Provenance Database Functionality //////////////////////////////////

// This function is mostly just a placeholder.  Currently it just registers the actions.  We will probably want to 
// create a Providence Object and then add it to the db.
bool Project::add_to_provenance_database( ProvenanceStepHandle& step )
{
  // Print diagnostics
  //step->print();
  
  std::string action_desc = step->get_action();
  std::string user_name = step->get_username();
  std::string timestamp = step->get_timestamp();
    
  ProvenanceIDList output_list = step->get_output_provenance_ids();
  ProvenanceIDList input_list = step->get_input_provenance_ids();
  ProvenanceIDList deleted_list = step->get_deleted_provenance_ids();
  
  std::string insert_statement;
  
  for( size_t i = 0; i < output_list.size(); ++i )
  {
    insert_statement = 
      "INSERT INTO provenance_step (provenance_id, action, timestamp, user, output_number) VALUES(" + 
      Core::ExportToString( output_list[ i ] )+ ", '" + action_desc + "', '" + 
      timestamp + "', '" + user_name + "', " + Core::ExportToString( i+1 )+ ");";
    
    std::string error;
    if( !this->private_->run_sql_statement( insert_statement, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
    
    for( size_t j = 0; j < input_list.size(); ++j )
    {
      insert_statement = "INSERT INTO provenance_input_relations (provenance_id, input_provenance_id) VALUES(" + 
        Core::ExportToString( output_list[ i ] ) + ", " +
        Core::ExportToString( input_list[ j ] ) + ");";

      if( !this->private_->run_sql_statement( insert_statement, error ) )
      {
        CORE_LOG_ERROR( error );
        return false;
      }
    }

    for( size_t j = 0; j < output_list.size(); ++j )
    {
      insert_statement = "INSERT INTO provenance_output_relations (provenance_id, output_provenance_id) VALUES(" + 
        Core::ExportToString( output_list[ i ] ) + ", " +
        Core::ExportToString( output_list[ j ] ) + ");";

      if( !this->private_->run_sql_statement( insert_statement, error ) )
      {
        CORE_LOG_ERROR( error );
        return false;
      }
    }

    for( size_t j = 0; j < deleted_list.size(); ++j )
    {
      insert_statement = "INSERT INTO provenance_deleted_relations "
        "(provenance_id, deleted_provenance_id) VALUES(" + 
        Core::ExportToString( output_list[ i ] ) + ", " +
        Core::ExportToString( deleted_list[ j ] ) + ");";

      if ( !this->private_->run_sql_statement( insert_statement, error ) )
      {
        CORE_LOG_ERROR( error );
        return false;     
      }
    } 
    
  }
  return true;  
}


bool Project::get_all_sessions( std::vector< SessionInfo >& sessions )
{
  // lets clean up any sessions that dont exist first
  this->private_->cleanup_session_database();
  return this->private_->get_all_sessions( sessions );
}

bool Project::get_session( SessionInfo& session, const std::string& session_name )
{
  std::string error;
  
  ResultSet result_set;
  std::string select_statement = "SELECT * FROM sessions WHERE session_name ='" + 
    session_name + "';";
    
  if( !this->private_->run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( result_set.size() == 0 )
  {
    CORE_LOG_ERROR( "Session '" + session_name + "' not found." );
    return false;
  }
  
  session = SessionInfo( 
    boost::any_cast< std::string >( ( result_set[ 0 ] )[ "session_name" ] ),
    boost::any_cast< std::string >( ( result_set[ 0 ] )[ "username" ] ),
    boost::any_cast< std::string >( ( result_set[ 0 ] )[ "timestamp" ] ) );

  return true;
}

void Project::add_data_file( const std::string& data_file )
{
  // Only add the file if it is not already there
  if ( std::find( this->private_->data_files_.begin(), this->private_->data_files_.end(), 
    data_file ) == this->private_->data_files_.end() )
  {
    this->private_->data_files_.push_back( data_file );
  }
}


boost::posix_time::ptime Project::get_last_saved_session_time_stamp() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->private_->last_saved_session_time_stamp_;
}

void Project::request_signal_provenance_record( ProvenanceID prov_id )
{
  std::vector< std::pair< ProvenanceID, std::string > > provenance_trail;
  std::set< ProvenanceID > provenance_ids;
  this->private_->provenance_recursive_helper( provenance_trail, provenance_ids, prov_id );


  std::sort( provenance_trail.begin(), provenance_trail.end() );

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

} // end namespace Seg3D
