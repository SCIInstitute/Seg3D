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

// File         : itkImageMosaicVarianceMetric.txx
// Author       : Pavel A. Koshevoy
// Created      : 2005/06/22 17:19
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A metric class measuring mean pixel variance within
//                the mosaic tile overlap regions. The metric derivative

#ifndef _itkImageMosaicVarianceMetric_txx
#define _itkImageMosaicVarianceMetric_txx

// local includes:
#include <Core/ITKCommon/Optimizers/itkImageMosaicVarianceMetric.h>

#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/ThreadUtils/the_terminator.hxx>


// system includes:
#include <list>

// ITK includes:
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>


namespace itk
{

//----------------------------------------------------------------
// setup_param_map
//
template <class TImage, class TInterpolator>
void
ImageMosaicVarianceMetric<TImage, TInterpolator>::
setup_param_map(const std::vector<bool> & param_shared,
    const std::vector<bool> & param_active)
{
  const unsigned int num_transforms = transform_.size();
  assert(num_transforms > 0);

  const unsigned int n_params = param_shared.size();
  assert(n_params > 0 &&
   n_params == param_active.size() &&
   n_params == transform_[0]->GetNumberOfParameters());

  // save the active parameters mask:
  param_active_ = param_active;

  // count the number of shared/unique params:
  n_shared_ = 0;
  n_unique_ = 0;
  for (unsigned int i = 0; i < n_params; i++)
  {
    n_shared_ += param_active[i] &&  param_shared[i];
    n_unique_ += param_active[i] && !param_shared[i];
  }

  // build a mapping into the concatenated parameters vector,
  // shared parameters will be stored at the head of the vector
  // (to help with debugging) followed by the unique parameters:
  resize(address_, num_transforms, n_params);

  unsigned int u_off = n_shared_;
  for (unsigned int i = 0; i < num_transforms; i++)
  {
    unsigned int s_off = 0;
    for (unsigned int j = 0; j < n_params; j++)
    {
      if (!param_active[j]) continue;

      if (param_shared[j])
      {
  address_[i][j] = s_off;
  s_off++;
      }
      else
      {
  address_[i][j] = u_off;
  u_off++;
      }

      // FIXME:
      // cout << "address[" << i << "][" << j << "] = " << address_[i][j]
      //      << endl;
    }
  }
}

//----------------------------------------------------------------
// GetTransformParameters
//
template <class TImage, class TInterpolator>
typename ImageMosaicVarianceMetric<TImage, TInterpolator>::params_t
ImageMosaicVarianceMetric<TImage, TInterpolator>::
GetTransformParameters() const
{
  const unsigned int num_transforms = transform_.size();
  assert(num_transforms > 0);

  const unsigned int n_params = param_active_.size();
  assert(n_params > 0);

  params_t parameters(GetNumberOfParameters());
  for (unsigned int i = 0; i < num_transforms; i++)
  {
    params_t params = transform_[i]->GetParameters();
    for (unsigned int k = 0; k < n_params; k++)
    {
      if (!param_active_[k]) continue;
      parameters[address_[i][k]] = params[k];
    }
  }

  return parameters;
}

//----------------------------------------------------------------
// SetTransformParameters
//
template <class TImage, class TInterpolator>
void
ImageMosaicVarianceMetric<TImage, TInterpolator>::
SetTransformParameters(const params_t & parameters) const
{
  const unsigned int num_transforms = transform_.size();
  assert(num_transforms > 0);

  const unsigned int n_params = param_active_.size();
  assert(n_params > 0);

  // extract individual transform parameter values:
  for (unsigned int i = 0; i < num_transforms; i++)
  {
    params_t params = transform_[i]->GetParameters();
    for (unsigned int k = 0; k < n_params; k++)
    {
      if (!param_active_[k]) continue;
      params[k] = parameters[address_[i][k]];
    }
    transform_[i]->SetParameters(params);
  }
}

//----------------------------------------------------------------
// GetNumberOfParameters
//
template <class TImage, class TInterpolator>
unsigned int
ImageMosaicVarianceMetric<TImage, TInterpolator>::
GetNumberOfParameters() const
{
  const unsigned int num_transforms = transform_.size();
  return n_shared_ + n_unique_ * num_transforms;
}

//----------------------------------------------------------------
// Initialize
//
template <class TImage, class TInterpolator>
void
ImageMosaicVarianceMetric<TImage, TInterpolator>::
Initialize() throw (ExceptionObject)
{
#if 0
  cout << "sizeof(pixel_t) = " << sizeof(pixel_t) << endl
       << "sizeof(scalar_t) = " << sizeof(scalar_t) << endl
       << "sizeof(measure_t) = " << sizeof(measure_t) << endl
       << "sizeof(gradient_pixel_t) = " << sizeof(gradient_pixel_t) << endl
       << endl;
#endif

  const unsigned int num_transforms = address_.size();

  if (num_transforms == 0)
  {
    itkExceptionMacro(<< "call setup_param_map first");
  }

  for (unsigned int i = 0; i < num_transforms; i++)
  {
    if (!transform_[i])
    {
      itkExceptionMacro(<< "One of the transforms is missing");
    }

    if (!image_[i])
    {
      itkExceptionMacro(<< "One of the images is missing");
    }
    else if (image_[i]->GetSource())
    {
      // if the image is provided by a source, update the source:
      image_[i]->GetSource()->Update();
    }
  }

  // setup the interpolators, calculate the gradient images:
  interpolator_.resize(num_transforms);
  gradient_.resize(num_transforms);
  for (unsigned int i = 0; i < num_transforms; i++)
  {
    interpolator_[i] = interpolator_t::New();
    interpolator_[i]->SetInputImage(image_[i]);

    // calculate the image gradient:
    typename gradient_filter_t::Pointer gradient_filter =
      gradient_filter_t::New();
    gradient_filter->SetInput(image_[i]);

    const typename image_t::SpacingType & spacing = image_[i]->GetSpacing();
    double maximum_spacing = 0.0;
    for (unsigned int j = 0; j < ImageDimension; j++)
    {
      maximum_spacing = std::max(maximum_spacing, spacing[j]);
    }

    gradient_filter->SetSigma(maximum_spacing);
    gradient_filter->SetNormalizeAcrossScale(true);

    try
    {
      gradient_filter->Update();
    }
    catch (itk::ExceptionObject & exception)
    {
      // oops:
      std::cerr << "gradient filter threw an exception:" << std::endl
     << exception << std::endl;
      throw exception;
    }

    gradient_[i] = gradient_filter->GetOutput();
  }

  // If there are any observers on the metric, call them to give the
  // user code a chance to set parameters on the metric:
  this->InvokeEvent(InitializeEvent());
}

//----------------------------------------------------------------
// GetValueAndDerivative
//
// FIXME: the following code assumes a 2D mosaic:
//
template <class TImage, class TInterpolator>
void
ImageMosaicVarianceMetric<TImage, TInterpolator>::
GetValueAndDerivative(const params_t & parameters,
          measure_t & measure,
          derivative_t & derivative) const
{
  WRAP(itk_terminator_t
       terminator("ImageMosaicVarianceMetric::GetValueAndDerivative"));

  typedef typename image_t::IndexType index_t;
  typedef typename image_t::SpacingType spacing_t;
  typedef typename image_t::RegionType::SizeType imagesz_t;
  typedef typename image_t::RegionType region_t;

  itkDebugMacro("GetValueAndDerivative( " << parameters << " ) ");

  // shortcuts:
  const unsigned int num_images = interpolator_.size();
  if (num_images == 0)
  {
    itkExceptionMacro(<< "You forgot to call Initialize()");
  }

  // number of parameters per transform:
  const unsigned int n_params = param_active_.size();
  if (n_params == 0)
  {
    itkExceptionMacro(<< "You forgot to call setup_param_map()");
  }

  // the derivative vector size:
  const unsigned int n_concat = GetNumberOfParameters();

  // compare the previous parameters to the next set of parameters:
#if 0
  {
    params_t prev = GetTransformParameters();
    params_t diff = parameters;
    cout << "0 diff: ";
    for (unsigned int i = 0; i < n_concat; i++)
    {
      diff[i] -= prev[i];
      cout << setw(8 + 3) << diff[i];
      if (i == (n_concat - 1) / 2) cout << endl << "1 diff: ";
      else if (i == (n_concat - 1)) cout << endl;
    }
    cout << endl;
  }
#endif

#if 0
  {
    cout << "new parameters:\n" << parameters << endl;
    for (unsigned int i = 0; i < transform_.size(); i++)
    {
      cout << i << ". " << transform_[i] << endl;
    }
  }
#endif

  // update the transforms:
  SetTransformParameters(parameters);

  // calculate image bounding boxes in the mosaic space, as well as the
  // mosaic bounding box:
  pnt2d_t mosaic_min = pnt2d(std::numeric_limits<double>::max(),
         std::numeric_limits<double>::max());
  pnt2d_t mosaic_max = pnt2d(-mosaic_min[0], -mosaic_min[1]);
  std::vector<pnt2d_t> min(num_images);
  std::vector<pnt2d_t> max(num_images);
  CalcMosaicBBox(mosaic_min, mosaic_max, min, max);

  // mosaic will have the same spacing as the first image in the mosaic:
  spacing_t spacing = image_[0]->GetSpacing();
  imagesz_t mosaic_sz;
  {
    double nx = (mosaic_max[0] - mosaic_min[0]) / spacing[0];
    double ny = (mosaic_max[1] - mosaic_min[1]) / spacing[1];

    mosaic_sz[0] = (unsigned int)(nx + 0.5);
    mosaic_sz[1] = (unsigned int)(ny + 0.5);
  }

  // update the region of interest if necessary:
  region_t ROI = m_MosaicRegion;
  if (ROI.GetNumberOfPixels() == 0)
  {
    // use largest possible region:
    index_t index;
    index[0] = 0;
    index[1] = 0;
    ROI.SetIndex(index);
    ROI.SetSize(mosaic_sz);
  }

  // setup the mosaic image, but don't allocate any buffers for it:
  typename image_t::Pointer mosaic = image_t::New();
  mosaic->SetRegions(mosaic_sz);
  mosaic->SetSpacing(spacing);
  mosaic->SetOrigin(pnt2d(mosaic_min[0], mosaic_min[1]));

  // reset the accumulators:
  derivative = derivative_t(n_concat);
  derivative.Fill(NumericTraits<typename derivative_t::ValueType>::Zero);
  measure = NumericTraits<measure_t>::Zero;
  m_NumberOfPixelsCounted = 0;

  // pre-allocate and initialize image data for each image:
  std::vector<image_data_t> image_data(num_images);
  for (unsigned int i = 0; i < num_images; i++)
  {
    image_data[i].id_ = i;
  }

  // preallocate derivatives of the mean and variance:
  std::vector<measure_t> dMu(n_concat);
  std::vector<measure_t> dV(n_concat);

  // iterate over the mosaic, evaluate the metric in the overlapping regions:
  typedef itk::ImageRegionConstIteratorWithIndex<image_t> itex_t;
  // bool FIXME_FIRST_RUN = true;
  itex_t itex(mosaic, ROI);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());

