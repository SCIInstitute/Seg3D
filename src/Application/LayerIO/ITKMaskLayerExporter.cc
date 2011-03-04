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

// itk includes
#include <itkImageSeriesWriter.h>
#include <itkImageSeriesReader.h>
#include <itkNumericSeriesFileNames.h>
#include <itkGDCMImageIO.h>
#include <itkMetaDataObject.h>

// gdcm includes
#include <gdcmUIDGenerator.h>

// Core includes
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/ITKMaskLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

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

  Core::GridTransform grid_transform = temp_handle->get_grid_transform();
  Core::ITKImageDataHandle image_data = typename Core::ITKImageDataT< InputPixelType >::Handle( 
    new Core::ITKImageDataT< InputPixelType >( 
    temp_handle->get_mask_volume()->get_mask_data_block()->get_data_block(), 
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

  ///////////////////////////////////////////////////////////////////////////

  // Check whether header file was included
  bool has_header_file = false;
  bool is_dicom = false;

  LayerMetaData meta_data = temp_handle->get_meta_data();
  std::vector< std::string > header_files;
  if ( PreferencesManager::Instance()->export_dicom_headers_state_->get() )
  {
    if ( meta_data.meta_data_info_ == "dicom_filename" )
    {
      Core::ImportFromString( meta_data.meta_data_, header_files );
      boost::filesystem::path file( header_files[ 0 ] ); 
      has_header_file = true;
      if ( header_files.size() == 0 )
      {
        has_header_file = false;    
      }

      for ( size_t j = 0; j < header_files.size(); j++ )
      {
        if ( ! boost::filesystem::exists( header_files[ j ] ) )
        {
          has_header_file = false;
          break;
        }
      }
    }
  }
  
  std::string extension = boost::filesystem::path( file_name ).extension();
  boost::to_lower( extension );

  if ( extension == ".dcm" || extension == ".dicom" || extension == ".ima" )
  {
    is_dicom = true;
  } 

  typedef typename itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader;
  typename ReaderType::DictionaryArrayType dict_array;

  if ( has_header_file && is_dicom )
  { 
    itk::GDCMImageIO::Pointer gdcmio;

    gdcmio = itk::GDCMImageIO::New();

    reader = ReaderType::New();
    reader->SetFileNames( header_files ); 
    reader->SetImageIO( gdcmio );
    try 
    {
      reader->Update();
    } 
    catch ( ... ) 
    {
      return false;
    }

    dict_array = *( reader->GetMetaDataDictionaryArray() );

    std::string uid_prefix = gdcmio->GetUIDPrefix();
    gdcm::UIDGenerator::SetRoot( uid_prefix.c_str() );
    gdcm::UIDGenerator uid;

    std::string series_uid = uid.Generate();    
    std::string description = "Mask Generated by Seg3D";
    
    double z_offset = 0.0;
    for ( typename ReaderType::DictionaryArrayType::iterator dict_iter = 
      dict_array.begin(); dict_iter != dict_array.end(); dict_iter++ )
    {
      std::string sop_uid = uid.Generate();

      // [Series Instance UID]
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0020|000e", series_uid );
      // [SOP Instance UID]
      // itk::EncapsulateMetaData<std::string>( **dict_iter, "0008|0018", sop_uid );
      // [Media Stored SOP Instance UID] 
      // itk::EncapsulateMetaData<std::string>( **dict_iter, "0002|0003", sop_uid );
      // [Series Description]
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0008|103e", description );
      // [Bits Allocated] 
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0100", "");
      // [Bits Stored]
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0101", "");
      // [High Bit] 
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0102", "");
      // [Pixel Representation]
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0103", ""); 

      Core::Point slice_origin = grid_transform.project( Core::Point( 0.0, 0.0, z_offset ) );
      z_offset += 1.0;
      std::string patient_location = Core::ExportToString( slice_origin.x() ) + "\\" +
        Core::ExportToString( slice_origin.y() ) + "\\" + 
        Core::ExportToString( slice_origin.z() );
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0020|0032", patient_location ); 
  
      double spacing_x = grid_transform.project( Core::Vector( 1.0, 0.0, 0.0 ) ).length();
      double spacing_y = grid_transform.project( Core::Vector( 0.0, 1.0, 0.0 ) ).length();
      double spacing_z = grid_transform.project( Core::Vector( 0.0, 0.0, 1.0 ) ).length();

      std::string z_spacing = Core::ExportToString( spacing_z );
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0018|0050", z_spacing ); 
      
      typedef itk::Array< double > DoubleArrayType;
      DoubleArrayType originArray( 3 );
      DoubleArrayType spacingArray( 3 );
      
      originArray[ 0 ]  = slice_origin.x();
      originArray[ 1 ]  = slice_origin.y();
      originArray[ 2 ]  = slice_origin.z();
      spacingArray[ 0 ] = spacing_x;
      spacingArray[ 1 ] = spacing_y;
      spacingArray[ 2 ] = spacing_z;

      itk::EncapsulateMetaData< DoubleArrayType >( **dict_iter, itk::ITK_Origin, originArray );
      itk::EncapsulateMetaData< DoubleArrayType >( **dict_iter, itk::ITK_Spacing, spacingArray );
    }

    // now tell the Dicom writer to use the provided UIDs
    gdcmio->KeepOriginalUIDOn();
    writer->SetMetaDataDictionaryArray( &dict_array );
    writer->SetImageIO( gdcmio ); 
  }
  ////////////////////////////////////////////////////////////////////////////

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
  
  CORE_LOG_SUCCESS( "Segmentation export has been successfully completed." );
  // If we have successfully gone through all the layers return true
  return true;
}
  




  




} // end namespace seg3D
