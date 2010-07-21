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

const size_t Project::VERSION_NUMBER_C = 1;

Project::Project( const std::string& project_name ) :
  StateHandler( "project", VERSION_NUMBER_C, false ),
  name_set_( false )
{ 
  add_state( "project_name", this->project_name_state_, project_name );
  add_state( "save_custom_colors", this->save_custom_colors_state_, false );
  
  std::vector< std::string> empty_vector;
  add_state( "sessions", this->sessions_state_, empty_vector );
  add_state( "project_notes", this->project_notes_state_, empty_vector );
  
  this->color_states_.resize( 12 );
  for ( size_t j = 0; j < 12; j++ )
  {
    std::string stateid = std::string( "color_" ) + Core::ExportToString( j );
    this->add_state( stateid, this->color_states_[ j ], PreferencesManager::Instance()->get_default_colors()[ j ] );
  }

  this->current_session_ = SessionHandle( new Session( "default_session" ) );
  this->data_manager_ = DataManagerHandle( new DataManager() );
}
  
Project::~Project()
{

}

bool Project::initialize_from_file( boost::filesystem::path project_path, 
  const std::string& project_name )
{
  if( this->import_states( project_path, project_name, true ) ) 
  {
    if( this->load_session( project_path, 0 ) )
    {
      this->data_manager_->initialize( project_path );
      return true;
    }
  }
  return false;
}
  
bool Project::load_session( boost::filesystem::path project_path, int state_index )
{
  std::vector< std::string > session = 
    Core::SplitString( ( this->sessions_state_->get() )[ state_index ], "|" );
  
  if( session[ 0 ] == "]" )
  {
    return false;
  }

  boost::filesystem::path session_path = project_path / session[ 0 ];
  return  this->current_session_->initialize_from_file( session_path, session[ 1 ] );
}
  
bool Project::save_session( boost::filesystem::path project_path, const std::string& session_name )
{
  this->current_session_->session_name_state_->set( session_name );
  
  
  if( this->current_session_->save_session_settings( 
    ( project_path / "sessions" ), session_name ) )
  {
    this->add_session_to_list( project_path, session_name );
    return true;
  }
  return false;

}
  
bool Project::delete_session( boost::filesystem::path project_path, int state_index )
{
  boost::filesystem::path session_path = project_path / "sessions";
  session_path = session_path / (
    ( Core::SplitString( ( this->sessions_state_->get() )[ state_index ], "|" ) )[ 1 ] + ".xml");
  
  std::vector< std::string > temp_sessions_vector = this->sessions_state_->get();
  temp_sessions_vector.erase( temp_sessions_vector.begin() + state_index );
  this->sessions_state_->set( temp_sessions_vector );
  
  try 
  {
    boost::filesystem::remove_all( session_path );
  }
  catch(  std::exception& e ) 
  {
    CORE_LOG_ERROR( e.what() );
    return false;
  }
  
  std::string session_deleted_name = Core::SplitString( session_path.leaf(), "." )[ 0 ];

  this->data_manager_->remove_session( session_deleted_name );

  this->export_states( project_path, this->project_name_state_->get(), true );
  return true;
}
  
void Project::add_session_to_list( boost::filesystem::path project_path, const std::string& session_name )
{
  std::vector< std::string > temp_sessions_vector = this->sessions_state_->get();
  
  for( int i = 0; i < static_cast< int >( temp_sessions_vector.size() ); ++i )
  {
    if( ( temp_sessions_vector[ i ] == "" ) || ( temp_sessions_vector[ i ] == "]" ) )
    {
      continue;
    }
    std::string stored_session_name = ( Core::SplitString( temp_sessions_vector[ i ], "|" ) )[ 1 ];
    if( ( stored_session_name.substr( 0, 5 ) == "AS - " ) && 
      ( session_name.substr( 0, 5 ) == "AS - " ) )
    {
      temp_sessions_vector.erase( temp_sessions_vector.begin() + i );
      this->delete_session( project_path, i );
    }
  }
  temp_sessions_vector.insert( temp_sessions_vector.begin(), ( "sessions|" + session_name ) );
  this->sessions_state_->set( temp_sessions_vector );

  this->data_manager_->save_datamanager_state( project_path, session_name );
  
}

bool Project::get_session_name( int index, std::string& session_name )
{
  std::vector< std::string > temp_sessions_vector = this->sessions_state_->get();
  for( int i = 0; i < static_cast< int >( temp_sessions_vector.size() ); ++i )
  {
    if( i == index )
    {
      session_name = ( Core::SplitString( temp_sessions_vector[ i ], "|" ) )[ 1 ];
      return true;
    }
  }
  return false;
}

bool Project::pre_save_states()
{
  if( this->save_custom_colors_state_->get() )
  {
    for ( size_t j = 0; j < 12; j++ )
    {
      this->color_states_[ j ]->set( PreferencesManager::Instance()->color_states_[ j ]->get() );
    }
  }
  return true;
}

bool Project::post_load_states()
{
  if( this->save_custom_colors_state_->get() )
  {
    for ( size_t j = 0; j < 12; j++ )
    {
      PreferencesManager::Instance()->color_states_[ j ]->set( this->color_states_[ j ]->get() );
    }
  }
  return true;
}

} // end namespace Seg3D