    pnt2d_t point;
    mosaic->TransformIndexToPhysicalPoint(itex.GetIndex(), point);

    // find pixels in overlapping regions of the mosaic:
    std::list<const image_data_t *> overlap;
    for (unsigned int k = 0; k < num_images; k++)
    {
      if (point[0] < min[k][0] || point[0] > max[k][0] ||
    point[1] < min[k][1] || point[1] > max[k][1]) continue;

      typename transform_t::Pointer & t = transform_[k];
      pnt2d_t pt_k = t->TransformPoint(point);
      index_t index;
      // make sure the pixel maps into the image:
      image_[k]->TransformPhysicalPointToIndex(pt_k, index);
      if (!interpolator_[k]->IsInsideBuffer(pt_k)) continue;

      // make sure the pixel maps into the mask:
      if (mask_[k].GetPointer() && mask_[k]->GetPixel(index) == 0) continue;

      // shortcut:
      image_data_t & data = image_data[k];

      // get the image value:
      // data.P_ = image_[k]->GetPixel(index); // faster
      data.P_ = interpolator_[k]->Evaluate(pt_k); // slower

      // get the image gradient:
      const gradient_pixel_t & gradient = gradient_[k]->GetPixel(index);
      data.dPdx_ = gradient[0];
      data.dPdy_ = gradient[1];

      // get the transform Jacobian:
      t->ComputeJacobianWithRespectToPosition(point, data.J_);

      // add to the list:
      overlap.push_back(&data);
    }

