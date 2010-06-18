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
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Utils/StringUtil.h>

// Application includes
#include <Application/Project/Project.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

Project::Project( const std::string& project_name ) :
  StateHandler( "project", false ),
  name_set_( false )
{ 
  add_state( "project_name", this->project_name_state_, project_name );
  add_state( "auto_consolidate_files", this->auto_consolidate_files_state_, true );
  add_state( "save_custom_colors", this->save_custom_colors_state_, false );
  
  std::vector< std::string> sessions;
  add_state( "sessions", this->sessions_state_, sessions );

  SessionHandle new_session( new Session( "default_session" ) );
  this->current_session_ = new_session;
  
  
}
  
Project::~Project()
{

}

bool Project::initialize_from_file( boost::filesystem::path project_path, 
  const std::string& project_name )
{
  if( import_states( project_path, project_name ) ) 
  {
    project_path = project_path / 
      ( Core::SplitString( ( this->sessions_state_->get() )[0], "|" ) )[ 0 ];
    
    return this->current_session_->initialize_from_file( project_path,
      ( Core::SplitString( ( this->sessions_state_->get() )[0], "|" ) )[ 1 ] );
  }
  return false;
}
  
bool Project::load_session( boost::filesystem::path project_path, int state_index )
{
  project_path = project_path / 
    ( Core::SplitString( ( this->sessions_state_->get() )[ state_index ], "|" ) )[ 0 ];
  
  return this->current_session_->initialize_from_file( project_path,
    ( Core::SplitString( ( this->sessions_state_->get() )[ state_index ], "|" ) )[ 1 ] );
  
}
  
bool Project::save_session( boost::filesystem::path project_path, const std::string& session_name, 
  const std::string& notes )
{
  this->current_session_->session_name_state_->set( session_name );
  
  boost::filesystem::path temp_path = "sessions";
  temp_path = temp_path / session_name;
  this->add_session_to_list( temp_path.string() + "|" + session_name + "|" + notes );

  return this->current_session_->save_session_settings( 
    ( project_path / "sessions" / session_name ), session_name );
}
  
bool Project::delete_session( boost::filesystem::path project_path, int state_index )
{
  project_path = project_path / 
    ( Core::SplitString( ( this->sessions_state_->get() )[ state_index ], "|" ) )[ 0 ];
  
  std::vector< std::string > temp_sessions_vector = this->sessions_state_->get();
  temp_sessions_vector.erase( temp_sessions_vector.begin() + state_index );
  this->sessions_state_->set( temp_sessions_vector );
  
  try 
  {
    boost::filesystem::remove_all( project_path );
  }
  catch(  std::exception& e ) 
  {
    return false;
  }
  
  return true;
}
  
void Project::add_session_to_list( const std::string& session_path_and_name )
{
  std::vector< std::string > temp_sessions_vector = this->sessions_state_->get();
  
  for( int i = 0; i < static_cast< int >( temp_sessions_vector.size() ); ++i )
  {
    if( temp_sessions_vector[ i ] == session_path_and_name )
    {
      temp_sessions_vector[ i ] = "";
    }
  }
  temp_sessions_vector.insert( temp_sessions_vector.begin(), session_path_and_name );
  this->sessions_state_->set( temp_sessions_vector );
}

} // end namespace Seg3D

