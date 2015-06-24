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

// STL includes
#include <ctime>
#include <sstream>

// Core includes
#include <Core/State/StateIO.h>
#include <Core/State/Actions/ActionAdd.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/AutoSave.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/DatabaseManager/DatabaseManager.h>

#include "ApplicationConfiguration.h"

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ProjectManager );

// File name of the project database
static const std::string PROJECT_DATABASE_C( 
  std::string( CORE_APPLICATION_VERSION ) + "_recentprojects.sqlite" );

// File name of the ProjectManager state config file
static const std::string CONFIGURATION_FILE_C( 
  std::string( CORE_APPLICATION_VERSION ) + "_projectmanager.xml" );

// Version number of the project database
static const int PROJECT_DATABASE_VERSION_C = 2;

class ProjectManagerPrivate : public DatabaseManager 
{
public:
  // INITIALIZE_PROJECT_DATABASE:
  // Create the project database.
  bool initialize_project_database();

  // LOAD_OR_CREATE_PROJECT_DATABASE:
  // Load the project database from file if it exists, or create a new one.
  // It also handles converting old version database into the new format.
  bool load_or_create_project_database();

  // SET_CURRENT_PROJECT:
  // Set the current project
  void set_current_project( const ProjectHandle& project );

  // CREATE_PROJECT_DIRECTORY:
  // Create a new directory for a seg3d project
  bool create_project_directory( const std::string& project_location,
    const std::string& project_name, boost::filesystem::path& project_path );

  // CLEANUP_PROJECTS_LIST:
  // this function cleans up projects from the recent projects list that don't exist.
  void cleanup_project_database();
  
  // INSERT_OR_UPDATE_PROJECT_ENTRY:
  // Add a project to the database if it doesn't exist yet, otherwise update
  // its last_access_time to the current time.
  bool insert_or_update_project_entry( const boost::filesystem::path& project_file );

  // RESET_CURRENT_PROJECT_FOLDER:
  // Reset the current project folder variable to the one the preference
  void reset_current_project_folder();

public:
  // public handle to the current project
  ProjectHandle current_project_;

  // Pointer back to project manager class
  ProjectManager* project_manager_;

  // A database of recently loaded projects
  DatabaseManagerHandle project_database_;

  // Path of the project database file
  boost::filesystem::path project_db_file_;
};

bool ProjectManagerPrivate::initialize_project_database()
{
  this->project_database_.reset( new DatabaseManager );
  std::ostringstream sql_statements;

  // Create table for storing the database version
  sql_statements << "CREATE TABLE database_version "
                 << "(version INTEGER NOT NULL PRIMARY KEY);";

  // Create table for storing projects
  sql_statements << "CREATE TABLE project "
                 << "(project_id INTEGER NOT NULL PRIMARY KEY, "
                 << "name TEXT NOT NULL, "
                 << "path TEXT NOT NULL UNIQUE, "
                 << "last_access_time INTEGER NOT NULL);";

  // Set the database version to 2
  sql_statements << "INSERT INTO database_version VALUES ("
                 << Core::ExportToString( PROJECT_DATABASE_VERSION_C ) << ");";

  std::string error;
  if (! this->project_database_->run_sql_script( sql_statements.str(), error ) )
  {
    std::ostringstream oss;
    oss << "Failed to initialize the project database: " << error;
    CORE_LOG_ERROR( oss.str() );
    return false;
  }

  return true;
}

