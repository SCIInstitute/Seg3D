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

// Utils includes
#include <Utils/DataBlock/NrrdData.h>
#include <Utils/DataBlock/NrrdDataBlock.h>
#include <Utils/Volume/DataVolume.h>
#include <Utils/Volume/MaskVolume.h>

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
  if ( ! ( Utils::NrrdData::LoadNrrd( get_filename() , nrrd_data_, error ) ) )
  {
    set_error( error );
    return false;
  }
  return true;
}


Utils::GridTransform NrrdLayerImporter::get_grid_transform()
{
  if ( nrrd_data_ )  return nrrd_data_->get_grid_transform();
  else return Utils::GridTransform(1,1,1);
}


Utils::DataType NrrdLayerImporter::get_data_type()
{
  if ( nrrd_data_ ) return nrrd_data_->get_data_type();
  else return Utils::DataType::UNKNOWN_E;
}


bool NrrdLayerImporter::has_importer_mode( LayerImporterMode mode )
{
  Utils::DataType data_type = nrrd_data_->get_data_type();
  
  if ( mode == LayerImporterMode::DATA_E && ( Utils::IsInteger( data_type ) ||
    Utils::IsReal( data_type ) ) )
  {
    return true;
  }
  
  if ( ( mode == LayerImporterMode::SINGLE_MASK_E || mode == LayerImporterMode::BITPLANE_MASK_E ||
     mode == LayerImporterMode::LABEL_MASK_E ) && Utils::IsInteger( data_type ) ) 
  {
    return true;
  }

  return false;
}


bool NrrdLayerImporter::import_layer( LayerImporterMode mode, std::vector<LayerHandle>& layers )
{
  // ensure that the data has been read
  if ( ! nrrd_data_ ) import_header();
  
  switch (mode)
  {
    case LayerImporterMode::DATA_E:
    {
      layers.resize( 1 );
          
      Utils::DataBlockHandle datablock( new Utils::NrrdDataBlock( nrrd_data_ ) );
      Utils::DataVolumeHandle datavolume( new 
        Utils::DataVolume( nrrd_data_->get_grid_transform(), datablock ) );

      layers[0] = LayerHandle( new DataLayer( get_base_filename(), datavolume ) );
      return true;
    }
    case LayerImporterMode::SINGLE_MASK_E:
      return false;
    case LayerImporterMode::BITPLANE_MASK_E:
      return false;
    case LayerImporterMode::LABEL_MASK_E:
      return false; 
    default:
      return false;
  }
}

} // end namespace seg3D
