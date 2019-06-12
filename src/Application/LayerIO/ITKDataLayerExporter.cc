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

// boost includes
#include <boost/algorithm/string/case_conv.hpp>

// itk includes
#include <itkImageFileWriter.h>
#include <itkImageSeriesWriter.h>
#include <itkImageSeriesReader.h>
#include <itkNumericSeriesFileNames.h>
#include <itkGDCMImageIO.h>
#include <itkMetaDataObject.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// GDCM includes
#include <gdcmImageHelper.h>
#include <gdcmUIDGenerator.h>

// Core includes
#include <Core/Volume/DataVolume.h>
#include <Core/Utils/FilesystemUtil.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/LayerIO/ITKDataLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

#define DIM_3D 3
#define DIM_2D 2

SEG3D_REGISTER_EXPORTER( Seg3D, ITKDataLayerExporter );

namespace Seg3D
{

template< class InputPixelType, class OutputPixelType >
typename itk::Image< OutputPixelType, DIM_3D >::Pointer
cast_image( typename Core::ITKImageDataT< InputPixelType >::Handle image_data )
{
  typedef itk::Image< InputPixelType, DIM_3D > InputImageType;
  typedef itk::Image< OutputPixelType, DIM_3D > OutputImageType;
  typedef typename Core::ITKImageDataT< OutputPixelType > OutputType;

  typename InputImageType::Pointer itk_image = image_data->get_image();

  typedef itk::RescaleIntensityImageFilter< InputImageType, InputImageType > RescaleType;
  typename RescaleType::Pointer rescale = RescaleType::New();

  typedef itk::CastImageFilter< InputImageType, OutputImageType > CastFilterType;
  typename CastFilterType::Pointer castFilter = CastFilterType::New();

  // assumes casting to type with smaller range...
  rescale->SetInput( itk_image );
  rescale->Update();
  rescale->SetOutputMinimum( itk::NumericTraits< OutputPixelType >::min() );
  rescale->SetOutputMaximum( itk::NumericTraits< OutputPixelType >::max() );

  auto outputMin = rescale->GetOutputMinimum();
  auto outputMax = rescale->GetOutputMaximum();

  if (outputMin > outputMax)
  {
    CORE_LOG_ERROR("Signed/unsigned mismatch: please convert your data to float using the arithmetic filter.");
  }
  else
  {
    
    castFilter->SetInput(rescale->GetOutput());
    castFilter->Update();
  }

   return castFilter->GetOutput();
}

template< class InputPixelType, class OutputPixelType >
typename itk::Image< OutputPixelType, DIM_3D >::Pointer
direct_cast_image(typename Core::ITKImageDataT< InputPixelType >::Handle image_data)
{
  typedef itk::Image< InputPixelType, DIM_3D > InputImageType;
  typedef itk::Image< OutputPixelType, DIM_3D > OutputImageType;
  typedef typename Core::ITKImageDataT< OutputPixelType > OutputType;

  typename InputImageType::Pointer itk_image = image_data->get_image();

  typedef itk::MinimumMaximumImageCalculator<InputImageType> MinimumMaximumImageCalculatorType;
  typename MinimumMaximumImageCalculatorType::Pointer MinMaxCalc = MinimumMaximumImageCalculatorType::New();
  MinMaxCalc->SetImage(itk_image);
  MinMaxCalc->Compute();
  auto inputMain = MinMaxCalc->GetMinimum();
  auto inputMax = MinMaxCalc->GetMaximum();

  typedef itk::CastImageFilter< InputImageType, OutputImageType > CastFilterType;
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(itk_image);
  castFilter->Update();

  typedef itk::MinimumMaximumImageCalculator<OutputImageType> MinimumMaximumImageCalculatorCastType;
  auto MinMaxCalcCast = MinimumMaximumImageCalculatorCastType::New();
  MinMaxCalcCast->SetImage(castFilter->GetOutput());
  MinMaxCalcCast->Compute();
  auto inputMain_1 = MinMaxCalcCast->GetMinimum();
  auto inputMax_1 = MinMaxCalcCast->GetMaximum();

  return castFilter->GetOutput();
}

void set_data_series_names( itk::NumericSeriesFileNames::Pointer& name_series_generator, 
                            const std::string& file_path, const std::string& file_name,
                            const size_t size, const std::string& extension )
{
  boost::filesystem::path path = boost::filesystem::path( file_path );

  std::string ext, filename_base;
  std::tie( ext, filename_base ) = Core::GetFullExtension( boost::filesystem::path( file_name ) );

  boost::filesystem::path filename_path = path / filename_base;

  if ( size < 100 )
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

template< class PixelType >
bool export_volume( const std::string& file_path,
                    const std::string& file_name,
                    typename itk::Image< PixelType, DIM_3D >::Pointer itk_image,
                    const std::string& extension )
{
  boost::filesystem::path full_file(file_path);
  full_file /= file_name;
  full_file.replace_extension( extension );

  typedef itk::Image< PixelType, DIM_3D > OutputImageType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  writer->SetInput( itk_image );
  writer->SetFileName( full_file.string() );

  try
  {
    writer->Update();
  }
  catch ( itk::ExceptionObject &err )
  {
    std::string itk_error = err.GetDescription();
    CORE_LOG_ERROR(itk_error);
    return false;
  }

  return true;
}

////////////// - Templated functions for exporting bitmaps and DICOM's - ///////////////
////////////////////////////////////////////////////////////////////////////////////////
template< class PixelType >
bool export_image_series( const std::string& file_path,
                          const std::string& file_name,
                          typename itk::Image< PixelType, DIM_3D >::Pointer itk_image,
                          const std::string& extension )
{
  typedef itk::Image< PixelType, DIM_3D > ImageType;
  typedef itk::Image< PixelType, DIM_2D > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  typename ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
  typename ImageType::IndexType start = region.GetIndex();
  typename ImageType::SizeType size = region.GetSize();
 
  if (size[0] == 0 || size[1] == 0 || size[2] == 0)
  {
    return false;
  }

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
    CORE_LOG_ERROR(itk_error);
    return false;
  }

  return true;
}

// see http://www.vtk.org/Wiki/index.php?title=ITK/Examples/DICOM/ResampleDICOM
void copy_dictionary( const itk::MetaDataDictionary &fromDict, itk::MetaDataDictionary &toDict )
{
  typedef itk::MetaDataDictionary DictionaryType;

  DictionaryType::ConstIterator itr = fromDict.Begin();
  DictionaryType::ConstIterator end = fromDict.End();
  typedef itk::MetaDataObject< std::string > MetaDataStringType;

  while ( itr != end )
  {
    itk::MetaDataObjectBase::Pointer entry = itr->second;

    MetaDataStringType::Pointer entryvalue = dynamic_cast<MetaDataStringType *>( entry.GetPointer() ) ;
    if ( entryvalue )
    {
      std::string tagkey( itr->first );
      std::string tagvalue( entryvalue->GetMetaDataObjectValue() );
      itk::EncapsulateMetaData<std::string>( toDict, tagkey, tagvalue );
    }
    ++itr;
  }
}

template< class PixelType >
bool export_dicom_series( const std::string& file_path,
                          const std::string& file_name,
                          typename itk::Image< PixelType, DIM_3D >::Pointer itk_image,
                          DataLayerHandle temp_handle,
                          const std::string& extension )
{
  typedef itk::GDCMImageIO ImageIOType;
  ImageIOType::Pointer dicom_io = ImageIOType::New();
  
  typedef itk::Image< PixelType, DIM_3D > ImageType;
  typedef itk::Image< PixelType, DIM_2D > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  
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
  
  // TODO: this is weird - why is it needed???
  // If validation check, should be at function beginning!
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
    size_t dict_size;
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
        std::string itk_error = err.GetDescription();
        CORE_LOG_ERROR(itk_error);
        return false;
      }
      catch ( ... )
      {
        return false;
      }
      dict_size = reader->GetMetaDataDictionaryArray()->size();
    }
    else
    {
      dict_size = names_generator->GetFileNames().size();
    }
    
    std::string uid_prefix = dicom_io->GetUIDPrefix();
    gdcm::UIDGenerator::SetRoot( uid_prefix.c_str() );
    gdcm::UIDGenerator uid;
    
    double min_value = temp_handle->min_value_state_->get();
    double max_value = temp_handle->max_value_state_->get();
    Core::GridTransform grid_transform = temp_handle->get_grid_transform();
    
    std::string series_uid = uid.Generate();
    std::string description = "Data Generated by Seg3D";
    std::string data_center = Core::ExportToString( 0.5 * ( min_value + max_value ) );
    std::string data_width = Core::ExportToString( max_value - min_value );
    
    double z_offset = 0.0;
    
    for ( size_t i = 0; i < dict_size; ++i )
    {
      typename ReaderType::DictionaryRawPointer dict = new typename ReaderType::DictionaryType;
      if ( has_header_file && reader )
      {
        typename ReaderType::DictionaryRawPointer inputDict = (*(reader->GetMetaDataDictionaryArray()))[i];
        if (inputDict) copy_dictionary( *inputDict, *dict );
      }
      dict_array.push_back( dict );
      
      std::string sop_uid = uid.Generate();
      
      // [Series Instance UID]
      itk::EncapsulateMetaData<std::string>( *dict, "0020|000e", series_uid );
      // [ Window Center ]
      itk::EncapsulateMetaData<std::string>( *dict, "0028|1050", data_center );
      // [ Window Width ]
      itk::EncapsulateMetaData<std::string>( *dict, "0028|1051", data_width );
      // [Series Description]
      itk::EncapsulateMetaData<std::string>( *dict, "0008|103e", description );
      // [Bits Allocated]
      itk::EncapsulateMetaData<std::string>( *dict, "0028|0100", "" );
      // [Bits Stored]
      itk::EncapsulateMetaData<std::string>( *dict, "0028|0101", "" );
      // [High Bit]
      itk::EncapsulateMetaData<std::string>( *dict, "0028|0102", "" );
      // [Pixel Representation]
      itk::EncapsulateMetaData<std::string>( *dict, "0028|0103", "" );
      
      Core::Point slice_origin = grid_transform.project( Core::Point( 0.0, 0.0, z_offset ) );
      z_offset += 1.0;
      std::ostringstream patient_location;
      patient_location << Core::ExportToString( slice_origin.x() ) << "\\" <<
      Core::ExportToString( slice_origin.y() ) << "\\" <<
      Core::ExportToString( slice_origin.z() );
      itk::EncapsulateMetaData<std::string>( *dict, "0020|0032", patient_location.str() );
      
      double spacing_x = grid_transform.project( Core::Vector( 1.0, 0.0, 0.0 ) ).length();
      double spacing_y = grid_transform.project( Core::Vector( 0.0, 1.0, 0.0 ) ).length();
      double spacing_z = grid_transform.project( Core::Vector( 0.0, 0.0, 1.0 ) ).length();
      
      std::string z_spacing = Core::ExportToString( spacing_z );
      itk::EncapsulateMetaData<std::string>( *dict, "0018|0050", z_spacing );
      
      typedef itk::Array< double > DoubleArrayType;
      DoubleArrayType originArray( 3 );
      DoubleArrayType spacingArray( 3 );
      
      originArray[ 0 ]  = slice_origin.x();
      originArray[ 1 ]  = slice_origin.y();
      originArray[ 2 ]  = slice_origin.z();
      spacingArray[ 0 ] = spacing_x;
      spacingArray[ 1 ] = spacing_y;
      spacingArray[ 2 ] = spacing_z;
      
      itk::EncapsulateMetaData< unsigned int >( *dict, itk::ITK_NumberOfDimensions, 3 );
      itk::EncapsulateMetaData< DoubleArrayType >( *dict, itk::ITK_Origin, originArray );
      itk::EncapsulateMetaData< DoubleArrayType >( *dict, itk::ITK_Spacing, spacingArray );
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
    CORE_LOG_ERROR(itk_error);
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
  if ( ! layers[ 0 ] ) return;
  this->pixel_type_ = layers[ 0 ]->get_data_type();
}

