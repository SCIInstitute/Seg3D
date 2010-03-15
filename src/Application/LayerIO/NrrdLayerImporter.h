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

#ifndef APPLICATION_LAYERIO_NRRDLAYERIMPORTER_H
#define APPLICATION_LAYERIO_NRRDLAYERIMPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Utils includes
#include <Utils/DataBlock/NrrdData.h>
#include <Utils/DataBlock/NrrdDataBlock.h>

// Application includes
#include <Application/LayerIO/LayerImporter.h>
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

class NrrdLayerImporter : public LayerImporter
{
  SCI_IMPORTER_TYPE("Teem Importer",".nrrd;.nhdr",10)

  // -- Constructor/Destructor --
public:
  // Construct a new layer file importer
  NrrdLayerImporter(const std::string& filename) :
    LayerImporter(filename)
  {
  }

  // Virtual destructor for memory management of derived classes
  virtual ~NrrdLayerImporter()
  {
  }

  // -- Import a file --
public:

  // IMPORT_HEADER:
  // Import all the information needed to translate the header and metadata information, but not
  // necessarily read the whole file. NOTE: Some external packages do not support reading a header
  // and hence these importers should read the full file here.
  virtual bool import_header();
  
  // IMPORT_DATA:
  // Import all the of the file including the data.
  virtual bool import_data(); 

  // -- Data type information --
public:

  // GET_GRID_TRANSFORM:
  // Get the grid transform of the grid that we are importing
  virtual Utils::GridTransform get_grid_transform();

  // GET_DATA_TYPE:
  // Get the type of data that is being imported
  virtual Utils::DataType get_data_type();

  // --Import the data as a specific type --  
public: 
  // HAS_IMPORT_MODE:
  // Test whether the importer a specific importer mode
  virtual bool has_importer_mode( LayerImporterMode mode );

  // IMPORT_LAYER
  // Import the layer from the file
  virtual bool import_layer( std::vector<LayerHandle>& layers, LayerImporterMode mode );

private:
  Utils::NrrdDataHandle nrrd_data_;

  bool import_as_data( std::vector<LayerHandle>& layers );

  bool import_as_single_mask( std::vector<LayerHandle>& layers );
  
  bool import_as_bitplane_mask( std::vector<LayerHandle>& layers );
  
  bool import_as_label_mask( std::vector<LayerHandle>& layers );

};

} // end namespace seg3D

#endif
