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

#ifndef DATAVOLUME_DATAVOLUME_DATAVOLUME_H
#define DATAVOLUME_DATAVOLUME_DATAVOLUME_H

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Util includes
#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Transform.h>

#include <DataVolume/DataVolume/DataBlock.h>

namespace Seg3D {

enum DataVolumeType {
  DATA_VOLUME_E  = 1,
  MASK_VOLUME_E  = 2,
  LABEL_VOLUME_E = 3
};


// Forward Declaration
class DataVolume;
typedef boost::shared_ptr<DataVolume> DataVolumeHandle;

// Class definition
class DataVolume : public boost::noncopyable {

// -- constructor/destructor --
  public:
    DataVolume();
    virtual ~DataVolume()
    
// -- accessors --
  public:
    // Get the type of the data layer
    DataVolumeType   type() const { return type_; }

    // Get the dimensions of the data layer
    size_t           nx() const { return nx_; }
    size_t           ny() const { return ny_; }
    size_t           nz() const { return nz_; }
    
    // Get the transform of the data layer
    Transform        transform const { return transform_; }

    // Get the handle to the data block
    DataBlockHandle  data_block() const { return data_block_; }

    // Get the bit used in case the data is a mask
    int              mask_bit() const { return mask_bit_; }

// -- internals of the datavolume --
  private:
  
    // Type of the data volume
    DataVolumeType   type_;

    // Dimensions of the datavolume object
    size_t           nx_;
    size_t           ny_;
    size_t           nz_;
  
    // Location of the datavolime object in space
    // NOTE: Currently only axis aligned transforms are allowed
    Utils::Transform transform_;
    
    // The datablock associated with this volume
    DataBlockHandle  data_block_;

    // For mask layers specify the bit used
    int              mask_bit_;

};

} // end namespace Seg3D

#endif
