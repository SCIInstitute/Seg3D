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

#ifndef APPLICATION_LAYERIO_LAYERIMPORTERFILEDATA_H
#define APPLICATION_LAYERIO_LAYERIMPORTERFILEDATA_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/DataBlock.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/Utils/EnumClass.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/Layer/LayerMetaData.h>

namespace Seg3D
{

/// CLASS LayerImporterFileData
/// This class contains the data extracted out of a file

class LayerImporterFileData;
class LayerImporterFileDataPrivate;
typedef boost::shared_ptr< LayerImporterFileData > LayerImporterFileDataHandle;
typedef boost::shared_ptr< LayerImporterFileDataPrivate > LayerImporterFileDataPrivateHandle;


class LayerImporterFileData : boost::noncopyable
{
  // -- constructor --
public:
  LayerImporterFileData();
  virtual ~LayerImporterFileData();

  // -- accessors --
public:
  /// GET_GRID_TRANSFORM
  /// Get the transform of the data
  const Core::GridTransform& get_grid_transform() const;

  /// SET_GRID_TRANSFORM
  /// Set the grid transform of the data
  void set_grid_transform( const Core::GridTransform& grid_transform );

  /// GET_META_DATA
  /// Get the meta data associated with the data
  const LayerMetaData& get_meta_data() const;
  
  /// SET_META_DATA
  /// Set the meta data associated with the data
  void set_meta_data( const LayerMetaData& meta_data );
  
  /// GET_DATABLOCK
  /// Get the datablock with the data
  Core::DataBlockHandle get_data_block() const;
    
  /// SET_DATABLOCK
  /// Set the data block that was loaded from file
  void set_data_block( const Core::DataBlockHandle& data_block );
  
  /// GET_DATA_TYPE
  /// Get the type of the data
  const std::string& get_name() const;
  
  /// SET_DATA_TYPE
  /// Set the data type of the data
  void set_name( const std::string& data_type );
    
  // -- converter functions --
public:                     
  
  /// CONVERT_TO_LAYERS
  /// Convert the data contained in this structure into layers that can be inserted into the
  /// LayerManager
  bool convert_to_layers( const std::string& mode, std::vector< LayerHandle >& layers );
  
  // -- internals --
private:
  LayerImporterFileDataPrivateHandle private_;
};

} // end namespace seg3D

#endif
