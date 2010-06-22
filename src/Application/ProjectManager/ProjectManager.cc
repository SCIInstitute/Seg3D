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

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>



namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ProjectManager );

ProjectManager::ProjectManager() :
  StateHandler( "projectmanager", false ),
  auto_save_timer_( 10 )
{ 
  Core::Application::Instance()->get_config_directory( this->local_projectmanager_path_ );
  
  std::vector< std::string> projects;
  projects.resize( 20, "" );

  add_state( "recent_projects", this->recent_projects_state_, projects );
  add_state( "current_project_path", this->current_project_path_state_, 
    PreferencesManager::Instance()->project_path_state_->get() );
  add_state( "default_project_name_counter", this->default_project_name_counter_state_, 0 );
  add_state( "auto_save", auto_save_state_,
    PreferencesManager::Instance()->auto_save_state_->get() );

  try
  {
    boost::filesystem::path path = complete( boost::filesystem::path( this->
      current_project_path_state_->get().c_str(), boost::filesystem::native ) );
    boost::filesystem::create_directory( path );
  }
  catch ( std::exception& e ) 
  {
    
  }
  
  this->initialize();
  
  ProjectHandle new_project( new Project( "default_project" ) );
  this->current_project_ = new_project;
  
  // Connect the signals from the LayerManager to the GUI
  this->add_connection( this->current_project_->project_name_state_->value_changed_signal_.connect( 
    boost::bind( &ProjectManager::rename_project_folder, this, _1, _2 ) ) );

  this->add_connection( PreferencesManager::Instance()->auto_save_timer_state_->
    value_changed_signal_.connect( boost::bind( 
    &ProjectManager::set_auto_save_timer, this, _1, _2 ) ) );

/*  this->timer_ = new boost::asio::deadline_timer( this->io_ );*/
}

ProjectManager::~ProjectManager()
{
}

void ProjectManager::initialize()
{
  import_states( this->local_projectmanager_path_, "projectmanager" );
}
  
void ProjectManager::save_projectmanager_state()
{
  export_states( this->local_projectmanager_path_, "projectmanager" );
}
  
void ProjectManager::rename_project_folder( const std::string& new_name, Core::ActionSource source )
{
  if( !this->current_project_->name_status() )
  {
    this->current_project_->name_is_set( true );
    return;
  }
    
  std::vector< std::string > old_name_vector = 
    Core::SplitString( this->recent_projects_state_->get()[ 0 ], "|" );
  
  if( old_name_vector.size() < 2 )
    return;

  std::string old_name = old_name_vector[ 1 ];

  if( old_name == new_name )
    return;
  
  if( old_name == "" )
  {
    return;
  }
  
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  try
  {
    boost::filesystem::rename( ( path / old_name ), ( path / new_name ) );
    boost::filesystem::rename( ( path / new_name / ( old_name + ".xml" ) ), 
      ( path / new_name / ( new_name + ".xml" ) ) );
  }
  catch ( std::exception& e ) 
  {
    //return;
  }
  
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
        temp_projects_vector[ i ] = "";
      }
    }
  }
  this->recent_projects_state_->set( temp_projects_vector );
  
  boost::filesystem::path project_path = boost::filesystem::path( 
    current_project_path_state_->get().c_str() );
  project_path = project_path / this->current_project_->project_name_state_->get().c_str();
  
  this->current_project_->populate_session_states();
  this->add_to_recent_projects( path.string(), new_name );

}

void ProjectManager::new_project( const std::string& project_name, bool consolidate )
{
  if( create_project_folders( project_name ) )
  {
    if( project_name.compare( 0, 11, "New Project" ) == 0 )
    {
      this->default_project_name_counter_state_->set( 
        this->default_project_name_counter_state_->get() + 1 ); 
    }

    this->current_project_->project_name_state_->set( project_name );
    this->current_project_->auto_consolidate_files_state_->set( consolidate );


    this->save_project_session();
  }
}
  
void ProjectManager::open_project( const std::string& project_path, const std::string& project_name )
{
  this->current_project_->project_name_state_->set( project_name );
  boost::filesystem::path path = project_path;
  
  this->current_project_->initialize_from_file( path / project_name, project_name );
  this->current_project_path_state_->set( project_path );
  this->add_to_recent_projects( project_path, project_name );
}
  
void ProjectManager::save_project( bool autosave /*= false*/ )
{
  if( this->save_project_session( autosave ) )
  {
//    boost::filesystem::path project_path = boost::filesystem::path( 
//      current_project_path_state_->get().c_str() );
//    project_path = project_path / this->current_project_->project_name_state_->get().c_str();
//    this->current_project_->export_states( project_path, 
//      this->current_project_->project_name_state_->get() );
    this->save_project_only();

  }

  this->start_auto_save_timer();
}
  
  
void ProjectManager::save_project_as()
{
    
}
  
