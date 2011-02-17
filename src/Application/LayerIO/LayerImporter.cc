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
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerIO/LayerImporter.h>

namespace Seg3D
{

std::string ExportToString( LayerImporterMode mode )
{
  switch ( mode )
  {
    case LayerImporterMode::DATA_E: return "data";
    case LayerImporterMode::SINGLE_MASK_E: return "single_mask";
    case LayerImporterMode::BITPLANE_MASK_E: return "bitplane_mask";
    case LayerImporterMode::LABEL_MASK_E: return "label_mask";
    default: return "unknown";
  }
}

bool ImportFromString( const std::string& import_type_string, LayerImporterMode& mode )
{
  std::string import_type = import_type_string;
  boost::to_lower( import_type );
  boost::erase_all( import_type , " " );

  if ( import_type == "data" )
  {
    mode = LayerImporterMode::DATA_E;
    return true;
  }
  else if ( import_type == "singlemask" || import_type == "single_mask" || import_type == "mask" )
  {
    mode = LayerImporterMode::SINGLE_MASK_E;
    return true;
  }
  else if ( import_type == "bitplane_mask" || import_type == "bitplanemask" )
  {
    mode = LayerImporterMode::BITPLANE_MASK_E;
    return true;
  }
  else if ( import_type == "label_mask" || import_type == "labelmask" )
  {
    mode = LayerImporterMode::LABEL_MASK_E;
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////
// Class LayerImporterPrivate
//////////////////////////////////////////////////////////////////////////

class LayerImporterPrivate
{
public:
  std::string error_; 
  std::string filename_;
  bool swap_xy_spacing_;
};

//////////////////////////////////////////////////////////////////////////
// Class LayerImporter
//////////////////////////////////////////////////////////////////////////

LayerImporter::LayerImporter( const std::string& filename ) :
  private_( new LayerImporterPrivate )
{
  this->private_->filename_ = filename;
  this->private_->swap_xy_spacing_ = false;
}

LayerImporter::~LayerImporter()
{
}

std::string LayerImporter::get_filename() 
{ 
  return this->private_->filename_; 
}

std::string LayerImporter::get_base_filename() 
{ 
  boost::filesystem::path full_filename( this->private_->filename_ );
  return full_filename.stem() ; 
}

bool LayerImporter::check_header()
{
  return true; 
}

bool LayerImporter::import_layer( LayerImporterMode mode, std::vector< LayerHandle >& layers )
{
  layers.clear();

  Core::DataBlockHandle data_block;
  Core::GridTransform grid_transform;
  LayerMetaData meta_data;

  if ( !this->load_data( data_block, grid_transform, meta_data ) )
  {
    return false;
  }
  
  Core::DataVolumeHandle src_volume( new Core::DataVolume( grid_transform, data_block ) );
  Core::DataVolumeHandle canonical_vol;
  Core::DataVolume::ConvertToCanonicalVolume( src_volume, canonical_vol );

  switch ( mode )
  {
  case LayerImporterMode::DATA_E:
    {
      CORE_LOG_DEBUG( std::string( "Importing data layer: " ) + this->get_base_filename() );

      canonical_vol->get_data_block()->update_histogram();

      layers.resize( 1 );
      layers[0] = LayerHandle( new DataLayer( this->get_layer_name(), canonical_vol ) );
      layers[ 0 ]->set_meta_data( meta_data );

      CORE_LOG_DEBUG( std::string( "Successfully imported: " ) + this->get_base_filename() );
      return true;
    }
  case LayerImporterMode::SINGLE_MASK_E:
    {
      CORE_LOG_DEBUG( std::string( "Importing mask layer: " ) + this->get_base_filename() );

      Core::MaskDataBlockHandle maskdatablock;

      if ( !( Core::MaskDataBlockManager::CreateMaskFromNonZeroData( 
        canonical_vol->get_data_block(), canonical_vol->get_grid_transform(), maskdatablock ) ) ) 
      {
        return false;
      }

      Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
        canonical_vol->get_grid_transform(), maskdatablock ) );

      layers.resize( 1 );
      layers[0] = LayerHandle( new MaskLayer( this->get_layer_name(), maskvolume ) );
      layers[ 0 ]->set_meta_data( meta_data );

      CORE_LOG_DEBUG( std::string( "Successfully imported: " ) + this->get_base_filename() );
      return true;
    }
  case LayerImporterMode::BITPLANE_MASK_E:
    {
      CORE_LOG_DEBUG( std::string( "Importing mask layer: " ) + this->get_base_filename() );

      std::vector<Core::MaskDataBlockHandle> maskdatablocks;

      if ( !( Core::MaskDataBlockManager::CreateMaskFromBitPlaneData( 
        canonical_vol->get_data_block(), canonical_vol->get_grid_transform(), maskdatablocks ) ) ) 
      {
        return false;
      }

      layers.resize( maskdatablocks.size() );

      for ( size_t j = 0; j < layers.size(); j++ )
      {
        Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
          canonical_vol->get_grid_transform(), maskdatablocks[ j ] ) );
        layers[ j ] = LayerHandle( new MaskLayer( this->get_layer_name(), maskvolume ) );
        layers[ j ]->set_meta_data( meta_data );
      }

      CORE_LOG_DEBUG( std::string( "Successfully imported: " ) + this->get_base_filename() );
      return true;
    }
  case LayerImporterMode::LABEL_MASK_E:
    {
      CORE_LOG_DEBUG( std::string( "Importing mask layer: " ) + this->get_base_filename() );

      std::vector<Core::MaskDataBlockHandle> maskdatablocks;

      if ( !( Core::MaskDataBlockManager::CreateMaskFromLabelData( 
        canonical_vol->get_data_block(), canonical_vol->get_grid_transform(), maskdatablocks ) ) ) 
      {
        return false;
      }

      layers.resize( maskdatablocks.size() );

      for ( size_t j = 0; j < layers.size(); j++ )
      {
        Core::MaskVolumeHandle maskvolume( new Core::MaskVolume( 
          canonical_vol->get_grid_transform(), maskdatablocks[ j ] ) );
        layers[ j ] = LayerHandle( new MaskLayer( get_layer_name(), maskvolume ) );
        layers[ j ]->set_meta_data( meta_data );        
      }

      CORE_LOG_DEBUG( std::string( "Successfully imported: " ) + get_base_filename() );
      return true;
    }
  default:
    return false;
  }
}

std::string LayerImporter::get_layer_name()
{
  return this->get_base_filename();
}

bool LayerImporter::set_file_list( const std::vector< std::string >& file_list ) 
{ 
  return false; 
}

std::vector< std::string > LayerImporter::get_file_list()
{
  std::vector< std::string > empty_list;
  return empty_list;
}

void LayerImporter::set_error( const std::string& error )
{
  this->private_->error_ = error;
}

std::string LayerImporter::get_error() const
{
  return this->private_->error_;
}

void LayerImporter::set_swap_xy_spacing( bool swap )
{
  this->private_->swap_xy_spacing_ = swap;
}

bool LayerImporter::get_swap_xy_spacing() const
{
  return this->private_->swap_xy_spacing_;
}

} // end namespace seg3D
