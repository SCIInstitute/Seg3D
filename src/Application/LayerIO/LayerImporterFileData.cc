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

// Core includes
#include <Core/Volume/DataVolume.h>
#include <Core/Volume/MaskVolume.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerIO/LayerImporterFileData.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>


namespace Seg3D
{

class LayerImporterFileDataPrivate
{
public:
  LayerImporterFileDataPrivate() :
    name_( "New Layer" )
  {
  }

  // The transform of the data contained in the file
  Core::GridTransform grid_transform_;

  // Meta data contained in the file
  LayerMetaData meta_data_;

  // DataBlock with all the data
  Core::DataBlockHandle data_block_;

  // Suggested name for the new layer
  std::string name_;
};


LayerImporterFileData::LayerImporterFileData() :
  private_( new LayerImporterFileDataPrivate )
{
}

LayerImporterFileData::~LayerImporterFileData()
{
}

const Core::GridTransform& LayerImporterFileData::get_grid_transform() const
{
  return this->private_->grid_transform_;
}

void LayerImporterFileData::set_grid_transform( const Core::GridTransform& grid_transform )
{
  this->private_->grid_transform_ = grid_transform;
}

const std::string& LayerImporterFileData::get_name() const
{
  return this->private_->name_;
}
  
void LayerImporterFileData::set_name( const std::string& name )
{
  this->private_->name_ = name;
}
  
const LayerMetaData& LayerImporterFileData::get_meta_data() const
{
  return this->private_->meta_data_;
}
  
void LayerImporterFileData::set_meta_data( const LayerMetaData& meta_data )
{
  this->private_->meta_data_ = meta_data;
}
  
Core::DataBlockHandle LayerImporterFileData::get_data_block() const
{
  return this->private_->data_block_;
}
  
void LayerImporterFileData::set_data_block( const Core::DataBlockHandle& data_block )
{
  this->private_->data_block_ = data_block;
}

bool LayerImporterFileData::convert_to_layers( const std::string& mode, 
  std::vector< LayerHandle >& layers )
{
  // Clear out the data that is contained in this vector to ensure it is empty in case we fail.
  layers.clear();
  
  // Create a datablock structure which we use for importing the data.
  Core::DataVolumeHandle src_volume( new Core::DataVolume( 
    this->private_->grid_transform_, this->private_->data_block_ ) );

  // NOTE:
  // As data can be oriented in any order, we need to correct it, so the fastest data direction
  // conincides with x, the second fastest with y and the last one with z. Also ensure that the
  // data has a right handed coordinate system and that the data is located along the positive
  // x, y, and z axis
  // NOTE:
  // If data is not properly aligned with any axis, this function will ensure that it will 
  // afterwards, as the current architecture does not support any non axis aligned data.
  Core::DataVolumeHandle canonical_vol;
  Core::DataVolume::ConvertToCanonicalVolume( src_volume, canonical_vol );

  if ( mode == LayerIO::DATA_MODE_C )
  {
    // Data Layers use the histogram. Hence update the histogram, so it is present when needed
    // in the user interface.
    canonical_vol->get_data_block()->update_histogram();

    // This mode only generates 1 layer.
    layers.resize( 1 );
    
    // Create the new layer using the suggested name.
    // NOTE: If the name already exists a new name will be generated on the fly.
    layers[ 0 ] = LayerHandle( new DataLayer( this->private_->name_, canonical_vol ) );

    // Copy the meta data that will be propagated throughout the full system.
    // In Seg3D each layer has a "parent" and will copy the meta data from this parent.
    // Hence this will ensure that data naturally flows through the system.
    layers[ 0 ]->set_meta_data( this->private_->meta_data_ );

    // Done
    return true;
  }
  else if ( mode == LayerIO::SINGLE_MASK_MODE_C )
  {
    // Handle to a new MaskDataBlock
    Core::MaskDataBlockHandle maskdatablock;

    // Create a new MaskData Block and copy the just imported data into this new mask data block.
    if ( !( Core::MaskDataBlockManager::CreateMaskFromNonZeroData( 
      canonical_vol->get_data_block(), canonical_vol->get_grid_transform(), maskdatablock ) ) ) 
    {
      // Convertion failed
      return false;
    }

    // Create a new MaskVolume
    Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
      canonical_vol->get_grid_transform(), maskdatablock ) );

