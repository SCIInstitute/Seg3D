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

// File         : fft.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/03 10:16
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Wrapper class and helper functions for working with
//                FFTW3 and ITK images.

// system includes:
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <string.h>
#include <math.h>

// ITK includes:
#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>

// Boost includes:
#include <boost/thread/tss.hpp>

// local includes:
#include <Core/ITKCommon/FFT/fft.hxx>
#include <Core/ITKCommon/ThreadUtils/the_mutex_interface.hxx>
#include <Core/ITKCommon/the_utils.hxx>


namespace itk_fft
{
  //----------------------------------------------------------------
  // fftw_mutex
  // 
  static the_mutex_interface_t *
  fftw_mutex()
  {
    static the_mutex_interface_t * mutex = the_mutex_interface_t::create();
    return mutex;
  }
  
  //----------------------------------------------------------------
  // NUM_FFTW_THREADS
  // 
  static std::size_t NUM_FFTW_THREADS = 1;
  
  //----------------------------------------------------------------
  // set_num_fftw_threads
  // 
  // set's number of threads used by fftw, returns previous value:
  std::size_t set_num_fftw_threads(std::size_t num_threads)
  {
    the_lock_t<the_mutex_interface_t> lock(fftw_mutex());
    std::size_t prev = NUM_FFTW_THREADS;
    if (num_threads > 0)
    {
      
      NUM_FFTW_THREADS = num_threads;
    }
    
    return prev;
  }
  
  
  //----------------------------------------------------------------
  // fft_cache_t
  // 
  class fft_cache_t
  {
  public:
    fft_cache_t():
    fwd_(NULL),
    inv_(NULL),
    w_(0),
    h_(0),
    h_complex_(0),
    h_padded_(0),
    buffer_(NULL)
    {}
    
    ~fft_cache_t()
    {
      clear();
    }
    
    void clear()
    {
      if (buffer_)
      {
        // fftw is not thread safe:
        the_lock_t<the_mutex_interface_t> lock(fftw_mutex());
        
        fftwf_destroy_plan(fwd_);
        fwd_ = NULL;
        
        fftwf_destroy_plan(inv_);
        inv_ = NULL;
        
        fftwf_free(buffer_);
        buffer_ = NULL;
      }
      
      w_ = 0;
      h_ = 0;
      h_complex_ = 0;
      h_padded_ = 0;
    }
    
    void update(const unsigned int & w, const unsigned int & h)
    {
      if (w_ == w && h_ == h)
      {
        return;
      }
      
      // fftw is not thread safe:
      the_lock_t<the_mutex_interface_t> lock(fftw_mutex());
      
      if (buffer_)
      {
        fftwf_destroy_plan(fwd_);
        fftwf_destroy_plan(inv_);
        fftwf_free(buffer_);
      }
      
      w_ = w;
      h_ = h;
      
      h_complex_ = h_ / 2 + 1;
      h_padded_ = h_complex_ * 2;
      
      buffer_ = (float *)(fftwf_malloc(w_ * h_padded_ * sizeof(float)));
      fftwf_plan_with_nthreads(NUM_FFTW_THREADS);
      fwd_ = fftwf_plan_dft_r2c_2d(w_,
                                   h_,
                                   buffer_,
                                   (fftwf_complex *)buffer_,
                                   FFTW_DESTROY_INPUT | FFTW_ESTIMATE);
      
      fft_data_t dummy_in(4, 4);
      fft_data_t dummy_out(4, 4);
      fftwf_plan_with_nthreads(NUM_FFTW_THREADS);
      inv_ = fftwf_plan_dft_2d(w_,
                               h_,
                               (fftwf_complex *)(dummy_in.data()),
                               (fftwf_complex *)(dummy_out.data()),
                               FFTW_BACKWARD,
                               FFTW_ESTIMATE);
    }
    
    fftwf_plan fwd_; // analysis, forward fft
    fftwf_plan inv_; // synthesis, inverse fft
    unsigned int w_;
    unsigned int h_;
    unsigned int h_complex_;
    unsigned int h_padded_;
    float * buffer_;
  };
  
  //----------------------------------------------------------------
  // tss
  // 
  static boost::thread_specific_ptr<fft_cache_t> tss;
  
  //----------------------------------------------------------------
  // fft_data_t::fft_data_t
  // 
  fft_data_t::fft_data_t(const itk_image_t::Pointer & real):
  data_(NULL),
  nx_(0),
  ny_(0)
  {
    setup(real);
  }
  
