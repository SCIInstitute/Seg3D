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

#ifndef itkFastGrowCut_h
#define itkFastGrowCut_h

#include <Application/Tools/Algorithm/FastGrowCut.h>

#include <itkObject.h>
#include <itkMacro.h>
#include <itkImageToImageFilter.h>

namespace itk
{
template <typename TInputImage, typename TLabelImage>
class ITK_TEMPLATE_EXPORT FastGrowCut : public ImageToImageFilter<TInputImage, TLabelImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(FastGrowCut);

  /** Standard class type aliases. */
  using Self = FastGrowCut;
  using Superclass = ImageToImageFilter<TInputImage, TLabelImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(FastGrowCut, ImageToImageFilter);

  using InputImageType = TInputImage;
  using InputImagePointer = typename InputImageType::Pointer;
  using InputImageRegionType = typename InputImageType::RegionType;
  using InputImagePixelType = typename InputImageType::PixelType;
  using IndexType = typename InputImageType::IndexType;
  using SizeType = typename InputImageType::SizeType;

  using LabelImageType = TLabelImage;
  using LabelPixelType = typename LabelImageType::PixelType;
  using LabelImagePointer = typename LabelImageType::Pointer;
  using LabelImageRegionType = typename LabelImageType::RegionType;
  using LabelImagePixelType = typename LabelImageType::PixelType;

  using SeedsContainerType = std::vector<IndexType>;

  using InputRealType = typename NumericTraits<InputImagePixelType>::RealType;

  itkSetMacro(InitializationFlag, bool);

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  void
  SetSeedImage(const LabelImageType* seedImage)
  {
    // Process object is not const-correct so the const casting is required.
    this->SetNthInput(1, const_cast<LabelImageType*>(seedImage));
  }
  const LabelImageType*
  GetSeedImage()
  {
    return static_cast<const LabelImageType*>(this->ProcessObject::GetInput(1));
  }


#ifdef ITK_USE_CONCEPT_CHECKING
  // Begin concept checking
  static_assert(TInputImage::ImageDimension == 3, "FastGrowCut only works with 3D images");
  static_assert(TLabelImage::ImageDimension == 3, "FastGrowCut only works with 3D images");
  itkConceptMacro(InputHasNumericTraitsCheck, (Concept::HasNumericTraits<InputImagePixelType>));
  itkConceptMacro(OutputHasNumericTraitsCheck, (Concept::HasNumericTraits<LabelImagePixelType>));
  // End concept checking
#endif

protected:
  FastGrowCut() = default;
  ~FastGrowCut() override;

  // Override since the filter needs all the data for the algorithm
  void
  GenerateInputRequestedRegion() override;

  // Override since the filter produces the entire dataset
  void
  EnlargeOutputRequestedRegion(DataObject * output) override;

  void
  GenerateData() override;

private:
  std::vector<LabelPixelType> m_imSeedVec;
  std::vector<LabelPixelType> m_imLabVec;
  std::vector<short> m_imSrcVec;
  std::vector<long> m_imROI;

  //logic code
  //Make smart pointer
  FGC::FastGrowCut<short, LabelPixelType>* m_fastGC = new FGC::FastGrowCut<short, typename LabelImageType::PixelType>();

  //state variables
  bool m_InitializationFlag = false;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkFastGrowCut.hxx"
#endif

#endif // ifndef itkFastGrowCut_h
