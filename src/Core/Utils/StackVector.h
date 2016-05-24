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

#ifndef CORE_UTILS_STACKVECTOR_H
#define CORE_UTILS_STACKVECTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <assert.h>

namespace Core 
{

// This implements a subclass of the std::vector class, except that
// the vector is statically allocated on the stack for performance.
template < class T, int CAPACITY >
class StackVector 
{

public:
// -- required typedefs for vector type class --
  typedef T        value_type;
  typedef T*       pointer_type;
  typedef T&       reference_type;
  typedef const T& const_reference_type;
  typedef T*       iterator_type;
  typedef const T* const_iterator_type;
  typedef size_t   size_type;

// -- constructor/destructor --
  StackVector() 
  { 
    this->size_ = 0; 
  }
  
  StackVector( size_t s ) 
  { 
    assert( s <= CAPACITY ); 
    this->size_ = s; 
  }
  
  StackVector( size_t s, T val )
  { 
    assert( s <= CAPACITY ); 
    this->size_ = s; 
    for ( size_t i = 0; i < s; i++ ) 
    {
      this->data_[ i ] = val; 
    }
  }
  
  ~StackVector() {}

// -- iterators --
  iterator_type begin()
  { 
    return this->data_; 
  }
  
  iterator_type end() 
  { 
    return this->data_ + this->size_; 
  }
  
  const_iterator_type begin() const 
  { 
    return this->data_; 
  }
  
  const_iterator_type end() const 
  { 
    return this->data_ + this->size_; 
  }
  
// -- capacity -- 
  size_t size() const 
  { 
    assert( this->size_ <= CAPACITY ); 
    return this->size_; 
  }
  
  size_t max_size() const 
  { 
    return CAPACITY; 
  }
  
  size_t capacity() const 
  { 
    return CAPACITY; 
  }
  
  bool empty() const 
  { 
    return this->size_; 
  }

// -- accessors --
  reference_type operator[]( size_t n ) 
  { 
    return this->data_[ n ]; 
  }
  
  const_reference_type operator[]( size_t n ) const 
  { 
    return this->data_[ n ]; 
  }
  
  void resize( size_t s ) 
  { 
    assert( s <= CAPACITY ); 
    this->size_ = s; 
  }
  
  void reserve( size_t ) {}  // Do nothing: We already have a certain capacity
  
  void push_back( T t ) 
  { 
    assert( this->size_ < CAPACITY ); 
    this->data_[ this->size_ ] = t; 
    this->size_++; 
  }
  
  void clear() 
  { 
    this->size_ = 0; 
  }
  
// -- internals --  
private:
  size_t size_;
  T data_[ CAPACITY ];
};

} // End namespace Core

#endif  

