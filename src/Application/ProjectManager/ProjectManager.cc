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

// STL includes
#include <time.h>

// Boost includes
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/State/StateIO.h>
#include <Core/State/Actions/ActionAdd.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/AutoSave.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/DatabaseManager/DatabaseManager.h>


namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ProjectManager );


class ProjectManagerPrivate : public DatabaseManager 
{
public:
  // SET_CURRENT_PROJECT:
  // Set the current project
  void set_current_project( const ProjectHandle& project );

  // CREATE_PROJECT_DIRECTORY:
  // Create a new directory for a seg3d project
  bool create_project_directory( const std::string& project_location,
    const std::string& project_name, boost::filesystem::path& project_path );

  // CLEANUP_PROJECTS_LIST:
  // this function cleans up projects from the recent projects list that don't exist.
  void cleanup_recent_projects_database();
     
  // SETUP_DATABASE:
  // Create a new database 
  bool setup_database();
  
  // INSERT_RECENT_PROJECTS_ENTRY:
  // Add a recent project to the database
  bool insert_recent_projects_entry( const std::string& project_name, 
    const std::string& project_path, const std::string& project_date );
    
  // DELETE_RECENT_PROJECTS_ENTRY:
  // Remove a recent project from the database
  bool delete_recent_projects_entry( const std::string& project_name, 
    const std::string& project_path, const std::string& project_date );

  // RESET_CURRENT_PROJECT_FOLDER:
  // Reset the current project folder variable to the one the preference
  void reset_current_project_folder();

  // GET_RECENT_PROJECTS_FROM_DATABASE:
  // gets projects from database
  bool get_recent_projects_from_database( std::vector< RecentProject >& recent_projects );

public:
  // public handle to the current project
  ProjectHandle current_project_;

