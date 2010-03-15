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

// Utils includes
#include <Utils/DataBlock/NrrdData.h>
#include <Utils/DataBlock/NrrdDataBlock.h>

// Application includes
#include <Application/LayerIO/NrrdLayerImporter.h>

namespace Seg3D
{

SCI_REGISTER_IMPORTER(NrrdLayerImporter);


bool NrrdLayerImporter::import_header()
{
  // Only import the data once
  if ( ( nrrd_data_ ) ) return true;
  
  // NOTE: We load the full data set, as Teem does not support reading headers only :(
  // Hence we need to read the full file
  std::string error;
  if ( ! ( Utils::NrrdData::LoadNrrd( filename_, nrrd_data_, error ) ) )
  {
    set_error( error );
    return false;
  }
  return true;
}

bool NrrdLayerImporter::import_data()
{
  if ( !( nrrd_data_ ) ) return import_header();
  return true;
}

Utils::GridTransform NrrdLayerImporter::get_grid_transform()
{
  if ( nrrd_data_ )  return nrrd_data_->get_grid_transform();
  else return LayerImporter::get_grid_transform();
}

bool NrrdLayerImporter::has_import_mode( LayerImporterMode mode )
{
  if ( nrrd_data_->is_integer() ) return true;
  else if ( nrrd_data_->is_real() && mode == LayerImporterMode::DATA_E ) return true;
  return false;
}


bool NrrdLayerImporter::import_layer( std::vector<LayerHandle>& layers, LayerImporterMode mode )
{
  return false;
}

} // end namespace seg3D
