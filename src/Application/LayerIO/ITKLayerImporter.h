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

#ifndef APPLICATION_LAYERIO_ITKLAYERIMPORTER_H
#define APPLICATION_LAYERIO_ITKLAYERIMPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// ITK includes
#include "itkRGBPixel.h"
#include "itkTIFFImageIO.h"

// Boost includes 
#include <boost/filesystem.hpp>

// Core includes
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerIO/LayerImporter.h>
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

class ITKLayerImporter : public LayerImporter
{
  // The ITKLayerImporter is capable of importing DICOMS, tiffs, and pngs.  It assumes that
  // when a file name does not include an extension that it is a DICOM
  SCI_IMPORTER_TYPE("ITK Importer", ".dcm;.tiff;.tif;.png;", 5)

  // -- Constructor/Destructor --
public:
  // Construct a new layer file importer
  ITKLayerImporter( const std::string& filename );

  // Virtual destructor for memory management of derived classes
  virtual ~ITKLayerImporter()
  {
  }

  // -- Import a file information --
public:

  // IMPORT_HEADER:
  // Import all the information needed to translate the header and metadata information, but not
  // necessarily read the whole file. NOTE: Some external packages do not support reading a header
  // and hence these importers should read the full file here.
  virtual bool import_header();

  // GET_GRID_TRANSFORM:
  // Get the grid transform of the grid that we are importing
  virtual Core::GridTransform get_grid_transform();

  // GET_DATA_TYPE:
  // Get the type of data that is being imported
  virtual Core::DataType get_data_type();

  // GET_IMPORTER_MODES:
  // Get then supported importer modes
  virtual int get_importer_modes();
  
  // --Import the data as a specific type --  
public: 

  // SET_FILE_LIST:
  // we need a list of files to import, this function provides the list, the list must be set 
  // before import_layer is called.
  virtual bool set_file_list( const std::vector< std::string >& file_list )
  {
    this->file_list_ = file_list;
    this->set_extension();
    return true;
  }
  
private:
  // SCAN_DICOM:
  // this function is called by import_header to scan a single dicom and determine what kind of 
  // data type to use for the import
  bool scan_dicom();

  // SCAN_PNG:
  // this function is called by import_header to scan a single png and determine what kind of 
  // data type to use for the import
  bool scan_png();

  // SCAN_TIFF:
  //  this function is called by import_header to scan a single tiff and determine what kind of 
  // data type to use for the import
  bool scan_tiff();

  // IMPORT_DICOM_SERIES:
  // Templated function that reads in a series of dicoms, creates the data_block_ and the
  // image_data_
  template< class PixelType >
  bool import_dicom_series()
  {
    // Step 1: setup the image type
    const unsigned int dimension = 3;
    typedef itk::Image< PixelType, dimension > ImageType;

    // Step 2: using the image type, create a ITK reader
    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();

    // Step 3: now because we are importing dicoms we create a GDCM IO object
    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer dicom_io = ImageIOType::New();

    // Step 4: now we set the io and the file list in the reader
    reader->SetImageIO( dicom_io );
    reader->SetFileNames( this->file_list_ );

    // Step 5: now we attempt to actually read in the file and catch potential errors
    try
    {
      reader->Update();
    }
    catch( ... )
    {
      return false;
    }

    // Step 6: here we instantiate a new DataBlock using the output from the reader
    this->data_block_ = Core::ITKDataBlock::New< PixelType >( 
      typename itk::Image< PixelType, 3 >::Pointer( reader->GetOutput() ) );

    // Step 6: here we instantiate a new ITKImageData using the output from the reader
    this->image_data_ = typename Core::ITKImageDataT< PixelType >::Handle( 
      new typename Core::ITKImageDataT< PixelType >( reader->GetOutput() ) );

    // Step 7: now we check to see if we were successful creating our datablock and image
    // if we were then we set the grid transform.
    if( this->image_data_ && this->data_block_ )
    {
      return true;
    }
    else
    {
      // otherwise we return false
      return false;
    }
  }

  // IMPORT_PNG_SERIES:
  // Templated function for importing a series of PNG's
  template< class PixelType >
  bool import_png_series()
  {
    const unsigned int dimension = 3;
    
    typedef itk::Image< PixelType, dimension > ImageType;
 
    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
 
    reader->SetImageIO( itk::PNGImageIO::New() );
    reader->SetFileNames( this->file_list_ );
 
    try
    {
      reader->Update();
    }
    catch( ... )
    {
      return false;
    }
 
    this->data_block_ = Core::ITKDataBlock::New< PixelType >( 
      typename itk::Image< PixelType, 3 >::Pointer( reader->GetOutput() ) );
 
    this->image_data_ = typename Core::ITKImageDataT< PixelType >::Handle( 
      new typename Core::ITKImageDataT< PixelType >( reader->GetOutput() ) );
 
    if( this->image_data_ && this->data_block_ )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // IMPORT_PNG_SERIES: // HASNT BEEN TESTED! NEED PNG's
  // Templated function for importing a series of PNG's
  template< class PixelType >
  bool import_tiff_series()
  {
    const unsigned int dimension = 3;
    typedef itk::Image< PixelType, dimension > ImageType;

    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();

    reader->SetImageIO( itk::TIFFImageIO::New() );
    reader->SetFileNames( this->file_list_ );

    try
    {
      reader->Update();
    }
    catch( ... )
    {
      return false;
    }

    this->data_block_ = Core::ITKDataBlock::New< PixelType >( 
      typename itk::Image< PixelType, 3 >::Pointer( reader->GetOutput() ) );

    this->image_data_ = typename Core::ITKImageDataT< PixelType >::Handle( 
      new typename Core::ITKImageDataT< PixelType >( reader->GetOutput() ) );

    if( this->image_data_ && this->data_block_ )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // SET_EXTENSION:
  // we need to know which type of file we are dealing with, this function provides that ability,
  // the extension must be set before import_layer is called.
  void set_extension()
  {
    this->extension_ = boost::filesystem::path( this->file_list_[ 0 ] ).extension();
    
    // now we force it to be lower case, just to be safe.
    boost::to_lower( this->extension_ );
  }
  
protected:
  // LOAD_DATA:
  // Load the data from the file(s).
  // NOTE: This function is called by import_layer internally.
  virtual bool load_data( Core::DataBlockHandle& data_block, 
    Core::GridTransform& grid_trans );

  // GET_LAYER_NAME:
  // Return the string that will be used to name the layers.
  virtual std::string get_layer_name();

private:
  Core::ITKImageDataHandle        image_data_;
  Core::DataBlockHandle         data_block_;
  std::vector< std::string >        file_list_;
  size_t                  bits_;
  bool                  signed_data_;
  Core::DataType              pixel_type_;
  std::string               extension_;
};

} // end namespace seg3D

#endif
