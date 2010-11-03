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

#include <Application/LayerManager/LayerCheckPoint.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

class LayerCheckPointPrivate : public boost::noncopyable
{
public:
    Core::VolumeHandle volume_;
};


LayerCheckPoint::LayerCheckPoint( LayerHandle layer ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_volume( layer );
}

LayerCheckPoint::LayerCheckPoint( LayerHandle layer, Core::VolumeSliceType slice_type, size_t idx ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_slice( layer, slice_type, idx );
}

LayerCheckPoint::~LayerCheckPoint()
{
}
  
bool LayerCheckPoint::apply( LayerHandle layer ) const
{
  // If there is a full volume in the check point insert it into the layer
  if ( this->private_->volume_ )
  {
    LayerManager::DispatchInsertVolumeIntoLayer( layer, this->private_->volume_ );
  }
  
  return true;
}
  
bool LayerCheckPoint::create_volume( LayerHandle layer )
{
  this->private_->volume_ = layer->get_volume();
  return false;
}

bool LayerCheckPoint::create_slice( LayerHandle layer, Core::VolumeSliceType slice_type, size_t idx )
{
  return false;
}

size_t LayerCheckPoint::get_byte_size() const
{
  return 0;
}

  
} // end namespace Seg3D
