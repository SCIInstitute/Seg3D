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
#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Transform.h>
#include <Utils/Geometry/GridTransform.h>
#include <Utils/Core/EnumClass.h>

namespace Utils
{

// Forward Declaration
class Volume;
typedef boost::shared_ptr< Volume > VolumeHandle;
typedef boost::weak_ptr< Volume > VolumeWeakHandle;

SCI_ENUM_CLASS
(
  VolumeType,
  DATA_E = 1, 
  MASK_E = 2, 
  LABEL_E = 3
)

// Class definition
class Volume : public boost::noncopyable
{
public:
  typedef DataBlock::mutex_type mutex_type;
  typedef DataBlock::lock_type lock_type;

  // -- constructor/destructor --
public:

  // Called by the constructors of subclasses
  Volume( const GridTransform& grid_transform );

  virtual ~Volume();

  // -- accessors --
public:

  // TYPE
  // Get the type of the data layer
  virtual VolumeType type() const = 0;

  // GRIDTRANSFORM
  // Get the grid location
  const Utils::GridTransform& get_grid_transform() const
  {
    return this->grid_transform_;
  }

  // NX, NY, NZ, SIZE
  // The volume dimensions
  inline size_t nx() const
  {
    return this->nx_;
  }

  inline size_t ny() const
  {
    return this->ny_;
  }

  inline size_t nz() const
  {
    return this->nz_;
  }

  inline size_t size() const
  {
    return this->nx_ * this->ny_ * this->nz_;
  }

  inline size_t to_index( size_t x, size_t y, size_t z ) const
  {
    assert( x < this->nx_ && y < this->ny_ && z < this->nz_ );
    return z * this->nx_ * this->ny_ + y * this->nx_ + x;
  }

  Point apply_grid_transform( const Point& pt ) const;
  Point apply_inverse_grid_transform( const Point& pt ) const;

  virtual mutex_type& get_mutex() = 0;

  // -- internals of volume --
private:

  // Location of the volume object in space
  // NOTE: Currently only axis aligned transforms are allowed
  GridTransform grid_transform_;

  Transform inverse_grid_transform_;

  // Cached size information of the GridTransform
  size_t nx_;
  size_t ny_;
  size_t nz_;

  // Histogram
//  HistogramHandle histogram_;

};

} // end namespace Utils

#endif
