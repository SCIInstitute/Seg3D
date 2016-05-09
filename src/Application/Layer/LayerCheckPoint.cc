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

// Core includes
#include <Core/DataBlock/DataSlice.h>
#include <Core/DataBlock/MaskDataSlice.h>
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Provenance/Provenance.h>
#include <Application/Layer/LayerCheckPoint.h>
#include <Application/Layer/LayerManager.h>

// Boost includes
#include <boost/smart_ptr.hpp>

namespace Seg3D
{

class LayerCheckPointPrivate : public boost::noncopyable
{
public:
  // Check point consisting of a full volume
  Core::VolumeHandle volume_;
  
  // Check point consisting of a slice
  typedef std::vector<Core::DataSliceHandle> data_slice_vector_type;
  data_slice_vector_type data_slices_;

  typedef std::vector<Core::MaskDataSliceHandle> mask_slice_vector_type;
  mask_slice_vector_type mask_slices_;
  
  ProvenanceID provenance_id_;
};


LayerCheckPoint::LayerCheckPoint( LayerHandle layer ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_volume( layer );
}

LayerCheckPoint::LayerCheckPoint( LayerHandle layer, 
  Core::SliceType type, Core::DataBlock::index_type index ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_slice( layer, type, index );
}

LayerCheckPoint::LayerCheckPoint( LayerHandle layer, Core::SliceType type, 
  Core::DataBlock::index_type start, Core::DataBlock::index_type end  ) :
  private_( new LayerCheckPointPrivate )
{
  this->create_slice( layer, type, start, end );
}

LayerCheckPoint::~LayerCheckPoint()
{
}
  
bool LayerCheckPoint::apply( LayerHandle layer ) const
{
  // If there is a full volume in the check point insert it into the layer
  if ( this->private_->volume_ )
  {
    LayerManager::DispatchInsertVolumeIntoLayer( layer, this->private_->volume_, 
      this->private_->provenance_id_ );
    return true;
  }
  
  if ( !( this->private_->data_slices_.empty() ) )
  {
    DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );
    if ( ! data_layer ) return false;

    LayerManager::DispatchInsertDataSlicesIntoLayer( data_layer, 
      this->private_->data_slices_, this->private_->provenance_id_ );
    return false;
  }

  if ( !( this->private_->mask_slices_.empty() ) )
  {
    MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
    if ( ! mask_layer ) return false;

    LayerManager::DispatchInsertMaskSlicesIntoLayer( mask_layer, 
      this->private_->mask_slices_, this->private_->provenance_id_ );
    return false;
  }

  return false;
}
  
bool LayerCheckPoint::create_volume( LayerHandle layer )
{
  this->private_->provenance_id_ = layer->provenance_id_state_->get();

  this->private_->volume_ = layer->get_volume();
  return false;
}

bool LayerCheckPoint::create_slice( LayerHandle layer, Core::SliceType type, 
  Core::DataBlock::index_type index )
{
  this->private_->provenance_id_ = layer->provenance_id_state_->get();

  if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
    if ( ! mask->has_valid_data() ) return false;
    
    Core::MaskDataSliceHandle slice;
    if ( !( mask->get_mask_volume()->extract_slice( type, index, slice ) ) ) return false;
    
    this->private_->mask_slices_.push_back( slice );
    return true;
  }
  else if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    DataLayerHandle data = boost::dynamic_pointer_cast<DataLayer>( layer );
    if ( ! data->has_valid_data() ) return false;
    
    Core::DataSliceHandle slice;
    if ( !( data->get_data_volume()->extract_slice( type, index, slice ) ) ) return false;
    
    this->private_->data_slices_.push_back( slice );
    return true;
  }
  return false;
}


bool LayerCheckPoint::create_slice( LayerHandle layer, Core::SliceType type, 
  Core::DataBlock::index_type start, Core::DataBlock::index_type end )
{
  this->private_->provenance_id_ = layer->provenance_id_state_->get();

  if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    for ( Core::DataBlock::index_type j = start; j <= end; j++ )
    {

      MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
      if ( ! mask->has_valid_data() ) return false;
      
      Core::MaskDataSliceHandle slice;
      if ( !( mask->get_mask_volume()->extract_slice( type, j, slice ) ) ) return false;
      
      this->private_->mask_slices_.push_back( slice );
    }
    return true;
  }
  else if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    for ( Core::DataBlock::index_type j = start; j <= end; j++ )
    {
      DataLayerHandle data = boost::dynamic_pointer_cast<DataLayer>( layer );
      if ( ! data->has_valid_data() ) return false;
      
      Core::DataSliceHandle slice;
      if ( !( data->get_data_volume()->extract_slice( type, j, slice ) ) ) return false;
      
      this->private_->data_slices_.push_back( slice );
      return true;
    }
    return true;
  }
  return false;
}

size_t LayerCheckPoint::get_byte_size() const
{
  size_t size = 0;
  if ( this->private_->volume_ ) size += this->private_->volume_->get_byte_size();

  {
    LayerCheckPointPrivate::data_slice_vector_type::iterator it = this->private_->data_slices_.begin();
    LayerCheckPointPrivate::data_slice_vector_type::iterator it_end = this->private_->data_slices_.end();

    while ( it != it_end )
    {
      size += (*it)->get_byte_size();
      ++it;
    }
  }
  {
    LayerCheckPointPrivate::mask_slice_vector_type::iterator it = this->private_->mask_slices_.begin();
    LayerCheckPointPrivate::mask_slice_vector_type::iterator it_end = this->private_->mask_slices_.end();

    while ( it != it_end )
    {
      size += (*it)->get_byte_size();
      ++it;
    }
  }
  
  return size;
}

  
} // end namespace Seg3D
