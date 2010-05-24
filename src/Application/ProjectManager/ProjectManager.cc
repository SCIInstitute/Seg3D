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
  this->local_projectmanager_path_ = this->local_projectmanager_path_ / "seg3d2_project_settings.cfg";
  
  std::vector< std::string> projects;
  projects.resize( 10, "" );

  add_state( "recent_projects", this->recent_projects_state_, projects );
  add_state( "current_project_path", this->current_project_path_state_, PreferencesManager::Instance()->project_path_state_->get() );
  
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

}

ProjectManager::~ProjectManager()
{
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
  this->current_project_->project_name_state_->set( project_name );
  this->current_project_->auto_consolidate_files_state_->set( consolidate );
  
  //boost::filesystem::path temp_path = project_path / project_name;
  this->add_to_recent_projects( this->current_project_path_state_->export_to_string(),
    project_name );
}
  
void ProjectManager::open_project( const std::string& project_path, const std::string& project_name )
{
  this->current_project_->initialize_from_file( project_path );
  this->add_to_recent_projects( project_path, project_name );
}
  
void ProjectManager::save_project()
{
    
}
  
void ProjectManager::save_project_as()
{
    
}
  
void ProjectManager::add_to_recent_projects( const std::string& project_path, const std::string& project_name )
{
  std::vector< std::string > temp_projects_vector = this->recent_projects_state_->get();
  
  temp_projects_vector.insert( temp_projects_vector.begin(), ( project_path + "|" + project_name ) );
  temp_projects_vector.resize(10);
  
  this->recent_projects_state_->set( temp_projects_vector );
  this->save_projectmanager_state();
}
  


} // end namespace seg3D