bool ProjectManager::save_project_session( bool autosave /*= false */ )
{
  // Here we check to see if its an autosave and if it is, just save over the previous autosave
  std::string session_name;
  if( autosave )
    session_name = "autosave";
  else
    session_name = this->get_timestamp();

  std::string user_name;
  Core::Application::Instance()->get_user_name( user_name );

  session_name = session_name + " - " + user_name;
  
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  try // to create a project session folder
  {
    boost::filesystem::create_directory( path / this->current_project_->project_name_state_->
      get() / "sessions" / session_name );
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    return false;
  }
  this->add_to_recent_projects( this->current_project_path_state_->export_to_string(),
    this->current_project_->project_name_state_->get() );
  
  return this->current_project_->save_session( ( path /
    this->current_project_->project_name_state_->get() ), session_name );
}
  

bool ProjectManager::load_project_session( int session_index )
{
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  return this->current_project_->load_session( ( path /
    this->current_project_->project_name_state_->get() ), session_index );
    
}
  
bool ProjectManager::delete_project_session( int session_index )
{
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  return this->current_project_->delete_session( ( path /
    this->current_project_->project_name_state_->get() ), session_index );
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
      //if( temp_projects_vector[ i ] == ( project_path + "|" + project_name ) )
      if( ( ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 0 ] + "|" +
        ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 1 ] )
        == ( project_path + "|" + project_name ) )
      {
        temp_projects_vector[ i ] = "";
      }
    }
  }
  
  

  // now we add id to the beginning of the list
  temp_projects_vector.insert( temp_projects_vector.begin(), 
    ( project_path + "|" + project_name + "|" + this->get_timestamp() ) );
  temp_projects_vector.resize(20);
  
  this->recent_projects_state_->set( temp_projects_vector );
  this->save_projectmanager_state();
}
  
bool ProjectManager::create_project_folders( const std::string& project_name )
{
  try // to create a project folder
  {
    boost::filesystem::path path = complete( boost::filesystem::path( this->
      current_project_path_state_->get().c_str(), boost::filesystem::native ) );
    boost::filesystem::create_directory( path / project_name );
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    return false;
  }
  
  try // to create a project sessions folder
  {
    boost::filesystem::path path = complete( boost::filesystem::path( this->
      current_project_path_state_->get().c_str(), boost::filesystem::native ) );
    boost::filesystem::create_directory( path / project_name / "sessions");
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    return false;
  }
  
  try // to create a project sessions folder
  {
    boost::filesystem::path path = complete( boost::filesystem::path( this->
      current_project_path_state_->get().c_str(), boost::filesystem::native ) );
    boost::filesystem::create_directory( path / project_name / "data");
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
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
  
  strftime ( time_buffer, 80, "%Y-%b%d-%H-%M-%S", timeinfo );
  std::string current_time_stamp = time_buffer;
  return current_time_stamp;
 }

void ProjectManager::set_auto_save_timer( int timeout, Core::ActionSource source )
{
  this->auto_save_timer_ = timeout * 60;
}


void ProjectManager::start_auto_save_timer()
{
  //boost::asio::io_service io_;
//  this->timer_->cancel();
//  this->timer_->expires_from_now(boost::posix_time::seconds( 10 ));
//  timer_->async_wait( boost::bind( &ProjectManager::save_project, this, true ) );
//  this->io_.run();
}

boost::filesystem::path ProjectManager::get_project_data_path() const
{
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  return path / this->current_project_->project_name_state_->get() / "data";

  
}

void ProjectManager::save_note( const std::string& note )
{
//  std::string new_note = note;
//  int note_length = static_cast< int >( new_note.length() );
//  int splits = note_length / 32;
// 
//  // now for some fancy word wrapping.
//  for( int i = 1; i <= splits; i++ )
//  {
//    size_t last_location = new_note.find_last_of( " ", ( ( 30 * i ) + ( i ) ) );
//    new_note.replace( last_location, 1, "\n" );
//  }

  std::string user_name;
  Core::Application::Instance()->get_user_name( user_name );



  std::vector< std::string > notes = this->current_project_->project_notes_state_->get();

  notes.push_back( get_timestamp() + " - " + user_name + "|" + note );

  this->current_project_->project_notes_state_->set( notes );

  this->save_project_only();
}

bool ProjectManager::save_project_only()
{
  boost::filesystem::path project_path = boost::filesystem::path( 
    current_project_path_state_->get().c_str() );

  project_path = project_path / this->current_project_->project_name_state_->get().c_str();

  return this->current_project_->export_states( project_path, 
    this->current_project_->project_name_state_->get() );

}






} // end namespace seg3D