    // There will be only only layer
    layers.resize( 1 );
    layers[ 0 ] = LayerHandle( new MaskLayer( this->private_->name_, maskvolume ) );

    if ( !layers[ 0 ] )
    {
      // Most likely out of memory
      layers.clear();
      return false;
    }

    // Copy the meta data that will be propagated throughout the full system.
    // In Seg3D each layer has a "parent" and will copy the meta data from this parent.
    // Hence this will ensure that data naturally flows through the system.
    layers[ 0 ]->set_meta_data( this->private_->meta_data_ );

    return true;
  }
  else if ( mode == LayerIO::BITPLANE_MASK_MODE_C )
  {
    // Create a vector of mask data block handles in which we will store the mask data blocks.
    std::vector<Core::MaskDataBlockHandle> maskdatablocks;

    // The next function takes a normal data block and extracts every bit plane that has data
    // contained in it.
    if ( !( Core::MaskDataBlockManager::CreateMaskFromBitPlaneData( 
      canonical_vol->get_data_block(), canonical_vol->get_grid_transform(), 
      maskdatablocks ) ) ) 
    {
      // Failed to convert to a vector of layers. Most likely it ran out of memory
      return false;
    }

    // Resize the output vector to the number of maskdatablocks we have
    layers.resize( maskdatablocks.size() );

    // Loop over all of them and generate the actual layer structures
    for ( size_t j = 0; j < layers.size(); j++ )
    {
      // Create the new mask volume
      Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
        canonical_vol->get_grid_transform(), maskdatablocks[ j ] ) );
      // NOTE: If the name alrady exists, a new name will be generated on the fly
      layers[ j ] = LayerHandle( new MaskLayer( this->private_->name_, maskvolume ) );
      
      // Check if it was actually succesful
      if ( !layers[ j ] )
      {
        // Most likely we are out of memory
        layers.clear();
        return false;
      }
      
      // Copy the meta data that will be propagated throughout the full system.
      // In Seg3D each layer has a "parent" and will copy the meta data from this parent.
      // Hence this will ensure that data naturally flows through the system.
      layers[ j ]->set_meta_data( this->private_->meta_data_ );
    }
    
    // Successful
    return true;
  }
  else if ( mode == LayerIO::LABEL_MASK_MODE_C )
  {
    std::vector<Core::MaskDataBlockHandle> maskdatablocks;

    // Extract a mask for each value mentioned in the data
    if ( !( Core::MaskDataBlockManager::CreateMaskFromLabelData( 
      canonical_vol->get_data_block(), canonical_vol->get_grid_transform(), maskdatablocks ) ) ) 
    {
      // Most likely ran out of memory
      // If there are a lot of labels and this is not really a labelmap, this may happen
      // Hence we should fail gracefully.
      return false;
    }

    // Create the vector layers
    layers.resize( maskdatablocks.size() );

    for ( size_t j = 0; j < layers.size(); j++ )
    {
      Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
        canonical_vol->get_grid_transform(), maskdatablocks[ j ] ) );
        
      // NOTE: If the name alrady exists, a new name will be generated on the fly
      layers[ j ] = LayerHandle( new MaskLayer( this->private_->name_, maskvolume ) );
      // Copy the meta data that will be propagated throughout the full system.
      // In Seg3D each layer has a "parent" and will copy the meta data from this parent.
      // Hence this will ensure that data naturally flows through the system.

      // Check if it was actually succesful
      if ( !layers[ j ] )
      {
        // Most likely we are out of memory
        layers.clear();
        return false;
      }

      layers[ j ]->set_meta_data( this->private_->meta_data_ );
    }

    return true;
  }
  
  // If we did not hit any of the if statements the importer mode is invalid
  // hence return false.
  return false;
}

} // end namespace seg3D