  //----------------------------------------------------------------
  // fft_data_t::fft_data_t
  // 
  fft_data_t::fft_data_t(const unsigned int w, const unsigned int h):
  data_(NULL),
  nx_(0),
  ny_(0)
  {
    resize(w, h);
  }
  
  //----------------------------------------------------------------
  // fft_data_t::fft_data_t
  // 
  fft_data_t::fft_data_t(const itk_image_t::Pointer & real,
                         const itk_image_t::Pointer & imag):
  data_(NULL),
  nx_(0),
  ny_(0)
  {
    setup(real, imag);
  }
  
  //----------------------------------------------------------------
  // fft_data_t::fft_data_t
  // 
  fft_data_t::fft_data_t(const fft_data_t & data):
  data_(NULL),
  nx_(0),
  ny_(0)
  {
    *this = data;
  }
  
  //----------------------------------------------------------------
  // fft_data_t::operator =
  // 
  fft_data_t &
  fft_data_t::operator = (const fft_data_t & data)
  {
//    assert(this != &data);
    
    if (data.data_ != NULL)
    {
      resize(data.nx_, data.ny_);
      data_ = (fft_complex_t *)(memcpy(data_,
                                       data.data_,
                                       nx_ * ny_ * sizeof(fft_complex_t)));
    }
    else
    {
      cleanup();
    }
    
    return *this;
  }
  
  //----------------------------------------------------------------
  // fft_data_t::cleanup
  // 
  void
  fft_data_t::cleanup()
  {
    if (data_ != NULL) fftwf_free((fft_complex_t *)(data_));
    data_ = NULL;
    nx_ = 0;
    ny_ = 0;
  }
  
  //----------------------------------------------------------------
  // fft_data_t::resize
  // 
  void
  fft_data_t::resize(const unsigned int w, const unsigned int h)
  {
    const unsigned int new_sz = w  * h;
    const unsigned int old_sz = nx_ * ny_;
    if (old_sz == new_sz) return;
    
    if (data_ != NULL) fftwf_free((fft_complex_t *)(data_));
    data_ = static_cast<fft_complex_t *>( fftwf_malloc(new_sz * sizeof(fft_complex_t)) );
//    assert(data_ != NULL);
    if (data_ == NULL)
    {
      CORE_THROW_EXCEPTION("fftwf_malloc failed");
    }
    
    nx_ = w;
    ny_ = h;
  }
  
  //----------------------------------------------------------------
  // fft_data_t::fill
  // 
  void
  fft_data_t::fill(const float real, const float imag)
  {
    fftwf_complex * hack = reinterpret_cast<fftwf_complex *>(data_);
    
    for (unsigned int x = 0; x < nx_; x++)
    {
      for (unsigned int y = 0; y < ny_; y++)
      {
        unsigned int i = y + ny_ * x;
        
        hack[i][0] = real;
        hack[i][1] = imag;
      }
    }
  }
  
  //----------------------------------------------------------------
  // fft_data_t::setup
  // 
  void
  fft_data_t::setup(const itk_image_t::Pointer & real,
                    const itk_image_t::Pointer & imag)
  {
    typedef itk::ImageRegionConstIteratorWithIndex<itk_image_t> itex_t;
    typedef itk_image_t::IndexType index_t;
    
    itk::Size<2> size = real->GetLargestPossibleRegion().GetSize();
    resize(size[0], size[1]);
    
    // shortcut:
    fftwf_complex * hack = reinterpret_cast<fftwf_complex *>(data_);
    
    // iterate over the real component image:
    itex_t itex(real, real->GetLargestPossibleRegion());
    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
    {
      const index_t index = itex.GetIndex();
      const unsigned int x = index[0];
      const unsigned int y = index[1];
      const unsigned int i = y + ny_ * x;
      
      hack[i][0] = itex.Get();
      hack[i][1] = 0.0;
    }
    
    // iterate over the imaginary component image:
    if (imag.GetPointer() == NULL) return;
    itex = itex_t(imag, real->GetLargestPossibleRegion());
    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
    {
      const index_t index = itex.GetIndex();
      const unsigned int x = index[0];
      const unsigned int y = index[1];
      const unsigned int i = y + ny_ * x;
      
      hack[i][1] = itex.Get();
    }
  }
  
