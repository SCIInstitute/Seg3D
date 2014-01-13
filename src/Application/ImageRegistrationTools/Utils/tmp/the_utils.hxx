/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

// File         : the_utils.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/15 11:25:00
// Copyright    : (C) 2004-2008 University of Utah
// Description  : utility functions for working with arrays,
//                lists, numbers, angles, etc...

#ifndef THE_UTILS_HXX_
#define THE_UTILS_HXX_

// system includes:
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <list>
#include <stack>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>

// local includes:
#include "utils/the_dynamic_array.hxx"


//----------------------------------------------------------------
// array2d
// 
#define array2d( T ) std::vector<std::vector<T> >

//----------------------------------------------------------------
// array3d
// 
#define array3d( T ) std::vector<std::vector<std::vector<T> > >

//----------------------------------------------------------------
// TWO_PI
// 
static const double TWO_PI = 2.0 * M_PI;


//----------------------------------------------------------------
// clamp_angle
// 
inline double
clamp_angle(const double & absolute_angle)
{
  double a = fmod(absolute_angle + TWO_PI, TWO_PI);
  assert(a >= 0.0 && a < TWO_PI);
  return a;
}

//----------------------------------------------------------------
// calc_angle
// 
inline double
calc_angle(const double & x,
	   const double & y,
	   const double & reference_angle = 0.0)
{
  return clamp_angle(fmod(atan2(y, x) + TWO_PI, TWO_PI) -
		     fmod(reference_angle, TWO_PI));
}

//----------------------------------------------------------------
// sleep_msec
// 
extern void sleep_msec(size_t msec);

//----------------------------------------------------------------
// drand
// 
inline static double drand()
{ return double(rand()) / double(RAND_MAX); }

//----------------------------------------------------------------
// integer_power
// 
template <typename scalar_t>
inline scalar_t
integer_power(scalar_t x, size_t p)
{
  scalar_t result = scalar_t(1);
  while (p != 0u)
  {
    if (p & 1) result *= x;
    x *= x;
    p >>= 1;
  }
  
  return result;
}

//----------------------------------------------------------------
// closest_power_of_two_larger_than_given
// 
template <typename scalar_t>
inline scalar_t
closest_power_of_two_larger_than_given(const scalar_t & given)
{
  size_t n = sizeof(given) * 8;
  scalar_t closest = scalar_t(1);
  for (size_t i = 0;
       (i < n) && (closest < given);
       i++, closest *= scalar_t(2)) {}
  
  return closest;
}


//----------------------------------------------------------------
// divide
// 
template <typename data_t>
data_t
divide(const data_t & numerator, const data_t & denominator)
{
  static data_t zero = data_t(0);
  return (denominator != zero) ? (numerator / denominator) : zero;
}

//----------------------------------------------------------------
// clear_stack
// 
template <typename data_t>
void
clear_stack(std::stack<data_t> & s)
{
  while (!s.empty())
  {
    s.pop();
  }
}

//----------------------------------------------------------------
// resize
// 
template <class data_t>
void
resize(array2d(data_t) & array,
       const size_t & rows,
       const size_t & cols)
{
  array.resize(rows);
  for (size_t j = 0; j < rows; j++)
  {
    array[j].resize(cols);
  }
}

//----------------------------------------------------------------
// assign
// 
template <class data_t>
void
assign(array2d(data_t) & array,
       const size_t & rows,
       const size_t & cols,
       const data_t & value)
{
  array.resize(rows);
  for (size_t j = 0; j < rows; j++)
  {
    array[j].assign(cols, value);
  }
}

//----------------------------------------------------------------
// resize
// 
template <class data_t>
void
resize(array3d(data_t) & array,
       const size_t & slices,
       const size_t & rows,
       const size_t & cols)
{
  array.resize(slices);
  for (size_t i = 0; i < slices; i++)
  {
    array[i].resize(rows);
    for (size_t j = 0; j < rows; j++)
    {
      array[i][j].resize(cols);
    }
  }
}


