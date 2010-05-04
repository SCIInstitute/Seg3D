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

namespace Core
{

// Forward Declaration
class NrrdData;
typedef boost::shared_ptr< NrrdData > NrrdDataHandle;

// Class definition
class NrrdData : boost::noncopyable
{

  // -- Constructor/destructor --
public:
  NrrdData( Nrrd* nrrd, bool own_data = true );
  virtual ~NrrdData();

  // -- Accessors --
public:
  // NRRD:
  // Return the nrrd structure
  Nrrd* nrrd() const
  {
    return nrrd_;
  }

  // GET_DATA:
  // Get the pointer to the data block within the nrrd
  void* get_data() const
  {
    if ( nrrd_ ) return nrrd_->data;
    else return 0;
  }

  // OWN_DATA:
  // Return whether the nrrd owns the data
  bool own_data() const
  {
    return own_data_;
  }

  // GRID_TRANSFORM
  // Extract the transform from the nrrd
  GridTransform get_grid_transform() const;

  // TRANSFORM
  // Extract the transform from the nrrd
  Transform get_transform() const;

  // SET_TRANSFORM
  // Set the transfrom in the nrrd data
  void set_transform( Transform& transform );

  // NX, NY, NZ
  // Get the dimensions of the nrrd
  size_t nx() const;
  size_t ny() const;
  size_t nz() const;

  // IS_<TYPE>
  // Test whether nrrd is of a certain type
  DataType get_data_type() const;

  // -- Information for retrieving nrrd --
private:
  // Location where the original nrrd is stored
  Nrrd* nrrd_;

  // Do we need to clear the nrrd when done
  bool own_data_;

  // -- Data IO for nrrds --
public:

  // LOADNRRD
  // Load a nrrd into the nrrd data structure
  static bool
      LoadNrrd( const std::string& filename, NrrdDataHandle& nrrddata, std::string& error );

  // SAVENRRD
  // Save a nrrd to file from nrrd data structure
  static bool SaveNrrd( const std::string& filename, NrrdDataHandle nrrddata, std::string& error );

  // -- Lock and Unlock Teem (Some parts of Teem are not thread safe) --
public:
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;

  // GETMUTEX:
  // Get the mutex that protects the Teem library
  static mutex_type& GetMutex()
  {
    return teem_mutex_;
  }

private:
  // Mutex protecting Teem calls like nrrdLoad and nrrdSave that are known
  // to be not thread safe
  static mutex_type teem_mutex_;
};

} // end namespace Core

#endif
