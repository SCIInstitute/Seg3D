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

#ifndef UTILS_DATABLOCK_DATABLOCK_H
#define UTILS_DATABLOCK_DATABLOCK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>

namespace Utils {

// CLASS DataBlock
// This class is an abstract representation of a block of volume data in
// memory. It stores the pointer to where the data is located as well as 
// its dimensions, and the type of the data

// NOTE: This is a base class that does not do any memory allocation, use one
// of the derived classes to generate a datablock. The implementation in this
// class is just the common access to the data.

// Forward Declaration
class DataBlock;
typedef boost::shared_ptr<DataBlock> DataBlockHandle;

// Class definition
class DataBlock : public boost::noncopyable {

// -- typedefs --
  public:
    // Lock types
    typedef boost::recursive_mutex            mutex_type;
    typedef boost::unique_lock<mutex_type>    lock_type;

    // Data types
    enum data_type {
      UNKNOWN_E,
      CHAR_E,
      UCHAR_E,
      SHORT_E,
      USHORT_E,
      INT_E,
      UINT_E,
      FLOAT_E,
      DOUBLE_E
    };

// -- Constructor/destructor --
  public:
    DataBlock();    
    virtual ~DataBlock();

// -- Access properties of data block --
  public:
    
    // NX, NY, NZ, SIZE
    // The dimensions of the datablock
    size_t nx() const { return nx_; }
    size_t ny() const { return ny_; }
    size_t nz() const { return nz_; }
    size_t size() const { return nx_*ny_*nz_; }

    // TYPE
    // The type of the data
    data_type type() const { return data_type_; }

    // DATA
    // Pointer to the block of data
    void* data() const { return data_; }

// -- Pointer to where the data is stored
  protected:

    // SET_NX, SET_NY, SET_NZ
    // Set the dimensions of the datablock
    void set_nx(size_t nx) { nx_ = nx; }
    void set_ny(size_t ny) { ny_ = ny; }
    void set_nz(size_t nz) { nz_ = nz; }

    // SET_TYPE
    // Set the type of the data
    void set_type(data_type type) { data_type_ = type; }

    // SET_DATA
    // Set the data pointer of the data
    void set_data(void* data) { data_ =  data; }

// -- Locking of the datablock --
  public:
  
    // LOCK:
    // Lock the datablock
    void lock() { mutex_.lock(); }
    
    // UNLOCK:
    // Unlock the datablock
    void unlock() { mutex_.unlock(); }
    
    // GETMUTEX:
    // Get the mutex that locks the datablock
    mutex_type& get_mutex() { return mutex_;}

// -- internals of the DataBlock --
  private:
    // The dimensions of the datablock
    size_t nx_;
    size_t ny_;
    size_t nz_;
      
    // The type of the data in this data block
    data_type data_type_;
                
    // The mutex that protects the data
    // As data is shared between the renderer and the application threads, we
    // need locking.
    mutex_type mutex_;    
  
    // Pointer to the data
    void* data_;
};

} // end namespace Utils

#endif
