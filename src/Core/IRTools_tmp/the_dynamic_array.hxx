// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: t -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// File         : the_dynamic_array.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Oct 31 17:16:25 MDT 2003
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : Implementation of a dynamically resizable array
//                that grows automatically.

#ifndef THE_DYNAMIC_ARRAY_HXX_
#define THE_DYNAMIC_ARRAY_HXX_

// system includes:
#include <algorithm>
#include <iostream>
#include <vector>

// forward declarations:
template<class T> class the_dynamic_array_t;

#undef min
#undef max


//----------------------------------------------------------------
// the_dynamic_array_ref_t
// 
template<class T>
class the_dynamic_array_ref_t
{
public:
  the_dynamic_array_ref_t(the_dynamic_array_t<T> & array,
			  size_t index = 0):
    array_(array),
    index_(index)
  {}
  
  inline the_dynamic_array_ref_t<T> & operator << (const T & elem)
  {
    array_[index_++] = elem;
    return *this;
  }
  
private:
  // reference to the array:
  the_dynamic_array_t<T> & array_;
  
  // current index into the array:
  size_t index_;
};


//----------------------------------------------------------------
// the_dynamic_array_t
// 
template<class T>
class the_dynamic_array_t
{
public:
  the_dynamic_array_t():
    array_(NULL),
    page_size_(16),
    size_(0),
    init_value_()
  {}
  
  the_dynamic_array_t(const size_t & init_size):
    array_(NULL),
    page_size_(init_size),
    size_(0),
    init_value_()
  {}
  
  the_dynamic_array_t(const size_t & init_size,
		      const size_t & page_size,
		      const T & init_value):
    array_(NULL),
    page_size_(page_size),
    size_(0),
    init_value_(init_value)
  {
    resize(init_size);
  }
  
  // copy constructor:
  the_dynamic_array_t(const the_dynamic_array_t<T> & a):
    array_(NULL),
    page_size_(0),
    size_(0),
    init_value_(a.init_value_)
  {
    (*this) = a;
  }
  
  // destructor:
  ~the_dynamic_array_t()
  {
    clear();
  }
  
  // remove all contents of this array:
  void clear()
  {
    size_t num = num_pages();
    for (size_t i = 0; i < num; i++)
    {
      delete (*array_)[i];
    }
    
    delete array_;
    array_ = NULL;
    
    size_ = 0;
  }
  
  // the assignment operator:
  the_dynamic_array_t<T> & operator = (const the_dynamic_array_t<T> & array)
  {
    clear();
    
    page_size_  = array.page_size_;
    init_value_ = array.init_value_;
    
    resize(array.size_);
    for (size_t i = 0; i < size_; i++)
    {
      (*this)[i] = array[i];
    }
    
    return *this;
  }
  
  // resize the array, all contents will be preserved:
  void resize(const size_t & new_size)
  {
    // bump the current size value:
    size_ = new_size;
    
    // do nothing if resizing is unnecessary:
    if (size_ <= max_size()) return;
    
    // we'll have to do something about the existing data:
    size_t old_num_pages = num_pages();
    size_t new_num_pages =
      std::max((size_t)(2 * old_num_pages),
	       (size_t)(1 + size_ / page_size_));
    
    // create a new array:
    std::vector< std::vector<T> * > * new_array =
      new std::vector< std::vector<T> * >(new_num_pages);
    
    // shallow-copy the old content:
    for (size_t i = 0; i < old_num_pages; i++)
    {
      (*new_array)[i] = (*array_)[i];
    }
    
    // initialize the new pages:
    for (size_t i = old_num_pages; i < new_num_pages; i++)
    {
      (*new_array)[i] = new std::vector<T>(page_size_);
      for (size_t j = 0; j < page_size_; j++)
      {
	(*(*new_array)[i])[j] = init_value_;
      }
    }
    
    // get rid of the old array:
    delete array_;
    
    // put the new array in place of the old array:
    array_ = new_array;
  }
  
