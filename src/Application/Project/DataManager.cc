/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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
#include <Application/Project/DataManager.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Core/State/StateIO.h>

namespace Seg3D
{

DataManager::DataManager() :
  StateHandler( "datamanager", true )
{ 
  this->add_state( "sessions_and_datafiles", this->sessions_and_datafiles_state_ );
}

DataManager::~DataManager()
{
}
  
void DataManager::initialize( const boost::filesystem::path& project_path )
{
  Core::StateIO stateio;
  if ( stateio.import_from_file( project_path / "data" / "datamanager.xml" ) )
  {
    this->load_states( stateio );
  }
}

DataManager::mutex_type& DataManager::get_mutex()
{
  return Core::StateEngine::GetMutex();
}

bool DataManager::get_session_files_vector( const std::string& session_name, std::vector< std::string >& files )
{
  lock_type lock( this->get_mutex() );
  std::vector< std::string > sessions_and_data = this->sessions_and_datafiles_state_->get();
  for( int i = 0; i < static_cast< int >( sessions_and_data.size() ); ++i )
  {
    std::vector< std::string > session_datafiles = Core::SplitString( sessions_and_data[ i ], "," );
    if( session_datafiles[ 0 ] == session_name )
    {
      for( int j = 1; j < static_cast< int >( session_datafiles.size() ); ++j )
      {
        files.push_back( session_datafiles[ j ] );
      }
      return true;
    }
  }
  return false;
}

} // end namespace seg3D