//----------------------------------------------------------------
// push_back_unique
// 
template <class container_t, typename data_t>
void
push_back_unique(container_t & container,
		 const data_t & data)
{
  typename container_t::const_iterator where =
    std::find(container.begin(), container.end(), data);
  if (where != container.end()) return;
  
  container.push_back(data);
}

//----------------------------------------------------------------
// push_front_unique
// 
template <class container_t, typename data_t>
void
push_front_unique(container_t & container,
		  const data_t & data)
{
  typename container_t::const_iterator where =
    std::find(container.begin(), container.end(), data);
  if (where != container.end()) return;
  
  container.push_front(data);
}

//----------------------------------------------------------------
// remove_head
// 
template <typename data_t>
data_t
remove_head(std::list<data_t> & container)
{
  data_t head = container.front();
  container.pop_front();
  return head;
}

//----------------------------------------------------------------
// remove_tail
// 
template <typename data_t>
data_t
remove_tail(std::list<data_t> & container)
{
  data_t tail = container.back();
  container.pop_back();
  return tail;
}

//----------------------------------------------------------------
// remove_head
// 
template <typename data_t>
data_t
remove_head(std::vector<data_t> & container)
{
  data_t head = container.front();
  container.erase(container.begin());
  return head;
}

//----------------------------------------------------------------
// remove_tail
// 
template <typename data_t>
data_t
remove_tail(std::vector<data_t> & container)
{
  data_t tail = container.back();
  container.pop_back();
  return tail;
}

//----------------------------------------------------------------
// is_size_two_or_larger
// 
template <typename container_t>
inline bool
is_size_two_or_larger(const container_t & c)
{
  typename container_t::const_iterator i = c.begin();
  typename container_t::const_iterator e = c.end();
  return (i != e) && (++i != e);
}

//----------------------------------------------------------------
// is_size_three_or_larger
// 
template <typename container_t>
inline bool
is_size_three_or_larger(const container_t & c)
{
  typename container_t::const_iterator i = c.begin();
  typename container_t::const_iterator e = c.end();
  return (i != e) && (++i != e) && (++i != e);
}

//----------------------------------------------------------------
// is_size_one
// 
template <typename container_t>
inline bool
is_size_one(const container_t & c)
{
  typename container_t::const_iterator i = c.begin();
  typename container_t::const_iterator e = c.end();
  return (i != e) && (++i == e);
}


//----------------------------------------------------------------
// replace
// 
template <class container_t, typename data_t>
bool
replace(container_t & container, const data_t & a, const data_t & b)
{
  typename container_t::iterator end = container.end();
  typename container_t::iterator i = std::find(container.begin(), end, a);
  if (i == end) return false;
  
  *i = b;
  return true;
}


//----------------------------------------------------------------
// iterator_at_index
// 
template <typename data_t>
typename std::list<data_t>::const_iterator
iterator_at_index(const std::list<data_t> & container,
		  const size_t & index)
{
  typename std::list<data_t>::const_iterator iter = container.begin();
  for (size_t i = 0; i < index && iter != container.end(); i++, ++iter) ;
  return iter;
}

//----------------------------------------------------------------
// iterator_at_index
// 
template <typename data_t>
typename std::list<data_t>::iterator
iterator_at_index(std::list<data_t> & container,
		  const size_t & index)
{
  typename std::list<data_t>::iterator iter = container.begin();
  for (size_t i = 0; i < index && iter != container.end(); i++, ++iter) ;
  return iter;
}

//----------------------------------------------------------------
// index_of
// 
template <typename data_t>
size_t
index_of(const std::list<data_t> & container, const data_t & data)
{
  typename std::list<data_t>::const_iterator iter = container.begin();
  for (size_t i = 0; iter != container.end(); i++, ++iter)
  {
    if (data == *iter) return i;
  }
  
  return ~0;
}

