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

// File         : itkImageMosaicVarianceMetric.h
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/22 17:19
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A metric class measuring mean pixel variance within
//                the mosaic tile overlap regions. The metric derivative

#ifndef __itkImageMosaicVarianceMetric_h
#define __itkImageMosaicVarianceMetric_h

#include <itkImageBase.h>
#include <itkTransform.h>
#include <itkInterpolateImageFunction.h>
#include <itkSingleValuedCostFunction.h>
#include <itkMacro.h>
#include <itkGradientRecursiveGaussianImageFilter.h>

/** .
    This class computes the mean pixel variance across several images
    within the overlapping regions of the mosaic. Each image is warped
    by a corresponding transform. All of the transforms must be of the
    same type (and therefore have the same number of parameters). Some
    of the transforms parameters may be shared across transforms. The
    shared/unique parameters are specified by a bit vector
    (true - shared, false - unique). This is usefull for radial
    distortion transforms where the translation parameters are unique
    for each image but the distortion parameters are the same across
    all images (all images are assumed to have been distorted by
    the same lens).
 */

namespace itk
{

/** \class ImageMosaicVarianceMetric
 * \brief Computes mean pixel variance within the overlapping regions
 * of a mosaic.
 *
 */
template <class TImage, class TInterpolator>
class ITK_EXPORT ImageMosaicVarianceMetric : public SingleValuedCostFunction
{
public:
  /** Standard class typedefs. */
  typedef ImageMosaicVarianceMetric Self;
  typedef SingleValuedCostFunction  Superclass;
  typedef SmartPointer<Self>    Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageMosaicVarianceMetric, SingleValuedCostFunction);

  /** Type of the moving Image. */
  typedef TInterpolator     interpolator_t;
  typedef TImage      image_t;
  typedef typename TImage::PixelType  pixel_t;

  /** Constant for the image dimension */
  itkStaticConstMacro(ImageDimension, unsigned int, TImage::ImageDimension);

  typedef itk::Image<unsigned char,
         itkGetStaticConstMacro(ImageDimension)> mask_t;

  /** Type of the transform base class */
  typedef Transform<Superclass::ParametersValueType,
                    itkGetStaticConstMacro(ImageDimension),
                    itkGetStaticConstMacro(ImageDimension)> transform_t;

  typedef typename transform_t::InputPointType  point_t;
  typedef typename transform_t::ParametersType  params_t;
  typedef typename transform_t::JacobianType  jacobian_t;

  /** Gaussian filter to compute the gradient of the mosaic images */
  // typedef typename NumericTraits<pixel_t>::RealType
  typedef float
  scalar_t;

  typedef CovariantVector<scalar_t, itkGetStaticConstMacro(ImageDimension)>
  gradient_pixel_t;

  typedef Image<gradient_pixel_t, itkGetStaticConstMacro(ImageDimension)>
  gradient_image_t;

  typedef GradientRecursiveGaussianImageFilter<image_t, gradient_image_t>
  gradient_filter_t;

  /** Type of the measure. */
  typedef Superclass::MeasureType measure_t;

  /** Type of the derivative. */
  typedef Superclass::DerivativeType derivative_t;

  /** Get the number of pixels considered in the computation. */
  itkGetConstReferenceMacro( NumberOfPixelsCounted, unsigned long );

  /** Set/Get the region over which the metric will be computed */
  itkSetMacro( MosaicRegion, typename image_t::RegionType );
  itkGetConstReferenceMacro( MosaicRegion, typename image_t::RegionType );

  /** Initialize transform parameter offsets.

      Setup the mapping from individual transform parameter indices to
      the corresponding indices in the  concatenated parameter vector.

      The radial distortion transforms typically share the same parameters
      across all images in the mosaic, therefore it is slow and unnecessary
      to duplicate the same parameters parameters in the concatenated
      parameters vector. Instead, the parameters are store only once.
      This also allows a significant speedup of the optimization. On the
      other hand, translation parameters are typically unique for each
      image.

      params_shared is a vector of boolean flags indicating which of
      the transform parameters are shared (true) or unique (false).

      NOTE: this function will fail if the transform_ has not been
      initialized.
  */
  void setup_param_map(const std::vector<bool> & params_shared,
           const std::vector<bool> & params_active);

  /** Concatenate parameters of each transform into one big vector.

      NOTE: this function will fail if SetupTransformParameterOffsets
      has not been called yet.
  */
  params_t GetTransformParameters() const;

