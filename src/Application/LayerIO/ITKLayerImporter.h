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

// Boost includes 
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

// Application includes
#include <Application/LayerIO/LayerImporter.h>
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

class ITKLayerImporter : public LayerImporter
{
  SCI_IMPORTER_TYPE( "ITK Importer", ".dcm;.tiff;.png;.", 5)

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

  // IMPORT_LAYER:
  // Import the layer from the file
  virtual bool import_layer( LayerImporterMode mode, std::vector<LayerHandle>& layers );

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

  // Templated function that reads in a series of dicoms and creates the data_block_ and the
  // image_data_ so that we can create new layers
  template< class PixelType >
  bool import_dicom_series()
  {
    const unsigned int Dimension = 3;
    typedef itk::Image< PixelType, Dimension > ImageType;

    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();

    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer dicomIO = ImageIOType::New();

    reader->SetImageIO( dicomIO );
    reader->SetFileNames( this->file_list_ );

    try
    {
      reader->Update();
    }
    catch ( itk::ExceptionObject &err )
    {
      std::string itk_error = err.GetDescription();
    }
    catch( gdcm::Exception &error )
    {
      std::string format_error = error.getError();
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

  // IMPORT_PNG_SERIES:
  // does nothing yet
  bool import_png_series();

  // SET_EXTENSION:
  // we need to know which type of file we are dealing with, this function provides that ability,
  // the extension must be set before import_layer is called.
  void set_extension()
  {
    this->extension_ = boost::filesystem::path( this->file_list_[ 0 ] ).extension();
    
    // now we force it to be lower case, just to be safe.
    boost::to_lower( this->extension_ );
  }

private:
  Core::ITKImageDataHandle image_data_;
  Core::DataBlockHandle data_block_;
  std::vector< std::string > file_list_;
  int bits_;
  bool signed_data_;
  int pixel_type_;
  std::string extension_;

};

} // end namespace seg3D

#endif
