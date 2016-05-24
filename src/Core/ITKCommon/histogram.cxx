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

// File         : histogram.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 12:36
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for working with circular histograms.

// local includes:
#include <Core/ITKCommon/histogram.hxx>
#include <Core/ITKCommon/the_utils.hxx>

// system includes:
#include <vector>
#include <algorithm>
#include <limits> // gcc 3.3.5 ?
#include <math.h>


//----------------------------------------------------------------
// smoothout_orientation_histogram
// 
void
smoothout_orientation_histogram(double * orientation,
        const unsigned int & bins,
        const unsigned int iterations)
{
  std::vector<double> tmp(bins);
  for (unsigned int i = 0; i < iterations; i++)
  {
    for (unsigned int j = 0; j < bins; j++)
    {
      tmp[j] =
  (orientation[(j - 1 + bins) % bins] +
   orientation[j] +
   orientation[(j + 1) % bins]) / 3.0;
    }
    
    for (unsigned int j = 0; j < bins; j++)
    {
      orientation[j] = tmp[j];
    }
  }
}

//----------------------------------------------------------------
// isolate_orientation_histogram_peaks
// 
bool
isolate_orientation_histogram_peaks(double * histogram,
            const unsigned int & bins,
            const bool normalize)
{
  // find the min/max of the histogram so that it can be remapped
  // into the [0, 1] range:
  double v_min = std::numeric_limits<double>::max();
  double v_max = -v_min;
  double v_avg = 0.0;
  for (unsigned int i = 0; i < bins; i++)
  {
    const double & v = histogram[i];
    v_avg += v;
    v_min = std::min(v_min, v);
    v_max = std::max(v_max, v);
  }
  v_avg /= double(bins);
  
  // calculate the min/max range:
  const double v_rng = v_max - v_min;
  if (v_rng == 0.0) return false;
  
  for (unsigned int i = 0; i < bins; i++)
  {
    const double v = histogram[i];
    histogram[i] = (v - v_min) / v_rng;
  }
  
  // FIXME:
  // cout << "before: " << endl; dump(cout, &histogram[0], bins);
  
  // isolate the peaks:
  {
    std::vector<double> histogram_smoothed;
    histogram_smoothed.assign(&histogram[0], &histogram[bins]);
    smoothout_orientation_histogram(&histogram_smoothed[0], bins, 3);
    
    // 1. split up the peaks:
    double tmp_avg = (v_avg - v_min) / v_rng;
    double new_avg = 0.0;
    unsigned int counter = 0;
    for (unsigned int i = 0; i < bins; i++)
    {
      // double v = histogram_smoothed[i];
      double v = tmp_avg + 0.9 * (histogram_smoothed[i] - tmp_avg);
      
      double d = histogram[i] - v;
      if (d < 0.0) histogram[i] = 0.0;
      else
      {
  new_avg += histogram[i];
  counter++;
      }
    }
    
    new_avg /= double(counter);
    
    // FIXME:
    // cout << "isolated a: " << endl; dump(cout, &histogram[0], bins);
    
    // 2. weed out the weak peaks:
    for (unsigned int i = 0; i < bins; i++)
    {
      double v = new_avg + 0.9 * (histogram[i] - new_avg);
      double d = histogram[i] - v;
      if (d < 0.0) histogram[i] = 0.0;
    }
    
    // FIXME:
    // cout << "isolated: " << endl; dump(cout, &histogram[0], bins);
    // cout << endl;
  }
  
  if (!normalize)
  {
    // restore the peaks to their original range:
    for (unsigned int i = 0; i < bins; i++)
    {
      const double v = histogram[i];
      histogram[i] = v * v_rng + v_min;
    }
  }
  
  return true;
}

//----------------------------------------------------------------
// calc_histogram_donations
// 
unsigned int
calc_histogram_donations(const unsigned int & bins,
       const double & r0,
       const double & r1,
       const double & r,
       unsigned int * donation_bin,
       double * donation)
{
  const unsigned int a0 = 0;
  const unsigned int a1 = bins - 1;
  
  const double t_radius = divide(r - r0, r1 - r0);
  const double ta = t_radius * double(bins);
  const unsigned int a = std::min(bins - 1, (unsigned int)(floor(ta)));
  const double t = ta - a;
  
  donation_bin[1] = a;
  donation_bin[2] = std::min(bins - 1, donation_bin[1] + 1);
  donation_bin[0] = std::min(bins - 1, donation_bin[1] - 1);
  
  donation[0] = a > a0 ? std::max(0.0, 0.5 - t) : 0.0;
  donation[2] = a < a1 ? std::max(0.0, t - 0.5) : 0.0;
  donation[1] = 1.0 - (donation[0] + donation[2]);
  
  // FIXME:
  assert(donation[0] <= 0.5 && donation[0] >= 0.0);
  assert(donation[1] <= 1.0 && donation[1] >= 0.0);
  assert(donation[2] <= 0.5 && donation[2] >= 0.0);
  
  return a;
}

//----------------------------------------------------------------
// calc_orientation_histogram_donations
// 
unsigned int
calc_orientation_histogram_donations(const unsigned int & bins,
             const double & angle,
             unsigned int * donation_bin,
             double * donation)
{
  double direction = clamp_angle(angle);
  
  // calculate the bin weights:
  const double t = fmod(double(bins) * direction / TWO_PI, double(bins));
  
  donation_bin[1] = (unsigned int)(floor(t));
  donation_bin[2] = (donation_bin[1] + 1) % bins;
  donation_bin[0] = (donation_bin[1] + bins - 1) % bins;
  
  // a parameter within the bin in the range [0, 1):
  double s = t - double(donation_bin[1]);
  
  donation[0] = std::max(0.0, 0.5 - s);
  donation[2] = std::max(0.0, s - 0.5);
  donation[1] = 1.0 - (donation[0] + donation[2]);
  
  return donation_bin[1];
}

//----------------------------------------------------------------
// update_orientation_histogram
// 
void
update_orientation_histogram(double * orientation,
           const unsigned int & bins,
           const double & angle,
           const double & value)
{
  static unsigned int bin[3];
  static double w[3];
  calc_orientation_histogram_donations(bins, angle, bin, w);
  
  // update the histogram:
  orientation[bin[0]] += value * w[0];
  orientation[bin[1]] += value * w[1];
  orientation[bin[2]] += value * w[2];
}
