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
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>



// Application includes
#include <Application/LayerIO/ITKMaskLayerExporter.h>
#include <Application/Layer/DataLayer.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

SEG3D_REGISTER_EXPORTER( Seg3D, ITKMaskLayerExporter );

namespace Seg3D
{


void set_mask_series_names( itk::NumericSeriesFileNames::Pointer& name_series_generator, 
  const std::string& file_path, const std::string& file_name, const size_t size )
{ 
  boost::filesystem::path path = boost::filesystem::path( file_path );

  std::string extension = boost::filesystem::extension( boost::filesystem::path( file_name ) );

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

bool export_dicom_series( const std::string& file_path, const std::string& file_name, 
  Core::DataVolumeHandle data_volume, LayerMetaData meta_data )
{
  typedef itk::GDCMImageIO ImageIOType;
  ImageIOType::Pointer dicom_io = ImageIOType::New();
    
  typedef itk::Image< unsigned char, 3 > ImageType;
  typedef itk::Image< unsigned char, 2 > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;

  WriterType::Pointer writer = WriterType::New();

  Core::GridTransform grid_transform = data_volume->get_grid_transform();
    
  Core::ITKImageDataHandle image_data = Core::ITKImageDataT<unsigned char >::Handle( 
            new Core::ITKImageDataT< unsigned char >( data_volume->get_data_block(), grid_transform ) );

  ImageType* itk_image = dynamic_cast< ImageType* >( 
    image_data->get_base_image().GetPointer() );

  ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
    ImageType::IndexType start = region.GetIndex();
  ImageType::SizeType size = region.GetSize();

  unsigned int first_slice = start[ 2 ];
  unsigned int last_slice = start[ 2 ] + size[ 2 ] - 1;

  std::string extension = 
    boost::to_lower_copy( boost::filesystem::extension( boost::filesystem::path( file_name ) ) );

  typedef itk::NumericSeriesFileNames NamesGeneratorType;
  NamesGeneratorType::Pointer names_generator = NamesGeneratorType::New();
  set_mask_series_names( names_generator, file_path, file_name, size[ 2 ] );

  names_generator->SetStartIndex( first_slice );
  names_generator->SetEndIndex( last_slice );
  names_generator->SetIncrementIndex( 1 );

  ///////////////////////////////////////////////////////////////////////////

  // Check whether header file was included
  bool has_header_file = false;

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

  writer->SetInput( itk_image );
    writer->SetImageIO( dicom_io ); 
  writer->SetFileNames( names_generator->GetFileNames() );

  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  ReaderType::Pointer reader;
  ReaderType::DictionaryArrayType dict_array;

    if ( has_header_file )
    {
        reader = ReaderType::New();
        reader->SetFileNames( header_files ); 
        reader->SetImageIO( dicom_io );
        try 
        {
            reader->Update();
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

    std::string series_uid = uid.Generate();    
    std::string description = "Mask Generated by Seg3D";
    
    double z_offset = 0.0;
    for ( ReaderType::DictionaryArrayType::iterator dict_iter = 
        dict_array.begin(); dict_iter != dict_array.end(); dict_iter++ )
    {
        std::string sop_uid = uid.Generate();

        // [Series Instance UID]
        itk::EncapsulateMetaData<std::string>( **dict_iter, "0020|000e", series_uid );
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

        itk::EncapsulateMetaData< unsigned int >( **dict_iter, itk::ITK_NumberOfDimensions, 3 );
        itk::EncapsulateMetaData< DoubleArrayType >( **dict_iter, itk::ITK_Origin, originArray );
        itk::EncapsulateMetaData< DoubleArrayType >( **dict_iter, itk::ITK_Spacing, spacingArray );
    }
    // now tell the Dicom writer to use the provided UIDs
    dicom_io->KeepOriginalUIDOn();
    writer->SetMetaDataDictionaryArray( &dict_array );

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



bool export_mask_series( const std::string& file_path, const std::string& file_name, 
  Core::DataVolumeHandle data_volume, LayerMetaData meta_data )
{    
  typedef itk::Image< unsigned char, 3 > ImageType;
  typedef itk::Image< unsigned char, 2 > OutputImageType;
  typedef itk::ImageSeriesWriter< ImageType, OutputImageType > WriterType;

  WriterType::Pointer writer = WriterType::New();

  Core::GridTransform grid_transform = data_volume->get_grid_transform();
    
  Core::ITKImageDataHandle image_data = Core::ITKImageDataT<unsigned char >::Handle( 
            new Core::ITKImageDataT< unsigned char >( data_volume->get_data_block(), grid_transform ) );

  ImageType* itk_image = dynamic_cast< ImageType* >( 
    image_data->get_base_image().GetPointer() );

  ImageType::RegionType region = itk_image->GetLargestPossibleRegion();
    ImageType::IndexType start = region.GetIndex();
  ImageType::SizeType size = region.GetSize();

  unsigned int first_slice = start[ 2 ];
  unsigned int last_slice = start[ 2 ] + size[ 2 ] - 1;

  std::string extension = 
    boost::to_lower_copy( boost::filesystem::extension( boost::filesystem::path( file_name ) ) );

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


////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  ITKMaskLayerExporter ////////////////////////////////
ITKMaskLayerExporter::ITKMaskLayerExporter( std::vector< LayerHandle >& layers ) :
  LayerExporter( layers ),
  pixel_type_( Core::DataType::UCHAR_E )
{
  if( !layers[ 0 ] ) return;
  this->pixel_type_ = layers[ 0 ]->get_data_type();
}



bool ITKMaskLayerExporter::export_layer( const std::string& mode, const std::string& file_path, 
  const std::string& name )
{
    bool is_dicom = false;

  if ( this->extension_ == ".dcm" || this->extension_ == ".dicom" || this->extension_ == ".ima" )
  {
        is_dicom = true;
    }

    if ( mode == LayerIO::LABEL_MASK_MODE_C )
    {
        MaskLayer* temp_mask = dynamic_cast< MaskLayer* >( this->layers_[ 1 ].get() );
        // Step 2: Get a handle to its MaskDataBlock and use that to build a new DataBlockHandle of the 
        // same size and type.
        Core::DataBlockHandle data_block = Core::StdDataBlock::New( temp_mask->get_mask_volume()->get_nx(),
            temp_mask->get_mask_volume()->get_ny(), temp_mask->get_mask_volume()->get_nz(), Core::DataType::UCHAR_E );

        data_block->clear();
        Core::MaskDataBlockManager::Inscribe( temp_mask->get_mask_volume()->get_mask_data_block(), data_block,
            static_cast<double>( this->label_values_[ 0 ] ), true );
        
        for ( size_t j = 1; j < this->layers_.size(); j++ )
        {
            MaskLayer* mask = dynamic_cast< MaskLayer* >( this->layers_[ j ].get() );
            Core::MaskDataBlockManager::Inscribe( mask->get_mask_volume()->get_mask_data_block(), data_block,  
                static_cast<double>( this->label_values_[ j ] ) );
        }
        
        Core::DataVolumeHandle volume( new Core::DataVolume( this->layers_[ 1 ]->get_grid_transform(), data_block ) );
        
        MaskLayer* mask = dynamic_cast< MaskLayer* >( this->layers_[ 1 ].get() );
        
        if ( is_dicom )
        {
            return  export_dicom_series( file_path, ( name + this->extension_ ), 
                volume, mask->get_meta_data() );            
        }
        else
        {
            return  export_mask_series( file_path, ( name + this->extension_ ), 
                volume, mask->get_meta_data() );    
        }
    }
    else
    {
        bool success = true;
        for( size_t j = 0; j < this->layers_.size(); j++ )
        {
            MaskLayerHandle mask = boost::dynamic_pointer_cast< MaskLayer >( this->layers_[ j ] );

            Core::DataBlockHandle data_block;
            Core::MaskDataBlockManager::Convert( mask->get_mask_volume()->get_mask_data_block(), data_block, Core::DataType::UCHAR_E );
            
            Core::DataVolumeHandle volume( new Core::DataVolume( mask->get_grid_transform(), data_block ) );

            if ( is_dicom )
            {
                success = export_dicom_series( file_path, ( mask->get_layer_name() + this->extension_ ), 
                    volume, mask->get_meta_data() );            
            }
            else
            {
                success = export_mask_series( file_path, ( mask->get_layer_name() + this->extension_ ), 
                    volume, mask->get_meta_data() );
            }
        }
        
        return success;
    }
    
  CORE_LOG_SUCCESS( "Segmentation export has been successfully completed." );
  // If we have successfully gone through all the layers return true
  return true;
}

} // end namespace seg3D
