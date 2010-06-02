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
  StateHandler( "projectmanager", false )
{ 
  Core::Application::Instance()->get_config_directory( this->local_projectmanager_path_ );
  this->local_projectmanager_path_ = this->local_projectmanager_path_ / 
    "seg3d2_project_settings.cfg";
  
  std::vector< std::string> projects;
  projects.resize( 20, "" );

  add_state( "recent_projects", this->recent_projects_state_, projects );
  add_state( "current_project_path", this->current_project_path_state_, 
    PreferencesManager::Instance()->project_path_state_->get() );
  
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
  
  

}

ProjectManager::~ProjectManager()
{
}
  
void ProjectManager::rename_project_folder( const std::string& new_name, Core::ActionSource source )
{
  std::string old_name = ( Core::SplitString( this->recent_projects_state_->get()[ 0 ], "|" ) )[ 1 ];
  
  if ( old_name == "" )
  {
    return;
  }
  
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  try
  {
    boost::filesystem::rename( ( path / old_name ), ( path / new_name ) );
    boost::filesystem::rename( ( path / new_name / old_name ), ( path / new_name / new_name ) );
  }
  catch ( std::exception& e ) 
  {
    return;
  }
  
  std::vector< std::string > temp_projects_vector = this->recent_projects_state_->get();
  
  // first we are going to remove this project from the list if its in there.
  size_t projects_vector_size = temp_projects_vector.size();
  for( size_t i = 0; i < projects_vector_size; ++i )
  {
    if( temp_projects_vector[ i ] == ( path.string() + "|" + old_name ) )
    {
      temp_projects_vector.erase( temp_projects_vector.begin() + i );
      projects_vector_size--;
    }
  }
  this->recent_projects_state_->set( temp_projects_vector );
  
  boost::filesystem::path project_path = boost::filesystem::path( 
    current_project_path_state_->get().c_str() );
  project_path = project_path / this->current_project_->project_name_state_->get().c_str();
  this->current_project_->save_states( project_path / 
    this->current_project_->project_name_state_->get() );
  
  this->add_to_recent_projects( path.string(), new_name );

}
  

void ProjectManager::initialize()
{
  load_states( this->local_projectmanager_path_ );
}
  
void ProjectManager::save_projectmanager_state()
{
  save_states( this->local_projectmanager_path_ );
}


void ProjectManager::new_project( const std::string& project_name, bool consolidate )
{
  if( create_project_folders( project_name ) )
  {
    this->current_project_->project_name_state_->set( project_name );
    this->current_project_->auto_consolidate_files_state_->set( consolidate );
    this->add_to_recent_projects( this->current_project_path_state_->export_to_string(),
      project_name );
    
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
  
void ProjectManager::save_project()
{
  if( this->save_project_session() )
  {
    boost::filesystem::path project_path = boost::filesystem::path( 
      current_project_path_state_->get().c_str() );
    project_path = project_path / this->current_project_->project_name_state_->get().c_str();
    this->current_project_->save_states( project_path / 
      this->current_project_->project_name_state_->get() );
  }
}
  
  
void ProjectManager::save_project_as()
{
    
}
  
bool ProjectManager::save_project_session()
{
  time_t rawtime;
  struct tm * timeinfo;
  char time_buffer [80];
  
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  
  strftime ( time_buffer, 80, "%Y-%b%d-%H-%M-%S", timeinfo );
  std::string current_time_stamp = time_buffer;
  
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  try // to create a project session folder
  {
    boost::filesystem::create_directory( path / this->current_project_->project_name_state_->
      get() / "sessions" / current_time_stamp );
  }
  catch ( std::exception& e ) // any errors that we might get thrown
  {
    return false;
  }
  
  return this->current_project_->save_session( ( path /
    this->current_project_->project_name_state_->get() ), current_time_stamp );
}
  

bool ProjectManager::load_project_session( int session_index )
{
  boost::filesystem::path path = complete( boost::filesystem::path( this->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );
  
  return this->current_project_->load_session( ( path /
    this->current_project_->project_name_state_->get() ), session_index );
    
}
  
void ProjectManager::add_to_recent_projects( const std::string& project_path, 
  const std::string& project_name )
{
  std::vector< std::string > temp_projects_vector = this->recent_projects_state_->get();
  
  // first we are going to remove this project from the list if its in there.
  size_t projects_vector_size = temp_projects_vector.size();
  for( size_t i = 0; i < projects_vector_size; ++i )
  {
    if( temp_projects_vector[ i ] == ( project_path + "|" + project_name ) )
    {
      temp_projects_vector.erase( temp_projects_vector.begin() + i );
      projects_vector_size--;
    }
  }
  
  // now we add id to the beginning of the list
  temp_projects_vector.insert( temp_projects_vector.begin(), 
    ( project_path + "|" + project_name ) );
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

} // end namespace seg3D
