/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/MRCLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// Boost includes
#include <boost/algorithm/string/case_conv.hpp>

// STL includes
#include <fstream>
#include <string>

#include <mrcheader.h>
#include <MRCUtil.h>

SEG3D_REGISTER_EXPORTER( Seg3D, MRCLayerExporter );

namespace Seg3D {

MRCLayerExporter::MRCLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers )
{}

bool MRCLayerExporter::export_layer( const std::string& mode,
                                     const std::string& file_path, 
                                     const std::string& name )
{
  boost::filesystem::path path = boost::filesystem::path( file_path ) / ( name + ".mrc" );
  bool success = false;

  // Export data as single volume file
  if ( mode == LayerIO::DATA_MODE_C )
  {
    success = this->export_mrc( path.string() );
  }
  // Export data as separate files
  else if ( mode == LayerIO::SINGLE_MASK_MODE_C )
  {
    success = this->export_single_masks( file_path );
  }
  // Export data as a single file
  else if ( mode == LayerIO::LABEL_MASK_MODE_C )
  {
    success = this->export_mask_label( path.string() );
  }
  
  if (success)
    CORE_LOG_SUCCESS( "MRC2000 export has been successfully completed." );

  return success;
}

bool MRCLayerExporter::export_single_masks( const std::string& path )
{
  try
  {
    for ( int i = 0; i < static_cast< int >( this->layers_.size() ); ++i )
    {
      // Step 1: Get a pointer to the mask so that we can get at it's MaskDataBlock
      MaskLayer* temp_handle = dynamic_cast< MaskLayer* >( this->layers_[ i ].get() );
      
      // Step 2: Get a pointer to the mask's MaskDataBlock
      Core::MaskDataBlockHandle mask_block = temp_handle->get_mask_volume()->get_mask_data_block();
      LayerMetaData meta_data = temp_handle->get_meta_data();
      
      std::string type = boost::to_lower_copy(meta_data.meta_data_info_);
      if ( type.find( "mrc2000") == std::string::npos )
      {
        CORE_LOG_ERROR( "Currently can only export data layers in MRC2000 format that were originally imported as MRC2000 data" );
        return false;
      }
      
      Core::DataBlock::shared_lock_type lock( mask_block->get_mutex() );    
      
      // Step 3: Using the size and type information from our mask's MaskDataBlock, we create a 
      // new empty DataBlock
      Core::DataType dtype = Core::DataType::SHORT_E;
      Core::DataBlockHandle new_data_block = Core::StdDataBlock::New( mask_block->get_nx(),
                                               mask_block->get_ny(), mask_block->get_nz(), dtype );
      
      // Step 4: Using the data in our mask's MaskDataBlock we set the values in our new DataBlock
      for ( size_t j = 0; j < mask_block->get_size(); ++j )
      {
        if ( mask_block->get_mask_at( j ) )
        {
          new_data_block->set_data_at( j, 1 );
        }
        else
        { 
          new_data_block->set_data_at( j, 0 );
        }
      }
      
      lock.unlock();
      
      // Step 5: Write MRC file (TODO: move to helper function)
      MRC2000IO::MRCUtil mrcutil;
      MRC2000IO::MRCHeader header;
      mrcutil.import_header(meta_data.meta_data_, header);
//    // TODO: need function to check grid transform against saved metadata
//    Core::GridTransform grid_transform = layer->get_grid_transform();
//    Core::Point origin = grid_transform * Core::Point( 0, 0, 0 );
//    header.nx = static_cast< int >( grid_transform->get_nx() );
//    header.ny = static_cast< int >( grid_transform->get_ny() );
//    header.nz = static_cast< int >( grid_transform->get_nz() );
//    header.nxstart = 0;
//    header.nystart = 0;
//    header.nzstart = 0;
      header.mode = MRC2000IO::MRC_SHORT;
      
      if ( Core::DataBlock::IsLittleEndian() )
        header.machinestamp = MRC2000IO::LITTLE_ENDIAN_STAMP;
      else if ( Core::DataBlock::IsLittleEndian() )
        header.machinestamp = MRC2000IO::BIG_ENDIAN_STAMP;
      
      size_t length = mask_block->get_size() * Core::GetSizeDataType( dtype );
      boost::filesystem::path mask_path = boost::filesystem::path( path ) /
        ( temp_handle->get_layer_name() + ".mrc" );

      std::ofstream out(mask_path.c_str(), std::ios::out | std::ios::binary);
      if (! out )
      {
    std::ostringstream oss;
    oss << "Failed to open file " << mask_path.c_str() << " for export.";
        CORE_LOG_ERROR( oss.str() );
        return false;
      }
      
      out.write(reinterpret_cast<char*>(&header), MRC_HEADER_LENGTH);
      out.write(reinterpret_cast<char*>(new_data_block->get_data()), length);    
      
    }
    return true;
  }
  catch (...)
  {
    CORE_LOG_ERROR("Failed to export MRC2000 mask file.");
    return false;
  }
}


