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

#ifndef APPLICATION_FILTERS_ALGORITHMS_IslandRemovalFilter_H
#define APPLICATION_FILTERS_ALGORITHMS_IslandRemovalFilter_H

// itk includes
#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkCommand.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

namespace Seg3D
{

using itk::ImageToImageFilter;
using itk::SmartPointer;

template <class TInputImage, class TOutputImage = TInputImage>
class IslandRemovalFilter :
  public ImageToImageFilter < TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro( InputImageDimension, unsigned int,
                       TInputImage::ImageDimension );
  itkStaticConstMacro( OutputImageDimension, unsigned int,
                       TOutputImage::ImageDimension );

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef IslandRemovalFilter Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( IslandRemovalFilter, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  typedef typename InputImageType::PointType PointType;

  typedef itk::ImageRegionConstIteratorWithIndex<InputImageType> ConstIteratorType;

  typedef itk::Image< unsigned int, InputImageDimension> UINT_IMAGE_TYPE;
  typedef typename itk::ConnectedComponentImageFilter< InputImageType, UINT_IMAGE_TYPE > ConnectedComponentFilterType;

  typedef itk::ImageRegionConstIteratorWithIndex<UINT_IMAGE_TYPE> UintConstIteratorType;

  typedef itk::BinaryThresholdImageFilter<UINT_IMAGE_TYPE, UINT_IMAGE_TYPE> UintThresholdType;

  itkSetMacro( IslandSize, int );   // pixels
  itkGetConstReferenceMacro( IslandSize, int );   // pixels

  /** IslandRemovalFilter needs a larger input requested region than
   * the output requested region.  As such, IslandRemovalFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  //virtual void GenerateInputRequestedRegion() throw(itk::InvalidRequestedRegionError);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( SameDimensionCheck,
                   ( itk::Concept::SameDimension<InputImageDimension, OutputImageDimension>) );
  itkConceptMacro( InputConvertibleToOutputCheck,
                   ( itk::Concept::Convertible<InputPixelType, OutputPixelType>) );
  itkConceptMacro( InputLessThanComparableCheck,
                   ( itk::Concept::LessThanComparable<InputPixelType>) );
  /** End concept checking */
#endif // ifdef ITK_USE_CONCEPT_CHECKING

protected:
  IslandRemovalFilter();
  virtual ~IslandRemovalFilter() {}

  void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  void GenerateData();

private:
  IslandRemovalFilter( const Self& ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented

  int m_IslandSize;
};  // end class IslandRemovalFilter

// DEFINITIONS:

template <class TInputImage, class TOutputImage>
IslandRemovalFilter<TInputImage, TOutputImage>
::IslandRemovalFilter()
  : m_IslandSize( 1 )
{
  // constructor
}

template< class TInputImage, class TOutputImage>
void IslandRemovalFilter< TInputImage, TOutputImage>
::GenerateData()
{
  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
  output->SetBufferedRegion( output->GetLargestPossibleRegion() );
  output->Allocate();
  output->FillBuffer( itk::NumericTraits<OutputPixelType>::Zero );

  typename InputImageType::ConstPointer input = this->GetInput();
  // step 1 : run connected component filter

  typename ConnectedComponentFilterType::Pointer cc = ConnectedComponentFilterType::New();
  cc->SetInput( input );
  cc->Update();

  // step 2 : assign sizes to each pixel

  typename UINT_IMAGE_TYPE::Pointer ccImage = cc->GetOutput();

  UintConstIteratorType ccIt( ccImage, ccImage->GetLargestPossibleRegion() );
  unsigned int maxLabel = 0;
  for ( ccIt.GoToBegin(); !ccIt.IsAtEnd(); ++ccIt )
  {
    maxLabel = std::max( maxLabel, ccIt.Get() );
  }

  std::vector<unsigned int> hist;
  hist.resize( maxLabel + 1, 0 );

  for ( ccIt.GoToBegin(); !ccIt.IsAtEnd(); ++ccIt )
  {
    hist[ccIt.Get()]++;
  }
  hist[0] = 0;

  for ( ccIt.GoToBegin(); !ccIt.IsAtEnd(); ++ccIt )
  {
    ccImage->SetPixel( ccIt.GetIndex(), hist[ccIt.Get()] );
  }

  // step 3 : threshold by island size
  typename UintThresholdType::Pointer threshold = UintThresholdType::New();
  threshold->SetLowerThreshold( this->GetIslandSize() );
  threshold->SetInput( ccImage );
  threshold->SetOutsideValue( 0 );
  threshold->SetInsideValue( 1 );
  threshold->Update();

  // step 4 : copy result to output
  typename UINT_IMAGE_TYPE::Pointer result = threshold->GetOutput();
  itk::ImageRegionIterator< UINT_IMAGE_TYPE > resultIt( result, result->GetLargestPossibleRegion() );
  itk::ImageRegionIterator< OutputImageType > outputIt( output, output->GetLargestPossibleRegion() );
  for (; !outputIt.IsAtEnd() && !resultIt.IsAtEnd(); ++outputIt, ++resultIt )
  {
    outputIt.Set( resultIt.Get() );
  }
}

template <class TInputImage, class TOutputImage>
void IslandRemovalFilter<TInputImage, TOutputImage>
::PrintSelf( std::ostream &os, itk::Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "IslandSize: " << this->GetIslandSize() << std::endl;
}
} // end namespace Seg3D

#endif // ifndef APPLICATION_FILTERS_ALGORITHMS_IslandRemovalFilter_H