bool ProjectManagerPrivate::load_or_create_project_database()
{
  // Find the directory where user settings are stored
  if ( Core::Application::Instance()->get_config_directory( this->project_db_file_ ) )
  {
    this->project_db_file_ = this->project_db_file_ / PROJECT_DATABASE_C;
  }
  else
  {
    CORE_LOG_ERROR( "Could not access user directory." );
  }

  this->project_database_.reset( new DatabaseManager );
  std::string error;
  if ( boost::filesystem::exists( this->project_db_file_ ) &&
       this->project_database_->load_database( this->project_db_file_, error ) )
  {
    // Check the database version
    long long version = 0;
    std::string sql_str = "SELECT version FROM database_version ORDER BY version DESC LIMIT 1;";
    ResultSet results;
    if ( this->project_database_->run_sql_statement( sql_str, results, error ) && results.size() == 1 )
    {
      try
      {
        version = boost::any_cast< long long >( results[ 0 ][ "version" ] );
      }
      catch ( ... )
      {
        CORE_LOG_DEBUG("Casting SELECT result from database query failed.");
      }
    }

    // If the version is the same as current version
    if ( version == PROJECT_DATABASE_VERSION_C )
    {
      // Clean out any projects that don't exist
      this->cleanup_project_database();
      return true;
    }

    // Otherwise log a warning
    std::ostringstream oss;
    oss << "The project database is not compatible with this version of " <<
      Core::Application::GetApplicationName() << ". A new one will be created.";
    CORE_LOG_WARNING( oss.str() );
  }

  // The project database doesn't exist or isn't compatible
  return this->initialize_project_database();
}

void ProjectManagerPrivate::set_current_project( const ProjectHandle& current_project )
{
  this->current_project_ = current_project;
}

// NOTE: This function is here, so we can create the directory before building the
// actual project
bool ProjectManagerPrivate::create_project_directory( const std::string& project_location,
  const std::string& project_name, boost::filesystem::path& project_path )
{
  // Location where the project needs to be generated
  boost::filesystem::path project_loc( project_location );

  try 
  {
    project_loc = boost::filesystem::absolute( project_loc );
  }
  catch ( ... ) 
  {
    std::ostringstream error;
    error << "Directory '" << project_loc.string() << "' does not exist.";
    CORE_LOG_ERROR( error.str() );
    return false;
  }

  if (! boost::filesystem::exists( project_loc ) )
  {
    std::ostringstream error;
    error << "Directory '" << project_loc.string() << "' does not exist.";
    CORE_LOG_ERROR( error.str() );
    return false;
  }

  // Generate the new name of the directory
  project_path = project_loc / ( project_name + Project::GetDefaultProjectPathExtension() );

  // Check if the directory already existed
  if ( boost::filesystem::exists( project_path ) )
  {
    std::ostringstream error;
    error << "Directory '"  << project_path.string() << "' already exists.";
    CORE_LOG_ERROR( error.str() );
    return false;
  }

  // try to create a project folder
  try   
  {
    boost::filesystem::create_directory( project_path );
  }
  catch ( ... ) // any errors that we might get thrown
  {
    std::ostringstream error;
    error << "Could not create '" << project_path.filename().string() << "'.";
    CORE_LOG_ERROR( error.str() );
    return false;
  }

  // The directory has been made
  return true;
}

void ProjectManagerPrivate::reset_current_project_folder()
{
  this->project_manager_->current_project_folder_state_->set( 
    PreferencesManager::Instance()->project_path_state_->get() );
}

