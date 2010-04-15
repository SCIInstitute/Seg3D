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

// Boost includes 

// Application includes
#include <Application/Viewer/Viewer.h> 
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Interface/Interface.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ViewerManager );

ViewerManager::ViewerManager() :
  StateHandler( "view" )
{
  // Step (1)
  // Set the default state of this element
  this->add_state( "layout", this->layout_state_, "1and3", 
    "single|1and1|1and2|1and3|2and2|2and3|3and3" );
  this->add_state( "active_viewer", this->active_viewer_state_, 0 );

  // Step (2)
  // Create the viewers that are part of the application
  // Currently a maximum of 6 viewers can be created
  this->viewers_.resize( 6 );
  for ( size_t j = 0; j < viewers_.size(); j++ )
  {
    this->viewers_[ j ] = ViewerHandle( new Viewer( j ) );
    this->add_connection( this->viewers_[ j ]->content_changed_signal_.connect(
      boost::bind( &ViewerManager::viewer_content_changed, this, _1 ) ) );
  }
}

ViewerManager::~ViewerManager()
{
  disconnect_all();
}

ViewerHandle ViewerManager::get_viewer( size_t idx )
{
  ViewerHandle handle;
  if ( idx < this->viewers_.size() ) handle = this->viewers_[ idx ];
  return handle;
}

ViewerHandle ViewerManager::get_viewer( const std::string viewer_name )
{
  ViewerHandle handle;
  for ( size_t i = 0; i < this->viewers_.size(); i++ )
  {
    if ( this->viewers_[ i ]->get_stateid() == viewer_name )
    {
      handle = this->viewers_[ i ];
      break;
    }
  }
  return handle;
}

void ViewerManager::viewer_content_changed(size_t viewer_id)
{
  this->viewer_content_changed_signal_( viewer_id );
}

} // end namespace Seg3D