  //----------------------------------------------------------------
  // fft_data_t::component
  // 
  itk_image_t::Pointer
  fft_data_t::component(const bool imag) const
  {
    typedef itk::ImageRegionIteratorWithIndex<itk_image_t> itex_t;
    typedef itk_image_t::IndexType index_t;
    
    itk_image_t::Pointer out = itk_image_t::New();
    itk::Size<2> size;
    size[0] = nx_;
    size[1] = ny_;
    out->SetRegions(size);
    out->Allocate();
    
    // shortcut:
    const fftwf_complex * hack = reinterpret_cast<const fftwf_complex *>(data_);
    
    // iterate over the image:
    itex_t itex(out, out->GetLargestPossibleRegion());
    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
    {
      const index_t index = itex.GetIndex();
      const unsigned int x = index[0];
      const unsigned int y = index[1];
      const unsigned int i = y + ny_ * x;
      
      itex.Set(hack[i][imag]);
    }
    
    return out;
  }
  
  //----------------------------------------------------------------
  // fft_data_t::apply_lp_filter
  // 
  void
  fft_data_t::apply_lp_filter(const double r, const double s)
  {
    if (r > ::sqrt(2.0)) return;
    
    const unsigned int hx = nx_ / 2;
    const unsigned int hy = ny_ / 2;
    
    const double r0 = (r - s);
    const double r1 = (r + s);
    const double dr = r1 - r0;
    
    for (unsigned int x = 0; x < nx_; x++)
    {
      double sx = 2.0 * (double((x + hx) % nx_) - double(hx)) / double(nx_);
      double x2 = sx * sx;
      
      for (unsigned int y = 0; y < ny_; y++)
      {
        double sy = 2.0 * (double((y + hy) % ny_) - double(hy)) / double(ny_);
        double y2 = sy * sy;
        double d = ::sqrt(x2 + y2);
        
        double v =
        (d < r0) ? 1.0 :
        (d > r1) ? 0.0 :
        (1.0 + cos(M_PI * (d - r0) / dr)) / 2.0;
        
        unsigned int i = y + ny_ * x;
        data_[i] *= v;
      }
    }
  }
  
  
  //----------------------------------------------------------------
  // fft
  // 
  bool
  fft(const itk_image_t::Pointer & in, fft_data_t & out)
  {
    typedef itk::ImageRegionConstIteratorWithIndex<itk_image_t> itex_t;
    typedef itk_image_t::IndexType index_t;
    
    itk::Size<2> size = in->GetLargestPossibleRegion().GetSize();
    const unsigned int w = size[0];
    const unsigned int h = size[1];
    
    fft_cache_t * cache = tss.get();
    if (!cache)
    {
      cache = new fft_cache_t();
      tss.reset(cache);
    }
    cache->update(w, h);
    
    // iterate over the image:
    itex_t itex(in, in->GetLargestPossibleRegion());
    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
    {
      const index_t index = itex.GetIndex();
      const unsigned int x = index[0];
      const unsigned int y = index[1];
      const unsigned int i = y + cache->h_padded_ * x;
      
      cache->buffer_[i] = itex.Get();
    }
    
    if (!cache->fwd_) return false;
    fftwf_execute_dft_r2c(cache->fwd_,
                          cache->buffer_,
                          (fftwf_complex *)(cache->buffer_));
    
    // shortcuts:
    const unsigned int h_complex = cache->h_complex_;
    fft_complex_t * buffer = (fft_complex_t *)cache->buffer_;
    
    // fill in the rest of the output data:
    out.resize(w, h);
    for (unsigned int x = 0; x < w; x++)
    {
      for (unsigned int y = 0; y < h_complex; y++)
      {
        unsigned int i = y + h_complex * x;
        out(x, y) = buffer[i];
      }
      
      for (unsigned int y = h_complex; y < h; y++)
      {
        unsigned int i = (h - y) + h_complex * ((w - x) % w);
        out(x, y) = std::conj(buffer[i]);
      }
    }
    
    return true;
  }
  
  
  //----------------------------------------------------------------
  // ifft
  // 
  bool
  ifft(const fft_data_t & in, fft_data_t & out)
  {
    out.resize(in.nx(), in.ny());
    
    fft_cache_t * cache = tss.get();
    if (!cache)
    {
      return false;
    }
    
    fftwf_execute_dft(cache->inv_,
                      (fftwf_complex *)(in.data()),
                      (fftwf_complex *)(out.data()));
    return true;
  }
  
