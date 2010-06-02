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

// Application includes
#include <Application/Project/Session.h>
#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Layer/LayerGroup.h>


namespace Seg3D
{

Session::Session( const std::string& session_name ) :
  StateHandler( "session", false )
{ 
  this->add_state( "session_name", this->session_name_state_, session_name );
}
  
Session::~Session()
{

}

bool Session::initialize_from_file( boost::filesystem::path path, const std::string& session_name )
{
//  boost::filesystem::path path = session_path;
  if( this->load_states( path / session_name ) )
  {
    InterfaceManager::Instance()->load_states( path / "interfacemanager" );
    
    ViewerManager::Instance()->load_states( path / "viewermanager" );
    ViewerManager::Instance()->get_viewer( "viewer0" )->load_states( path / "viewer0" );
    ViewerManager::Instance()->get_viewer( "viewer1" )->load_states( path / "viewer1" );
    ViewerManager::Instance()->get_viewer( "viewer2" )->load_states( path / "viewer2" );
    ViewerManager::Instance()->get_viewer( "viewer3" )->load_states( path / "viewer3" );
    ViewerManager::Instance()->get_viewer( "viewer4" )->load_states( path / "viewer4" );
    ViewerManager::Instance()->get_viewer( "viewer5" )->load_states( path / "viewer5" );
    
    LayerManager::Instance()->load_states( path / "layermanager" );

    return true;
  }
  
  return false;
}
  
bool Session::save_session_settings( boost::filesystem::path path, const std::string& session_name )
{
  InterfaceManager::Instance()->save_states( path / "interfacemanager" );
  
  ViewerManager::Instance()->save_states( path / "viewermanager" );
  ViewerManager::Instance()->get_viewer( "viewer0" )->save_states( path / "viewer0" );
  ViewerManager::Instance()->get_viewer( "viewer1" )->save_states( path / "viewer1" );
  ViewerManager::Instance()->get_viewer( "viewer2" )->save_states( path / "viewer2" );
  ViewerManager::Instance()->get_viewer( "viewer3" )->save_states( path / "viewer3" );
  ViewerManager::Instance()->get_viewer( "viewer4" )->save_states( path / "viewer4" );
  ViewerManager::Instance()->get_viewer( "viewer5" )->save_states( path / "viewer5" );
  
  LayerManager::Instance()->save_states( path / "layermanager" );
  std::vector< std::string > group_vector = LayerManager::Instance()->groups_state_->get();
  for( size_t i = 0; i < group_vector.size(); ++i )
  {
    if( ( group_vector[ i ] != "]" ) && ( group_vector[ i ] != "\0" ) )
    {
      LayerManager::Instance()->get_layer_group( group_vector[ i ] )->save_states( 
        path / group_vector[ i ] );
      std::vector< std::string > layer_vector = LayerManager::Instance()->
        get_layer_group( group_vector[ i ] )->layers_state_->get();
      for( size_t j = 0; j < layer_vector.size(); ++j )
      {
        std::string layer_name = ( Core::SplitString( layer_vector[ i ], "|" ) )[ 0 ];
        LayerManager::Instance()->get_layer_by_id( layer_name )->
          save_states( path / layer_name );
      }
    }
  }
  
  
  return this->save_states( path / session_name );
}
    
} // end namespace Seg3d