bool ProjectManagerPrivate::insert_or_update_project_entry( const boost::filesystem::path& project_file )
{
  std::ostringstream sql_str;
  sql_str << "SELECT project_id FROM project WHERE path = '" <<
    DatabaseManager::EscapeQuotes( project_file.generic_string() ) << "';";
  ResultSet results;
  std::string error;
  if (! this->project_database_->run_sql_statement( sql_str.str(), results, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  if ( results.size() > 0 )
  {
    // TODO: this code replaces an assert.
    // Test (unit tests) since an assert was found to be necessary.
    if ( results.size() != 1 )
    {
      CORE_LOG_WARNING("One result should be returned from project entry query.");
    }

    long long project_id = boost::any_cast< long long >( results[ 0 ][ "project_id" ] );
    std::ostringstream sql_str_update;
    sql_str_update << "UPDATE project SET last_access_time = strftime('%s', 'now') WHERE project_id = " <<
      Core::ExportToString( project_id ) << ";";
    if (! this->project_database_->run_sql_statement( sql_str_update.str(), error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
  }
  else
  {
    std::ostringstream sql_str_insert;
    sql_str_insert << "INSERT INTO project (name, path, last_access_time) VALUES ('" <<
      DatabaseManager::EscapeQuotes( project_file.stem().string() ) << "', '" <<
      DatabaseManager::EscapeQuotes( project_file.generic_string() ) << "', strftime('%s', 'now'));";
    if (! this->project_database_->run_sql_statement( sql_str_insert.str(), error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
  }
  
  // if we've successfully added recent projects to our database
  // then we let everyone know things have changed.
  this->project_manager_->recent_projects_changed_signal_();
  return true;
}

void ProjectManagerPrivate::cleanup_project_database()
{
  // Get all the entries from the database
  std::string sql_str("SELECT project_id, path FROM project;");
  std::string error;
  ResultSet results;
  if (! this->project_database_->run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return;
  }
  
  // Whether the database has been changed
  bool changed = false;

  // For every entry, check if the project file still exists
  for ( size_t i = 0; i < results.size(); ++i )
  {
    bool exists = false;
    try
    {
      std::string file_str = boost::any_cast< std::string >( results[ i ][ "path" ] );
      exists = boost::filesystem::exists( file_str ) && boost::filesystem::is_regular_file( file_str );
    }
    catch( ... )
    {
      CORE_LOG_DEBUG("Verifying project file existence failed.");
    }

    if (! exists )
    {
      try
      {
        long long project_id = boost::any_cast< long long >( results[ i ][ "project_id" ] );
        std::ostringstream sql_str;
        sql_str << "DELETE FROM project WHERE project_id = " << Core::ExportToString( project_id ) << ";";
        if (! this->project_database_->run_sql_statement( sql_str.str(), error ) )
        {
          CORE_LOG_ERROR( error );
        }
        else
        {
          changed = true;
        }
      }
      catch( ... )
      {
        CORE_LOG_DEBUG("Project database cleanup failed.");
      }
    }
  }
  
  // If the database was changed, save it out
  if ( changed && !this->project_database_->save_database( this->project_db_file_, error ) )
  {
    CORE_LOG_ERROR( error );
  }
}

//////////////////////////////////////////////////////////


ProjectManager::ProjectManager() :
  StateHandler( "projectmanager", false ),
  private_( new ProjectManagerPrivate )
{ 
  // Ensure we have a pointer back for the private functions
  this->private_->project_manager_ = this;

  // NOTE: This constructor is called from the Qt thread, hence state variables can only be set
  // in initializing mode.
  this->set_initializing( true );

  // Add state variables
  // This variable keeps track of the 'New Project' names
  this->add_state( "default_project_name_counter", this->default_project_name_counter_state_, 0 );
  // This state variable keeps track of what the last directory was that we
  // used for saving/loading projects
  this->add_state( "current_project_folder", this->current_project_folder_state_, "" );

  // This state variable keeps track of what the last directory was that we
  // used for saving/loading layer files
  this->add_state( "current_file_folder", this->current_file_folder_state_, "" );

  // Find the local configuration directory
  boost::filesystem::path configuration_dir;
  Core::Application::Instance()->get_config_directory( configuration_dir );

  // Update the states from the configuration file
  Core::StateIO stateio;
  if ( stateio.import_from_file( configuration_dir / CONFIGURATION_FILE_C ) ||
    stateio.import_from_file( configuration_dir / "projectmanager.xml" ) )
  {
    this->load_states( stateio ); 
  }
  
  // Check whether directory exists and whether it can be used, if not update it to something
  // useful. The next function will go over a number of potential directories and finds the
  // one that it can use.
  boost::filesystem::path current_project_folder = this->get_current_project_folder();

  boost::filesystem::path current_file_folder = this->get_current_file_folder();

  // Update the current_project_folder to the most recent version 
  this->current_project_folder_state_->set( current_project_folder.string() );
  this->current_file_folder_state_->set( current_file_folder.string() );
  
  // Here we check to see if the recent projects database exists, otherwise we create it
  this->private_->load_or_create_project_database();
  
  // Create a new project
  this->private_->current_project_ = ProjectHandle( new Project( std::string( "Untitled Project" ) ) ); 

  // Start the auto save thread
  AutoSave::Instance()->start();

  PreferencesManager::Instance()->project_path_state_->state_changed_signal_.connect(
    boost::bind( &ProjectManagerPrivate::reset_current_project_folder, this->private_ ) );

  this->set_initializing( false );
}


ProjectManager::~ProjectManager()
{
}


boost::filesystem::path ProjectManager::get_current_project_folder()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() ); 

  std::string current_project_folder_string = this->current_project_folder_state_->get(); 

  if ( current_project_folder_string.empty() )
  {
    current_project_folder_string = PreferencesManager::Instance()->project_path_state_->get();     
  }

  boost::filesystem::path current_project_folder( current_project_folder_string );
  try
  {
    // Complete the path to have an absolute path
    current_project_folder = boost::filesystem::absolute( current_project_folder );
  }
  catch ( ... )
  {
    std::ostringstream oss;
    oss << "Converting '" << current_project_folder_string << "' to absolute path failed.";
    CORE_LOG_DEBUG( oss.str() );
}

  // If it does not exist reset the path to another path
  if (! boost::filesystem::exists( current_project_folder ) )
  {
    current_project_folder = boost::filesystem::path(
      PreferencesManager::Instance()->project_path_state_->get() );

    // Try the user directory
    if (! boost::filesystem::exists( current_project_folder ) )
    {
      Core::Application::Instance()->get_user_directory( current_project_folder );
      
      current_project_folder = current_project_folder / 
        ( Core::Application::GetApplicationName() + "-Projects" );
      // Try current working path
      if (! boost::filesystem::exists( current_project_folder ) )
      {     
        current_project_folder = current_project_folder.parent_path();
      }
    }     
  }
  
  return current_project_folder;
}


boost::filesystem::path ProjectManager::get_current_file_folder()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() ); 

  boost::filesystem::path current_file_folder( this->current_file_folder_state_->get() );
  try
  {
    // Complete the path to have an absolute path
    current_file_folder = boost::filesystem::absolute( current_file_folder );
  }
  catch ( ... )
  {
    std::ostringstream oss;
    oss << "Converting '" << current_file_folder.string() << "' to absolute path failed.";
    CORE_LOG_DEBUG( oss.str() );
  }

  // If it does not exist reset the path to another path
  if (! boost::filesystem::exists( current_file_folder ) )
  {
    Core::Application::Instance()->get_user_directory( current_file_folder );
      
    // Try current working path
    if (! boost::filesystem::exists( current_file_folder ) )
    {
      current_file_folder = current_file_folder.parent_path();
    }
  }
  
  return current_file_folder;
}


bool ProjectManager::new_project( const std::string& project_location,
  const std::string& project_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  boost::filesystem::path project_path;

  // If a project location is given, make the directory and check whether it is a new empty
  // directory.
  if (! project_location.empty() )
  {
    if (! this->private_->create_project_directory( project_location, project_name,
      project_path ) )
    {
      return false;
    }
  }
  
  // Reset the application.
  Core::Application::Reset();

  // Ensure that the next project default name will be increased by 1.
  if ( project_name.substr( 0, 11 ) == "New Project" )
  {
    this->default_project_name_counter_state_->set( 
      this->default_project_name_counter_state_->get() + 1 ); 
  }

  // Generate a new project
  this->private_->set_current_project( ProjectHandle( new Project( project_name ) ) );
  
  // Open the default tools
  ToolManager::Instance()->open_default_tools();
  
  if (! project_location.empty() )
  {
    if (! this->get_current_project()->save_project( project_path, project_name, false ) )
    {
      // Need to update this so the old project
      this->current_project_changed_signal_();
      // An error should have been logged by the save_project function. 
      return false;
    }

    // Update recent file list, the current folder and save them to disk
    this->checkpoint_projectmanager();

    // Make sure auto save is recomputed
    AutoSave::Instance()->recompute_auto_save();  
  }

  // NOTE: Even if the project location is empty (Quick Open), it is not necessary (or safe) 
  // to turn off autosave.  Autosave will not run if it sees an empty project location, even
  // if autosave is enabled.

  this->current_project_changed_signal_();

  
  return true;
}

  
bool ProjectManager::open_project( const std::string& project_file )
{ 
  // This function sets state variables directly, hence we need to be on the application thread.
  ASSERT_IS_APPLICATION_THREAD();
  
  // Try to resolve the filename before we reset the application.
  boost::filesystem::path full_filename( project_file );
  
  try 
  {
    full_filename = boost::filesystem::absolute( full_filename );
  }
  catch( ... ) 
  {
    std::ostringstream error;
    error << "Converting '" << project_file << "' to absolute path failed.";
    CORE_LOG_ERROR( error.str() );
    return false;
  }
  
  // Reset the application.
  Core::Application::Reset();

  // Create a new project, the old one will go out of scope as soon as 
  // the GUI releases the old project
  ProjectHandle new_proj;
  bool succeeded = true;
  try
  {
    new_proj.reset( new Project( full_filename ) );
  }
  catch ( ... )
  {
    new_proj.reset( new Project( std::string( "Untitled Project" ) ) );
    succeeded = false;
  }

  this->private_->set_current_project( new_proj );

  if ( succeeded )
  {
    succeeded = new_proj->load_last_session();
    if (! succeeded )
    {
      this->private_->set_current_project( ProjectHandle( 
        new Project( std::string( "Untitled Project" ) ) ) );
    }
  }

  if ( succeeded )
  {
    // Reset the auto save system
    AutoSave::Instance()->recompute_auto_save();

    // Update recent file list, the current folder and save them to disk
    this->checkpoint_projectmanager();
  }
  
  // TODO: Need to fix the widget, so it connects to the state variables correctly
  this->current_project_changed_signal_();

  return succeeded;
}
  
  
bool ProjectManager::save_project_as( const std::string& project_location, 
  const std::string& project_name, bool anonymize )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();
  
  if ( project_name.substr( 0, 12 ) == "New Project " )
  {
    int number;
    if ( Core::ImportFromString( project_name.substr( 12 ), number ) )
    {
      int default_name_count = 
        ProjectManager::Instance()->default_project_name_counter_state_->get();
      if ( number >= default_name_count )
      {
        ProjectManager::Instance()->default_project_name_counter_state_->set( number + 1 );
      }
    }
  }
  
  // Create a new directory for the project
  boost::filesystem::path project_path;
  if (! this->private_->create_project_directory( project_location, project_name, project_path ) )
  {
    return false;
  }

  if (! this->get_current_project()->save_project( project_path, project_name, anonymize ) )
  {
    // An error should have been logged by the save_project function. 
    return false;
  }

  // Ensure it is not auto saving a project with no new data
  this->get_current_project()->reset_project_changed();

  // Reset the auto save system
  AutoSave::Instance()->recompute_auto_save();
  
  // Update recent file list, the current folder and save them to disk
  this->checkpoint_projectmanager();

  return true;
}

  
bool ProjectManager::export_project( const std::string& export_path, 
                  const std::string& project_name, long long session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Create a new directory for the project
  boost::filesystem::path project_path;
  if (! this->private_->create_project_directory( export_path, project_name, project_path ) )
  {
    return false;
  }

  if (! this->get_current_project()->export_project( project_path, project_name, session_id ) )
  {
    return false;
  }

  return true;
}


bool ProjectManager::save_project_session( const std::string& session_name  )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  if ( this->get_current_project()->save_session( session_name ) )
  {
    // Ensure it is not auto saving a project with no new data
    this->get_current_project()->reset_project_changed();

    // Recompute when auto save needs to happen
    AutoSave::Instance()->recompute_auto_save();
    return true;
  }
  
  return false;
}


