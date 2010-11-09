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

#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

#include <Application/LayerManager/LayerCheckPoint.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

class PartialCheckPoint;
typedef boost::shared_ptr<PartialCheckPoint> PartialCheckPointHandle;


class PartialCheckPoint
{
public:
  // Which axis to use for the parial check point
  int axis_;
  
  // Which slice is stored in this record
  int slice_;
  
  // If it is a mask this pointer is used
  Core::MaskDataBlockHandle mask_;

  // If it is a data volume this pointer is used
  Core::DataBlockHandle data_;
  
  // Byte size of this slice
  size_t size_;
};

class LayerCheckPointPrivate : public boost::noncopyable
{
public:
    // Check point consisting of a full volume
    Core::VolumeHandle volume_;
    
    // Check point consisting of a slice
    typedef std::list<PartialCheckPointHandle> slice_list_type;
    slice_list_type slices_;
};


LayerCheckPoint::LayerCheckPoint( LayerHandle layer ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_volume( layer );
}

LayerCheckPoint::LayerCheckPoint( LayerHandle layer, int slice, int axis ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_slice( layer, slice, axis );
}

LayerCheckPoint::LayerCheckPoint( LayerHandle layer, int min_slice, int max_slice, int axis ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_slice( layer, min_slice, max_slice, axis );
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
  
  LayerCheckPointPrivate::slice_list_type::iterator it = this->private_->slices_.begin();
  LayerCheckPointPrivate::slice_list_type::iterator it_end = this->private_->slices_.end();
  
  while ( it != it_end )
  {
    if ( (*it)->mask_ && layer->get_type() == Core::VolumeType::MASK_E )
    {
      MaskLayerHandle mask = boost::shared_dynamic_cast<MaskLayer>( layer );
      if ( mask && mask->has_valid_data() )
      {
        Core::MaskDataBlockManager::PutSlice( (*it)->mask_, 
          mask->get_mask_volume()->get_mask_data_block(), (*it)->slice_,
          (*it)->axis_ );
      }
    }
    else if ( (*it)->data_ && layer->get_type() == Core::VolumeType::DATA_E )
    {
      DataLayerHandle data = boost::shared_dynamic_cast<DataLayer>( layer );
      if ( data && data->has_valid_data() )
      {
        Core::DataBlock::PutSlice( (*it)->data_, 
          data->get_data_volume()->get_data_block(), (*it)->slice_,
          (*it)->axis_ );
      }
    }
    
    ++it;
  } 
  return true;
}
  
bool LayerCheckPoint::create_volume( LayerHandle layer )
{
  this->private_->volume_ = layer->get_volume();
  return false;
}

bool LayerCheckPoint::create_slice( LayerHandle layer, int slice, int axis )
{
  if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    PartialCheckPointHandle partial_mask_check_point( new PartialCheckPoint );
  
    MaskLayerHandle mask = boost::shared_dynamic_cast<MaskLayer>( layer );
    if ( ! mask->has_valid_data() ) return false;
    
    if ( !( Core::MaskDataBlockManager::GetSlice( mask->get_mask_volume()->get_mask_data_block(),
      partial_mask_check_point->mask_, slice, axis ) ) )
    {
      return false;
    }
    
    partial_mask_check_point->slice_ = slice;
    partial_mask_check_point->axis_ = axis;
    partial_mask_check_point->size_ = partial_mask_check_point->mask_->get_byte_size();

    this->private_->slices_.push_back( partial_mask_check_point );
    return true;
  }
  else if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    PartialCheckPointHandle partial_data_check_point( new PartialCheckPoint );
  
    DataLayerHandle data = boost::shared_dynamic_cast<DataLayer>( layer );
    if ( ! data->has_valid_data() ) return false;
    
    if ( !( Core::DataBlock::GetSlice( data->get_data_volume()->get_data_block(),
      partial_data_check_point->data_, slice, axis ) ) )
    {
      return false;
    }
    
    partial_data_check_point->slice_ = slice;
    partial_data_check_point->axis_ = axis;
    partial_data_check_point->size_ = partial_data_check_point->data_->get_byte_size();
    this->private_->slices_.push_back( partial_data_check_point );
    return true;
  }
  return false;
}


bool LayerCheckPoint::create_slice( LayerHandle layer, int start, int end, int axis )
{
  if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    for ( int j = start; j < end; j++ )
    {
      PartialCheckPointHandle partial_mask_check_point( new PartialCheckPoint );
    
      MaskLayerHandle mask = boost::shared_dynamic_cast<MaskLayer>( layer );
      if ( ! mask->has_valid_data() ) return false;
      
      if ( !( Core::MaskDataBlockManager::GetSlice( mask->get_mask_volume()->get_mask_data_block(),
        partial_mask_check_point->mask_, j, axis ) ) )
      {
        return false;
      }
      
      partial_mask_check_point->slice_ = j;
      partial_mask_check_point->axis_ = axis;
      partial_mask_check_point->size_ = partial_mask_check_point->mask_->get_byte_size();

      this->private_->slices_.push_back( partial_mask_check_point );
    }
    return true;
  }
  else if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    for ( int j = start; j < end; j++ )
    {
      PartialCheckPointHandle partial_data_check_point( new PartialCheckPoint );
    
      DataLayerHandle data = boost::shared_dynamic_cast<DataLayer>( layer );
      if ( ! data->has_valid_data() ) return false;
      
      if ( !( Core::DataBlock::GetSlice( data->get_data_volume()->get_data_block(),
        partial_data_check_point->data_, j, axis ) ) )
      {
        return false;
      }
      
      partial_data_check_point->slice_ = j;
      partial_data_check_point->axis_ = axis;
      partial_data_check_point->size_ = partial_data_check_point->data_->get_byte_size();
      this->private_->slices_.push_back( partial_data_check_point );
    }
    return true;
  }
  return false;
}



size_t LayerCheckPoint::get_byte_size() const
{
  size_t size = 0;
  if ( this->private_->volume_ ) size += this->private_->volume_->get_byte_size();

  LayerCheckPointPrivate::slice_list_type::iterator it = this->private_->slices_.begin();
  LayerCheckPointPrivate::slice_list_type::iterator it_end = this->private_->slices_.end();

  while ( it != it_end )
  {
    size += (*it)->size_;
    ++it;
  }

  return size;
}

  
} // end namespace Seg3D