  /** virtual: Set the parameters defining the transforms:

      NOTE: this function will fail if SetupTransformParameterOffsets
      has not been called yet.
  */
  void SetTransformParameters(const params_t & parameters) const;

  /** virtual: Return the number of parameters required by the transforms.

      NOTE: this function will fail if SetupTransformParameterOffsets
      has not been called yet.
  */
  unsigned int GetNumberOfParameters() const;

  /** Initialize the Metric by making sure that all the components
      are present and plugged together correctly.
  */
  virtual void Initialize() throw (ExceptionObject);

  /** virtual: Get the value for single valued optimizers. */
  measure_t GetValue(const params_t & parameters) const
  {
    measure_t measure;
    derivative_t derivative;
    GetValueAndDerivative(parameters, measure, derivative);
    return measure;
  }

  /** virtual: Get the derivatives of the match measure. */
  void GetDerivative(const params_t & parameters,
         derivative_t & derivative) const
  {
    measure_t measure;
    GetValueAndDerivative(parameters, measure, derivative);
  }

  /** virtual: Get value and derivatives for multiple valued optimizers. */
  void GetValueAndDerivative(const params_t & parameters,
                             measure_t & value,
           derivative_t & derivative) const;

  //----------------------------------------------------------------
  // image_data_t
  //
  // This datastructure is used to speed up access to relevant
  // information when evaluating GetValueAndDerivative:
  //
  class image_data_t
  {
  public:
    image_data_t():
      id_(~0),
      P_(measure_t(0)),
      dPdx_(measure_t(0)),
      dPdy_(measure_t(0))
    {}

    // image id:
    unsigned int id_;

    // image value:
    measure_t P_;

    // partial derivative with respect to x:
    measure_t dPdx_;

    // partial derivative with respect to y:
    measure_t dPdy_;

    // a pointer to the Jacobian of the transform:
    jacobian_t J_;
  };

  // calculate the bounding box for a given set of image bounding boxes:
  void CalcMosaicBBox(point_t & mosaic_min,
          point_t & mosaic_max,
          std::vector<point_t> & image_min,
          std::vector<point_t> & image_max) const;

  // calculate the mosaic variance image as well as maximum and mean variance:
  typename image_t::Pointer variance(measure_t & max_var,
             measure_t & avg_var) const;

  typename image_t::Pointer variance(const typename TImage::SpacingType & sp,
             const pnt2d_t & mosaic_min,
             const pnt2d_t & mosaic_max,
             measure_t & max_var,
             measure_t & avg_var) const;

  typename image_t::Pointer variance(const typename TImage::SpacingType & sp,
             const pnt2d_t & mosaic_min,
             const typename TImage::SizeType & sz,
             measure_t & max_var,
             measure_t & avg_var) const;

  typename image_t::Pointer variance() const
  {
    measure_t max_var;
    measure_t avg_var;
    return variance(max_var, avg_var);
  }

  // mosaic images:
  std::vector<typename image_t::ConstPointer> image_;
  std::vector<typename mask_t::ConstPointer>  mask_;

  // image transforms (cascaded):
  mutable std::vector<typename transform_t::Pointer> transform_;

  // image interpolators:
  std::vector<typename interpolator_t::Pointer> interpolator_;

  // image gradients:
  std::vector<typename gradient_image_t::ConstPointer> gradient_;

  // adresses of the individual transform parameter indices within the
  // concatenated parameter vector:
  std::vector<std::vector<unsigned int> > address_;

  // number of shared/unique parameters per transform:
  unsigned int n_shared_;
  unsigned int n_unique_;

  // this mask defines which of the individual transform parameters are
  // active and will be included into the metric parameter vector:
  std::vector<bool> param_active_;

protected:
  ImageMosaicVarianceMetric():
    n_shared_(0),
    n_unique_(0),
    param_active_(0),
    m_NumberOfPixelsCounted(0)
  {}

  virtual ~ImageMosaicVarianceMetric()
  {}

  // standard ITK debugging helper:
  void PrintSelf(std::ostream & os, Indent indent) const;

  // number of pixels in the overlapping regions of the mosaic:
  mutable unsigned long m_NumberOfPixelsCounted;

private:
  // unimplemented on purpose:
  ImageMosaicVarianceMetric(const Self &);
  void operator = (const Self &);

  typename image_t::RegionType m_MosaicRegion;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <Core/ITKCommon/Optimizers/itkImageMosaicVarianceMetric.txx>
#endif // ITK_MANUAL_INSTANTIATION

#endif //  __itkImageMosaicVarianceMetric_h