    // skip over the regions that do not overlap:
    if (overlap.size() < 2) continue;

    // found a pixel in an overlapping region, increment the counter:
    m_NumberOfPixelsCounted++;

    // shortcut:
    measure_t normalization_factor = measure_t(1) / measure_t(overlap.size());

    // calculate the mean and derivative of the mean:
    measure_t Mu = measure_t(0);
    dMu.assign(n_concat, measure_t(0));
    for (typename std::list<const image_data_t *>::const_iterator
     i = overlap.begin(); i != overlap.end(); ++i)
    {
      const image_data_t & data = *(*i);
      const unsigned int * addr = &(address_[data.id_][0]);
      const jacobian_t & J = data.J_;

      Mu += data.P_;

      for (unsigned int j = 0; j < n_params; j++)
      {
  dMu[addr[j]] +=
    (J[0][j] * data.dPdx_ + J[1][j] * data.dPdy_)
    * normalization_factor;
      }
    }
    Mu *= normalization_factor;
#if 1
    // normalize the shared portion of dMu:
    for (unsigned int i = 0; i < n_shared_; i++)
    {
      dMu[i] *= normalization_factor;
    }
#endif

    // calculate the variance:
    measure_t V = measure_t(0);
    dV.assign(n_concat, measure_t(0));
    for (typename std::list<const image_data_t *>::const_iterator
     i = overlap.begin(); i != overlap.end(); ++i)
    {
      const image_data_t & data = *(*i);
      const unsigned int * addr = &(address_[data.id_][0]);
      const jacobian_t & J = data.J_;

      measure_t d = data.P_ - Mu;
      V += d * d;

      for (unsigned int j = 0; j < n_params; j++)
      {
  dV[addr[j]] +=
    measure_t(2) * d *
    (J[0][j] * data.dPdx_ + J[1][j] * data.dPdy_ - dMu[addr[j]])
    * normalization_factor;
      }
    }
    V *= normalization_factor;
#if 1
    // normalize the shared portion of dV:
    for (unsigned int i = 0; i < n_shared_; i++)
    {
      dV[i] *= normalization_factor;
    }
#endif

#if 0
    if (FIXME_FIRST_RUN && overlap.size() > 2)
    {
      FIXME_FIRST_RUN = false;
      for (typename std::list<const image_data_t *>::const_iterator
       i = overlap.begin(); i != overlap.end(); ++i)
      {
  const image_data_t & data = *(*i);
  const unsigned int * addr = &(address_[data.id_][0]);
  const jacobian_t & J = data.J_;

  for (unsigned int k = 0; k < 2; k++)
  {
    cout << "J[" << k << "] =";
    for (unsigned int j = 0; j < n_params; j++)
    {
      cout << ' ' << J[k][j];
    }
    cout << endl;
  }
  cout << endl;
      }
    }
#endif
    // update the measure:
    measure += V;

