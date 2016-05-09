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

// This implements a subclass of the std::vector class, except that
// the vector is statically allocated on the stack for performance.

#ifndef CORE_UTILS_STACKBASEDVECTOR_H
#define CORE_UTILS_STACKBASEDVECTOR_H

// STL includes
#include <memory>
#include <vector>

//! This vector type is similar in preformance to the StackVector, but checks
//! for overflow. In case of overflow memory will be reserved to store data_ 

namespace Core
{

template< class T, int CAPACITY >
class StackBasedVector
{
public:
  typedef T value_type;
  typedef value_type* iterator_type;
  typedef const value_type* const_iterator_type;
  typedef value_type& reference_type;
  typedef const value_type& const_reference_type;
  typedef size_t size_type;

  StackBasedVector()
  {
    initialize();
  }

  ~StackBasedVector()
  {
    if ( this->data_ != &( this->fixed_[ 0 ] ) )
    {
      delete[] this->data_;
    }
  }

  StackBasedVector( size_type n )
  {
    this->initialize();
    this->resize( n );
  }

  StackBasedVector( size_type n, T val )
  {
    this->initialize();
    this->resize( n );
    for ( size_t i = 0; i < n; i++ )
    {
      this->data_[ i ] = val;
    }
  }

  void initialize()
  {
    this->curalloc_ = CAPACITY;
    this->cursize_ = 0;
    this->data_ = &( this->fixed_[ 0 ] );
  }

  inline iterator_type begin()
  {
    return this->data_;
  }

  inline const_iterator_type begin() const
  {
    return this->data_;
  }

  inline iterator_type end()
  {
    return this->data_ + this->cursize_;
  }

  inline const_iterator_type end() const
  {
    return this->data_ + this->cursize_;
  }

  inline void reserve( size_type s )
  {
    this->enlarge( s );
  }

  inline void clear()
  {
    this->cursize_ = 0;
  }

  inline void push_back( const T& x )
  {
    if ( this->cursize_ == this->curalloc_ )
    {
      this->enlarge();
    }
    this->data_[ this->cursize_ ] = x;
    this->cursize_++;
  }

  template< class InputIterator >
  void insert( iterator_type pos, InputIterator first, InputIterator last )
  {
    size_type count = 0;
    for ( InputIterator iter = first; iter != last; iter++ )
    {
      count++;
    }

    if ( this->curalloc_ < this->cursize_ + count ) 
    {
      this->enlarge( this->cursize_ + count );
    }

    // adjust (start at end and go to the beginning to not overwrite self)
    for ( iterator_type iter = this->end() - 1; iter != pos - 1; iter-- )
    {
      *( iter + count ) = *iter;
    }
    // insert
    for ( ; first != last; first++, pos++ )
    {
      *pos = *first;
    }
    this->cursize_ += count;
  }

  void enlarge( size_type new_size = 0 )
  {
    if ( new_size == 0 )
    {
      this->curalloc_ = ( this->curalloc_ << 1 );
    }
    else
    {
      if ( this->curalloc_ < new_size )
      {
        this->curalloc_ = new_size;
      }
    }

    if ( this->curalloc_ > CAPACITY )
    {
      T* newdata = new T[ this->curalloc_ ];
      for ( size_type i = 0; i < this->cursize_; i++ )
      {
        newdata[ i ] = this->data_[ i ];
      }
      if ( this->data_ != &( this->fixed_[ 0 ] ) ) 
      {
        delete[] this->data_;
      }
      this->data_ = newdata;
    }
    else
    {
      this->data_ = &( this->fixed_[ 0 ] );
    }
  }

  inline size_type size() const
  {
    return this->cursize_;
  }

  inline void resize( size_type newsize )
  {
    this->cursize_ = newsize;
    if ( newsize > this->curalloc_ ) 
    {
      this->enlarge( newsize );
    }
  }

  inline reference_type operator[]( size_type n )
  {
    return this->data_[ n ];
  }

  inline const_reference_type operator[]( size_type n ) const
  {
    return this->data_[ n ];
  }

  StackBasedVector( const StackBasedVector< T,CAPACITY >& copy )
  {
    this->initialize();
    this->resize( copy.cursize_ );
    for ( size_type j = 0; j < this->cursize_; j++ )
    {
      this->data_[ j ] = copy.data_[ j ];
    }
  }

  StackBasedVector< T, CAPACITY >& operator=( const StackBasedVector< T, CAPACITY >& copy )
  {
    this->clear();
    this->resize( copy.cursize_ );
    for ( size_type j = 0; j < this->cursize_; j++ )
    {
      this->data_[ j ] = copy.data_[ j ];
    }
    return *this;
  }

private:

  T* data_;
  T fixed_[ CAPACITY ];
  size_type cursize_;
  size_type curalloc_;
};

} // end namespace

#endif
