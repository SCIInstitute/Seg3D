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

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ProjectManager );

ProjectManager::ProjectManager() :
  StateHandler( "projectmanager", false ),
  session_saving_( false ),
  changing_projects_( false )
{ 
  Core::Application::Instance()->get_config_directory( this->local_projectmanager_path_ );
  
  this->recent_projects_database_path_ = local_projectmanager_path_ / "recentprojects.sqlite";
  this->local_projectmanager_path_ = this->local_projectmanager_path_ / "projectmanager.xml";
  
  std::vector< std::string> projects;
  projects.resize( 20, "" );

  this->add_state( "current_project_path", this->current_project_path_state_, 
    PreferencesManager::Instance()->project_path_state_->get() );
  this->add_state( "default_project_name_counter", this->default_project_name_counter_state_, 0 );
  this->add_state( "project_saved", this->project_saved_state_, false );
  this->project_saved_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  try
  {
    boost::filesystem::path path = complete( boost::filesystem::path(
      PreferencesManager::Instance()->project_path_state_->get().c_str(), 
      boost::filesystem::native ) );
    boost::filesystem::create_directory( path );
  }
  catch ( std::exception& e ) 
  {
    CORE_LOG_ERROR( e.what() );
  }
  
  // This constructor is called from the Qt thread, hence state variables can only be set
  // in initializing mode.
  this->set_initializing( true );

  // Here we import the saved project settings from file
  Core::StateIO stateio;
  if( stateio.import_from_file( this->local_projectmanager_path_ ) )
  {
    this->load_states( stateio );
  }
  
  // Here we check to see if the recent projects database exists, otherwise we create it
  if( !boost::filesystem::exists( this->recent_projects_database_path_ ) )
  {
    this->create_database_scheme();
  }
  
  // Here we clean out any projects that dont exist where we think they should
  this->cleanup_recent_projects_database();

  this->set_initializing( false );
    
  this->current_project_ = ProjectHandle( new Project( "untitled_project" ) );
  
  this->set_last_saved_session_time_stamp();
  
  AutoSave::Instance()->start();
}

ProjectManager::~ProjectManager()
{
}


bool ProjectManager::check_if_file_is_valid_project( const boost::filesystem::path& path )
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
  
void ProjectManager::save_projectmanager_state()
{
  Core::StateIO stateio;
  stateio.initialize();
  this->save_states( stateio );
  stateio.export_to_file( this->local_projectmanager_path_ );
}
  

void ProjectManager::new_project( const std::string& project_name, const std::string& project_path, 
  bool save_on_creation )
{
  // Reset the application.
  Core::Application::Reset();
  
  this->project_saved_state_->set( false );
  this->changing_projects_ = true;

  std::vector< std::string > empty_vector;
  this->current_project_->sessions_state_->set( empty_vector );
  this->current_project_->project_notes_state_->set( empty_vector );
  this->current_project_->save_custom_colors_state_->set( false );
  this->current_project_->clear_datamanager_list();
  ToolManager::Instance()->open_default_tools();
  
  if( save_on_creation )
  {
    this->current_project_->project_name_state_->set( project_name );
    
    if( project_name.compare( 0, 11, "New Project" ) == 0 )
    {
      this->default_project_name_counter_state_->set( 
        this->default_project_name_counter_state_->get() + 1 ); 
    }
    
    boost::filesystem::path path = complete( boost::filesystem::path( project_path, 
      boost::filesystem::native ) );

    if( this->create_project_folders( path, this->current_project_->project_name_state_->get() ) )
    {
      boost::filesystem::path path = project_path;
      path = path / project_name;
      this->set_project_path( path );
      this->save_project( true );
      this->set_last_saved_session_time_stamp();
      AutoSave::Instance()->recompute_auto_save();
      this->project_saved_state_->set( true );
    }
  }
  else
  {
    PreferencesManager::Instance()->auto_save_state_->set( false );
  }

  
  this->changing_projects_ = false;

}
  
