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

#ifndef CORE_DATABLOCK_NRRDDATA_H
#define CORE_DATABLOCK_NRRDDATA_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Teem includes
#include <teem/nrrd.h>

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/Geometry/Vector.h>

#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/Histogram.h>

namespace Core
{

// CLASS NRRDDATA
/// This class is a wrapper for the Teem nrrd structure. It can be generated on the fly from a nrrd
/// object that the Teem library produces or it can be generated based on a DataBlock from this
/// core library.

/// The purpose of this class is to do garbage collection and sharing of memory between different
/// pieces of the third party libraries.
class NrrdData;
typedef boost::shared_ptr< NrrdData > NrrdDataHandle;

class NrrdDataPrivate;
typedef boost::shared_ptr< NrrdDataPrivate > NrrdDataPrivateHandle;

// Class definition
class NrrdData : boost::noncopyable
{
  // -- Constructor/destructor --
public:
  /// Construct the NrrdData wrapper from a nrrd
  /// own_data tells whether the data should be destroyed when this object is deleted
  NrrdData( Nrrd* nrrd, bool own_data = true );
  
  /// Construct a NrrdData object from an existing DataBlock of data
  /// The datablock handle will be stored internally until the object is deleted
  /// and the memory with the data is shared between the object and the nrrd object.
  NrrdData( DataBlockHandle data_block );
  NrrdData( DataBlockHandle data_block, GridTransform transform );

  virtual ~NrrdData();

  // -- Accessors --
public:
  // NRRD:
  /// Return the nrrd structure
  Nrrd* nrrd() const;

  // GET_DATA:
  /// Get the pointer to the data block within the nrrd
  void* get_data() const;

  // OWN_DATA:
  /// Return whether the nrrd owns the data
  bool own_data() const;

  // GRID_TRANSFORM:
  /// Extract the transform from the nrrd
  GridTransform get_grid_transform() const;

  // TRANSFORM:
  /// Extract the cell-centered transform from the nrrd 
  Transform get_transform() const;

  // SET_TRANSFORM:
  /// Set the transfrom in the nrrd data
  void set_transform( GridTransform& transform );

  // GET_HISTOGRAM:
  /// Get the histogram from the nrrd
  /// NOTE: If the data source can be trusted, the histogram is taken from the meta data,
  /// otherwise it is recomputed
  Histogram get_histogram( bool trust_meta_data = false );

  // SET_HISTOGRAM:
  /// Insert a histogram into a nrrd's meta data
  void set_histogram( const Histogram& histogram );

  // GET_NX, GET_NY, GET_NZ:
  /// Get the dimensions of the nrrd
  size_t get_nx() const;
  size_t get_ny() const;
  size_t get_nz() const;

  // GET_SIZE:
  /// Get the total size of the nrrd in pixels
  size_t get_size() const;

  // GET_DATA_TYPE:
  /// Get the data type of the nrrd
  DataType get_data_type() const;

  // GET_ORIGINALLY_NODE_CENTERED:
  /// Get whether the nrrd was originally node-centered  
  /// Internally Seg3D converts the transform to be cell-centered, but we keep track of what the 
  /// original centering was so that the transform can be converted back to be node-centered on 
  /// export.
  bool get_originally_node_centered() const;

  // -- Internals of this class --
private:
  NrrdDataPrivateHandle private_;

  // -- Data IO for nrrds --
public:

  // LOADNRRD:
  /// Load a nrrd into the nrrd data structure
  static bool LoadNrrd( const std::string& filename, NrrdDataHandle& nrrddata, 
    std::string& error );

  // SAVENRRD:
  /// Save a nrrd to file from nrrd data structure
  /// If compress is false, level will be overridden and set to 0, which
  /// corresponds to zlib setting for no compression
  static bool SaveNrrd( const std::string& filename,
                        NrrdDataHandle nrrddata,
                        std::string& error,
                        bool compress,
                        int level );

  // -- Lock and Unlock Teem (Some parts of Teem are not thread safe) --
public:
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  // GETMUTEX:
  /// Get the mutex that protects the Teem library
  static mutex_type& GetMutex();
};

} // end namespace Core

#endif