//----------------------------------------------------------------
// has
// 
template <typename data_t>
bool
has(const std::list<data_t> & container, const data_t & data)
{
  typename std::list<data_t>::const_iterator iter =
    std::find(container.begin(), container.end(), data);
  
  return iter != container.end();
}

//----------------------------------------------------------------
// expand
// 
template <class container_t>
container_t &
expand(container_t & a,
       const container_t & b,
       const bool unique = false)
{
  for (typename container_t::const_iterator i = b.begin(); i != b.end(); ++i)
  {
    if (unique)
    {
      push_back_unique(a, *i);
    }
    else
    {
      a.push_back(*i);
    }
  }
  
  return a;
}

//----------------------------------------------------------------
// next
// 
template <class iterator_t>
iterator_t
next(const iterator_t & curr)
{
  iterator_t tmp(curr);
  return ++tmp;
}

//----------------------------------------------------------------
// prev
// 
template <class iterator_t>
iterator_t
prev(const iterator_t & curr)
{
  iterator_t tmp(curr);
  return --tmp;
}


//----------------------------------------------------------------
// dump
// 
template <typename stream_t, typename data_t>
stream_t &
dump(stream_t & so, const std::list<data_t> & c)
{
  for (typename std::list<data_t>::const_iterator
	 i = c.begin(); i != c.end(); ++i)
  {
    so << *i << ' ';
  }
  
  return so;
}


//----------------------------------------------------------------
// operator <<
// 
template <typename stream_t, typename data_t>
std::ostream &
operator << (stream_t & so, const std::list<data_t> & c)
{
  return dump<stream_t, data_t>(so, c);
}


//----------------------------------------------------------------
// operator +
// 
// Construct an on-the-fly linked list containing two elements:
//
template <typename T>
inline std::list<T>
operator + (const T & a, const T & b)
{
  std::list<T> ab;
  ab.push_back(a);
  ab.push_back(b);
  return ab;
}

//----------------------------------------------------------------
// operator +
// 
// Construct an on-the-fly linked list containing list a with item b appended:
template <typename T>
inline std::list<T>
operator + (const std::list<T> & a, const T & b)
{
  std::list<T> ab(a);
  ab.push_back(b);
  return ab;
}

//----------------------------------------------------------------
// inserter_t
// 
template <class container_t, typename data_t>
class inserter_t
{
public:
  inserter_t(container_t & container,
	     const typename container_t::iterator & iter,
	     const bool & expand):
    container_(container),
    iter_(iter),
    expand_(expand)
  {}
  
  inline inserter_t<container_t, data_t> & operator << (const data_t & data)
  {
    if (iter_ == container_.end())
    {
      if (expand_)
      {
	iter_ = container_.insert(iter_, data);
      }
      else
      {
	assert(0);
      }
    }
    else
    {
      *iter_ = data;
      ++iter_;
    }
    
    return *this;
  }
  
private:
  // reference to the container:
  container_t & container_;
  
  // current index into the container:
  typename container_t::iterator iter_;
  
  // a flag indicating whether the container should be expanded to
  // accomodate the insertions:
  bool expand_;
};

//----------------------------------------------------------------
// operator <<
// 
template <typename data_t>
inserter_t<std::vector<data_t>, data_t>
operator << (std::vector<data_t> & container, const data_t & data)
{
  inserter_t<std::vector<data_t>, data_t>
    inserter(container, container.begin(), false);
  return inserter << data;
}

//----------------------------------------------------------------
// operator <<
// 
template <typename data_t>
inserter_t<std::list<data_t>, data_t>
operator << (std::list<data_t> & container, const data_t & data)
{
  inserter_t<std::list<data_t>, data_t>
    inserter(container, container.begin(), true);
  return inserter << data;
}