void ProjectManager::open_project( const boost::filesystem::path& project_path )
{ 
  ASSERT_IS_APPLICATION_THREAD();

  // Reset the application.
  Core::Application::Reset();
  
  this->changing_projects_ = true;
  
  this->set_project_path( project_path );
    
  this->current_project_->initialize_from_file( project_path.leaf() );
  this->add_to_recent_projects( project_path.parent_path().string(), project_path.leaf() );
  
  this->set_last_saved_session_time_stamp();
  this->changing_projects_ = false;

  CORE_LOG_SUCCESS( "Project: '" + this->current_project_->project_name_state_->get()
     + "' has been successfully opened." );
  
  this->set_last_saved_session_time_stamp();
  AutoSave::Instance()->recompute_auto_save();
  
  PreferencesManager::Instance()->export_path_state_->set( project_path.parent_path().string() );

  this->project_saved_state_->set( true );
}
  
bool ProjectManager::save_project( bool autosave /*= false*/, std::string session_name /*= "" */ )
{
  ASSERT_IS_APPLICATION_THREAD();
  this->session_saving_ = true;
  bool save_success = false;
  try
  {
    if( this->save_project_session( autosave, session_name ) )
    {
      if( this->save_project_only( this->current_project_path_state_->get(), 
        this->current_project_->project_name_state_->get() ) )
      {
        save_success = true;
      }
    }
  }
  catch( ... )
  {
    save_success = false;
  } 

  this->session_saving_ = false;
  
  if( save_success )
  {
    if( autosave )
    {
      CORE_LOG_SUCCESS( "Autosave completed successfully for project: '" 
        +  this->current_project_->project_name_state_->get() + "'" );
    }
    else
    {
      CORE_LOG_SUCCESS( "Project: '" + this->current_project_->project_name_state_->get() 
        + "' has been successfully saved" );
    }
  }
  else
  {
    if( autosave )
    {
      CORE_LOG_CRITICAL_ERROR( "Autosave FAILED for project: '" 
        +  this->current_project_->project_name_state_->get() + "'. Please perform a "
        "'Save As' as soon as possible to preserve your data." );
    }
    else
    {
      CORE_LOG_CRITICAL_ERROR( "Save FAILED for project: '" 
        +  this->current_project_->project_name_state_->get() + "'. Please perform a "
        "'Save As' as soon as possible to preserve your data." );
    }
  }
  return save_success;
}
  
bool ProjectManager::export_project( const std::string& export_path, const std::string& project_name, const std::string& session_name )
{
  boost::filesystem::path path = complete( boost::filesystem::path( export_path.c_str(), 
    boost::filesystem::native ) );

  this->create_project_folders( path, project_name );
  this->save_project_only( export_path, project_name );
  return this->current_project_->project_export( path, project_name, session_name );
  
}
  
bool ProjectManager::save_project_session( bool autosave /*= false */, std::string session_name  )
{
  // Here we check to see if its an autosave and if it is, just save over the previous autosave
  if( session_name == "" )
  {
    session_name = "UnnamedSession";
  }
  if( autosave )
  {
    session_name = "AutoSave";
  } 
  session_name = this->get_timestamp() + " - " + session_name;

  std::string user_name;
  Core::Application::Instance()->get_user_name( user_name );

  session_name = session_name + " - " + user_name;
  
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  // This saves the project to the list that appears on the splash screen.
  this->add_to_recent_projects( this->current_project_path_state_->export_to_string(),
    this->current_project_->project_name_state_->get() );
  
  bool result = this->current_project_->save_session( session_name );

  if( result )
  {
    this->set_last_saved_session_time_stamp();
    this->project_saved_state_->set( true );
  }
  else
  {
    this->project_saved_state_->set( false );
  }

  AutoSave::Instance()->recompute_auto_save();

  return result;

}
  