    // update the derivative:
    for (unsigned int i = 0; i < n_concat; i++)
    {
      derivative[i] += dV[i];
    }
  }

  if (m_NumberOfPixelsCounted == 0)
  {
    itkExceptionMacro(<< "mosaic contains no overlapping images");
    return;
  }

  measure_t normalization_factor =
    measure_t(1) / measure_t(m_NumberOfPixelsCounted);
  measure *= normalization_factor;

  for (unsigned int i = 0; i < n_concat; i++)
  {
    derivative[i] *= normalization_factor;
  }

#if 0
  cout << "stdV: " << measure << endl;
  cout << "0 dV: ";
  for (unsigned int i = 0; i < n_concat; i++)
  {
    cout << setw(7 + 3) << derivative[i];
    if (i == (n_concat - 1) / 2) cout << endl << "1 dV: ";
    else if (i == (n_concat - 1)) cout << endl;
    else cout << ' ';
  }
  cout << endl;
#endif
}

//----------------------------------------------------------------
// CalcMosaicBBox
//
template <class TImage, class TInterpolator>
void
ImageMosaicVarianceMetric<TImage, TInterpolator>::
CalcMosaicBBox(point_t & mosaic_min,
         point_t & mosaic_max,
         std::vector<point_t> & min,
         std::vector<point_t> & max) const
{
  // image space bounding boxes:
  std::vector<point_t> image_min;
  std::vector<point_t> image_max;
  calc_image_bboxes<typename image_t::ConstPointer>(image_,
                image_min,
                image_max);

  // mosaic space bounding boxes:
  calc_mosaic_bboxes<point_t, typename transform_t::Pointer>(transform_,
                   image_min,
                   image_max,
                   min,
                   max);

  // mosiac bounding box:
  calc_mosaic_bbox<point_t>(min, max, mosaic_min, mosaic_max);
}