//----------------------------------------------------------------
// calc_euclidian_distance_sqrd
// 
template <size_t dimensions, typename data_t>
data_t
calc_euclidian_distance_sqrd(const std::vector<data_t> & a,
			     const std::vector<data_t> & b)
{
  data_t distance_sqrd = data_t(0);
  for (size_t i = 0; i < dimensions; i++)
  {
    data_t d = a[i] - b[i];
    distance_sqrd += d * d;
  }
  
  return distance_sqrd;
}

//----------------------------------------------------------------
// calc_euclidian_distance
// 
template <size_t dimensions, typename data_t>
data_t
calc_euclidian_distance(const std::vector<data_t> & a,
			const std::vector<data_t> & b)
{
  data_t dist_sqrd = calc_euclidian_distance_sqrd<dimensions, data_t>(a, b);
  double dist = sqrt(double(dist_sqrd));
  return data_t(dist);
}

//----------------------------------------------------------------
// calc_frobenius_norm_sqrd
//
template <typename data_t>
data_t
calc_frobenius_norm_sqrd(const std::vector<data_t> & vec)
{
  data_t L2_norm_sqrd = data_t(0);
  
  const size_t len = vec.size();
  for (size_t i = 0; i < len; i++)
  {
    L2_norm_sqrd += vec[i] * vec[i];
  }
  
  return L2_norm_sqrd;
}

//----------------------------------------------------------------
// calc_frobenius_norm
// 
template <typename data_t>
data_t
calc_frobenius_norm(const std::vector<data_t> & vec)
{
  data_t norm_sqrd = calc_frobenius_norm_sqrd<data_t>(vec);
  double norm = sqrt(double(norm_sqrd));
  return data_t(norm);
}

//----------------------------------------------------------------
// normalize
// 
template <typename data_t>
void
normalize(std::vector<data_t> & vec)
{
  data_t norm = calc_frobenius_norm<data_t>(vec);
  
  const size_t len = vec.size();
  for (size_t i = 0; i < len; i++)
  {
    vec[i] /= norm;
  }
}


//----------------------------------------------------------------
// the_sign
// 
template <class T>
inline T
the_sign(const T & a)
{
  if (a < 0) return -1;
  if (a > 0) return  1;
  return 0;
}


//----------------------------------------------------------------
// copy_a_to_b
// 
// list -> array:
// 
template <class T>
void
copy_a_to_b(const std::list<T> & container_a,
	    std::vector<T> & container_b)
{
  container_b.assign(container_a.begin(), container_a.end());
}

//----------------------------------------------------------------
// copy_a_to_b
// 
// list -> array:
// 
template <class T>
void
copy_a_to_b(const std::list<T> & container_a,
	    the_dynamic_array_t<T> & container_b)
{
  container_b.resize(container_a.size());
  
  const size_t size = container_a.size();
  typename std::list<T>::const_iterator iter = container_a.begin();
  for (size_t i = 0; i < size; i++, ++iter)
  {
    container_b[i] = *iter;
  }
}

//----------------------------------------------------------------
// copy_a_to_b
// 
// dynamic_array -> array
// 
template <class T>
void
copy_a_to_b(const the_dynamic_array_t<T> & container_a,
	    std::vector<T> & container_b)
{
  container_b.resize(container_a.size());
  
  const size_t & size = container_a.size();
  for (size_t i = 0; i < size; i++)
  {
    container_b[i] = container_a[i];
  }
}


//----------------------------------------------------------------
// the_lock_t
// 
template <typename T>
class the_lock_t
{
public:
  the_lock_t(T * lock, bool lock_immediately = true):
    lock_(lock),
    armed_(false)
  { if (lock_immediately) arm(); }
  
  the_lock_t(T & lock, bool lock_immediately = true):
    lock_(&lock),
    armed_(false)
  { if (lock_immediately) arm(); }
  
  ~the_lock_t()
  { disarm(); }
  
  inline void arm()
  {
    if (!armed_ && lock_ != NULL)
    {
      lock_->lock();
      armed_ = true;
    }
  }
  