template< class InputPixelType >
bool ITKDataLayerExporter::export_layer_internal( const std::string& file_path,
                                                  const std::string& name,
                                                  DataLayerHandle temp_handle )
{
  typedef typename Core::ITKImageDataT< InputPixelType > ImageData;
  typedef typename ImageData::Handle ImageDataHandle;
  ImageDataHandle image_data = ImageDataHandle(
    new ImageData( temp_handle->get_data_volume()->get_data_block(),
                   temp_handle->get_grid_transform() ) );

  bool success = false;
  if ( this->extension_ == ".dcm" || this->extension_ == ".dicom" || this->extension_ == ".ima")
  {
    if ( this->pixel_type_ == Core::DataType::FLOAT_E || this->pixel_type_ == Core::DataType::DOUBLE_E ||
         this->pixel_type_ == Core::DataType::ULONGLONG_E || this->pixel_type_ == Core::DataType::LONGLONG_E )
    {
      //investigate unsigned long long to int conversion
      typename itk::Image< int, DIM_3D >::Pointer new_image_data = cast_image< InputPixelType, int >( image_data );
      success =  export_dicom_series< int >( file_path, name, new_image_data, temp_handle, this->extension_ );
    }
    else
    {
      success =  export_dicom_series< InputPixelType >( file_path, name, image_data->get_image(), temp_handle, this->extension_ );
    }
  }
  else if ( this->extension_ == ".tiff" || this->extension_ == ".tif" )
  {
    if ( this->pixel_type_ == Core::DataType::DOUBLE_E)
    {
      typename itk::Image< float, DIM_3D >::Pointer new_image_data = cast_image< InputPixelType, float >( image_data );
      success = export_image_series< float >( file_path, name, new_image_data, this->extension_ );
    }
    else if ( this->pixel_type_ == Core::DataType::ULONGLONG_E || this->pixel_type_ == Core::DataType::LONGLONG_E )
    {
      typename itk::Image< int, DIM_3D >::Pointer new_image_data = cast_image< InputPixelType, int >( image_data );
      success = export_image_series< int >( file_path, name, new_image_data, this->extension_ );
    }
    else
    {
      success = export_image_series< InputPixelType >( file_path, name, image_data->get_image(), this->extension_ );
    }
  }
  else if ( this->extension_ == ".png" )
  {
    if (! ( this->pixel_type_ == Core::DataType::CHAR_E || this->pixel_type_ == Core::DataType::UCHAR_E ||
            this->pixel_type_ == Core::DataType::USHORT_E ) )
    {
      typename itk::Image< unsigned short, DIM_3D >::Pointer new_image_data = cast_image< InputPixelType, unsigned short >(image_data);
      success = export_image_series< unsigned short >( file_path, name, new_image_data, this->extension_ );
    }
    else
    {
      success = export_image_series< InputPixelType >( file_path, name, image_data->get_image(), this->extension_ );
    }
  }
  else if ( this->extension_ == ".nii" || this->extension_ == ".nii.gz" || this->extension_ == ".mha" )
  {
    // supports all
    success =  export_volume< InputPixelType >( file_path, name, image_data->get_image(), this->extension_ );
  }
  else if ( ! this->extension_.empty() )
  {
    success = export_image_series< InputPixelType >( file_path, name, image_data->get_image(), this->extension_ );
  }

  if (success) CORE_LOG_SUCCESS("Data export has been successfully completed.");
  return success;
}