//----------------------------------------------------------------
// variance
//
template <class TImage, class TInterpolator>
typename TImage::Pointer
ImageMosaicVarianceMetric<TImage, TInterpolator>::
variance(measure_t & max_var,
   measure_t & avg_var) const
{
  max_var = measure_t(0);
  avg_var = measure_t(0);

  // shortcut:
  const unsigned int num_images = interpolator_.size();
  if (num_images == 0)
  {
    itkExceptionMacro(<< "You forgot to call Initialize()");
  }

  pnt2d_t mosaic_min = pnt2d(std::numeric_limits<double>::max(),
           std::numeric_limits<double>::max());
  pnt2d_t mosaic_max = pnt2d(-mosaic_min[0], -mosaic_min[1]);
  std::vector<pnt2d_t> min(num_images);
  std::vector<pnt2d_t> max(num_images);
  CalcMosaicBBox(mosaic_min, mosaic_max, min, max);

  return variance(image_[0]->GetSpacing(),
      mosaic_min,
      mosaic_max,
      max_var,
      avg_var);
}

//----------------------------------------------------------------
// variance
//
template <class TImage, class TInterpolator>
typename TImage::Pointer
ImageMosaicVarianceMetric<TImage, TInterpolator>::
variance(const typename TImage::SpacingType & mosaic_sp,
   const pnt2d_t & mosaic_min,
   const pnt2d_t & mosaic_max,
   measure_t & max_var,
   measure_t & avg_var) const
{
  typename TImage::SizeType mosaic_sz;
  mosaic_sz[0] = (unsigned int)((mosaic_max[0] - mosaic_min[0]) /
        mosaic_sp[0]);
  mosaic_sz[1] = (unsigned int)((mosaic_max[1] - mosaic_min[1]) /
        mosaic_sp[1]);
  return variance(mosaic_sp,
      mosaic_min,
      mosaic_sz,
      max_var,
      avg_var);
}

