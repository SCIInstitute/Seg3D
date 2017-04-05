/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLiveWireImageFunction.h,v $
  Language:  C++
  Date:      $Date: $
  Version:   $Revision: $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkLiveWireImageFunction_h
#define __itkLiveWireImageFunction_h

#include "itkImageFunction.h"

#include "itkGradientImageFilter.h"
#include "itkPolyLineParametricPath.h"
#include "itkPriorityQueueContainer.h"

#include <vector>
#include <queue>

namespace itk
{
  
/** \class LiveWireImageFunction
 * \brief Implements livewire image segmentation of Barret and Mortensen.
 *
 * LiveWireImageFunction implements the livewire segmentation 
 * algorithm of Barrett and Mortenson.  This class naturally derives 
 * from the ImageFunction class where an N-D dimensional image
 * is taken as input and the output consists of a path in that image.
 *
 * \reference
 * W. A. Barrett and E. N. Mortenson, "Interactive live-wire boundary 
 * extraction", Medical Image Analysis, 1(4):331-341, 1996/7.
 * \ingroup ImageFunctions
 */
template <typename TInputImage>
class ITK_EXPORT LiveWireImageFunction 
: public ImageFunction<TInputImage, typename itk::PolyLineParametricPath< TInputImage::ImageDimension >::Pointer >
{

public:
  /** ImageDimension constants */
  itkStaticConstMacro( ImageDimension, unsigned int,
                       TInputImage::ImageDimension );

  /** Standard class typedefs. */
  typedef LiveWireImageFunction                            Self;

  typedef PolyLineParametricPath<
    itkGetStaticConstMacro( ImageDimension )>              OutputType;
  typedef ImageFunction<TInputImage, 
    typename OutputType::Pointer>                          Superclass;
  typedef SmartPointer<Self>                               Pointer;
  typedef SmartPointer<const Self>                         ConstPointer;
  
  /** Run-time type information (and related methods). */
  itkTypeMacro( LiveWireImageFunction, ImageFunction );

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Some convenient typedefs. */
  typedef TInputImage                                       InputImageType;
  typedef typename InputImageType::Pointer                  InputImagePointer;
  typedef typename InputImageType::ConstPointer             InputImageConstPointer;
  typedef typename InputImageType::RegionType               InputImageRegionType; 
  typedef typename InputImageType::PixelType                InputImagePixelType; 

  typedef typename Superclass::IndexType                    IndexType;
  typedef typename Superclass::PointType                    PointType;
  typedef typename Superclass::ContinuousIndexType          ContinuousIndexType;
 
  typedef itk::IdentifierType                               ElementIdentifier;

  typedef typename OutputType::VertexType                   VertexType;

  typedef float                                             RealType;
  typedef Image<RealType, 
    itkGetStaticConstMacro( ImageDimension )>               RealImageType;
  typedef GradientImageFilter<InputImageType, RealType,
    RealType>                                               GradientFilterType;
  typedef typename GradientFilterType::OutputImageType      GradientImageType;
  typedef Image<typename InputImageType::OffsetType, 
    itkGetStaticConstMacro( ImageDimension )>               OffsetImageType;
  typedef Image<int, 
    itkGetStaticConstMacro( ImageDimension )>               MaskImageType;
  typedef typename MaskImageType::PixelType                 MaskPixelType;


  /**
   * Priority queue typedefs
   */
  typedef MinPriorityQueueElementWrapper
    <IndexType, RealType, ElementIdentifier>         PriorityQueueElementType;
  typedef PriorityQueueContainer<
    PriorityQueueElementType, 
    PriorityQueueElementType,
    RealType, long>                                  PriorityQueueType;
    

  /** Set the input image.
   * \warning this method caches BufferedRegion information.
   * If the BufferedRegion has changed, user must call
   * SetInputImage again to update cached values. */
  virtual void SetInputImage( const InputImageType * ptr ) override;

  /** Evaluate the function at specified Point position. */
  virtual typename OutputType::Pointer Evaluate( const PointType &point ) const override
				{
						IndexType index;
      this->ConvertPointToNearestIndex( point, index );
						return this->EvaluateAtIndex( index );   
				}

  /** Evaluate the function at specified ContinousIndex position.
   * Subclasses must provide this method. */
  virtual typename OutputType::Pointer 
    EvaluateAtContinuousIndex( const ContinuousIndexType &cindex ) const override
				{
						IndexType index;
      this->ConvertContinuousIndexToNearestIndex( cindex, index );
						return this->EvaluateAtIndex( index );   
				}

  /** Evaluate the function at specified Index position.
   * Subclasses must provide this method. */
  virtual typename OutputType::Pointer 
    EvaluateAtIndex( const IndexType &index ) const override;

  itkSetClampMacro( GradientMagnitudeWeight, RealType, 
                    0, NumericTraits<RealType>::max() );
  itkGetConstMacro( GradientMagnitudeWeight, RealType );

  itkSetClampMacro( GradientDirectionWeight, RealType, 
                    0, NumericTraits<RealType>::max() );
  itkGetConstMacro( GradientDirectionWeight, RealType );
  
  itkSetClampMacro( ZeroCrossingWeight, RealType, 0, 
                    NumericTraits<RealType>::max() );
  itkGetConstMacro( ZeroCrossingWeight, RealType );

  itkSetMacro( ZeroCrossingImage, typename RealImageType::Pointer );
  itkGetConstMacro( ZeroCrossingImage, typename RealImageType::Pointer );

  itkSetMacro( MaskImage, typename MaskImageType::Pointer );
  itkGetConstMacro( MaskImage, typename MaskImageType::Pointer );

  itkSetMacro( InsidePixelValue, MaskPixelType );
  itkGetConstMacro( InsidePixelValue, MaskPixelType );

  virtual void SetAnchorSeed( IndexType index )
    {
    itkDebugMacro( "setting AnchorSeed to " << index );
    if ( this->m_AnchorSeed != index ) 
      {
      this->m_AnchorSeed = index;
      if ( this->GetInputImage() != nullptr )
        {
        this->GeneratePathDirectionImage();
        }
      this->Modified(); 
      } 
    } 
  itkGetConstMacro( AnchorSeed, IndexType );

  itkSetMacro( UseFaceConnectedness, bool );
  itkGetConstMacro( UseFaceConnectedness, bool );
  itkBooleanMacro( UseFaceConnectedness );

  itkSetMacro( UseImageSpacing, bool );
  itkGetConstMacro( UseImageSpacing, bool );
  itkBooleanMacro( UseImageSpacing );

protected:

  LiveWireImageFunction(); 
  virtual ~LiveWireImageFunction();
  void PrintSelf(std::ostream& os, Indent indent) const override;
  
private:
  LiveWireImageFunction(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  void GeneratePathDirectionImage();

  RealType                                   m_GradientMagnitudeWeight;
  RealType                                   m_ZeroCrossingWeight;
  RealType                                   m_GradientDirectionWeight;

  typename GradientImageType::Pointer        m_GradientImage;
  typename RealImageType::Pointer            m_GradientMagnitudeImage;
  typename RealImageType::Pointer            m_RescaledGradientMagnitudeImage;  
  typename RealImageType::Pointer            m_ZeroCrossingImage;
  
  typename OffsetImageType::Pointer          m_PathDirectionImage;

  typename MaskImageType::Pointer            m_MaskImage;
  MaskPixelType                              m_InsidePixelValue;

  IndexType                                  m_AnchorSeed;

  bool                                       m_UseFaceConnectedness;
  bool                                       m_UseImageSpacing;
};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLiveWireImageFunction.hxx"
#endif

#endif

