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

#ifndef APPLICATION_LAYERIO_LAYERIMPORTERINFO_H
#define APPLICATION_LAYERIO_LAYERIMPORTERINFO_H

// Boost includes
#include <boost/utility.hpp>

// Application includes
#include <Application/LayerIO/LayerImporter.h>

namespace Seg3D
{

// ---- Auxilary Classes -----

class LayerImporterBuilderBase;
typedef boost::shared_ptr<LayerImporterBuilderBase> LayerImporterBuilderBaseHandle;

class LayerImporterBuilderBase
{
public:
  /// ensure we can delete the builder correctly
  virtual ~LayerImporterBuilderBase()
  {
  }
  
  /// the function call to build the object
  virtual LayerImporterHandle build() = 0;
};


/// LAYERIMPORTERBUILDER:
/// Auxillary class to build the layer importer class

template < class LAYERIMPORTER >
class LayerImporterBuilder: public LayerImporterBuilderBase
{
public:
  /// ensure we can delete the builder correctly
  virtual ~LayerImporterBuilder< LAYERIMPORTER >()
  {
  }

  /// The actual builder call
  virtual LayerImporterHandle build()
  { 
    return LayerImporterHandle( new LAYERIMPORTER );
  }
};

// -------------------------------------------------
/// LAYERIMPORTERINFO
/// Class that records the information of the different importers that are available and auxillary
/// functions that allow in the decision which importer to use.

// Class declaration
class LayerImporterInfo;
class LayerImporterInfoPrivate;
typedef boost::shared_ptr<LayerImporterInfo> LayerImporterInfoHandle;
typedef boost::shared_ptr<LayerImporterInfoPrivate> LayerImporterInfoPrivateHandle;

// Class definition
class LayerImporterInfo : public boost::noncopyable
{
  // -- constructor/destructor --
public:
  LayerImporterInfo( LayerImporterBuilderBaseHandle builder, std::string name,
    std::string file_type_string, int priority, LayerImporterType type );
  ~LayerImporterInfo();

  // -- description of the importer --
public:
  /// TYPE
  /// The type of importer this one is
  LayerImporterType get_type() const;

  /// BUILD
  /// Build the importer
  LayerImporterHandle build() const;

  /// GET_NAME
  /// Get the name of the importer
  std::string get_name() const;

  /// CONVERTS_FILE_TYPE
  /// Check whether this importer deals with a specific file type
  bool converts_file_type( const std::string& file_type, bool strict = true ) const;
  
  /// GET_FILE_TYPES
  /// Get the string that defines the allowed file types
  std::string get_file_type_string() const;
  
  /// GET_PRIORITY
  /// Get the priority of the importer
  int get_priority() const;
  
  // -- internals --
private:
  LayerImporterInfoPrivateHandle private_;
};

} // end namespace seg3D

#endif
