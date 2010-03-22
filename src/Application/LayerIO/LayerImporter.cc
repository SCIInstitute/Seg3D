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

// Application includes
#include <Application/LayerIO/LayerImporter.h>

namespace Seg3D
{

std::string ExportToString( LayerImporterMode mode )
{
  switch ( mode )
  {
    case LayerImporterMode::DATA_E: return "data";
    case LayerImporterMode::SINGLE_MASK_E: return "single_mask";
    case LayerImporterMode::BITPLANE_MASK_E: return "bitplane_mask";
    case LayerImporterMode::LABEL_MASK_E: return "label_mask";
    default: return "unknown";
  }
}

bool ImportFromString( const std::string& import_type_string, LayerImporterMode& mode )
{
  std::string import_type = import_type_string;
  boost::to_lower( import_type );
  boost::erase_all( import_type , " " );

  if ( import_type == "data" )
  {
    mode = LayerImporterMode::DATA_E;
    return true;
  }
  else if ( import_type == "singlemask" || import_type == "single_mask" || import_type == "mask" )
  {
    mode = LayerImporterMode::SINGLE_MASK_E;
    return true;
  }
  else if ( import_type == "bitplane_mask" || import_type == "bitplanemask" )
  {
    mode = LayerImporterMode::BITPLANE_MASK_E;
    return true;
  }
  else if ( import_type == "label_mask" || import_type == "labelmask" )
  {
    mode = LayerImporterMode::LABEL_MASK_E;
    return true;
  }
  else
  {
    return false;
  }
}


LayerImporter::LayerImporter( const std::string& filename ) :
  filename_( filename )
{
}

LayerImporter::~LayerImporter()
{
}

std::string LayerImporter::get_filename() 
{ 
  return filename_; 
}

std::string LayerImporter::get_base_filename() 
{ 
  boost::filesystem::path full_filename(filename_);
  return full_filename.stem() ; 
}

bool LayerImporter::check_header()
{
  return true; 
}

} // end namespace seg3D