bool ProjectManager::load_project_session( long long session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Reset the application.
  Core::Application::Reset();

  // Load the session
  if ( this->get_current_project()->load_session( session_id ) )
  {
    // Recompute when auto save needs to happen
    AutoSave::Instance()->recompute_auto_save();
    return true;
  }
  
  // Reset the application.
  Core::Application::Reset();
  return false;
}
  

bool ProjectManager::delete_project_session( long long session_id )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();
  
  return this->get_current_project()->delete_session( session_id );
}


ProjectHandle ProjectManager::get_current_project() const
{
  return this->private_->current_project_;
}


void ProjectManager::checkpoint_projectmanager()
{
  ProjectHandle current_project = this->get_current_project();
  
  boost::filesystem::path project_path( current_project->project_path_state_->get() );
  boost::filesystem::path project_file = project_path / current_project->project_file_state_->get();
  this->private_->insert_or_update_project_entry( project_file );

  this->current_project_folder_state_->set( project_path.parent_path().string() );

  // Get local configuration directory
  boost::filesystem::path configuration_dir;
  Core::Application::Instance()->get_config_directory( configuration_dir );
      
  // Write out the XML file
  Core::StateIO stateio;
  stateio.initialize();
  this->save_states( stateio );
    
  if ( boost::filesystem::exists( configuration_dir ) )
  {
    stateio.export_to_file( configuration_dir / CONFIGURATION_FILE_C );
  }
    
  // Write out the database
  std::string error;
  if (! this->private_->project_database_->save_database( this->private_->project_db_file_, error ) )
  {
    // Just log the error if things do not work
    CORE_LOG_ERROR( error );
  }
}

