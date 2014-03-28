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

// File         : extrema.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/04 11:10
// Copyright    : (C) 2004-2008 University of Utah
// Description  : SIFT key extrema point and descriptor vector classes.

#ifndef EXTREMA_HXX_
#define EXTREMA_HXX_

// system includes:
#include <vector>

// ITK includes:
#include <itkPoint.h>


//----------------------------------------------------------------
// KEY_SIZE
// 
static const unsigned int KEY_SIZE = 128;


//----------------------------------------------------------------
// extrema_t
// 
class extrema_t
{
public:
  
  // shorthand notation for 2D points:
  typedef itk::Point<double, 2> pnt2d_t;
  
  // local pixel coordinates (non-physical) within the octave image:
  pnt2d_t pixel_coords_;
  
  // coordinates in the physical space of the local image:
  pnt2d_t local_coords_;
  
  // physical coordinates within the target space:
  mutable pnt2d_t target_coords_;
  
  // FIXME: extrema mass (cluster mass normalized by cluster area):
  double mass_;
  
  // point ids:
  unsigned int pyramid_;
  unsigned int octave_;
  unsigned int scale_;
};

//----------------------------------------------------------------
// descriptor_t
// 
class descriptor_t
{
public:
  descriptor_t(extrema_t * extrema = NULL):
    extrema_(extrema)
  {}
  
  descriptor_t(const descriptor_t & key)
  { *this = key; }
  
  descriptor_t & operator = (const descriptor_t & key)
  {
    if (this == &key) return *this;
    
    extrema_ = key.extrema_;
    local_orientation_ = key.local_orientation_;
    target_orientation_ = key.target_orientation_;
    descriptor_ = key.descriptor_;
    
    return *this;
  }
  
  // reference to the extrema point:
  extrema_t * extrema_;
  
  // coordinate system orientation angle expressed in the local image space:
  double local_orientation_;
  
  // orientation angle in the target image space:
  mutable double target_orientation_;
  
  // the descriptor vector:
  std::vector<double> descriptor_;
};

//----------------------------------------------------------------
// ext_wrapper_t
// 
class ext_wrapper_t
{
public:
  ext_wrapper_t():
    key_(NULL)
  {}
  
  // vector-like accessor to the extrema target space coordinates:
  inline const double & operator [] (const unsigned int & index) const
  { return key_->extrema_->target_coords_[index]; }
  
  // pointer to the key:
  const descriptor_t * key_;
};

//----------------------------------------------------------------
// key_wrapper_t
// 
class key_wrapper_t
{
public:
  key_wrapper_t():
    key_(NULL)
  {}
  
  // vector-like accessor to the key descriptor:
  inline const double & operator [] (const unsigned int & index) const
  { return key_->descriptor_[index]; }
  
  // pointer to the key:
  const descriptor_t * key_;
};


#endif // EXTREMA_HXX_
