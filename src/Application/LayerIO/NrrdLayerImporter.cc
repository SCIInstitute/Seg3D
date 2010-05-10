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
#include <Core/DataBlock/NrrdData.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Volume/DataVolume.h>
#include <Core/Volume/MaskVolume.h>

// Application includes
#include <Application/LayerIO/NrrdLayerImporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>

namespace Seg3D
{

SCI_REGISTER_IMPORTER(NrrdLayerImporter);


bool NrrdLayerImporter::import_header()
{
  // Only import the data once
  if ( nrrd_data_ ) return true;
  
  // NOTE: We load the full data set, as Teem does not support reading headers only :(
  // Hence we need to read the full file
  std::string error;
  if ( ! ( Core::NrrdData::LoadNrrd( get_filename() , nrrd_data_, error ) ) )
  {
    set_error( error );
    return false;
  }
  return true;
}


Core::GridTransform NrrdLayerImporter::get_grid_transform()
{
  if ( nrrd_data_ )  return nrrd_data_->get_grid_transform();
  else return Core::GridTransform(1,1,1);
}


Core::DataType NrrdLayerImporter::get_data_type()
{
  if ( nrrd_data_ ) return nrrd_data_->get_data_type();
  else return Core::DataType::UNKNOWN_E;
}


int NrrdLayerImporter::get_importer_modes()
{
  Core::DataType data_type = nrrd_data_->get_data_type();
  
  int importer_modes = 0;
  if ( Core::IsReal( data_type ) )
  {
    importer_modes |= LayerImporterMode::DATA_E;
  }
  
  if ( Core::IsInteger( data_type ) ) 
  {
    importer_modes |= LayerImporterMode::SINGLE_MASK_E | LayerImporterMode::BITPLANE_MASK_E |
      LayerImporterMode::LABEL_MASK_E | LayerImporterMode::DATA_E;
  }
  
  return importer_modes;
}


bool NrrdLayerImporter::import_layer( LayerImporterMode mode, std::vector<LayerHandle>& layers )
{
  layers.clear();
  
  // ensure that the data has been read
  if ( ! nrrd_data_ ) import_header();
  
  switch (mode)
  {
    case LayerImporterMode::DATA_E:
    {
      CORE_LOG_DEBUG( std::string("Importing data layer: ") + get_base_filename() );
          
      Core::DataBlockHandle datablock( new Core::NrrdDataBlock( nrrd_data_ ) );
      datablock->update_histogram();
      
      Core::DataVolumeHandle datavolume( new Core::DataVolume( 
        nrrd_data_->get_grid_transform(), datablock ) );

      layers.resize( 1 );
      layers[0] = LayerHandle( new DataLayer( get_base_filename(), datavolume ) );

      CORE_LOG_DEBUG( std::string("Successfully imported: ") + get_base_filename() );
      return true;
    }
    case LayerImporterMode::SINGLE_MASK_E:
    {
      CORE_LOG_DEBUG( std::string("Importing mask layer: ") + get_base_filename() );


      Core::DataBlockHandle datablock( new Core::NrrdDataBlock( nrrd_data_ ) );
      Core::MaskDataBlockHandle maskdatablock;
      
      if ( !( Core::MaskDataBlockManager::CreateMaskFromNonZeroData( 
        datablock, maskdatablock ) ) ) 
      {
        return false;
      }

      Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
        nrrd_data_->get_grid_transform(), maskdatablock ) );
      
      layers.resize( 1 );
      layers[0] = LayerHandle( new MaskLayer( get_base_filename(), maskvolume ) );

      CORE_LOG_DEBUG( std::string("Successfully imported: ") + get_base_filename() );
      return true;
    }
    
    case LayerImporterMode::BITPLANE_MASK_E:
    {
      CORE_LOG_DEBUG( std::string("Importing mask layer: ") + get_base_filename() );


      Core::DataBlockHandle datablock( new Core::NrrdDataBlock( nrrd_data_ ) );
      std::vector<Core::MaskDataBlockHandle> maskdatablocks;
      
      if ( !( Core::MaskDataBlockManager::CreateMaskFromBitPlaneData( 
        datablock, maskdatablocks ) ) ) 
      {
        return false;
      }

      layers.resize( maskdatablocks.size() );

      for ( size_t j = 0; j < layers.size(); j++ )
      {
        Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
          nrrd_data_->get_grid_transform(), maskdatablocks[j] ) );
        layers[j] = LayerHandle( new MaskLayer( get_base_filename(), maskvolume ) );
      }
      
      CORE_LOG_DEBUG( std::string("Successfully imported: ") + get_base_filename() );
      return true;
    }
    case LayerImporterMode::LABEL_MASK_E:
    {
      CORE_LOG_DEBUG( std::string("Importing mask layer: ") + get_base_filename() );


      Core::DataBlockHandle datablock( new Core::NrrdDataBlock( nrrd_data_ ) );
      std::vector<Core::MaskDataBlockHandle> maskdatablocks;
      
      if ( !( Core::MaskDataBlockManager::CreateMaskFromLabelData( 
        datablock, maskdatablocks ) ) ) 
      {
        return false;
      }

      layers.resize( maskdatablocks.size() );

      for ( size_t j = 0; j < layers.size(); j++ )
      {
        Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
          nrrd_data_->get_grid_transform(), maskdatablocks[j] ) );
        layers[j] = LayerHandle( new MaskLayer( get_base_filename(), maskvolume ) );
      }
      
      CORE_LOG_DEBUG( std::string("Successfully imported: ") + get_base_filename() );
      return true;
    }   default:
      return false;
  }
}

} // end namespace seg3D
