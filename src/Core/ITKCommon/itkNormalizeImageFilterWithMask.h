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

// File         : itkNormalizeImageFilterWithMask.h
// Author       : Pavel A. Koshevoy
// Created      : 2006/06/15 17:09
// Copyright    : (C) 2004-2008 University of Utah
// Description  : An enhanced version of the itk::NormalizeImageFilter
//                adding support for a mask image.

#ifndef __itkNormalizeImageFilterWithMask_h
#define __itkNormalizeImageFilterWithMask_h

// ITK includes:
#include <itkImageToImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkSpatialObject.h>
#include <itkEventObject.h>

// local includes:
#include "itkStatisticsImageFilterWithMask.h"

namespace itk {

/** \class NormalizeImageFilterWithMask 
 * \brief Normalize an image by setting its mean to zero and variance to one.
 *
 * NormalizeImageFilterWithMask shifts and scales an image so that the pixels
 * in the image have a zero mean and unit variance. This filter uses
 * StatisticsImageFilterWithMask to compute the mean and variance of the input
 * and then applies ShiftScaleImageFilter to shift and scale the pixels.
 *
 * NB: since this filter normalizes the data to lie within -1 to 1,
 * integral types will produce an image that DOES NOT HAVE a unit variance.
 * \ingroup MathematicalImageFilters
 */
template<class TInputImage,class TOutputImage>
class ITK_EXPORT NormalizeImageFilterWithMask : 
    public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard Self typedef */
  typedef NormalizeImageFilterWithMask Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>  Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Runtime information support. */
  itkTypeMacro(NormalizeImageFilterWithMask, ImageToImageFilter);
  
  /** Image related typedefs. */
  typedef typename TInputImage::Pointer InputImagePointer;
  typedef typename TOutputImage::Pointer OutputImagePointer;
  
  /**  Type for the mask of the fixed image. Only pixels that are "inside"
       this mask will be considered for the computation of the metric */
  typedef SpatialObject<TInputImage::ImageDimension> ImageMaskType;
  typedef typename ImageMaskType::Pointer ImageMaskPointer;
  
  /** Set/Get the image mask. */
  itkSetObjectMacro( ImageMask, ImageMaskType );
  itkGetConstObjectMacro( ImageMask, ImageMaskType );

protected:
  NormalizeImageFilterWithMask();

  /** GenerateData. */
  void  GenerateData ();

  // Override since the filter needs all the data for the algorithm
  void GenerateInputRequestedRegion();

private:
  NormalizeImageFilterWithMask(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typename StatisticsImageFilterWithMask<TInputImage>::Pointer m_StatisticsFilter;
  typename ShiftScaleImageFilter<TInputImage,TOutputImage>::Pointer m_ShiftScaleFilter;

  mutable ImageMaskPointer m_ImageMask;
} ; // end of class

} // end namespace itk
  
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNormalizeImageFilterWithMask.txx"
#endif

#endif
