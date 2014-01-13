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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_UTILS_NEIGHBOR_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_UTILS_NEIGHBOR_H

//----------------------------------------------------------------
// neighbor_t
// 
class neighbor_t
{
public:
  neighbor_t():
  id_(UINT_MAX),
  metric_(std::numeric_limits<double>::max())
  {}
  
  neighbor_t(const unsigned int & id,
             const double & metric,
             translate_transform_t::Pointer & t):
  id_(id),
  metric_(metric),
  t_(t)
  {}
  
  inline bool operator == (const neighbor_t & d) const
  { return id_ == d.id_; }
  
  inline bool operator < (const neighbor_t & d) const
  { return metric_ < d.metric_; }
  
  unsigned int id_;
  double metric_;
  translate_transform_t::Pointer t_;
};

#endif