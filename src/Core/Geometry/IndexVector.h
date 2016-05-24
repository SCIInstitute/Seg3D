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

#ifndef CORE_GEOMETRY_INDEXVECTOR_H
#define CORE_GEOMETRY_INDEXVECTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <ostream>

namespace Core {

class IndexVector {
  // -- type of index --
public:
  typedef long long index_type;

  // -- constructor
public:
  IndexVector() 
  {
    this->data_[0] = 0;
    this->data_[1] = 0;
    this->data_[2] = 0;
  }

  IndexVector( index_type x, index_type y, index_type z )
  {
    this->data_[0] = x;
    this->data_[1] = y;
    this->data_[2] = z;
  }

  IndexVector( const IndexVector& idx )
  {
    this->data_[0] = idx.data_[0];
    this->data_[1] = idx.data_[1];
    this->data_[2] = idx.data_[2];
  }

  index_type x() const { return this->data_[0]; }
  index_type y() const { return this->data_[1]; }
  index_type z() const { return this->data_[2]; }

  double xd() const { return static_cast<double>( this->data_[0] ); }
  double yd() const { return static_cast<double>( this->data_[1] ); }
  double zd() const { return static_cast<double>( this->data_[2] ); }

  void x( index_type x ) { this->data_[0] = x; }
  void y( index_type y ) { this->data_[1] = y; }
  void z( index_type z ) { this->data_[2] = z; }

  bool operator==( const IndexVector& idx ) const
  {
    return (data_[0] == idx.data_[0] && data_[1] == idx.data_[1] && data_[2] == idx.data_[2]);
  }

  bool operator!=( const IndexVector& idx ) const
  {
    return (data_[0] != idx.data_[0] || data_[1] != idx.data_[1] || data_[2] != idx.data_[2]);
  }

  IndexVector&  operator=( const IndexVector& idx )
  {
    data_[0] = idx.data_[0];
    data_[1] = idx.data_[1];
    data_[2] = idx.data_[2];
    return *this;
  }

  index_type& operator[]( size_t idx )
  {
    return data_[idx];
  }

  index_type operator[]( size_t idx ) const
  {
    return data_[idx];
  }

  IndexVector operator+( const IndexVector& idx ) const
  {
    return IndexVector(data_[0] + idx.data_[0], data_[1] + idx.data_[1], data_[2]
      + idx.data_[2]);
  }

  IndexVector operator-( const IndexVector& idx ) const
  {
    return IndexVector(data_[0] - idx.data_[0], data_[1] - idx.data_[1], data_[2]
      - idx.data_[2]);
  }

  IndexVector& operator+=( const IndexVector& idx )
  {
    data_[0] += idx.data_[0];
    data_[1] += idx.data_[1];
    data_[2] += idx.data_[2];
    return *this;
  }

  IndexVector& operator-=( const IndexVector& idx )
  {
    data_[0] -= idx.data_[0];
    data_[1] -= idx.data_[1];
    data_[2] -= idx.data_[2];
    return *this;
  }

  // -- index coordinates --
private:
  index_type data_[3];
};

/// Import/Export functions

std::ostream& operator<<(std::ostream& os, const IndexVector& idx);

std::string ExportToString(const IndexVector& idx);
bool ImportFromString(const std::string& str, IndexVector& idx);

} // Core

#endif
