/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#include <iostream>

#include "itkFastGrowCut.h"
#include "FastGrowCut.h"

#include <itkObjectFactory.h>
#include <itkSmartPointer.h>
#include <itkImage.h>
#include <itkTimeProbe.h>

namespace itk
{
template <typename TInputImage, typename TOutputImage>
FastGrowCut::~FastGrowCut()
{
  if ( this->m_fastGC != nullptr )
  {
    delete m_fastGC;
  }
}

template <typename TInputImage, typename TOutputImage>
void
FastGrowCut<TInputImage, TOutputImage>::GenerateData()
{
  InputImagePointer inputImage = this->GetInput();
  LabelImagePointer seedImage = this->GetSeedImage();
  LabelImagePointer outputImage = this->GetOutput();

  // Copy seedImage into the output
  OutputImageRegionType region = outputImage->GetRequestedRegion();
  outputImage->SetBufferedRegion(region);
  outputImage->Allocate();
  ImageAlgorithm::Copy(seedImage,outputImage,region,region);



  itk::TimeProbe timer;

  timer.Start();

  std::cerr << "InitializationFlag: " << InitializationFlag << std::endl;
  // Find ROI
  if ( !InitializationFlag )
  {
    FGC::FindITKImageROI<LabelPixelType>( outputImage, m_imROI );
    std::cerr << "image ROI = [" << m_imROI[0] << "," << m_imROI[1] << "," << m_imROI[2] << ";"  \
              << m_imROI[3] << "," << m_imROI[4] << "," << m_imROI[5] << "]" << std::endl;
    // SB: Find the ROI from the seed volume in the source volume and store it in m_imSrcVec
    FGC::ExtractITKImageROI<short>( inputImage, m_imROI, m_imSrcVec );
  }
  // SB: Store the ROI from the seed volume in m_imSeedVec
  FGC::ExtractITKImageROI<LabelPixelType>( outputImage, m_imROI, m_imSeedVec );

  // Initialize FastGrowCut
  std::vector<long> imSize( 3 );
  for ( int i = 0; i < 3; i++ )
  {
    imSize[i] = m_imROI[i + 3] - m_imROI[i];
  }
  m_fastGC->SetSourceImage( m_imSrcVec );
  m_fastGC->SetSeedlImage( m_imSeedVec );
  m_fastGC->SetImageSize( imSize );
  m_fastGC->SetWorkMode( InitializationFlag );

  // Do Segmentation
  m_fastGC->DoSegmentation();
  m_fastGC->GetForegroundmage( m_imLabVec );

  // Update result. SB: Seed volume is replaced with grow cut result
  FGC::UpdateITKImageROI<LabelPixelType>( m_imLabVec, m_imROI, outputImage );

  timer.Stop();

  if ( !InitializationFlag )
  {
    std::cout << "Initial fast GrowCut segmentation time: " << timer.GetMeanTime() << " seconds\n";
  }
  else
  {
    std::cout << "Adaptive fast GrowCut segmentation time: " << timer.GetMeanTime() << " seconds\n";
  }
}

template <typename TInputImage, typename TOutputImage>
void
FastGrowCut<TInputImage, TOutputImage>::EnlargeOutputRequestedRegion(DataObject * output)
{
  Superclass::EnlargeOutputRequestedRegion(output);
  output->SetRequestedRegionToLargestPossibleRegion();
}

template <typename TInputImage, typename TOutputImage>
void
FastGrowCut<TInputImage, TOutputImage>::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
  if (this->GetInput())
  {
    InputImagePointer input = const_cast<TInputImage *>(this->GetInput());
    input->SetRequestedRegionToLargestPossibleRegion();
  }
}

template <typename TInputImage, typename TOutputImage>
void
FastGrowCut<TInputImage, TOutputImage>::PrintSelf(std::ostream& os, Indent indent) const
{
  std::cout << "This function has been found." << std::endl; // TODO: add printing of ivars
}