//----------------------------------------------------------------
// variance
//
template <class TImage, class TInterpolator>
typename TImage::Pointer
ImageMosaicVarianceMetric<TImage, TInterpolator>::
variance(const typename TImage::SpacingType & mosaic_sp,
   const pnt2d_t & mosaic_min,
   const typename TImage::SizeType & mosaic_sz,
   measure_t & max_var,
   measure_t & avg_var) const
{
  WRAP(itk_terminator_t
       terminator("ImageMosaicVarianceMetric::GetValueAndDerivative"));

  typedef typename image_t::IndexType index_t;
  //typedef typename image_t::RegionType::SizeType imagesz_t;

  max_var = measure_t(0);
  avg_var = measure_t(0);

  // shortcut:
  const unsigned int num_images = interpolator_.size();
  if (num_images == 0)
  {
    itkExceptionMacro(<< "You forgot to call Initialize()");
  }

  // calculate image bounding boxes in the mosaic space, as well as the
  // mosaic bounding box:
  pnt2d_t global_min = pnt2d(std::numeric_limits<double>::max(),
           std::numeric_limits<double>::max());
  pnt2d_t global_max = pnt2d(-global_min[0], -global_min[1]);
  std::vector<pnt2d_t> min(num_images);
  std::vector<pnt2d_t> max(num_images);
  CalcMosaicBBox(global_min, global_max, min, max);

  // setup the mosaic image:
  typename image_t::Pointer mosaic = image_t::New();
  mosaic->SetOrigin(mosaic_min);
  mosaic->SetRegions(mosaic_sz);
  mosaic->SetSpacing(mosaic_sp);
  mosaic->Allocate();
  mosaic->FillBuffer(NumericTraits<pixel_t>::Zero);

  // iterate over the mosaic, evaluate the metric in the overlapping regions:
  typedef itk::ImageRegionIteratorWithIndex<image_t> itex_t;
  unsigned int pixel_count = 0;

  itex_t itex(mosaic, mosaic->GetLargestPossibleRegion());
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());

    pnt2d_t point;
    mosaic->TransformIndexToPhysicalPoint(itex.GetIndex(), point);

    // find pixels in overlapping regions of the mosaic:
    std::list<measure_t> overlap;
    for (unsigned int k = 0; k < num_images; k++)
    {
      if (point[0] < min[k][0] || point[0] > max[k][0] ||
    point[1] < min[k][1] || point[1] > max[k][1]) continue;

      const typename transform_t::Pointer & t = transform_[k];
      pnt2d_t pt_k = t->TransformPoint(point);
      index_t index;
      image_[k]->TransformPhysicalPointToIndex(pt_k, index);

      // make sure the pixel maps into the image:
      if (!interpolator_[k]->IsInsideBuffer(pt_k)) continue;

      // make sure the pixel maps into the mask:
      if (mask_[k].GetPointer() && mask_[k]->GetPixel(index) == 0) continue;

      // get the image value, add it to the list:
      overlap.push_back(interpolator_[k]->Evaluate(pt_k));
    }

    // skip over the regions that do not overlap:
    if (overlap.size() < 2) continue;

    // found a pixel in an overlapping region, increment the counter:
    pixel_count++;

    // shortcut:
    measure_t normalization_factor = measure_t(1) / measure_t(overlap.size());

    // calculate the mean and derivative of the mean:
    measure_t Mu = measure_t(0);
    for (std::list<measure_t>::const_iterator i = overlap.begin();
   i != overlap.end(); ++i)
    {
      Mu += *i;
    }
    Mu *= normalization_factor;

    // calculate the variance:
    measure_t V = measure_t(0);
    for (std::list<measure_t>::const_iterator i = overlap.begin();
   i != overlap.end(); ++i)
    {
      measure_t d = *i - Mu;
      V += d * d;
    }
    V *= normalization_factor;
    itex.Set(pixel_t(V));

    max_var = std::max(max_var, V);
    avg_var += V;
  }

  measure_t normalization_factor = measure_t(1) / measure_t(pixel_count);
  avg_var *= normalization_factor;

  return mosaic;
}

//----------------------------------------------------------------
// PrintSelf
//
template <class TImage, class TInterpolator>
void
ImageMosaicVarianceMetric<TImage, TInterpolator>::
PrintSelf(std::ostream & os, Indent indent) const
{
  // FIXME: write me:

  Superclass::PrintSelf( os, indent );
  os << indent << "MosaicRegion: " << m_MosaicRegion << std::endl
     << indent << "Number of Pixels Counted: " << m_NumberOfPixelsCounted
     << std::endl;
}

} // end namespace itk


#endif // _itkImageMosaicVarianceMetric_txx
