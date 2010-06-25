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
// #include <Application/InterfaceManager/InterfaceManager.h>
// #include <Application/ViewerManager/ViewerManager.h>
// #include <Application/LayerManager/LayerManager.h>
// #include <Application/Layer/LayerGroup.h>


// Core includes
#include <Core/State/StateEngine.h>
#include <Core/State/StateIO.h>


namespace Seg3D
{

const size_t Session::version_number_ = 1;

Session::Session( const std::string& session_name ) :
  StateHandler( "session", version_number_, false )
{ 
  this->add_state( "session_name", this->session_name_state_, session_name );
}
  
Session::~Session()
{

}

bool Session::initialize_from_file( boost::filesystem::path path, const std::string& session_name )
{
  std::vector< std::string > state_values;

  // Next we import the session information from file
  if( Core::StateIO::import_from_file( ( path / session_name ), state_values ) )
  {
    Core::StateEngine::Instance()->set_session_states( state_values );
    return Core::StateEngine::Instance()->load_session_states();
  }

  return false;
}
  
bool Session::save_session_settings( boost::filesystem::path path, const std::string& session_name )
{
  if( Core::StateEngine::Instance()->populate_session_vector() )
  {
    std::vector< std::string > state_values;
    Core::StateEngine::Instance()->get_session_states( state_values );
    return Core::StateIO::export_to_file( ( path / session_name ), state_values );
  }

  return false;
}
    
} // end namespace Seg3d