  inline void disarm()
  {
    if (armed_ && lock_ != NULL)
    {
      lock_->unlock();
      armed_ = false;
    }
  }
  
private:
  the_lock_t();
  the_lock_t(const the_lock_t &);
  the_lock_t & operator = (const the_lock_t &);
  
  T * lock_;
  bool armed_;
};

//----------------------------------------------------------------
// the_unlock_t
// 
template <typename T>
class the_unlock_t
{
public:
  the_unlock_t(T * lock):
    lock_(lock)
  {
    if (lock_ != NULL)
    {
      assert(lock_->try_lock() == false);
    }
  }
  
  the_unlock_t(T & lock):
    lock_(&lock)
  {
    if (lock_ != NULL)
    {
      assert(lock_->try_lock() == false);
    }
  }
  
  ~the_unlock_t()
  {
    if (lock_ != NULL)
    {
      lock_->unlock();
    }
  }
  
private:
  the_unlock_t();
  the_unlock_t(const the_unlock_t &);
  the_unlock_t & operator = (const the_unlock_t &);
  
  T * lock_;
};

//----------------------------------------------------------------
// the_scoped_variable_t
// 
template <typename T>
class the_scoped_variable_t
{
public:
  the_scoped_variable_t(T & variable,
			const T & in_scope_value,
			const T & out_of_scope_value):
    var_(variable),
    in_(in_scope_value),
    out_(out_of_scope_value)
  {
    var_ = in_;
  }
  
  ~the_scoped_variable_t()
  {
    var_ = out_;
  }

private:
  the_scoped_variable_t(const the_scoped_variable_t &);
  the_scoped_variable_t & operator = (const the_scoped_variable_t &);
  
  T & var_;
  const T in_;
  const T out_;
};

//----------------------------------------------------------------
// the_scoped_increment_t
// 
template <typename T>
class the_scoped_increment_t
{
public:
  the_scoped_increment_t(T & variable):
    var_(variable)
  {
    var_++;
  }
  
  ~the_scoped_increment_t()
  {
    var_--;
  }
  
private:
  the_scoped_increment_t(const the_scoped_increment_t &);
  the_scoped_increment_t & operator = (const the_scoped_increment_t &);
  
  T & var_;
};

//----------------------------------------------------------------
// THROW_ARG2_IF_FALSE
// 
#ifndef THROW_ARG2_IF_FALSE
#define THROW_ARG2_IF_FALSE(predicate, arg2) \
if (predicate) {} else throw arg2
#endif

//----------------------------------------------------------------
// restore_console_stdio
//
// Reopen stdin, stdout, stderr on windows, no-op everywhere else
// 
extern bool
restore_console_stdio();


//----------------------------------------------------------------
// off_t
// 
#ifdef _WIN32
#define off_t __int64
#endif

namespace the
{
  extern int open_utf8(const char * filename_utf8,
		       int oflag,
		       int pmode);
  
  extern void open_utf8(std::fstream & fstream_to_open,
			const char * filename_utf8,
			std::ios_base::openmode mode);
  
  extern FILE * fopen_utf8(const char * filename_utf8,
			   const char * mode);
  
  extern int rename_utf8(const char * old_utf8, const char * new_utf8);
  extern int remove_utf8(const char * filename_utf8);
  
  extern int rmdir_utf8(const char * path_utf8);
  extern int mkdir_utf8(const char * path_utf8);
  
  extern int fseek64(FILE * file, off_t offset, int whence);
  extern off_t ftell64(const FILE * file);
  
  inline static bool
  close_enough(const float & ref,
	       const float & given,
	       const float tolerance = 1e-6f)
  {
    float err = fabsf(given - ref);
    return err < tolerance;
  }
  
  inline static bool
  close_enough(const double & ref,
	       const double & given,
	       const double tolerance = 1e-6)
  {
    double err = fabs(given - ref);
    return err < tolerance;
  }
}


#endif // THE_UTILS_HXX_