  //----------------------------------------------------------------
  // elem_by_elem
  // 
  void
  elem_by_elem(fn_fft_c_t f,
               const fft_data_t & in,
               fft_data_t & out)
  {
    const unsigned nx = in.nx();
    const unsigned ny = in.ny();
    
    out.resize(nx, ny);
    fft_complex_t * dout = out.data();
    const fft_complex_t * din = in.data();
    
    const unsigned int size = nx * ny;
    for (unsigned int i = 0; i < size; i++)
    {
      dout[i] = f(din[i]);
    }
  }
  
  
  //----------------------------------------------------------------
  // elem_by_elem
  //
  void
  elem_by_elem(fft_complex_t(*f)(const double & a,
                                 const fft_complex_t & b),
               const double & a,
               const fft_data_t & b,
               fft_data_t & c)
  {
//    assert(&b != &c);
    if (&b == &c)
    {
      CORE_THROW_EXCEPTION("Searching same elements");
    }
    
    const unsigned nx = b.nx();
    const unsigned ny = b.ny();
    c.resize(nx, ny);
    
    fft_complex_t * dc = c.data();
    const fft_complex_t * db = b.data();
    
    const unsigned int size = nx * ny;
    for (unsigned int i = 0; i < size; i++)
    {
      dc[i] = f(a, db[i]);
    }
  }
  
  
  //----------------------------------------------------------------
  // elem_by_elem
  // 
  void
  elem_by_elem(fft_complex_t(*f)(const fft_complex_t & a,
                                 const double & b),
               const fft_data_t & a,
               const double & b,
               fft_data_t & c)
  {
//    assert(&a != &c);
    if (&a == &c)
    {
      CORE_THROW_EXCEPTION("Searching same elements");
    }
    
    const unsigned nx = a.nx();
    const unsigned ny = a.ny();
    c.resize(nx, ny);
    
    fft_complex_t * dc = c.data();
    const fft_complex_t * da = a.data();
    
    const unsigned int size = nx * ny;
    for (unsigned int i = 0; i < size; i++)
    {
      dc[i] = f(da[i], b);
    }
  }
  
  
  //----------------------------------------------------------------
  // elem_by_elem
  //
  void
  elem_by_elem(fft_complex_t(*f)(const fft_complex_t & a,
                                 const fft_complex_t & b),
               const fft_complex_t & a,
               const fft_data_t & b,
               fft_data_t & c)
  {
//    assert(&b != &c);
    if (&b == &c)
    {
      CORE_THROW_EXCEPTION("Searching same elements");
    }
    
    const unsigned nx = b.nx();
    const unsigned ny = b.ny();
    c.resize(nx, ny);
    
    fft_complex_t * dc = c.data();
    const fft_complex_t * db = b.data();
    
    const unsigned int size = nx * ny;
    for (unsigned int i = 0; i < size; i++)
    {
      dc[i] = f(a, db[i]);
    }
  }
  
  
  //----------------------------------------------------------------
  // elem_by_elem
  // 
  void
  elem_by_elem(fft_complex_t(*f)(const fft_complex_t & a,
                                 const fft_complex_t & b),
               const fft_data_t & a,
               const fft_complex_t & b,
               fft_data_t & c)
  {
//    assert(&a != &c);
    if (&a == &c)
    {
      CORE_THROW_EXCEPTION("Searching same elements");
    }
    
    const unsigned nx = a.nx();
    const unsigned ny = a.ny();
    c.resize(nx, ny);
    
    fft_complex_t * dc = c.data();
    const fft_complex_t * da = a.data();
    
    const unsigned int size = nx * ny;
    for (unsigned int i = 0; i < size; i++)
    {
      dc[i] = f(da[i], b);
    }
  }
  
  
  //----------------------------------------------------------------
  // elem_by_elem
  //
  void
  elem_by_elem(fn_fft_cc_t f,
               const fft_data_t & a,
               const fft_data_t & b,
               fft_data_t & c)
  {
//    assert(&a != &c);
//    assert(&b != &c);
    if ( (&a == &c) || (&b == &c) )
    {
      CORE_THROW_EXCEPTION("Searching same elements");
    }
    
    const unsigned nx = a.nx();
    const unsigned ny = a.ny();
//    assert(nx == b.nx() && ny == b.ny());
    if ( (nx != b.nx()) || (ny != b.ny()) )
    {
      CORE_THROW_EXCEPTION("Mismatched element sizes");
    }
    
    c.resize(nx, ny);
    fft_complex_t * dc = c.data();
    
    const fft_complex_t * da = a.data();
    const fft_complex_t * db = b.data();
    
    const unsigned int size = nx * ny;
    for (unsigned int i = 0; i < size; i++)
    {
      dc[i] = f(da[i], db[i]);
    }
  }
  
} // namespace itk_fft