  // Pointer back to project manager class
  ProjectManager* project_manager_;
};

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
    std::string error = std::string( "Directory '" ) + project_loc.string() +
      "' does not exist.";
    CORE_LOG_ERROR( error );
    return false;
  }

  if ( ! boost::filesystem::exists( project_loc ) )
  {
    std::string error = std::string( "Directory '" ) + project_loc.string() +
      "' does not exist.";
    CORE_LOG_ERROR( error );
    return false;
  }

  // Generate the new name of the directory
  project_path = project_loc / ( project_name + Project::GetDefaultProjectPathExtension() );

  // Check if the directory already existed
  if ( boost::filesystem::exists( project_path ) )
  {
    std::string error = std::string( "Directory '" ) + project_path.string() +
      "' already exists.";
    CORE_LOG_ERROR( error );
    return false;
  }

  // try to create a project folder
  try   
  {
    boost::filesystem::create_directory( project_path );
  }
  catch ( ... ) // any errors that we might get thrown
  {
    std::string error = std::string( "Could not create '" ) + project_path.filename().string()
      + "'.";
    CORE_LOG_ERROR( error );
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



bool ProjectManagerPrivate::setup_database()
{
  // Find the directory where user settings are stored
  boost::filesystem::path recent_project_database_file;
  Core::Application::Instance()->get_config_directory( recent_project_database_file );
  recent_project_database_file = recent_project_database_file / "recentprojects.sqlite";
  
  bool read_database = false;

  // Try to load the database first
  if ( boost::filesystem::exists( recent_project_database_file ) )
  {
    std::string error;
    if ( !this->load_database( recent_project_database_file, error ) )
    {
      CORE_LOG_ERROR( error );
      // We will generate a new database
    }
    else
    {
      // Successfully read the database
      read_database = true;
    }
  }
  
  // If we could not load it generate it from scratch
  if ( ! read_database )
  {
    std::vector< std::string > create_statements( 1, 
      "CREATE TABLE recentprojects "
      "(id INTEGER NOT NULL, "
      "name VARCHAR(255) NOT NULL, "
      "path VARCHAR(255) NOT NULL, "
      "date VARCHAR(255) NOT NULL, "
      "PRIMARY KEY (id));" );
  
    std::string error;
    if ( ! this->create_database( create_statements, error ) )
    {
      CORE_LOG_ERROR( error );
      return false;
    }
  }
  
  // Done
  return true;
}


bool ProjectManagerPrivate::insert_recent_projects_entry( const std::string& project_name, 
  const std::string& project_path, const std::string& project_date )
{
  std::string error;

  // First delete the old entry
  std::string delete_statement = "DELETE FROM recentprojects WHERE (name = '" + project_name
    + "') AND (path = '" + project_path + "')";
  
  // Run the statement to actually delete the entry
  this->run_sql_statement( delete_statement, error );
      
  // Now insert the new one 
  std::string insert_statement = "INSERT INTO recentprojects (name, path, date) "
    "VALUES('" + project_name + "', '" + project_path + "', '" + project_date + "')";
    
  if( this->run_sql_statement( insert_statement, error ) )
  {
    // if we've successfully added recent projects to our database
    // then we let everyone know things have changed.
    this->project_manager_->recent_projects_changed_signal_();
    return true;    
  } 
  else
  {
    CORE_LOG_ERROR( error );
    return false;
  }   
}


bool ProjectManagerPrivate::delete_recent_projects_entry( const std::string& project_name, 
  const std::string& project_path, const std::string& project_date )
{
  // Delete an entry from the database
  std::string delete_statement = "DELETE FROM recentprojects WHERE (name = '" + project_name
    + "') AND (path = '" + project_path + "')";

  std::string error;
  this->run_sql_statement( delete_statement, error );
  this->project_manager_->recent_projects_changed_signal_();
  
  return true;
}


void ProjectManagerPrivate::cleanup_recent_projects_database()
{
  // Get all the entries in the current database
  std::vector< RecentProject > recent_projects;
  this->get_recent_projects_from_database( recent_projects ); 

  // Check the entries

  std::vector<std::string> file_extensions = Project::GetProjectFileExtensions();

  for( size_t i = 0; i < recent_projects.size(); ++i )
  {
    bool found_project = false;
    
    for ( size_t j = 0; j < file_extensions.size(); j++ )
    {
  
      // Check whether the current one exists, if not we need to delete it from the database
      boost::filesystem::path path = boost::filesystem::path( recent_projects[ i ].path_ ) / 
        ( recent_projects[ i ].name_ + file_extensions[ j ] );

      if( boost::filesystem::exists( path ) ) 
      {
        found_project = true;
        break;
      }
    }
    

    if( !found_project )
    {
      this->delete_recent_projects_entry( recent_projects[ i ].name_, 
        recent_projects[ i ].path_, recent_projects[ i ].date_ );

      // Check if it by any chance is an older version where we assumed that data directory is the
      // same name as the top project name

      ///////// BACKWARDS COMPATIBILITY /////////////////////////
      boost::filesystem::path path = boost::filesystem::path( recent_projects[ i ].path_ ) / 
        recent_projects[ i ].name_ / ( recent_projects[ i ].name_ + ".s3d" );


      if ( boost::filesystem::exists( path ) )
      {
        path = boost::filesystem::path( recent_projects[ i ].path_ ) / 
          recent_projects[ i ].name_;
        this->insert_recent_projects_entry( recent_projects[ i ].name_, 
          path.string(), recent_projects[ i ].date_ );
      }
      ///////////////////////////////////////////////////////////
    }
  }

  // Find the directory where user settings are stored
  boost::filesystem::path recent_project_database_file;
  Core::Application::Instance()->get_config_directory( recent_project_database_file );
  recent_project_database_file = recent_project_database_file / "recentprojects.sqlite";

  std::string error;
  if ( ! this->save_database( recent_project_database_file, error ) )
  {
    // Just log the error if things do not work
    CORE_LOG_ERROR( error );
  }
}

bool ProjectManagerPrivate::get_recent_projects_from_database( 
  std::vector< RecentProject >& recent_projects )
{
  ResultSet result_set;
  std::string select_statement = "SELECT * FROM recentprojects ORDER BY id DESC LIMIT 20";

  // Get the 20 latest entries into this database
  std::string error;
  if( !this->run_sql_statement( select_statement, result_set, error ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }

  // Get the information from the database
  for( size_t i = 0; i < result_set.size(); ++i )
  {
    recent_projects.push_back( RecentProject( 
      boost::any_cast< std::string >( ( result_set[ i ] )[ "name" ] ),
      boost::any_cast< std::string >( ( result_set[ i ] )[ "path" ] ),
      boost::any_cast< std::string >( ( result_set[ i ] )[ "date" ] ), 
      static_cast< int >( boost::any_cast< long long >( ( result_set[ i ] )[ "id" ] ) ) ) );
  }

  return true;
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

  // Find the local configuration directory
  boost::filesystem::path configuration_dir;
  Core::Application::Instance()->get_config_directory( configuration_dir );

  // Update the states from the configuration file
  Core::StateIO stateio;
  if( stateio.import_from_file( configuration_dir / "projectmanager.xml" ) )
  {
    this->load_states( stateio );
  }
  
  // Check whether directory exists and whether it can be used, if not update it to something
  // useful. The next function will go over a number of potential directories and finds the
  // one that it can use.
  boost::filesystem::path current_project_folder = this->get_current_project_folder();

  // Update the current_project_folder to the most recent version 
  this->current_project_folder_state_->set( current_project_folder.string() );
  
  // Here we check to see if the recent projects database exists, otherwise we create it
  this->private_->setup_database();
  
  // Here we clean out any projects that dont exist where we think they should
  this->private_->cleanup_recent_projects_database();
  
  // Create a new project
  this->private_->current_project_ = ProjectHandle( new Project( "Untitled Project" ) );  

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

  boost::filesystem::path current_project_folder( this->current_project_folder_state_->get() );
  try
  {
    // Complete the path to have an absolute path
    current_project_folder = boost::filesystem::absolute( current_project_folder );
  }
  catch ( ... )
  {
  }

  // If it does not exist reset the path to another path
  if ( !boost::filesystem::exists( current_project_folder ) )
  {
    current_project_folder = boost::filesystem::path(
      PreferencesManager::Instance()->project_path_state_->get() );
          
    // Try the user directory
    if ( !boost::filesystem::exists( current_project_folder ) )
    {
      Core::Application::Instance()->get_user_directory( current_project_folder );
      
      current_project_folder = current_project_folder / "Seg3d-Projects";
      // Try current working path
      if ( !boost::filesystem::exists( current_project_folder ) )
      {
        current_project_folder = current_project_folder.parent_path();
      }
    }     
  }
  
  return current_project_folder;
}


bool ProjectManager::new_project( const std::string& project_location, 
  const std::string& project_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  boost::filesystem::path project_path;

  // If a project location is given, make the directory and check whether it is a new empty
  // directory.
  if ( !project_location.empty() )
  {
    if ( ! this->private_->create_project_directory( project_location, project_name, project_path ) )
    {
      return false;
    }
  }
  
  // Reset the application.
  Core::Application::Reset();

  // Ensure that the next project default name will be increase by 1.
  if( project_name.compare( 0, 11, "New Project" ) == 0 )
  {
    this->default_project_name_counter_state_->set( 
      this->default_project_name_counter_state_->get() + 1 ); 
  }

  // Generate a new project
  this->private_->set_current_project( ProjectHandle( new Project( project_name ) ) );
  
  // Open the default tools
  ToolManager::Instance()->open_default_tools();
  
  if ( !project_location.empty() )
  {
    if ( !this->get_current_project()->save_project( project_path, project_name ) )
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
  else
  {
    // Switch off auto save, as the project is not on disk one cannot save automatically.
    PreferencesManager::Instance()->auto_save_state_->set( false );
  }

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
    std::string error = std::string( "Could resolve filename '" ) + project_file + "'.";
    CORE_LOG_ERROR( error );
    return false;
  }
  
  // Reset the application.
  Core::Application::Reset();

  // Create a new project, the old one will go out of scope as soon as the GUI releases the old
  // project
  this->private_->set_current_project( ProjectHandle( new Project( "Untitled Project" ) ) );
  if ( ! this->get_current_project()->load_project( full_filename ) )
  {
    // TODO: Need to fix the widget, so it connects to the state variables correctly
    this->current_project_changed_signal_();
    return false;
  }

  // Reset the auto save system
  AutoSave::Instance()->recompute_auto_save();

  // Update recent file list, the current folder and save them to disk
  this->checkpoint_projectmanager();
  
  this->current_project_changed_signal_();

  return true;
}
  
  
bool ProjectManager::save_project_as( const std::string& project_location, 
  const std::string& project_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();
  
  // Create a new directory for the project
  boost::filesystem::path project_path;
  if ( ! this->private_->create_project_directory( project_location, project_name, project_path ) )
  {
    return false;
  }

  if ( !this->get_current_project()->save_project( project_path, project_name ) )
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

  
bool ProjectManager::export_project( const std::string& project_location, 
  const std::string& project_name, const std::string& session_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Create a new directory for the project
  boost::filesystem::path project_path;
  if ( ! this->private_->create_project_directory( project_location, project_name, project_path ) )
  {
    return false;
  }

  if ( !this->get_current_project()->export_project( project_path, project_name, session_name ) )
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


bool ProjectManager::load_project_session( const std::string& session_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();

  // Reset the application.
  Core::Application::Reset();

  // Load the session
  if ( this->get_current_project()->load_session( session_name ) )
  {
    // Recompute when auto save needs to happen
    AutoSave::Instance()->recompute_auto_save();
    return true;
  }
  
  return false;
}
  

bool ProjectManager::delete_project_session( const std::string& session_name )
{
  // This function sets state variables directly, hence we need to be on the application thread
  ASSERT_IS_APPLICATION_THREAD();
  
  return this->get_current_project()->delete_session( session_name );
}


ProjectHandle ProjectManager::get_current_project() const
{
  return this->private_->current_project_;
}


void ProjectManager::checkpoint_projectmanager()
{
  ProjectHandle current_project = this->get_current_project();
  
  std::string time_stamp = boost::posix_time::to_simple_string( 
    boost::posix_time::second_clock::local_time() );
  this->private_->insert_recent_projects_entry( current_project->project_name_state_->get(), 
    current_project->project_path_state_->get(), time_stamp );

  boost::filesystem::path project_path( current_project->project_path_state_->get() );
  this->current_project_folder_state_->set( project_path.parent_path().string() );

  // Get local configuration directory
  boost::filesystem::path configuration_dir;
  Core::Application::Instance()->get_config_directory( configuration_dir );
      
  // Write out the XML file
  Core::StateIO stateio;
  stateio.initialize();
  this->save_states( stateio );
  stateio.export_to_file( configuration_dir / "projectmanager.xml"  );
  
  // Write out the database

  // Find the directory where user settings are stored
  boost::filesystem::path recent_project_database_file;
  Core::Application::Instance()->get_config_directory( recent_project_database_file );
  recent_project_database_file = recent_project_database_file / "recentprojects.sqlite";

  std::string error;
  if ( ! this->private_->save_database( recent_project_database_file, error ) )
  {
    // Just log the error if things do not work
    CORE_LOG_ERROR( error );
  }
}


bool ProjectManager::get_recent_projects_from_database( 
  std::vector< RecentProject >& recent_projects )
{
  this->private_->cleanup_recent_projects_database();
  return this->private_->get_recent_projects_from_database( recent_projects );
}

//////////////////////////////////////////////

bool ProjectManager::CheckProjectFile( const boost::filesystem::path& path )
{
  Core::StateIO stateio;

  // Check whether the XML file can be imported
  if ( ! stateio.import_from_file( path ) ) return false;
  
  // Check whether the version is equal or lower to the program version
  if ( stateio.get_major_version() > Core::Application::GetMajorVersion() )
  {
    return false;
  }
  
  // Check the minor version
  if ( stateio.get_major_version() == Core::Application::GetMajorVersion() )
  {
    if ( stateio.get_minor_version() > Core::Application::GetMinorVersion() )
    {
      return false;
    }
  }

  // Everything seems OK
  return true;
}

} // end namespace seg3D