bool ITKDataLayerExporter::export_layer( const std::string& mode, const std::string& file_path, const std::string& name )
{
  DataLayerHandle temp_handle = boost::dynamic_pointer_cast< DataLayer >( this->layers_[ 0 ] );

  switch( this->pixel_type_ )
  {
    case Core::DataType::UCHAR_E:
      return export_layer_internal< unsigned char >( file_path, name, temp_handle );
      break;
    case Core::DataType::CHAR_E:
      return export_layer_internal< signed char >( file_path, name, temp_handle );
      break;
    case Core::DataType::USHORT_E:
      return export_layer_internal< unsigned short >( file_path, name, temp_handle );
      break;
    case Core::DataType::SHORT_E:
      return export_layer_internal< signed short >( file_path, name, temp_handle );
      break;
    case Core::DataType::UINT_E:
      return export_layer_internal< unsigned int >( file_path, name, temp_handle );
      break;
    case Core::DataType::INT_E:
      return export_layer_internal< signed int >( file_path, name, temp_handle );
      break;
    case Core::DataType::ULONGLONG_E:
      return export_layer_internal< unsigned long long >( file_path, name, temp_handle );
      break;
    case Core::DataType::LONGLONG_E:
      return export_layer_internal< long long >( file_path, name, temp_handle );
      break;
    case Core::DataType::FLOAT_E:
      return export_layer_internal< float >( file_path, name, temp_handle );
      break;
    case Core::DataType::DOUBLE_E:
      return export_layer_internal< double >( file_path, name, temp_handle );
      break;
    default:
      return false;
  }
}

} // end namespace seg3D