bool ProjectManager::load_project_session( const std::string& session_name )
{
  // Reset the application.
  Core::Application::Reset();

  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );

  bool result =  this->current_project_->load_session( session_name );
  
  if( result )
  {
    this->set_last_saved_session_time_stamp();
  }
  
  return result;
}
  
bool ProjectManager::delete_project_session( const std::string& session_name )
{
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  return this->current_project_->delete_session( session_name );

}
  
void ProjectManager::add_to_recent_projects( const boost::filesystem::path& project_path, 
  const std::string& project_name )
{
  this->insert_recent_projects_entry( project_name, project_path.string(), this->get_timestamp() );
  this->save_projectmanager_state();
}


bool ProjectManager::create_project_folders( const boost::filesystem::path& path, 
  const std::string& project_name )
{
  try // to create a project folder
  {
    boost::filesystem::create_directory( path / project_name );
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    CORE_LOG_ERROR( e.what() );
    return false;
  }
  
  try // to create a project sessions folder
  {
    boost::filesystem::create_directory( path / project_name / "sessions");
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    CORE_LOG_ERROR( e.what() );
    return false;
  }
  
  try // to create a project data folder
  {
    boost::filesystem::create_directory( path / project_name / "data");
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    CORE_LOG_ERROR( e.what() );
    return false;
  }
  
  return true;
}

std::string ProjectManager::get_timestamp()
{
  time_t rawtime;
  struct tm * timeinfo;
  char time_buffer [80];
  
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  
  strftime ( time_buffer, 80, "%d-%b-%Y-%H-%M-%S", timeinfo );
  std::string current_time_stamp = time_buffer;
  return current_time_stamp;
}

void ProjectManager::set_last_saved_session_time_stamp()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  this->last_saved_session_time_stamp_ = boost::posix_time::second_clock::local_time();
}

boost::posix_time::ptime ProjectManager::get_last_saved_session_time_stamp() const
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return this->last_saved_session_time_stamp_;
}

boost::filesystem::path ProjectManager::get_project_data_path() const
{
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  return path / this->current_project_->project_name_state_->get() / "data";
}

void ProjectManager::save_note( const std::string& note )
{
  std::string user_name;
  Core::Application::Instance()->get_user_name( user_name );

  Core::ActionAdd::Dispatch( Core::Interface::GetWidgetActionContext(),
    this->current_project_->project_notes_state_, 
    get_timestamp() + " - " + user_name + "|" + note );

  this->save_project_only( this->current_project_path_state_->get(), 
    this->current_project_->project_name_state_->get() );
}

bool ProjectManager::save_project_only( const std::string& project_path_string, const std::string& project_name )
{
  // When we are using this function as part of exporting a project, we may be saving it with
  // a different name, in that case, we need to temporarily change the project's name before
  // we export it to file and, of course, change it back when we are done.
  std::string current_project_name = this->current_project_->project_name_state_->get();
  bool temporarily_changing_name = false;
  if( project_name != current_project_name )
  {
    temporarily_changing_name = true;
    this->current_project_->set_signal_block( false );
    this->current_project_->project_name_state_->set( project_name );
  }

  boost::filesystem::path project_path( project_path_string );

  project_path = project_path / project_name / ( project_name + ".s3d" );

  Core::StateIO stateio;
  stateio.initialize();
  this->current_project_->save_states( stateio );
  stateio.export_to_file( project_path );
  
  // now if we changed the name, we change it back
  if( temporarily_changing_name )
  {
    this->current_project_->project_name_state_->set( current_project_name );
    this->current_project_->set_signal_block( true );
  }
  return true;
}

double ProjectManager::get_time_since_last_saved_session() const
{
  boost::posix_time::ptime last_save = this->get_last_saved_session_time_stamp();
  boost::posix_time::ptime current_time = boost::posix_time::second_clock::local_time();
  boost::posix_time::time_duration duration = current_time - last_save;
  return static_cast< double >( duration.total_milliseconds() ) * 0.001;
}

bool ProjectManager::is_saving() const
{
  ASSERT_IS_APPLICATION_THREAD();
  return this->session_saving_;
}

