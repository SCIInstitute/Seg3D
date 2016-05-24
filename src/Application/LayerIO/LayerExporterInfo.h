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

#ifndef APPLICATION_LAYERIO_LAYEREXPORTERINFO_H
#define APPLICATION_LAYERIO_LAYEREXPORTERINFO_H


// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerIO/LayerExporter.h>


namespace Seg3D
{

// ---- Auxilary Classes -----

class LayerExporterBuilderBase;
typedef boost::shared_ptr<LayerExporterBuilderBase> LayerExporterBuilderBaseHandle;

class LayerExporterBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~LayerExporterBuilderBase()
  {
  }
  
  // the function call to build the object
  virtual LayerExporterHandle build( std::vector< LayerHandle >& layers ) = 0;
};


/// LAYEREXPORTERBUILDER:
/// Auxiliary class to build the layer exporter class

template < class LAYEREXPORTER >
class LayerExporterBuilder: public LayerExporterBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~LayerExporterBuilder< LAYEREXPORTER >()
  {
  }

  // The actual builder call
  virtual LayerExporterHandle build( std::vector< LayerHandle >& layers )
  { 
    return LayerExporterHandle( new LAYEREXPORTER( layers ));
  }
};

/// -------------------------------------------------
/// LAYEREXPORTERINFO
/// Class that records the information of the different exporters that are available and auxiliary
/// functions that allow in the decision which exporter to use.

// Class declaration
class LayerExporterInfo;
typedef boost::shared_ptr<LayerExporterInfo> LayerExporterInfoHandle;

// Class definition
class LayerExporterInfo 
{
  // -- constructor/destructor --
public:
  LayerExporterInfo( LayerExporterBuilderBaseHandle builder,
    const std::string name, 
    const std::string file_type_string );
  
  ~LayerExporterInfo();

  /// BUILD:
  /// Build the exporter
  LayerExporterHandle build( std::vector< LayerHandle >& layers ) const;

  /// GET_NAME:
  /// Get the name of the exporter
  std::string get_name() const;

  /// CONVERTS_FILE_TYPE:
  /// Check whether this exporter deals with a specific file type
  bool converts_file_type( const std::string& file_type ) const;
  
  /// GET_FILE_TYPE_STRING:
  /// Get the string that defines the allowed file types
  std::string get_file_type_string() const;
  
private:
  /// Object that knows how to build the exporter
  LayerExporterBuilderBaseHandle builder_;
  
  /// Name of the exporter
  std::string name_;
  
  /// The file types the exporter will handle
  std::vector<std::string> file_types_; 
  
  /// String with name and file types, to be used as a filter in a file selector
  std::string file_type_string_;
  
  /// Any file type, e.g. dicom readers tend to use no extension
  bool any_type_;
  
};

} // end namespace seg3D

#endif //APPLICATION_LAYERIO_LAYEREXPORTERINFO_H
