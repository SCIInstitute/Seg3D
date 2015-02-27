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

// File         : histogram.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 12:36
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for working with circular histograms.

#ifndef HISTOGRAM_HXX_
#define HISTOGRAM_HXX_

//----------------------------------------------------------------
// smoothout_orientation_histogram
// 
extern void
smoothout_orientation_histogram(double * orientation,
				const unsigned int & bins,
				const unsigned int iterations = 3);

//----------------------------------------------------------------
// isolate_orientation_histogram_peaks
// 
extern bool
isolate_orientation_histogram_peaks(double * histogram,
				    const unsigned int & bins,
				    const bool normalize = false);

//----------------------------------------------------------------
// calc_histogram_donations
// 
extern unsigned int
calc_histogram_donations(const unsigned int & bins,
			 const double & r0,
			 const double & r1,
			 const double & r,
			 unsigned int * donation_bin,
			 double * donation);

//----------------------------------------------------------------
// calc_orientation_histogram_donations
// 
extern unsigned int
calc_orientation_histogram_donations(const unsigned int & bins,
				     const double & angle,
				     unsigned int * donation_bin,
				     double * donation);

//----------------------------------------------------------------
// update_orientation_histogram
// 
extern void
update_orientation_histogram(double * orientation,
			     const unsigned int & bins,
			     const double & angle,
			     const double & value);


#endif // HISTOGRAM_HXX_
