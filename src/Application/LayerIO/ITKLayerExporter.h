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

#ifndef APPLICATION_LAYERIO_ITKLAYEREXPORTER_H
#define APPLICATION_LAYERIO_ITKLAYEREXPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// GDCM includes
#include "gdcmImageHelper.h"

// ITK includes
#include "itkImageSeriesWriter.h"
#include "itkGDCMImageIO.h"
#include "itkNumericSeriesFileNames.h"

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/DataBlock/ITKImageData.h>
#include <Core/DataBlock/ITKDataBlock.h>

// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/LayerIO/LayerExporter.h>
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

class ITKLayerExporter : public LayerExporter
{
  SCI_EXPORTER_TYPE("ITK Exporter", ".dcm")

  // -- Constructor/Destructor --
public:
  // Construct a new layer file exporter
  ITKLayerExporter( std::vector< LayerHandle >& layers );

  // Virtual destructor for memory management of derived classes
  virtual ~ITKLayerExporter()
  {
  }

  // -- Import a file information --
public:
  // GET_GRID_TRANSFORM:
  // Get the grid transform of the grid that we are exporting
  virtual Core::GridTransform get_grid_transform();

  // GET_DATA_TYPE:
  // Get the type of data that is being exported
  virtual Core::DataType get_data_type();

  // GET_IMPORTER_MODES
  // Get then supported exporter modes
  virtual int get_exporter_modes();
  
  // --Import the data as a specific type --  
public: 

  // EXPORT_LAYER
  // Export the layer to file
  virtual bool export_layer( LayerExporterMode mode, const std::string& file_path, 
    const std::string& name );
    
private:
  // EXPORT_DICOM_SERIES:
  // Export the data as a series of DICOMS
  template< class InputPixelType, class OutputPixelType >
  bool export_dicom_series( const std::string& file_path, const std::string& file_name )
  {
    DataLayer* temp_handle = dynamic_cast< DataLayer* >( 
      this->layers_[ 0 ].get() );
  
    typedef itk::GDCMImageIO ImageIOType;
    typedef itk::NumericSeriesFileNames NamesGeneratorType;

    ImageIOType::Pointer dicom_io = ImageIOType::New();
    NamesGeneratorType::Pointer names_generator = NamesGeneratorType::New();

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
    
    boost::filesystem::path path = boost::filesystem::path( file_path );

    std::string filename_without_extension = file_name;
    filename_without_extension = filename_without_extension.substr( 0, 
      filename_without_extension.find_last_of( "." ) );
    boost::filesystem::path filename_path = path / filename_without_extension;

    if( size[ 2 ] < 100 )
    {
      names_generator->SetSeriesFormat( filename_path.string() + "-%02d.dcm" );
    }
    else if ( size[ 2 ] < 1000 )
    {
      names_generator->SetSeriesFormat( filename_path.string() + "-%03d.dcm" );
    }
    else if ( size[ 2 ] < 10000 )
    {
      names_generator->SetSeriesFormat( filename_path.string() + "-%04d.dcm" );
    }
    else
    {
      names_generator->SetSeriesFormat( filename_path.string() + "-%10d.dcm" );
    }
    
    names_generator->SetStartIndex( first_slice );
    names_generator->SetEndIndex( last_slice );
    names_generator->SetIncrementIndex( 1 );

    writer->SetInput( itk_image );
    writer->SetImageIO( dicom_io );
    writer->SetFileNames( names_generator->GetFileNames() );
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
private:
  Core::DataType pixel_type_;

};

} // end namespace seg3D

#endif
