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
#include <Application/LayerIO/LayerExporter.h>

namespace Seg3D
{

std::string ExportToString( LayerExporterMode mode )
{
  switch ( mode )
  {
    case LayerExporterMode::DATA_E: return "data";
    case LayerExporterMode::SINGLE_MASK_E: return "single_mask";
    case LayerExporterMode::BITPLANE_MASK_E: return "bitplane_mask";
    case LayerExporterMode::LABEL_MASK_E: return "label_mask";
    default: return "unknown";
  }
}

bool ImportFromString( const std::string& export_type_string, LayerExporterMode& mode )
{
  std::string export_type = export_type_string;
  boost::to_lower( export_type );
  boost::erase_all( export_type , " " );

  if ( export_type == "data" )
  {
    mode = LayerExporterMode::DATA_E;
    return true;
  }
  else if ( export_type == "singlemask" || export_type == "single_mask" || export_type == "mask" )
  {
    mode = LayerExporterMode::SINGLE_MASK_E;
    return true;
  }
  else if ( export_type == "bitplane_mask" || export_type == "bitplanemask" )
  {
    mode = LayerExporterMode::BITPLANE_MASK_E;
    return true;
  }
  else if ( export_type == "label_mask" || export_type == "labelmask" )
  {
    mode = LayerExporterMode::LABEL_MASK_E;
    return true;
  }
  else
  {
    return false;
  }
}


LayerExporter::LayerExporter( std::vector< LayerHandle >& layers ) :
  layers_( layers )
{
}

LayerExporter::~LayerExporter()
{
}



} // end namespace seg3D