void ProjectManager::set_project_path( boost::filesystem::path path )
{
  this->current_project_->set_project_path( path );
  this->current_project_path_state_->set( path.parent_path().string() );
}


Seg3D::ProjectHandle ProjectManager::get_current_project() const
{
  return this->current_project_;
}

bool ProjectManager::project_save_as( const boost::filesystem::path& export_path, 
  const std::string& project_name )
{
  this->changing_projects_ = true;

  this->create_project_folders( export_path, project_name );

  if( this->project_saved_state_->get() == true )
  {
    this->save_project_only( export_path.string(), project_name );
    
    if( !this->current_project_->save_as( export_path, project_name ) ) return false;
    this->set_project_path( export_path / project_name );
    this->current_project_->project_name_state_->set( project_name );
  }
  else
  {
    std::vector< std::string > empty_vector;
    this->current_project_->sessions_state_->set( empty_vector );
    this->set_project_path( export_path / project_name );
    this->current_project_->project_name_state_->set( project_name );
    this->save_project( true );
    this->project_saved_state_->set( true );
  }

  this->set_last_saved_session_time_stamp();

  CORE_LOG_SUCCESS( "'Save As' has been successfully completed." );

  this->set_last_saved_session_time_stamp();
  AutoSave::Instance()->recompute_auto_save();
  
  this->changing_projects_ = true;
  
  this->add_to_recent_projects( export_path, project_name );
  
  if( !this->save_project( true ) )
  {
    CORE_LOG_CRITICAL_ERROR( "'Save As' could not be successfully completed." );
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Recent Files Database Functionality //////////////////////////////////

void ProjectManager::create_database_scheme()
{
  lock_type lock( this->get_mutex() );
  if ( sqlite3_open( this->recent_projects_database_path_.string().c_str(), 
    &this->recent_projects_database_ ) == SQLITE_OK )
  {
    std::string create_query = "CREATE TABLE recentprojects "
      "(id INTEGER NOT NULL, "
      "name VARCHAR(255) NOT NULL, "
      "path VARCHAR(255) NOT NULL, "
      "date VARCHAR(255) NOT NULL, "
      "PRIMARY KEY (id));";

    sqlite3_exec( this->recent_projects_database_, create_query.c_str(), NULL, 0, NULL );
    sqlite3_close( this->recent_projects_database_ ); 
  }
}

bool ProjectManager::insert_recent_projects_entry( const std::string& project_name, 
  const std::string& project_path, const std::string& project_date )
{
  lock_type lock( this->get_mutex() );
  bool success = false;
  if ( sqlite3_open( this->recent_projects_database_path_.string().c_str(), 
    &this->recent_projects_database_ ) == SQLITE_OK )
  {
    const char* tail;
    sqlite3_stmt* statement = NULL;
    std::string delete_statement = "DELETE FROM recentprojects WHERE (name = ?) AND (path = ?)";
    sqlite3_prepare_v2( this->recent_projects_database_, delete_statement.c_str(), 
      static_cast< int >( delete_statement.size() ), &statement, &tail );
    sqlite3_bind_text( statement, 1, project_name.c_str(), 
      static_cast< int >( project_name.size() ), SQLITE_TRANSIENT );
    sqlite3_bind_text( statement, 2, project_path.c_str(), 
      static_cast< int >( project_path.size() ), SQLITE_TRANSIENT );
    success = ( sqlite3_step( statement ) == SQLITE_DONE );
    sqlite3_finalize( statement );

    statement = NULL;
    std::string insert_statement = "INSERT INTO recentprojects (name, path, date) VALUES(?, ?, ?)";
    sqlite3_prepare_v2( this->recent_projects_database_, insert_statement.c_str(), 
      static_cast< int >( insert_statement.size() ), &statement, &tail );
    sqlite3_bind_text( statement, 1, project_name.c_str(), 
      static_cast< int >( project_name.size() ), SQLITE_TRANSIENT );
    sqlite3_bind_text( statement, 2, project_path.c_str(), 
      static_cast< int >( project_path.size() ), SQLITE_TRANSIENT );
    sqlite3_bind_text( statement, 3, project_date.c_str(), 
      static_cast< int >( project_date.size() ), SQLITE_TRANSIENT );
    success = ( sqlite3_step( statement ) == SQLITE_DONE );
    sqlite3_finalize( statement );
    sqlite3_close( this->recent_projects_database_ );
  }

  if( success )
  {
    // if we've successfully added recent projects to our database
    // then we let everyone know things have changed.
    this->recent_projects_changed_signal_();    
  }

  return success;
}

bool ProjectManager::delete_recent_projects_entry( const std::string& project_name, 
  const std::string& project_path, const std::string& project_date )
{
  lock_type lock( this->get_mutex() );
  bool success = false;
  if ( sqlite3_open( this->recent_projects_database_path_.string().c_str(), 
    &this->recent_projects_database_ ) == SQLITE_OK )
  {
    const char* tail;
    sqlite3_stmt* statement = NULL;
    std::string delete_statement = 
      "DELETE FROM recentprojects WHERE (name = ?) AND (path = ?) AND (date = ?)";
    sqlite3_prepare_v2( this->recent_projects_database_, delete_statement.c_str(), 
      static_cast< int >( delete_statement.size() ), &statement, &tail );
    sqlite3_bind_text( statement, 1, project_name.c_str(), 
      static_cast< int >( project_name.size() ), SQLITE_TRANSIENT );
    sqlite3_bind_text( statement, 2, project_path.c_str(), 
      static_cast< int >( project_path.size() ), SQLITE_TRANSIENT );
    sqlite3_bind_text( statement, 3, project_date.c_str(), 
      static_cast< int >( project_date.size() ), SQLITE_TRANSIENT );
    success = ( sqlite3_step( statement ) == SQLITE_DONE );
    sqlite3_finalize( statement );
    sqlite3_close( this->recent_projects_database_ );
  }

  return success;
}


bool ProjectManager::get_recent_projects_from_database( std::vector< RecentProject >& recent_projects )
{
  lock_type lock( this->get_mutex() );
  bool success = false;

  if ( sqlite3_open( this->recent_projects_database_path_.string().c_str(), 
    &this->recent_projects_database_ ) == SQLITE_OK )
  {
    const char* tail;
    sqlite3_stmt* statement = NULL;
    std::string select_statement = "SELECT * FROM recentprojects ORDER BY id DESC";
    sqlite3_prepare_v2( this->recent_projects_database_, select_statement.c_str(), 
      static_cast< int >( select_statement.size() ), &statement, &tail );
    int i = 0;
    while ( sqlite3_step( statement ) == SQLITE_ROW )
    {
      recent_projects.push_back( RecentProject( 
        std::string( (char*)sqlite3_column_text( statement, 1 ) ),
        std::string( (char*)sqlite3_column_text( statement, 2 ) ),
        std::string( (char*)sqlite3_column_text( statement, 3 ) ), 
        std::string( (char*)sqlite3_column_text( statement, 0 ) ) ) );

      success = true;
    } 
    sqlite3_finalize( statement );
    sqlite3_close( this->recent_projects_database_ );
  }
  return success;

}

void ProjectManager::cleanup_recent_projects_database()
{
  std::vector< RecentProject > recent_projects;
  this->get_recent_projects_from_database( recent_projects ); 

  for( size_t i = 0; i < recent_projects.size(); ++i )
  {
    boost::filesystem::path path = 
      recent_projects[ i ].path_;

    path = path / recent_projects[ i ].name_ / ( recent_projects[ i ].name_ + ".s3d" );

    if( !boost::filesystem::exists( path ) )
    {
      this->delete_recent_projects_entry( recent_projects[ i ].name_, 
        recent_projects[ i ].path_, recent_projects[ i ].date_ );
    }
  }
}

} // end namespace seg3D
