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

// Application includes
#include <Application/Tools/SliceRange.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/Layer/Layer.h>

#include <Application/ViewerManager/ViewerManager.h>

#include <limits>

namespace Seg3D
{

SliceRange::SliceRange( LayerHandle src_layer ) :
  Core::StateHandler( "slicerange", false ),
  src_layer_( src_layer )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  if ( viewer->view_mode_state_->get() == Viewer::VOLUME_C )
  {
    int inf = std::numeric_limits< int >::infinity();
    this->add_state( "min", this->min_slice_state_, inf, inf, inf, 1 );
    this->add_state( "max", this->max_slice_state_, inf, inf, inf, 1 );
  }
  else
  {
    Core::VolumeSliceHandle vol_slice = viewer->get_active_volume_slice();
    int max = vol_slice->number_of_slices();
    this->add_state( "min", this->min_slice_state_, 1, 1, max, 1 );
    this->add_state( "max", this->max_slice_state_, max, 1, max, 1 );
  }
}

SliceRange::~SliceRange()
{
  this->disconnect_all();
}

void SliceRange::execute( Core::ActionContextHandle context )
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  ViewerHandle viewer = ViewerManager::Instance()->get_active_viewer();
  Core::VolumeSliceHandle vol_slice = viewer->get_active_volume_slice();

  // using 1-indexed state
  ActionPaste::Dispatch(
                         context,
                         this->src_layer_->get_layer_id(),
                         vol_slice->get_slice_type(),
                         this->min_slice_state_->get() - 1,
                         this->max_slice_state_->get() - 1
                        );
}

}