bool MRCLayerExporter::export_mask_label( const std::string& file_path )
{
  try
  {
    // first layer is background layer...not fully formed

    // Step 1: We get a pointer to one of the MaskLayers so we can get its MaskDataBlock
    MaskLayer* layer = dynamic_cast< MaskLayer* >( this->layers_[ 1 ].get() );
    LayerMetaData meta_data = layer->get_meta_data();

    std::string type = boost::to_lower_copy(meta_data.meta_data_info_);
    if ( type.find( "mrc2000") == std::string::npos )
    {
      CORE_LOG_ERROR( "Currently can only export data layers in MRC2000 format that were originally imported as MRC2000 data" );
      return false;
    }

    // Step 2: Get a handle to its MaskDataBlock and use that to build a new DataBlockHandle of the 
    // same size and type.
    Core::MaskDataBlockHandle mask_block = layer->get_mask_volume()->get_mask_data_block();
    Core::DataType dtype = Core::DataType::SHORT_E;
    Core::DataBlockHandle new_data_block = Core::StdDataBlock::New( mask_block->get_nx(),
                                             mask_block->get_ny(), mask_block->get_nz(), dtype );
    
    // Step 3: Get the value the user set for the background and preset the contents of our new
    // datablock to that value
    for ( size_t i = 0; i < mask_block->get_size(); ++i )
    {
      new_data_block->set_data_at( i, this->label_values_[ 0 ] );
    }

    // Step 4: Loop through all the MaskLayers and insert their values into our new DataBlock
    for ( int i = 1; i < static_cast< int >( this->layers_.size() ); ++i )
    {
      layer = dynamic_cast< MaskLayer* >( this->layers_[ i ].get() );
      mask_block = layer->get_mask_volume()->get_mask_data_block();
      
      Core::DataBlock::shared_lock_type lock( mask_block->get_mutex() );
      
      for ( size_t j = 0; j < mask_block->get_size(); ++j )
      {
        if ( mask_block->get_mask_at( j ) )
        {
          new_data_block->set_data_at( j, this->label_values_[ i ] );
        }
      } 
      
      lock.unlock();
    }
    
    // Step 5: Write MRC file (TODO: move to helper function)
    MRC2000IO::MRCUtil mrcutil;
    MRC2000IO::MRCHeader header;
    mrcutil.import_header(meta_data.meta_data_, header);
//    // TODO: need function to check grid transform against saved metadata
//    Core::GridTransform grid_transform = layer->get_grid_transform();
//    Core::Point origin = grid_transform * Core::Point( 0, 0, 0 );
//    header.nx = static_cast< int >( grid_transform->get_nx() );
//    header.ny = static_cast< int >( grid_transform->get_ny() );
//    header.nz = static_cast< int >( grid_transform->get_nz() );
//    header.nxstart = 0;
//    header.nystart = 0;
//    header.nzstart = 0;
    header.mode = MRC2000IO::MRC_SHORT;
    
    if ( Core::DataBlock::IsLittleEndian() )
      header.machinestamp = MRC2000IO::LITTLE_ENDIAN_STAMP;
    else if ( Core::DataBlock::IsLittleEndian() )
      header.machinestamp = MRC2000IO::BIG_ENDIAN_STAMP;
    
    size_t length = mask_block->get_size() * Core::GetSizeDataType( dtype );
    
    std::ofstream out(file_path.c_str(), std::ios::out | std::ios::binary);
    if (! out )
    {
      CORE_LOG_ERROR( std::string("Failed to open file ") + file_path + std::string( " for export.") );
      return false;
    }

    out.write(reinterpret_cast<char*>(&header), MRC_HEADER_LENGTH);
    out.write(reinterpret_cast<char*>(new_data_block->get_data()), length);    
  }
  catch (...)
  {
    CORE_LOG_ERROR("Failed to export MRC2000 mask file.");
    return false;
  }

  return true;
}
  

bool MRCLayerExporter::export_mrc( const std::string& file_path )
{
  try {
    DataLayerHandle temp_handle = boost::dynamic_pointer_cast< DataLayer >( this->layers_[ 0 ] );
    LayerMetaData meta_data = temp_handle->get_meta_data();
    std::string type = boost::to_lower_copy(meta_data.meta_data_info_);
    if ( type.find( "mrc2000") == std::string::npos )
    {
      CORE_LOG_ERROR( "Currently can only export data layers in MRC2000 format that were originally imported as MRC2000 data" );
      return false;
    }
    Core::DataBlockHandle data_block_handle = temp_handle->get_data_volume()->get_data_block();
    Core::DataType dtype = data_block_handle->get_data_type();    

    MRC2000IO::MRCUtil mrcutil;
    MRC2000IO::MRCHeader header;
    mrcutil.import_header(meta_data.meta_data_, header);
//    // TODO: need function to check grid transform against saved metadata
//    Core::GridTransform grid_transform = layer->get_grid_transform();
//    Core::Point origin = grid_transform * Core::Point( 0, 0, 0 );
//    header.nx = static_cast< int >( grid_transform->get_nx() );
//    header.ny = static_cast< int >( grid_transform->get_ny() );
//    header.nz = static_cast< int >( grid_transform->get_nz() );
//    header.nxstart = 0;
//    header.nystart = 0;
//    header.nzstart = 0;
    
    if ( Core::DataBlock::IsLittleEndian() )
      header.machinestamp = MRC2000IO::LITTLE_ENDIAN_STAMP;
    else if ( Core::DataBlock::IsLittleEndian() )
      header.machinestamp = MRC2000IO::BIG_ENDIAN_STAMP;

    size_t length = data_block_handle->get_size() * Core::GetSizeDataType( dtype );

    std::ofstream out(file_path.c_str(), std::ios::out | std::ios::binary);
    if (! out )
    {
      CORE_LOG_ERROR( std::string("Failed to open file ") + file_path + std::string( " for export.") );
      return false;
    }
    out.write(reinterpret_cast<char*>(&header), MRC_HEADER_LENGTH);
    out.write(reinterpret_cast<char*>(data_block_handle->get_data()), length);
  }
  catch (...)
  {
    CORE_LOG_ERROR("Failed to export MRC2000 data volume.");
    return false;
  }

  return true;
}


}