bool ProjectManager::get_recent_projects( ProjectInfoList& recent_projects )
{
  // Clean up the project database first
  this->private_->cleanup_project_database();

  std::string sql_str("SELECT * FROM project ORDER BY last_access_time DESC LIMIT 20");
  std::string error;
  ResultSet results;
  if (! this->private_->project_database_->run_sql_statement( sql_str, results, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  for ( size_t i = 0; i < results.size(); ++i )
  {
    std::string name = boost::any_cast< std::string >( results[ i ][ "name" ] );
    boost::filesystem::path path( boost::any_cast< std::string >( results[ i ][ "path" ] ) );
    time_t last_access_time = static_cast< time_t >( boost::any_cast< long long >( 
      results[ i ][ "last_access_time" ] ) );
    recent_projects.push_back( ProjectInfo( name, path,
      boost::posix_time::from_time_t( last_access_time ) ) );
  }
  
  return true;
}

//////////////////////////////////////////////

bool ProjectManager::CheckProjectFile( const boost::filesystem::path& path, std::string& error )
{
  Core::StateIO stateio;

  // Check whether the XML file can be imported
  if (! stateio.import_from_file( path, error ) )
  {
    return false;
  }
    
  // Check whether the version is equal or lower to the program version
  if ( stateio.get_major_version() > Core::Application::GetMajorVersion() )
  {
    error = "This project was saved with a newer version of Seg3D.";
    return false;
  }
  
  // Check the minor version
  if ( stateio.get_major_version() == Core::Application::GetMajorVersion() )
  {
    if ( stateio.get_minor_version() > Core::Application::GetMinorVersion() )
    {
      error = "This project was saved with a newer version of Seg3D.";
      return false;
    }
  }

  // Everything seems OK
  return true;
}

} // end namespace Seg3D