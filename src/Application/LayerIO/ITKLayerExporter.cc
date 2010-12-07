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

// Application includes
#include <Application/LayerIO/ITKLayerExporter.h>
#include <Application/Layer/DataLayer.h>

SCI_REGISTER_EXPORTER( Seg3D, ITKLayerExporter );

namespace Seg3D
{

ITKLayerExporter::ITKLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers ),
  pixel_type_( Core::DataType::UCHAR_E )
{
  if( !layers[ 0 ] ) return;
  this->pixel_type_ = layers[ 0 ]->get_data_type();
}


Core::GridTransform ITKLayerExporter::get_grid_transform()
{
  if( !this->layers_[ 0 ] ) return Core::GridTransform( 1, 1, 1 );
  return this->layers_[ 0 ]->get_grid_transform();
}


Core::DataType ITKLayerExporter::get_data_type()
{
  if( !this->layers_[ 0 ] ) return Core::DataType::UNKNOWN_E;
  return this->layers_[ 0 ]->get_data_type();
}

int ITKLayerExporter::get_exporter_modes()
{
  int exporter_modes = 0;
  exporter_modes |= LayerExporterMode::DATA_E | LayerImporterMode::SINGLE_MASK_E;
  return exporter_modes;
}

bool ITKLayerExporter::export_layer( LayerExporterMode mode, const std::string& file_path, 
  const std::string& name )
{
  if( mode != LayerExporterMode::DATA_E ) return false;
  switch( this->layers_[ 0 ]->get_data_type() )
  {
    case Core::DataType::UCHAR_E:
      return this->export_dicom_series< unsigned char, unsigned char >( file_path, name );
      break;
    case Core::DataType::CHAR_E:
      return this->export_dicom_series< signed char, signed char >( file_path, name );
      break;
    case Core::DataType::USHORT_E:
      return this->export_dicom_series< unsigned short, unsigned short >( file_path, name );
      break;
    case Core::DataType::SHORT_E:
      return this->export_dicom_series< signed short, signed short >( file_path, name );
      break;
    case Core::DataType::UINT_E:
      return this->export_dicom_series< signed int, signed int >( file_path, name );
      break;
    case Core::DataType::INT_E:
      return this->export_dicom_series< unsigned int, unsigned int >( file_path, name );
      break;
    case Core::DataType::FLOAT_E:
      return this->export_dicom_series< float, signed short >( file_path, name );
      break;
    case Core::DataType::DOUBLE_E:
      return this->export_dicom_series< double, signed short >( file_path, name );
      break;
    default:
      return false;
  }
}

} // end namespace seg3D
