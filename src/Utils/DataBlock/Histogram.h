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
 
 
#ifndef UTILS_DATABLOCK_HISTOGRAM_H
#define UTILS_DATABLOCK_HISTOGRAM_H

#include <vector>

namespace Utils
{

// Forward Declaration
class Histogram;

class Histogram
{

  // -- constructor/destructor --
public:

  Histogram();
  Histogram( const signed char* data, size_t size );
  Histogram( const unsigned char* data, size_t size );
  Histogram( const short* data, size_t size );
  Histogram( const unsigned short* data, size_t size );
  Histogram( const int* data, size_t size );
  Histogram( const unsigned int* data, size_t size );
  Histogram( const float* data, size_t size );
  Histogram( const double* data, size_t size );

  virtual ~Histogram();
  
  void compute( const signed char* data, size_t size );
  void compute( const unsigned char* data, size_t size );
  void compute( const short* data, size_t size );
  void compute( const unsigned short* data, size_t size );
  void compute( const int* data, size_t size );
  void compute( const unsigned int* data, size_t size );
  void compute( const float* data, size_t size );
  void compute( const double* data, size_t size );
  
  inline double get_min() const
  { 
    return this->min_;
  }
  
  inline double get_max() const
  { 
    return this->max_; 
  }
  
  inline double get_bin_size() const
  {
    return this->bin_size_;
  }
  
  inline double get_bin_start( size_t idx = 0) const
  {
    return this->bin_start_ + idx * this->bin_size_;
  }

  inline double get_bin_end( size_t idx = 0) const
  {
    return this->bin_start_ + ( idx + 1 ) * this->bin_size_;
  }
  
  inline std::vector<size_t> get_bins() const
  {
    return this->histogram_;
  }
    
private:

  double min_;
  double max_;

  double bin_start_;
  double bin_size_;
  
  std::vector<size_t> histogram_;

};

} // end namespace Utils

#endif
