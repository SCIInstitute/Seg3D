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
  
  std::vector< std::string> projects;
  projects.resize( 20, "" );

  this->add_state( "recent_projects", this->recent_projects_state_, projects );
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

  Core::StateIO stateio;
  if ( stateio.import_from_file( this->local_projectmanager_path_ / "projectmanager.xml" ) )
  {
    this->load_states( stateio );
  }
  
  this->cleanup_projects_list();

  this->set_initializing( false );
    
  this->current_project_ = ProjectHandle( new Project( "untitled_project" ) );

  // Connect the signals from the LayerManager to the GUI
  this->add_connection( this->current_project_->project_name_state_->value_changed_signal_.connect( 
    boost::bind( &ProjectManager::rename_project, this, _1, _2 ) ) );
  
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
  stateio.export_to_file( this->local_projectmanager_path_ / "projectmanager.xml" );
}
  
void ProjectManager::rename_project( const std::string& new_name, Core::ActionSource source )
{

  // If this is the first time the name has been set, then we set it, and return
  if( !this->current_project_->is_valid() )
  {
    this->current_project_->set_valid( true );
    return;
  }

  // return if rename gets called while we are in the middle of changing a project
  if( changing_projects_ )
  {
    return;
  }
  
  std::vector< std::string > old_name_vector = 
    Core::SplitString( this->recent_projects_state_->get()[ 0 ], "|" );
  
  // If the vector made from the old name, is less than 2, then we return
  if( old_name_vector.size() < 2 )
    return;

  std::string old_name = old_name_vector[ 1 ];

  // If the new name is the same as the old name we return
  if( old_name == new_name )
    return;
  
  // If the old name is empty then something is wrong and we return
  if( old_name == "" )
  {
    return;
  }
  
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  try
  {
    boost::filesystem::rename( ( path / old_name ), ( path / new_name ) );
    boost::filesystem::rename( ( path / new_name / ( old_name + ".s3d" ) ), 
      ( path / new_name / ( new_name + ".s3d" ) ) );
  }
  catch ( std::exception& e ) 
  {
    CORE_LOG_ERROR( e.what() );
  }
  
  this->current_project_->project_name_state_->set( new_name );
  
  std::vector< std::string > temp_projects_vector = this->recent_projects_state_->get();

  // first we are going to remove this project from the list if its in there.
  for( size_t i = 0; i < temp_projects_vector.size(); ++i )
  {
    if( temp_projects_vector[ i ] != "" )
    {
      std::string from_project_list = ( ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 0 ] 
        + "|" + ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 1 ] );
      std::string from_path_and_name = ( path.string() + "|" + old_name );
      
      if( from_project_list == from_path_and_name )
      {
        temp_projects_vector.erase( temp_projects_vector.begin() + i );
      }
    }
  }
  this->recent_projects_state_->set( temp_projects_vector );
  
  if( this->save_project_only( this->current_project_path_state_->get(), new_name ) )
  {
    this->add_to_recent_projects( this->current_project_path_state_->get(), new_name );
    
    //this->current_project_->set_project_path( path / new_name );
    this->set_project_path( path / new_name );

    CORE_LOG_SUCCESS( "The project name has been successfully changed to: '" + new_name  + "'" );
  }
  else
  {
    CORE_LOG_ERROR(  
      "There has been a problem setting the name of the project to: '" + new_name  + "'" );
  }
  
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
  
void ProjectManager::open_project( const std::string& project_path )
{ 
  ASSERT_IS_APPLICATION_THREAD();

  // Reset the application.
  Core::Application::Reset();
  
  this->changing_projects_ = true;
  boost::filesystem::path path = project_path;
  
  this->set_project_path( path );
    
  this->current_project_->initialize_from_file( path.leaf() );
  this->add_to_recent_projects( path.parent_path().string(), path.leaf() );

  this->set_last_saved_session_time_stamp();
  this->changing_projects_ = false;

  CORE_LOG_SUCCESS( "Project: '" + this->current_project_->project_name_state_->get()
     + "' has been successfully opened." );
  
  this->set_last_saved_session_time_stamp();
  AutoSave::Instance()->recompute_auto_save();

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
      CORE_LOG_ERROR( "Autosave FAILED for project: '" 
        +  this->current_project_->project_name_state_->get() + "'" );
    }
    else
    {
      CORE_LOG_ERROR( "Save FAILED for project: '" 
        +  this->current_project_->project_name_state_->get() + "'" );
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
  
void ProjectManager::add_to_recent_projects( const std::string& project_path, 
  const std::string& project_name )
{
  std::vector< std::string > temp_projects_vector = this->recent_projects_state_->get();
  
  // first we are going to remove this project from the list if its in there.
  for( size_t i = 0; i < temp_projects_vector.size(); ++i )
  {
    if( temp_projects_vector[ i ] != "" )
    {
      if( ( ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 0 ] + "|" +
        ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 1 ] )
        == ( project_path + "|" + project_name ) )
      {
        temp_projects_vector.erase( temp_projects_vector.begin() + i );
        break;
      }
    }
  }
  
  // now we add id to the beginning of the list
  temp_projects_vector.insert( temp_projects_vector.begin(), 
    ( project_path + "|" + project_name + "|" + this->get_timestamp() ) );
  temp_projects_vector.resize( 20 );
  
  this->recent_projects_state_->set( temp_projects_vector );
  this->save_projectmanager_state();
}

void ProjectManager::cleanup_projects_list()
{
  std::vector< std::string > projects_vector = this->recent_projects_state_->get();
  std::vector< std::string > new_projects_vector;

  for( int i = 0; i < static_cast< int >( projects_vector.size() ); ++i )
  {
    if( ( projects_vector[ i ] != "" ) && ( projects_vector[ i ] != "]" ) )
    {
      std::vector< std::string > single_project_vector = 
        Core::SplitString( projects_vector[ i ], "|" );
            
            std::string filepath;
            Core::ImportFromString( single_project_vector[ 0 ], filepath );

      boost::filesystem::path path = complete( boost::filesystem::path( 
        filepath.c_str(), boost::filesystem::native ) );

      boost::filesystem::path project_path = path / single_project_vector[ 1 ] 
        / ( single_project_vector[ 1 ] + ".s3d" );

      if( boost::filesystem::exists( project_path ) )
      {
        new_projects_vector.push_back( projects_vector[ i ] );
      }
    }
  }
  this->recent_projects_state_->set( new_projects_vector );
}

  
bool ProjectManager::create_project_folders( boost::filesystem::path& path, const std::string& project_name )
{
  //std::string project_name = this->current_project_->project_name_state_->get();
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

bool ProjectManager::project_save_as( const std::string& export_path, const std::string& project_name )
{
  this->changing_projects_ = true;
  
  boost::filesystem::path path = complete( boost::filesystem::path( export_path.c_str(), 
    boost::filesystem::native ) );

  this->create_project_folders( path, project_name );
  
  
  if( this->project_saved_state_->get() == true )
  {
    this->save_project_only( export_path, project_name );
    
    if( !this->current_project_->save_as( path, project_name ) ) return false;
    this->set_project_path( path / project_name );
    this->current_project_->project_name_state_->set( project_name );
  }
  else
  {
    this->set_project_path( path / project_name );
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
  
  return true;
}




} // end namespace seg3D
