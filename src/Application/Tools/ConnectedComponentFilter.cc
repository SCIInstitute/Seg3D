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
#include <Application/Layer/LayerFWD.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/ConnectedComponentFilter.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/Viewer/Viewer.h>
#include <Application/ViewerManager/ViewerManager.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, ConnectedComponentFilter )

namespace Seg3D 
{

class ConnectedComponentFilterPrivate
{
public:
  void handle_seed_points_changed();
};

void ConnectedComponentFilterPrivate::handle_seed_points_changed()
{
  size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
  for ( size_t i = 0; i < num_of_viewers; i++ )
  {
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
    if ( !viewer->is_volume_view() && viewer->viewer_visible_state_->get() )
    {
      viewer->redraw_overlay();
    }
  }
}

ConnectedComponentFilter::ConnectedComponentFilter( const std::string& toolid ) :
  SeedPointsTool( Core::VolumeType::DATA_E, toolid ),
  private_( new ConnectedComponentFilterPrivate )
{ 
  this->use_active_layer_state_->set( false );

  this->add_connection( this->seed_points_state_->state_changed_signal_.connect( boost::bind( 
    &ConnectedComponentFilterPrivate::handle_seed_points_changed, this->private_.get() ) ) );
}

ConnectedComponentFilter::~ConnectedComponentFilter()
{ 
  this->disconnect_all();
}

void ConnectedComponentFilter::activate()
{
}

void ConnectedComponentFilter::deactivate()
{
}
  
} // end namespace Seg3D


