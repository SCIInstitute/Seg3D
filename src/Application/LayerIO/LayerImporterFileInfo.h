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

#ifndef APPLICATION_LAYERIO_LAYERIMPORTERFILEINFO_H
#define APPLICATION_LAYERIO_LAYERIMPORTERFILEINFO_H

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

namespace Seg3D
{

/// CLASS LayerImporterFileInfo
/// This class is returned from the LayerImporter class to summarize the information in the header of
/// the file needed to import the data and make decisions on how to import it.

class LayerImporterFileInfo;
typedef boost::shared_ptr< LayerImporterFileInfo > LayerImporterFileInfoHandle;

class LayerImporterFileInfoPrivate;
typedef boost::shared_ptr< LayerImporterFileInfoPrivate > LayerImporterFileInfoPrivateHandle;


class LayerImporterFileInfo : boost::noncopyable
{
  // -- constructor --
public:
  LayerImporterFileInfo();
  virtual ~LayerImporterFileInfo();

  // -- accessors --
public:
  /// GET_GRID_TRANSFORM
  /// Get the transform of the data
  const Core::GridTransform& get_grid_transform() const;

  /// SET_GRID_TRANSFORM
  /// Set the grid transform of the data
  void set_grid_transform( const Core::GridTransform& grid_transform );

  /// GET_DATA_TYPE
  /// Get the type of the data
  const Core::DataType& get_data_type() const;
  
  /// SET_DATA_TYPE
  /// Set the data type of the data
  void set_data_type( const Core::DataType& data_type );
  
  /// GET_FILE_TYPE
  /// Get the file type that is being imported
  const std::string& get_file_type();
  
  /// SET_FILE_TYPE
  /// Set the file type of the file being imported
  void set_file_type( const std::string& file_type ); 
    
  /// GET_MASK_COMPATIBLE
  /// Whether the data can be converted into a mask
  bool get_mask_compatible() const;
  
  /// SET_MASK_COMPATIBLE
  /// Set whether the data can be converted into a mask
  void set_mask_compatible( bool mask_compatible );
          
  // -- internals --
private:
  LayerImporterFileInfoPrivateHandle private_;
};

} // end namespace seg3D

#endif
