/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2011 Scientific Computing and Imaging Institute,
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
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/MRCLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>

// Boost includes
#include <boost/algorithm/string/case_conv.hpp>

// STL includes
#include <fstream>
#include <string>

#include <mrcheader.h>
// test
#include <MRCUtil.h>
// test

SEG3D_REGISTER_EXPORTER( Seg3D, MRCLayerExporter );

namespace Seg3D {

MRCLayerExporter::MRCLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers )
{
  if ( !layers[ 0 ] ) return;
}

bool MRCLayerExporter::export_layer( const std::string& mode,
                                     const std::string& file_path, 
                                     const std::string& name )
{
  boost::filesystem::path path = boost::filesystem::path( file_path ) / ( name + ".mrc" );
  bool success = false;

  if ( mode == "data" )
  {
    success = this->export_mrc( path.string() );
  }
  
  if (success)
    CORE_LOG_SUCCESS( "MRC export has been successfully completed." );

  return success;
}

bool MRCLayerExporter::export_mrc( const std::string& file_path )
{
  try {
    DataLayerHandle temp_handle = boost::dynamic_pointer_cast< DataLayer >( this->layers_[ 0 ] );
    LayerMetaData meta_data = temp_handle->get_meta_data();
    std::string type = boost::to_lower_copy(meta_data.meta_data_info_);
    if ( type.find( "mrc2000") == std::string::npos )
    {
      // error: not right type
      // test
      //std::cout << "type " << type << " is not MRC2000 header" << std::endl;
      // test
      return false;
    }
    Core::DataBlockHandle data_block_handle = temp_handle->get_data_volume()->get_data_block();
    Core::DataType dtype = data_block_handle->get_data_type();    

    MRC2000IO::MRCUtil mrcutil;
    MRC2000IO::MRCHeader header;
    mrcutil.import_header(meta_data.meta_data_, header);
    // update if needed
    if (header.nx != data_block_handle->get_nx())
      header.nx = data_block_handle->get_nx();

    if (header.ny != data_block_handle->get_ny())
      header.ny = data_block_handle->get_ny();
    
    if (header.nz != data_block_handle->get_nz())
      header.nz = data_block_handle->get_nz();
      
    if ( Core::DataBlock::IsLittleEndian() )
      header.machinestamp = MRC2000IO::LITTLE_ENDIAN_STAMP;
    else if ( Core::DataBlock::IsLittleEndian() )
      header.machinestamp = MRC2000IO::BIG_ENDIAN_STAMP;

    size_t length = data_block_handle->get_size() * Core::GetSizeDataType( dtype );

    std::ofstream out(file_path.c_str(), std::ios::out | std::ios::binary);
    if (! out )
    {
      //error_ = std::string("Failed to open file ") + filename;
      return false;
    }
    out.write(reinterpret_cast<char*>(&header), MRC_HEADER_LENGTH);
    out.write(reinterpret_cast<char*>(data_block_handle->get_data()), length);
  }
  catch (...)
  {
    //error_ = std::string("Failed to read header for file ") + filename;
    return false;
  }
  
  return true;
}


}