  // the size of this array:
  inline const size_t & size() const
  { return size_; }
  
  inline const size_t & page_size() const
  { return page_size_; }
  
  // maximum usable size of the array that does not require resizing the array:
  inline size_t max_size() const
  { return num_pages() * page_size_; }
  
  // number of pages currently allocated:
  inline size_t num_pages() const
  { return (array_ == NULL) ? 0 : array_->size(); }
  
  inline const T * page(const size_t & page_index) const
  { return &((*(*array_)[page_index])[0]); }
  
  inline T * page(const size_t & page_index)
  { return &((*(*array_)[page_index])[0]); }
  
  // return either first or last index into the array:
  inline size_t end_index(bool last) const
  {
    if (last == false) return 0;
    return size_ - 1;
  }
  
  // return either first or last element in the array:
  inline const T & end_elem(bool last) const
  { return elem(end_index(last)); }
  
  inline T & end_elem(bool last)
  { return elem(end_index(last)); }
  
  inline const T & front() const
  { return end_elem(false); }
  
  inline T & front()
  { return end_elem(false); }
  
  inline const T & back() const
  { return end_elem(true); }
  
  inline T & back()
  { return end_elem(true); }
  
  // non-const accessors:
  inline T & elem(const size_t i)
  {
    if (i >= size_) resize(i + 1);
    return (*(*array_)[i / page_size_])[i % page_size_];
  }
  
  inline T & operator [] (const size_t & i)
  { return elem(i); }
  
  // const accessors:
  inline const T & elem(const size_t & i) const
  { return (*((*array_)[i / page_size_]))[i % page_size_]; }
  
  inline const T & operator [] (const size_t & i) const
  { return elem(i); }
  
  // this is usefull for filling-in the array:
  the_dynamic_array_ref_t<T> operator << (const T & elem)
  {
    (*this)[0] = elem;
    return the_dynamic_array_ref_t<T>(*this, 1);
  }
  
  // grow the array by one and insert a new element at the tail:
  inline void push_back(const T & elem)
  { (*this)[size_] = elem; }
  
  inline void append(const T & elem)
  { push_back(elem); }
  
  // return the index of the first occurrence of a given element in the array:
  size_t index_of(const T & element) const
  {
    for (size_t i = 0; i < size_; i++)
    {
      if (!(elem(i) == element)) continue;
      return i;
    }
    
    return ~0u;
  }
  
  // check whether this array contains a given element:
  inline bool has(const T & element) const
  { return index_of(element) != ~0u; }
  
  // remove an element from the array:
  bool remove(const T & element)
  {
    size_t idx = index_of(element);
    if (idx == ~0u) return false;
    
    for (size_t i = idx + 1; i < size_; i++)
    {
      elem(i - 1) = elem(i);
    }
    
    size_--;
    return true;
  }
  
  void assign(const size_t & size, const T & element)
  {
    resize(size);
    for (size_t i = 0; i < size; i++)
    {
      elem(i) = element;
    }
  }
  
  // for debugging, dumps this list into a stream:
  void dump(std::ostream & strm) const
  {
    strm << "the_dynamic_array_t(" << (void *)this << ") {\n";
    for (size_t i = 0; i < size_; i++)
    {
      strm << elem(i) << std::endl;
    }
    strm << '}';
  }
  
protected:
  // an array of pointers to arrays (pages) of data:
  std::vector< std::vector<T> *> * array_;
  
  // page size:
  size_t page_size_;
  
  // current array size:
  size_t size_;
  
  // init value used when resizing the array:
  T init_value_;
};

//----------------------------------------------------------------
// operator <<
// 
template <class T>
std::ostream &
operator << (std::ostream & s, const the_dynamic_array_t<T> & a)
{
  a.dump(s);
  return s;
}


#endif // THE_DYNAMIC_ARRAY_HXX_
