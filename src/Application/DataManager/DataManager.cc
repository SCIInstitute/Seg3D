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
#include <Application/DataManager/DataManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/LayerManager/LayerManager.h>



namespace Seg3D
{

const size_t DataManager::VERSION_NUMBER_C = 1;

CORE_SINGLETON_IMPLEMENTATION( DataManager );

DataManager::DataManager() :
  StateHandler( "datamanager", VERSION_NUMBER_C, false )
{ 
  this->project_datafile_path_ = ProjectManager::Instance()->get_project_data_path();

  std::vector< std::string> sessions_and_datafiles;
  add_state( "sessions_and_datafiles", sessions_and_datafiles_state_, sessions_and_datafiles );
  
}

DataManager::~DataManager()
{
}
  
void DataManager::initialize()
{
  import_states( this->project_datafile_path_, "datamanager" );
  
  this->add_connection( ProjectManager::Instance()->current_project_->session_deleted_signal_.
    connect( boost::bind( &DataManager::remove_session, this, _1 ) ) );
}

void DataManager::save_datamanager_state( const std::string& session_name )
{
  this->prep_for_save( session_name );
  export_states( this->project_datafile_path_, "datamanager" );
}
  
void DataManager::prep_for_save( const std::string& session_name )
{
  lock_type lock( this->get_mutex() );
  
  std::vector< LayerHandle > current_layers;
  LayerManager::Instance()->get_layers( current_layers );
  std::vector< std::string > sessions_and_data = this->sessions_and_datafiles_state_->get();
  std::string session = session_name;
  
  for( int i = 0; i < static_cast< int >( current_layers.size() ); ++i )
  {
    session = session + "," + boost::lexical_cast< std::string >
      ( current_layers[ i ]->get_generation() ) + ".nrrd";
  }
  sessions_and_data.push_back( session );
  this->sessions_and_datafiles_state_->set( sessions_and_data );
  
  std::vector< std::string > used_datafiles;
  for( int i = 0; i < static_cast< int >( sessions_and_data.size() ); ++i )
  {
    std::vector< std::string > session_datafiles = Core::SplitString( sessions_and_data[ i ], "," );
    if( session_datafiles.size() < 2 ) 
    {
      continue;
    }
    for( int j = 1; j < static_cast< int >( session_datafiles.size() ); ++j )
    {
      used_datafiles.push_back( session_datafiles[ j ] );
    }
  }
  
  if ( boost::filesystem::exists( this->project_datafile_path_ ) )
  {
    boost::filesystem::directory_iterator dir_end;
    for( boost::filesystem::directory_iterator dir_itr( this->project_datafile_path_ ); 
      dir_itr != dir_end; ++dir_itr )
    {
      // in the case that we find the datamanager.xml file, we skip it.
      if( dir_itr->leaf() == "datamanager.xml" )
      {
        continue;
      }
      
      bool found = false;
      for( int j = 0; j < static_cast< int >( used_datafiles.size() ); ++j )
      {
        if( dir_itr->leaf() == used_datafiles[ j ] )
        {
          found = true;
          break;
        }
      }
      if ( !found )
      {
        try 
        {
          boost::filesystem::remove_all( dir_itr->path() );
        }
        catch(  std::exception& e ) 
        {
          CORE_LOG_ERROR( e.what() );
        }
      }
    }
  }
}
  
void DataManager::remove_session( const std::string& session_name )
{
  lock_type lock( this->get_mutex() );
  std::vector< std::string > sessions_and_data = this->sessions_and_datafiles_state_->get();
  
  for( int i = 0; i < static_cast< int >( sessions_and_data.size() ); ++i )
  {
    std::vector< std::string > session_datafiles = Core::SplitString( sessions_and_data[ i ], "," );
    if( session_datafiles[ 0 ] == session_name )
    {
      sessions_and_data.erase( sessions_and_data.begin() + i );
      break;
    }
  }
  this->sessions_and_datafiles_state_->set( sessions_and_data );
}

DataManager::mutex_type& DataManager::get_mutex()
{
  return Core::StateEngine::GetMutex();
}

} // end namespace seg3D
