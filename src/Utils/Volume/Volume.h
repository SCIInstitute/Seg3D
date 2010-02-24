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

#ifndef UTILS_VOLUME_VOLUME_H
#define UTILS_VOLUME_VOLUME_H

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Util includes
#include <Utils/DataBlock/DataBlock.h>
#include <Utils/DataBlock/NrrdData.h>
#include <Utils/DataBlock/MaskDataBlock.h>

#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Transform.h>
#include <Utils/Geometry/GridTransform.h>

namespace Utils {

// Forward Declaration
class Volume;
typedef boost::shared_ptr<Volume> VolumeHandle;

enum VolumeType {
  DATA_E  = 1,
  MASK_E  = 2,
  LABEL_E = 3
};

// Class definition
class Volume : public boost::noncopyable {

// -- constructor/destructor --
  public:

    // For defining a data/label volume
    Volume(const GridTransform& grid_transform, 
           const DataBlockHandle& data_block, 
           VolumeType type = DATA_E);    

    // For definint a mask volume
    Volume(const GridTransform& grid_transform, 
           const MaskDataBlockHandle& mask_data_block);
               
    virtual ~Volume();
          
// -- accessors --
  public:

    // TYPE
    // Get the type of the data layer
    VolumeType type() const 
    { 
      return type_; 
    }

    // GRIDTRANSFORM
    // Get the grid location
    Utils::GridTransform grid_transform() const 
    { 
      return grid_transform_; 
    }
    
    // NX, NY, NZ, SIZE
    // The volume dimensions
    size_t nx() const { return grid_transform_.nx(); }
    size_t ny() const { return grid_transform_.ny(); }
    size_t nz() const { return grid_transform_.nz(); }
    size_t size() const { return nx()*ny()*nz(); }
    
    // DATA_BLOCK:
    // Get the datablock that contains the volume data   
    DataBlockHandle data_block() { return data_block_; }
    
    // MASK_DATA_BLOCK:
    // Get the datablock that contains the mask
    MaskDataBlockHandle mask_data_block() { return mask_data_block_; }

// -- internals of volume --
  private:
  
    // Type of volume
    VolumeType type_;

    // Location of the volume object in space
    // NOTE: Currently only axis aligned transforms are allowed
    GridTransform grid_transform_;

    // NOTE: Either a DataBlock or a mask DataBlock is defined
    // *not* both, hence one of the handles will remain empty

    // Handle to where the volume data is really stored
    DataBlockHandle data_block_;
    
    // Handle to where the mask volume is really stored
    MaskDataBlockHandle mask_data_block_;
    
// -- Static constructors --
  public:
  
    // CREATEDATAVOLUMEFROMNRRD:
    // Create a data volume from a nrrd
    static VolumeHandle CreateDataVolumeFromNrrd(NrrdDataHandle& nrrddata);
    
    // CREATEMASKVOLUMEFROMNRRD:
    // Create a data volume from a nrrd
    static VolumeHandle CreateMaskVolumeFromNrrd(NrrdDataHandle& nrrddata);
    
};

} // end namespace Utils

#endif
