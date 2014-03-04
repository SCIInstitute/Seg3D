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


// File         : itkNormalizeImageFilterWithMask.txx
// Author       : Pavel A. Koshevoy
// Created      : 2006/06/15 17:09
// Copyright    : (C) 2004-2008 University of Utah
// Description  : An enhanced version of the itk::NormalizeImageFilter
//                adding support for a mask image.

#ifndef _itkNormalizeImageFilterWithMask_txx
#define _itkNormalizeImageFilterWithMask_txx

// local includes:
#include "itkNormalizeImageFilterWithMask.h"
#include "itkStatisticsImageFilterWithMask.h"

// ITK includes:
#include <itkImageRegionIterator.h>
#include <itkShiftScaleImageFilter.h>
#include <itkProgressAccumulator.h>
#include <itkCommand.h>

namespace itk
{

template <class TInputImage, class TOutputImage>
NormalizeImageFilterWithMask<TInputImage, TOutputImage>
::NormalizeImageFilterWithMask()
{
  m_StatisticsFilter = 0;
  m_StatisticsFilter = StatisticsImageFilterWithMask<TInputImage>::New();
  m_ShiftScaleFilter = ShiftScaleImageFilter<TInputImage,TOutputImage>::New();
}

template <class TInputImage, class TOutputImage>
void
NormalizeImageFilterWithMask<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
  if ( this->GetInput() )
    {
    InputImagePointer image =
      const_cast< typename Superclass::InputImageType * >( this->GetInput() );
    image->SetRequestedRegionToLargestPossibleRegion();
    }
}

template <class TInputImage, class TOutputImage>
void 
NormalizeImageFilterWithMask<TInputImage, TOutputImage>
::GenerateData()
{
  ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
  progress->SetMiniPipelineFilter(this);

  progress->RegisterInternalFilter(m_StatisticsFilter,.5f);
  progress->RegisterInternalFilter(m_ShiftScaleFilter,.5f);

  // Gather statistics
  
  m_StatisticsFilter->SetInput(this->GetInput());
  m_StatisticsFilter->GetOutput()->SetRequestedRegion(this->GetOutput()->GetRequestedRegion());
  m_StatisticsFilter->SetImageMask(this->m_ImageMask);
  m_StatisticsFilter->Update();

  // Set the parameters for Shift
  m_ShiftScaleFilter->SetShift(-m_StatisticsFilter->GetMean());
  m_ShiftScaleFilter->SetScale(NumericTraits<ITK_TYPENAME StatisticsImageFilterWithMask<TInputImage>::RealType>::One
                               / m_StatisticsFilter->GetSigma());
  m_ShiftScaleFilter->SetInput(this->GetInput());
  
  m_ShiftScaleFilter->GetOutput()->SetRequestedRegion(this->GetOutput()->GetRequestedRegion());
  m_ShiftScaleFilter->Update();

  // Graft the mini pipeline output to this filters output
  this->GraftOutput(m_ShiftScaleFilter->GetOutput());
}

} // end namespace itk

#endif
