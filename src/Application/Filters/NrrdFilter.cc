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
 
// ITK includes 
#include <itkCommand.h>
  
// Core includes
#include <Core/Utils/Exception.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/NrrdData.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Filters/NrrdFilter.h>

 
namespace Seg3D
{


NrrdFilter::NrrdFilter() 
{
}

NrrdFilter::~NrrdFilter()
{
}

bool NrrdFilter::get_nrrd_from_layer( const LayerHandle& layer, Core::NrrdDataHandle& nrrd_data )
{
  nrrd_data.reset();

  Core::DataBlockHandle data_block;
  Core::GridTransform transform;
  
  // If the layer is a data layer
  if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    DataLayerHandle data = boost::dynamic_pointer_cast<DataLayer>( layer );
    Core::DataVolumeHandle volume = data->get_data_volume();
    
    data_block = volume->get_data_block();      
    transform = volume->get_grid_transform();
  }
  // If the layer is a mask layer
  else if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    MaskLayerHandle mask = boost::dynamic_pointer_cast<MaskLayer>( layer );
    Core::MaskVolumeHandle volume = mask->get_mask_volume();

    // We always need to convert the data, Teem does not support the compressed way of
    // dealing with bitplanes
    if ( ! ( Core::MaskDataBlockManager::Convert( volume->get_mask_data_block(), 
      data_block, Core::DataType::UCHAR_E ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }   
    transform = volume->get_grid_transform();
  }
  else
  {
    // Unknown data type
    return false;
  }

  // Create a new ITK Image wrapper
  nrrd_data = Core::NrrdDataHandle( new Core::NrrdData( data_block, transform ) );

  if ( !nrrd_data ) return false;
  // Success
  return true;    
}

bool NrrdFilter::create_nrrd( Core::NrrdDataHandle& nrrd_data )
{
  nrrd_data = Core::NrrdDataHandle( new Core::NrrdData( nrrdNew(), true) );
  return true;
}

bool NrrdFilter::insert_nrrd_into_layer( const LayerHandle& layer, Core::NrrdDataHandle nrrd_data )
{
  // If the layer is a data layer
  if ( layer->get_type() == Core::VolumeType::DATA_E )
  {
    DataLayerHandle data_layer = boost::dynamic_pointer_cast<DataLayer>( layer );

    // Wrap an NrrdData object around the nrrd object
    Core::DataVolumeHandle data_volume( new Core::DataVolume( 
      data_layer->get_grid_transform(), Core::NrrdDataBlock::New( nrrd_data ) ) );
      
    if ( !data_volume ) 
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    } 
          
    // NOTE: Do not need an update of the generation number as this volume 
    // was generated from scratch. But it will need a new histogram
    this->dispatch_insert_data_volume_into_layer( data_layer, data_volume, true );
    return true;
  }
  // If the layer is a mask layer
  else if ( layer->get_type() == Core::VolumeType::MASK_E )
  {
    MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( layer );
        
    // Need to make an intermediate representation as a datablock of the data
    // before it can be uploaded into the mask.
    // NOTE: This only generates a wrapper, no new data is generated
    Core::DataBlockHandle data_block = Core::NrrdDataBlock::New( nrrd_data );
    
    if ( !data_block )  
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }     
    
    // NOTE: The only reason we need the transform here, is that data blocks are
    // sorted by grid transform so we can use the nrrd library to save them, the
    // latter only allows storing one transform per data block, and since 8 masks
    // share one datablock, they are required for now to have the same transform.
    Core::MaskDataBlockHandle mask;

    if (!( Core::MaskDataBlockManager::Convert( data_block, 
      mask_layer->get_grid_transform(), mask ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }
    
    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      mask_layer->get_grid_transform(), mask ) );
      
    if ( !mask_volume ) 
    {
      this->report_error( "Could not allocate enough memory." );
      return false;
    }     

    // NOTE:Do not need an update of the generation number as this volume 
    // was generated from scratch.
    this->dispatch_insert_mask_volume_into_layer( mask_layer, mask_volume );
    return true;
  }
  return false;
}

void NrrdFilter::update_progress( LayerHandle layer, double amount )
{
  layer->update_progress_signal_( amount );
}

} // end namespace Core
