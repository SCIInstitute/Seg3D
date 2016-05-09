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

#ifndef CORE_DATABLOCK_DATASLICE_H
#define CORE_DATABLOCK_DATASLICE_H

// Boost includes
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/DataBlock/SliceType.h>
#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/DataBlockFWD.h>

namespace Core
{

// CLASS DataSlice
/// Class for storing a slice of data from a datablock
class DataSlice;
typedef boost::shared_ptr<DataSlice> DataSliceHandle;
typedef boost::weak_ptr<DataSlice> DataSliceWeakHandle; 

// Class definition
class DataSlice : public boost::noncopyable
{
public:
  /// Generation number compatible with DataBlock class
  typedef long long  generation_type;

  /// Index used for addressing memory inside the datablock underlying the slice
#ifdef SCI_64BITS
  typedef long long index_type;
#else
  typedef int index_type;
#endif

  // -- Constructor/destructor --
  DataSlice( DataBlockHandle datablock, SliceType slice_type, index_type index ); 

public:
  // Destructor that unregisters itself from the DataBlockManager
  virtual ~DataSlice();

  // -- Access properties of data block --
public:

  // GET_NX:
  /// Get the number of samples in the x direction.
  size_t get_nx() const;
  
  // GET_NY:
  /// Get the number of samples in the y direction.
  size_t get_ny() const;

  // GET_NZ:
  /// Get the number of samples in the z direction.
  size_t get_nz() const;
  
  // GET_SIZE:
  /// Get the total number of samples.
  size_t get_size() const;

  // GET_ELEM_SIZE:
  /// Get the size of the data elements.
  size_t get_elem_size() const;
  
  // GET_BYTE_SIZE:
  /// Get the size in bytes
  size_t get_byte_size() const;

  // GET_DATA_TYPE:
  /// The type of the data
  DataType get_data_type() const;

  // GET_SLICE_TYPE:
  /// Whether the slice is axial, coronal, or sagittal
  SliceType get_slice_type() const;
  
  // GET_GENERATION:
  /// Get the current generation number of the data volume.
  generation_type get_generation() const;
  
  // GET_DATABLOCK:
  /// Get the underlying data block that contains the data of the slice
  DataBlockHandle get_data_block() const;
  
  // GET_DATA:
  /// Get the pointer of the data in the dat block underlying this class
  void* get_data() const;
  
  // GET_INDEX:
  /// Get the index of the slice
  index_type get_index() const;
  
  // SET_INDEX:
  /// Set the index of the slice
  void set_index( index_type );
  
  // -- internals of the DataBlock --
private:

  /// Data block where the data is stored
  DataBlockHandle slice_;
  
  /// Whether the slice is axial, sagittal, or coronal
  SliceType type_;
  
  /// The index of the slice in the original data block
  index_type index_;
  
public:

  // DUPLICATE:
  /// Clone the data in a datablock by generating a new one and copying the data into it.
  static bool Duplicate( const DataSliceHandle& src_data_slice, DataSliceHandle& dst_data_slice );  
};

} // end namespace Core

#endif
