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

// boost includes
#include <boost/algorithm/string/case_conv.hpp>

// itk includes
#include <itkImageSeriesWriter.h>
#include <itkImageSeriesReader.h>
#include <itkNumericSeriesFileNames.h>
#include <itkGDCMImageIO.h>
#include <itkMetaDataObject.h>

// GDCM includes
#include <gdcmImageHelper.h>
#include <gdcmUIDGenerator.h>

// Core includes
#include <Core/Volume/DataVolume.h>

// Application includes
#include <Application/LayerIO/ITKDataLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

SEG3D_REGISTER_EXPORTER( Seg3D, ITKDataLayerExporter );

namespace Seg3D
{
    
void set_data_series_names( itk::NumericSeriesFileNames::Pointer& name_series_generator, 
                           const std::string& file_path, const std::string& file_name, 
                           const size_t size, const std::string& extension )
{ 
    boost::filesystem::path path = boost::filesystem::path( file_path );
    
    // here we make sure that we dont have an extension and then afterwards we add the correct one
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
    
    
////////////// - Templated functions for exporting bitmaps and DICOM's - ///////////////
////////////////////////////////////////////////////////////////////////////////////////
template< class InputPixelType, class OutputPixelType >
bool export_nondicom_series( const std::string& file_path, const std::string& file_name, 
  DataLayerHandle temp_handle, const std::string& extension )
{
  typedef itk::Image< InputPixelType, 3 > ImageType;
  typedef itk::Image< OutputPixelType, 2 > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  Core::ITKImageDataHandle image_data = typename Core::ITKImageDataT< InputPixelType >::Handle( 
    new Core::ITKImageDataT< InputPixelType >( temp_handle->get_data_volume()->get_data_block(), 
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
  set_data_series_names( names_generator, file_path, file_name, size[ 2 ], extension );

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

template< class InputPixelType, class OutputPixelType >
bool export_dicom_series( const std::string& file_path, const std::string& file_name, 
  DataLayerHandle temp_handle, const std::string& extension )
{
  typedef itk::GDCMImageIO ImageIOType;
  ImageIOType::Pointer dicom_io = ImageIOType::New();

  typedef itk::Image< InputPixelType, 3 > ImageType;
  typedef itk::Image< OutputPixelType, 2 > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  Core::GridTransform grid_transform = temp_handle->get_grid_transform();

  Core::ITKImageDataHandle image_data = typename Core::ITKImageDataT< InputPixelType >::Handle( 
    new Core::ITKImageDataT< InputPixelType >( temp_handle->get_data_volume()->get_data_block(), 
    grid_transform ) );

  ImageType* itk_image = dynamic_cast< ImageType* >( 
    image_data->get_base_image().GetPointer() );

  typename ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
  typename ImageType::IndexType start = region.GetIndex();
  typename ImageType::SizeType size = region.GetSize();

  unsigned int first_slice = start[ 2 ];
  unsigned int last_slice = start[ 2 ] + size[ 2 ] - 1;

  typedef itk::NumericSeriesFileNames NamesGeneratorType;
  NamesGeneratorType::Pointer names_generator = NamesGeneratorType::New();
  set_data_series_names( names_generator, file_path, file_name, size[ 2 ], extension );

  names_generator->SetStartIndex( first_slice );
  names_generator->SetEndIndex( last_slice );
  names_generator->SetIncrementIndex( 1 );


  ///////////////////////////////////////////////////////////////////////////

  // Check whether header file was included
  bool has_header_file = false;
  bool is_dicom = false;

  LayerMetaData meta_data = temp_handle->get_meta_data();
  std::vector< std::string > header_files;
  ProjectHandle project = ProjectManager::Instance()->get_current_project();
  InputFilesID inputfiles_id = -1;


  if ( PreferencesManager::Instance()->export_dicom_headers_state_->get() )
  {
    if ( meta_data.meta_data_info_ == "dicom_filename" )
    {
      std::vector<std::string> parts = Core::SplitString( meta_data.meta_data_, "|" );
      Core::ImportFromString( parts[ 0 ], header_files );

      boost::filesystem::path full_file;
      boost::filesystem::path cached_file;
      if ( parts.size() > 1 ) Core::ImportFromString( parts[ 1 ], inputfiles_id );

      has_header_file = true;   
      for ( size_t j = 0; j < header_files.size(); j++ )
      {
        full_file = boost::filesystem::path( header_files[ j ] );
        // Check whether the files are cached inside the project, if so use the files
        // in the cache
        if ( inputfiles_id > -1 )
        {
          if ( project->find_cached_file( full_file, inputfiles_id, cached_file ) )
          {
            full_file = cached_file;
            header_files[ j ] = full_file.string();
          }
        }

        if ( ! boost::filesystem::exists( full_file ) )
        {
          has_header_file = false;
          break;
        }
      }
    }
  }

  std::string l_extension = boost::to_lower_copy( extension );
  if ( l_extension == ".dcm" || l_extension == ".dicom" || l_extension == ".ima" )
  {
    is_dicom = true;
  } 

  writer->SetInput( itk_image );
  writer->SetImageIO( dicom_io );
  writer->SetFileNames( names_generator->GetFileNames() );

  typedef typename itk::ImageSeriesReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader;
  typename ReaderType::DictionaryArrayType dict_array;

  if ( is_dicom )
  { 
    if ( has_header_file )
    {
      reader = ReaderType::New();
      reader->SetImageIO( dicom_io );
      reader->SetFileNames( header_files ); 
      try 
      {
        reader->Update();
      }
      catch (itk::ExceptionObject & err ) 
      {
        std::cerr << "error " << err << std::endl;
        return false;   
      }
      catch ( ... ) 
      {
        return false;
      }

      dict_array = *( reader->GetMetaDataDictionaryArray() );

    }
    else
    {
      size_t num_slices = names_generator->GetFileNames().size();
      for ( size_t j = 0; j < num_slices; j++ )
      {
        dict_array.push_back( new itk::MetaDataDictionary );
      }
    }

    std::string uid_prefix = dicom_io->GetUIDPrefix();
    gdcm::UIDGenerator::SetRoot( uid_prefix.c_str() );
    gdcm::UIDGenerator uid;

    double min_value = temp_handle->min_value_state_->get();
    double max_value = temp_handle->max_value_state_->get();

    std::string series_uid = uid.Generate();    
    std::string description = "Data Generated by Seg3D";
    std::string data_center = Core::ExportToString( 0.5 * ( min_value + max_value ) );
    std::string data_width = Core::ExportToString( max_value - min_value );
    
    double z_offset = 0.0;
    
    for ( typename ReaderType::DictionaryArrayType::iterator dict_iter = 
      dict_array.begin(); dict_iter != dict_array.end(); dict_iter++ )
    {
      std::string sop_uid = uid.Generate();

      // [Series Instance UID]
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0020|000e", series_uid );
      // [ Window Center ]
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0028|1050", data_center );
      // [ Window Width ]
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0028|1051", data_width );
      // [Series Description]
      itk::EncapsulateMetaData<std::string>( **dict_iter, "0008|103e", description );
      // [Bits Allocated] 
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0100", "" );
      // [Bits Stored]
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0101", "" );
      // [High Bit] 
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0102", "" );
      // [Pixel Representation]
      itk::EncapsulateMetaData<std::string>(**dict_iter, "0028|0103", "" ); 

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

      itk::EncapsulateMetaData< unsigned int >( **dict_iter, itk::ITK_NumberOfDimensions, 3 );
      itk::EncapsulateMetaData< DoubleArrayType >( **dict_iter, itk::ITK_Origin, originArray );
      itk::EncapsulateMetaData< DoubleArrayType >( **dict_iter, itk::ITK_Spacing, spacingArray );
    }

    // now tell the Dicom writer to use the provided UIDs
    dicom_io->KeepOriginalUIDOn();
    writer->SetMetaDataDictionaryArray( &dict_array );
  }
  ////////////////////////////////////////////////////////////////////////////
  
  gdcm::ImageHelper::SetForcePixelSpacing( true );

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

////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ITKDataLayerExporter ////////////////////////////////

ITKDataLayerExporter::ITKDataLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers ),
  pixel_type_( Core::DataType::UCHAR_E )
{
  if( !layers[ 0 ] ) return;
  this->pixel_type_ = layers[ 0 ]->get_data_type();
}

bool ITKDataLayerExporter::export_layer( const std::string& mode, const std::string& file_path, 
  const std::string& name )
{
  if ( this->extension_ == ".dcm" )
  {
    if ( ! this->export_dcm_series( file_path, name ) ) return false;
  }
  else if ( ! this->extension_.empty() )
  {
    if ( ! this->export_itk_series( file_path ) ) return false;
  }
  
  CORE_LOG_SUCCESS( "Data export has been successfully completed." );
  return true;
}
  
bool ITKDataLayerExporter::export_dcm_series( const std::string& file_path, const std::string& name )
{
  DataLayerHandle temp_handle =  boost::dynamic_pointer_cast< DataLayer >( this->layers_[ 0 ] );
  switch( this->layers_[ 0 ]->get_data_type() )
  {
    case Core::DataType::UCHAR_E:
      return export_dicom_series< unsigned char, unsigned char >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::CHAR_E:
      return export_dicom_series< signed char, signed char >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::USHORT_E:
      return export_dicom_series< unsigned short, unsigned short >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::SHORT_E:
      return export_dicom_series< signed short, signed short >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::UINT_E:
      return export_dicom_series< signed int, signed int >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::INT_E:
      return export_dicom_series< unsigned int, unsigned int >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::FLOAT_E:
      return export_dicom_series< float, signed short >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    case Core::DataType::DOUBLE_E:
      return export_dicom_series< double, signed short >( file_path, name, 
        temp_handle, this->extension_ );
      break;
    default:
      return false;
  }
  return false;
}
  
bool ITKDataLayerExporter::export_itk_series( const std::string& file_path )
{
  DataLayerHandle temp_handle =  boost::dynamic_pointer_cast< DataLayer >( this->layers_[ 0 ] );
  switch( this->layers_[ 0 ]->get_data_type() )
  {
  case Core::DataType::UCHAR_E:
    return export_nondicom_series< unsigned char, unsigned char >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::CHAR_E:
    return export_nondicom_series< signed char, signed char >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::USHORT_E:
    return export_nondicom_series< unsigned short, unsigned short >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::SHORT_E:
    return export_nondicom_series< signed short, signed short >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::UINT_E:
    return export_nondicom_series< signed int, signed int >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::INT_E:
    return export_nondicom_series< unsigned int, unsigned int >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::FLOAT_E:
    return export_nondicom_series< float, unsigned short >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  case Core::DataType::DOUBLE_E:
    return export_nondicom_series< double, unsigned short >( file_path, 
      ( temp_handle->get_layer_name() + this->extension_ ), temp_handle, this->extension_ );
    break;
  default:
    return false;
  }
  return false;
}

} // end namespace seg3D
