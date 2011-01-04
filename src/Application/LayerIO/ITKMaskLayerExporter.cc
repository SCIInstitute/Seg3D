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
#include <Application/LayerIO/ITKMaskLayerExporter.h>
#include <Application/Layer/DataLayer.h>

SCI_REGISTER_EXPORTER( Seg3D, ITKMaskLayerExporter );

namespace Seg3D
{
////////////// - Templated functions for exporting bitmaps and DICOM's - ///////////////
////////////////////////////////////////////////////////////////////////////////////////
template< class InputPixelType, class OutputPixelType >
bool export_mask_series( const std::string& file_path, const std::string& file_name, 
  MaskLayerHandle temp_handle )
{
  typedef itk::Image< InputPixelType, 3 > ImageType;
  typedef itk::Image< OutputPixelType, 2 > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  Core::ITKImageDataHandle image_data = typename Core::ITKImageDataT< InputPixelType >::Handle( 
    new Core::ITKImageDataT< InputPixelType >( temp_handle->get_mask_volume()->get_mask_data_block()->get_data_block(), 
    temp_handle->get_grid_transform() ) );

  ImageType* itk_image = dynamic_cast< ImageType* >( 
    image_data->get_base_image().GetPointer() );

  typename ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
  typename ImageType::IndexType start = region.GetIndex();
  typename ImageType::SizeType size = region.GetSize();

  unsigned int first_slice = start[ 2 ];
  unsigned int last_slice = start[ 2 ] + size[ 2 ] - 1;


  typedef itk::NumericSeriesFileNames NamesGeneratorType;
  NamesGeneratorType::Pointer names_generator = NamesGeneratorType::New();
  set_mask_series_names( names_generator, file_path, file_name, size[ 2 ] );

  names_generator->SetStartIndex( first_slice );
  names_generator->SetEndIndex( last_slice );
  names_generator->SetIncrementIndex( 1 );

  writer->SetInput( itk_image );
  writer->SetFileNames( names_generator->GetFileNames() );

  try
  {
    writer->Update();
  }
  catch ( itk::ExceptionObject &err )
  {
    std::string itk_error = err.GetDescription();
    return false;
  }

  return true;
}

void set_mask_series_names( itk::NumericSeriesFileNames::Pointer& name_series_generator, 
  const std::string& file_path, const std::string& file_name, const size_t size )
{ 
  boost::filesystem::path path = boost::filesystem::path( file_path );

  std::string extension = boost::filesystem::path( file_name ).extension();

  std::string filename_without_extension = file_name;
  filename_without_extension = filename_without_extension.substr( 0, 
    filename_without_extension.find_last_of( "." ) );

  boost::filesystem::path filename_path = path / filename_without_extension;


  if( size < 100 )
  {
    name_series_generator->SetSeriesFormat( filename_path.string() + "-%02d" + extension );
  }
  else if ( size < 1000 )
  {
    name_series_generator->SetSeriesFormat( filename_path.string() + "-%03d" + extension );
  }
  else if ( size < 10000 )
  {
    name_series_generator->SetSeriesFormat( filename_path.string() + "-%04d" + extension );
  }
  else
  {
    name_series_generator->SetSeriesFormat( filename_path.string() + "-%10d" + extension );
  }
}


////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ITKMaskLayerExporter ////////////////////////////////
ITKMaskLayerExporter::ITKMaskLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers ),
  pixel_type_( Core::DataType::UCHAR_E )
{
  if( !layers[ 0 ] ) return;
  this->pixel_type_ = layers[ 0 ]->get_data_type();
}


Core::GridTransform ITKMaskLayerExporter::get_grid_transform()
{
  if( !this->layers_[ 0 ] ) return Core::GridTransform( 1, 1, 1 );
  return this->layers_[ 0 ]->get_grid_transform();
}


Core::DataType ITKMaskLayerExporter::get_data_type()
{
  if( !this->layers_[ 0 ] ) return Core::DataType::UNKNOWN_E;
  return this->layers_[ 0 ]->get_data_type();
}

int ITKMaskLayerExporter::get_exporter_modes()
{
  return LayerImporterMode::SINGLE_MASK_E;
}

bool ITKMaskLayerExporter::export_layer( LayerExporterMode mode, const std::string& file_path, 
  const std::string& name )
{
  for( int i = 0; i < static_cast< int >( this->layers_.size() ); ++i )
  {
    MaskLayerHandle temp_handle = boost::dynamic_pointer_cast< MaskLayer >( this->layers_[ i ] );

    switch( this->layers_[ i ]->get_data_type() )
    {
    case Core::DataType::UCHAR_E:
      export_mask_series< unsigned char, unsigned char >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::CHAR_E:
      export_mask_series< signed char, signed char >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::USHORT_E:
      export_mask_series< unsigned short, unsigned short >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::SHORT_E:
      export_mask_series< signed short, signed short >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::UINT_E:
      export_mask_series< signed int, signed int >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::INT_E:
      export_mask_series< unsigned int, unsigned int >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::FLOAT_E:
      export_mask_series< float, float >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    case Core::DataType::DOUBLE_E:
      export_mask_series< double, double >( file_path, 
        ( temp_handle->get_layer_name() + this->extension_ ), temp_handle );
      break;
    default:
      return false;
    }
  }
  // If we have successfully gone through all the layers return true
  return true;
}
  




  




} // end namespace seg3D
