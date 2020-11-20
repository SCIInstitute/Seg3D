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

#include <Application/ImageRegistrationTools/Actions/ActionSliceToSliceBruteFilter.h>

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Filtering/itkNormalizeImageFilterWithMask.h>
#include <Core/ITKCommon/Optimizers/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/STOS/stos.hxx>
#include <Core/ITKCommon/STOS/stos_common.hxx>
#include <Core/ITKCommon/FFT/fft_common.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>
#include <Core/ITKCommon/grid_common.hxx>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/the_utils.hxx>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegistrationMethod.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkFixedCenterOfRotationAffineTransform.h>
#include <itkCenteredRigid2DTransform.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>

// system includes:
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <sstream>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;


CORE_REGISTER_ACTION( Seg3D, SliceToSliceBruteFilter )

namespace Seg3D
{


//----------------------------------------------------------------
// DEBUG_EVERYTHING
//
//#define DEBUG_EVERYTHING


//----------------------------------------------------------------
// coarse_transform_t
//
typedef itk::FixedCenterOfRotationAffineTransform<double, 2>
coarse_transform_t;

//----------------------------------------------------------------
// order2_transform_t
//
typedef itk::LegendrePolynomialTransform<double, 1> order2_transform_t;

//----------------------------------------------------------------
// order4_transform_t
//
typedef itk::LegendrePolynomialTransform<double, 3> order4_transform_t;


//----------------------------------------------------------------
// interpolator_t
//
typedef itk::LinearInterpolateImageFunction<image_t, double> interpolator_t;

//----------------------------------------------------------------
// optimizer_t
//
typedef itk::RegularStepGradientDescentOptimizer2 optimizer_t;

//----------------------------------------------------------------
// registration_t
//
typedef itk::ImageRegistrationMethod<image_t, image_t>
registration_t;


//----------------------------------------------------------------
// refine_one_pair
//
template <class transform_t>
double
refine_one_pair(const image_t * i0,
                const image_t * i1,
                const mask_t * m0,
                const mask_t * m1,
                typename transform_t::Pointer & t01,
                const unsigned int iterations,
                const double & min_step,
                const double & max_step,
                const itk::Array<double> & optimizer_scales,

                const bfs::path & fn_prefix = "")
{
  // setup the registration object:
  registration_t::Pointer registration = registration_t::New();

  // setup the image interpolator:
  registration->SetInterpolator(interpolator_t::New());

  // setup the optimizer:
  optimizer_t::Pointer optimizer = optimizer_t::New();
  registration->SetOptimizer(optimizer);
  optimizer_observer_t<optimizer_t>::Pointer observer =
  optimizer_observer_t<optimizer_t>::New();
  optimizer->AddObserver(itk::IterationEvent(), observer);
  optimizer->SetMinimize(true);
  optimizer->SetNumberOfIterations(iterations);
  optimizer->SetMinimumStepLength(min_step);
  optimizer->SetMaximumStepLength(max_step);
  optimizer->SetGradientMagnitudeTolerance(1e-12);
  optimizer->SetRelaxationFactor(5e-1);
  optimizer->SetPickUpPaceSteps(5);
  // FIXME:
  optimizer->SetBackTracking(true);

  try { optimizer->SetScales(optimizer_scales); }
  catch (itk::ExceptionObject &err)
  {
    CORE_LOG_ERROR(err.GetDescription());
    throw;
  }

  // setup the image-to-image metric:
  typedef itk::NormalizedCorrelationImageToImageMetric
  // FIXME: typedef itk::MeanSquaresImageToImageMetric
  <image_t, image_t> metric_t;
  metric_t::Pointer metric = metric_t::New();
  metric->SetInterpolator(interpolator_t::New());
  metric->SetSubtractMean(true);
  registration->SetMetric(metric);

  registration->SetTransform(t01);
  registration->SetInitialTransformParameters(t01->GetParameters());

  // setup the masks:
  typedef itk::ImageMaskSpatialObject<2> mask_so_t;
  mask_t::ConstPointer fi_mask = m0;
  if (m0 != nullptr)
  {
    mask_so_t::Pointer fi_mask_so = mask_so_t::New();
    fi_mask_so->SetImage(fi_mask);
    metric->SetFixedImageMask(fi_mask_so);
  }

  mask_t::ConstPointer mi_mask = m1;
  if (m1 != nullptr)
  {
    mask_so_t::Pointer mi_mask_so = mask_so_t::New();
    mi_mask_so->SetImage(mi_mask);
    metric->SetMovingImageMask(mi_mask_so);
  }

  // setup the fixed and moving image:
  image_t::ConstPointer fi = i0;
  image_t::ConstPointer mi = i1;

  registration->SetFixedImageRegion(fi->GetLargestPossibleRegion());
  registration->SetFixedImage(fi);
  registration->SetMovingImage(mi);

  // evaluate the metric before the registration:
  double metric_before =
  eval_metric<metric_t, interpolator_t>(t01, fi, mi, fi_mask, mi_mask);
//  assert(metric_before != std::numeric_limits<double>::max());
  if (metric_before == std::numeric_limits<double>::max())
  {
    CORE_THROW_EXCEPTION("Metric failed evaluation step.");
  }

  typename transform_t::ParametersType params_before =
  t01->GetParameters();

  if (! fn_prefix.empty())
  {
    try
    {
      // try to save the failed mosaic -- sometimes the resulting image
      // may not fit in memory, so an exception will be thrown:
      save_rgb<image_t>(fn_prefix.string() + "v0.png",
                        remap_min_max<image_t>(fi),
                        remap_min_max<image_t>(mi),
                        t01, fi_mask, mi_mask);
    }
    catch (itk::ExceptionObject &err)
    {
      CORE_LOG_ERROR(err.GetDescription());
      throw;
    }
  }

  // perform the registration:
  try
  {
    std::cout << "starting image registration:" << std::endl;
    registration->Update();
  }
  catch (itk::ExceptionObject & err)
  {
    std::cerr << "image registration threw an exception: " << std::endl << err << std::endl;
    CORE_LOG_ERROR(err.GetDescription());

    // FIXME:
#ifdef DEBUG_EVERYTHING
    try
    {
      // try to save the failed mosaic -- sometimes the resulting image
      // may not fit in memory, so an exception will be thrown:

      save_rgb<image_t>(fn_prefix.string() + "exception.png",
                        remap_min_max<image_t>(fi),
                        remap_min_max<image_t>(mi),
                        t01, fi_mask, mi_mask);
    }
    catch (itk::ExceptionObject &err)
    {
      CORE_LOG_ERROR(err.GetDescription());
    }
#endif
    throw;
  }
  t01->SetParameters(optimizer->GetBestParams());

  // evaluate the metric after the registration:
  double metric_after = eval_metric<metric_t, interpolator_t>(t01, fi, mi, fi_mask, mi_mask);

  std::cout << "BEFORE: " << metric_before << std::endl
  << "AFTER:  " << metric_after << std::endl;

  if (! fn_prefix.empty())
  {
    try
    {
      // try to save the failed mosaic -- sometimes the resulting image
      // may not fit in memory, so an exception will be thrown:

      save_rgb<image_t>(fn_prefix.string() + "v1.png",
                        remap_min_max<image_t>(fi),
                        remap_min_max<image_t>(mi),
                        t01, fi_mask, mi_mask);
    }
    catch (itk::ExceptionObject &err)
    {
      CORE_LOG_ERROR(err.GetDescription());
      throw;
    }
  }

  if (metric_before <= metric_after || metric_after != metric_after)
  {
    std::cout << "NOTE: minimization failed, ignoring registration results..." << std::endl;
    t01->SetParameters(params_before);
    return metric_before;
  }

  return metric_after;
}

//----------------------------------------------------------------
// refine_one_pair
//
template <class transform_t>
double
refine_one_pair(int octave_start,
                int num_octaves,
                const pyramid_t & p0,
                const pyramid_t & p1,
                typename transform_t::Pointer & t01,
                const unsigned int iterations,
                const double & min_step,
                const double & max_step,
                const itk::Array<double> & optimizer_scales,

                const bfs::path & fn_prefix = "")
{
  int octave_end = octave_start + 1 - num_octaves;

  double metric = std::numeric_limits<double>::max();
  for (int i = octave_start; i >= octave_end; i--)
  {
    const unsigned int last_scale = p0.octave_[i].scales() - 1;
//    assert(last_scale + 1 == p1.octave_[i].scales());
    if (last_scale + 1 != p1.octave_[i].scales())
    {
      CORE_THROW_EXCEPTION("pair scales do not match");
    }

    for (int j = last_scale; j >= 0; j--)
    {
      std::ostringstream pfx;
      pfx << fn_prefix << "o" << the_text_t::number(i) << "-s" << the_text_t::number(j) << "-";

      metric = refine_one_pair<transform_t>(p0.octave_[i].L_[j],
                                            p1.octave_[i].L_[j],
                                            p0.octave_[i].mask_,
                                            p1.octave_[i].mask_,
                                            t01,
                                            iterations,
                                            min_step,
                                            max_step,
                                            optimizer_scales,
                                            bfs::path(pfx.str()));
    }
  }

  return metric;
}

//----------------------------------------------------------------
// refine_one_pair
//
template <class transform_t>
double
refine_one_pair(const pyramid_t & p0,
                const pyramid_t & p1,
                typename transform_t::Pointer & t01,
                const unsigned int iterations,
                const double & min_step,
                const double & max_step,
                const itk::Array<double> & optimizer_scales,

                const bfs::path & fn_prefix = "")
{
  int num_octaves = std::min(p0.octaves(), p1.octaves());
  int octave_start = num_octaves - 1;
  return refine_one_pair<transform_t>(p0,
                                      p1,
                                      t01,
                                      iterations,
                                      min_step,
                                      max_step,
                                      optimizer_scales,
                                      octave_start,
                                      num_octaves,
                                      bfs::path(fn_prefix.string()));
}

//----------------------------------------------------------------
// refine_one_pair
//
template <class transform_t>
double
refine_one_pair(const pyramid_t & p0,
                const pyramid_t & p1,
                typename transform_t::Pointer & t01,
                const unsigned int iterations,
                const double & min_step,
                const double & max_step,

                const bfs::path & fn_prefix = "")
{
  // FIXME: this is probably unnecessary:
  typedef optimizer_t::ScalesType optimizer_scales_t;
  optimizer_scales_t scales(t01->GetNumberOfParameters());
  scales.Fill(1.0);

  // encourage translation:
  // scales[transform_t::index_a(0, 0)] = 1e-1;
  // scales[transform_t::index_b(0, 0)] = 1e-1;

  return refine_one_pair<transform_t>(p0, p1, t01, iterations, min_step, max_step, scales, fn_prefix);
}

//----------------------------------------------------------------
// refine_one_pair
//
double
refine_one_pair(int octave_start,
                int num_octaves,
                const pyramid_t & p0,
                const pyramid_t & p1,
                const bool & use_low_order_intermediate,
                const coarse_transform_t * t_coarse,
                order4_transform_t::Pointer & t_order4,

                const unsigned int iterations,
                const double & min_step,
                const double & max_step,

                const bfs::path & fn_prefix = "")
{
  // encourage translation:
  itk::Array<double> scales2(
  order2_transform_t::New()->GetNumberOfParameters() );
  scales2.Fill(1e-0);
  scales2[order2_transform_t::index_a(0, 0)] = 1e-1;
  scales2[order2_transform_t::index_b(0, 0)] = 1e-1;

  // encourage translation:
  itk::Array<double> scales4(
  order4_transform_t::New()->GetNumberOfParameters() );
  scales4.Fill(1e-0);
  scales4[order4_transform_t::index_a(0, 0)] = 1e-1;
  scales4[order4_transform_t::index_b(0, 0)] = 1e-1;

  base_transform_t::ConstPointer t_previous = t_coarse;
  double metric = std::numeric_limits<double>::max();

  int octave_end = octave_start + 1 - num_octaves;
  for (int i = octave_start; i >= octave_end; i--)
  {
    const unsigned int last_scale = p0.octave_[i].scales() - 1;
//    assert(last_scale + 1 == p1.octave_[i].scales());
    if (last_scale + 1 != p1.octave_[i].scales())
    {
      CORE_THROW_EXCEPTION("pair scales do not match");
    }

    for (int j = last_scale; j >= 0; j--)
    {
      std::ostringstream pfx2;
      pfx2 << fn_prefix << "o" << the_text_t::number(i) << "-s" << the_text_t::number(j) << "-order2-";

      const image_t * a = p0.octave_[i].L_[j];
      const image_t * b = p1.octave_[i].L_[j];
      const mask_t * ma = p0.octave_[i].mask_;
      const mask_t * mb = p1.octave_[i].mask_;

      if (use_low_order_intermediate)
      {
        // bootstrap the low-order transform:
        order2_transform_t::Pointer
        t_order2 = setup_transform<order2_transform_t, image_t>(b);
        solve_for_transform<image_t, order2_transform_t>
        (b, mb, t_previous, t_order2, 16, 2, true);

        // refine the low-order transform:
        metric = refine_one_pair<order2_transform_t>(a,
                                                     b,
                                                     ma,
                                                     mb,
                                                     t_order2,
                                                     iterations,
                                                     min_step,
                                                     max_step,
                                                     scales2,
                                                     bfs::path(pfx2.str()));
        t_previous = t_order2;
      }

      // bootstrap the high-order transform:
      t_order4 = setup_transform<order4_transform_t, image_t>(b);
      solve_for_transform<image_t, order4_transform_t>
      (b, mb, t_previous, t_order4, 16, 1, true);

      // refine the high-order transform:
      std::ostringstream pfx4;
      pfx4 << fn_prefix << "o" << the_text_t::number(i) << "-s" << the_text_t::number(j) << "-order4-";

      metric = refine_one_pair<order4_transform_t>(a,
                                                   b,
                                                   ma,
                                                   mb,
                                                   t_order4,
                                                   iterations,
                                                   min_step,
                                                   max_step,
                                                   scales4,
                                                   bfs::path(pfx4.str()));
      t_previous = t_order4;
    }
  }

  return metric;
}

//----------------------------------------------------------------
// refine_one_pair
//
double
refine_one_pair(int octave_start,
                int num_octaves,
                const pyramid_t & p0,
                const pyramid_t & p1,
                const coarse_transform_t * t_coarse,
                order2_transform_t::Pointer & t_order2,

                const unsigned int iterations,
                const double & min_step,
                const double & max_step,

                const bfs::path & fn_prefix = "")
{
  // encourage translation:
  itk::Array<double> scales2(
  order2_transform_t::New()->GetNumberOfParameters() );
  scales2.Fill(1e-0);
  scales2[order2_transform_t::index_a(0, 0)] = 1e-1;
  scales2[order2_transform_t::index_b(0, 0)] = 1e-1;

  double metric = std::numeric_limits<double>::max();
  int octave_end = octave_start + 1 - num_octaves;

  t_order2 =
  setup_transform<order2_transform_t, image_t>
  (p1.octave_[octave_end].L_[0]);

  solve_for_transform<image_t, order2_transform_t>
  (p1.octave_[octave_end].L_[0],
   p1.octave_[octave_end].mask_,
   t_coarse,
   t_order2,
   16,
   2,
   true);

  for (int i = octave_start; i >= octave_end; i--)
  {
    const unsigned int last_scale = p0.octave_[i].scales() - 1;
//    assert(last_scale + 1 == p1.octave_[i].scales());
    if (last_scale + 1 != p1.octave_[i].scales())
    {
      CORE_THROW_EXCEPTION("pair scales do not match");
    }

    for (int j = last_scale; j >= 0; j--)
    {
      const image_t * a = p0.octave_[i].L_[j];
      const image_t * b = p1.octave_[i].L_[j];
      const mask_t * ma = p0.octave_[i].mask_;
      const mask_t * mb = p1.octave_[i].mask_;

      // bootstrap the low-order transform:
      // refine the low-order transform:
      std::ostringstream pfx2;
      pfx2 << fn_prefix << "o" << the_text_t::number(i) << "-s" << the_text_t::number(j) << "-order2-";

      metric = refine_one_pair<order2_transform_t>(a,
                                                   b,
                                                   ma,
                                                   mb,
                                                   t_order2,
                                                   iterations,
                                                   min_step,
                                                   max_step,
                                                   scales2,
                                                   bfs::path(pfx2.str()));
    }
  }

  return metric;
}


//----------------------------------------------------------------
// fast_variance
//
static
double
fast_variance(const image_t * a,
              const image_t * b,
              const mask_t * ma,
              const mask_t * mb,
              const base_transform_t * t_ab,
              double samples_per_64x64 = 128.0)
{
  if (t_ab == nullptr) return std::numeric_limits<double>::max();

  image_t::SpacingType a_sp = a->GetSpacing();
  image_t::SizeType a_sz = a->GetLargestPossibleRegion().GetSize();
  pnt2d_t a_min = a->GetOrigin();
  pnt2d_t a_max = a_min + vec2d((a_sz[0]) * a_sp[0],
                                (a_sz[1]) * a_sp[1]);

  const double w0 = a_max[0] - a_min[0];
  const double h0 = a_max[1] - a_min[1];
  const double samples_per_pixel = samples_per_64x64 / (4096.0);
  const double sx = samples_per_pixel * w0;
  const double sy = samples_per_pixel * h0;
  const unsigned int nx = static_cast<unsigned int>(sx + 0.5);
  const unsigned int ny = static_cast<unsigned int>(sy + 0.5);

  // uniformly sample the variance in the overlapping region:
  interpolator_t::Pointer ia = interpolator_t::New();
  ia->SetInputImage(a);

  interpolator_t::Pointer ib = interpolator_t::New();
  ib->SetInputImage(b);

  double v = 0.0;
  double n = 0.0;

  pnt2d_t pa;
  pnt2d_t pb;
  for (unsigned int i = 1; i <= nx; i++)
  {
    double tx = static_cast<double>(i) / static_cast<double>(nx + 1);
    pa[0] = a_min[0] + tx * w0;

    for (unsigned int j = 1; j <= ny; j++)
    {
      double ty = static_cast<double>(j) / static_cast<double>(ny + 1);
      pa[1] = a_min[1] + ty * h0;
      if (!ia->IsInsideBuffer(pa)) continue;

      image_t::IndexType index_a;
      if (!ma->TransformPhysicalPointToIndex(pa, index_a)) continue;
      if (ma->GetPixel(index_a) == 0) continue;

      pb = t_ab->TransformPoint(pa);
      if (!ib->IsInsideBuffer(pb)) continue;

      image_t::IndexType index_b;
      if (!mb->TransformPhysicalPointToIndex(pb, index_b)) continue;
      if (mb->GetPixel(index_b) == 0) continue;

      double pixel_a = ia->Evaluate(pa);
      double pixel_b = ib->Evaluate(pb);
      double mean = (pixel_a + pixel_b) / 2.0;

      double da = pixel_a - mean;
      double db = pixel_b - mean;

      v += da * da + db * db;
      n += 1.0;
    }
  }

  if (n == 0.0) return std::numeric_limits<double>::max();

  // return the mean variance:
  return v / n;
}

//----------------------------------------------------------------
// image_center
//
template <class image_t>
pnt2d_t
image_center(const image_t * image)
{
  typename image_t::SizeType sz = image->GetLargestPossibleRegion().GetSize();
  typename image_t::SpacingType sp = image->GetSpacing();

  pnt2d_t min = image->GetOrigin();
  pnt2d_t max = min + vec2d(static_cast<double>(sz[0]) * sp[0],
                            static_cast<double>(sz[1]) * sp[1]);
  pnt2d_t center = min + (max - min) * 0.5;

  return center;
}

//----------------------------------------------------------------
// brute_force
//
void
brute_force(const bool & brute_force_rotation,
            const bool & brute_force_translation,
            const double & min_overlap,
            double & best_overlap,
            double & best_angle,
            vec2d_t & best_shift,
            const bfs::path & fn_debug,
            const image_t * a,
            const image_t * b,
            const mask_t * ma,
            const mask_t * mb,
            double a0,
            double a1,
            int num_orientations,
            const int step_size = 1,
            bool verbose = false)
{
  // octave 0 is fixed, octave 1 is rotated and matched to octave 0:
  double best_metric = std::numeric_limits<double>::max();

  double v_min_global = std::numeric_limits<double>::max();
  double v_max_global = -v_min_global;
  std::vector<double> v_min(num_orientations, v_min_global);
  std::vector<double> v_max(num_orientations, v_max_global);
  std::vector<image_t::Pointer> vimage(num_orientations);

  image_t::SizeType z0 = a->GetLargestPossibleRegion().GetSize();
  image_t::SpacingType s1 = b->GetSpacing();
  const pnt2d_t center = image_center<image_t>(b);

  typedef itk::NearestNeighborInterpolateImageFunction
  <image_t, double> interpolator_t;
  interpolator_t::Pointer b_interpolator = interpolator_t::New();
  b_interpolator->SetInputImage(b);

#ifdef DEBUG_EVERYTHING
  if (! fn_debug.empty())
  {
    save<native_image_t>(cast<image_t, native_image_t>(a),
                         fn_debug.string() + "fixed.png");
  }
#endif

  // TODO: this needs to be a state instead
//  set_major_progress(0.05);

  bool done_rotation = false;
  for (int k = 0; k < num_orientations && !done_rotation; k++)
  {
    double angle = a0 + (a1 - a0) * static_cast<double>(k) / static_cast<double>(num_orientations - 1);

    if (!brute_force_rotation)
    {
      angle = best_angle;
      done_rotation = true;
    }

#ifdef DEBUG_EVERYTHING
    std::ostringstream fn_dbg_pfx;
    if (! fn_debug.empty())
    {
      fn_dbg_pfx << fn_debug << "a" << the_text_t::number(static_cast<int>(0.5 + angle * 360.0 / TWO_PI), 3, '0') << "-";
    }
#endif

    coarse_transform_t::Pointer rot = coarse_transform_t::New();
    rot->SetCenterOfRotationComponent(center);
    rot->Rotate2D(angle);

    // re-estimate the dimensions of the rotated tile:
    image_t::PointType min;
    image_t::PointType max;
    calc_tile_mosaic_bbox<image_t>(rot, b, min, max);

    // resample the moving image:
    image_t::Pointer b_warped = warp<image_t>(b, rot.GetPointer());
    mask_t::Pointer mb_warped = nullptr;
    if ( mb != nullptr )
      mb_warped = warp<mask_t>(mb, rot.GetPointer());
    static const pnt2d_t zero = pnt2d(0, 0);
    vec2d_t shift = b_warped->GetOrigin() - zero;
    b_warped->SetOrigin(zero);
    if ( mb != nullptr )
      mb_warped->SetOrigin(zero);

//#if 0 // def DEBUG_EVERYTHING
//    if (fn_debug.size() != 0)
//    {
//      save<native_image_t>(cast<image_t, native_image_t>(b_warped),
//                           fn_dbg_pfx.str() + "warped.png");
//    }
//#endif

    image_t::SizeType period_sz = calc_padding<image_t>(a, b_warped);

    image_t::PointType offset_min;
    min[0] = -std::numeric_limits<double>::max();
    min[1] = -std::numeric_limits<double>::max();
    image_t::PointType offset_max;
    max[0] = std::numeric_limits<double>::max();
    max[1] = std::numeric_limits<double>::max();


    translate_transform_t::Pointer translate;
    double v = match_one_pair<image_t>(translate,

                                       a,
                                       ma,
                                       b_warped.GetPointer(),
                                       mb_warped.GetPointer(),

                                       period_sz,

                                       // overlap min/max:
                                       0.6,
                                       1.0,

                                       // offset min/max:
                                       offset_min,
                                       offset_max,

                                       // low pass filters:
                                       0.9, // r
                                       0.1, // s

                                       // max peaks in the PDF:
                                       10,

                                       // don't consider the zero displacement:
                                       false);

//#if 0 // def DEBUG_EVERYTHING
//    if (fn_debug.size() != 0)
//    {
//      if (v != std::numeric_limits<double>::max())
//      {
//        save_rgb<image_t>(fn_dbg_pfx.str() + "000000.png",
//
//                          remap_min_max<image_t>(a),
//                          remap_min_max<image_t>(b_warped),
//
//                          translate,
//
//                          ma,
//                          mb_warped,
//                          false);
//      }
//    }
//#endif

    if (v < best_metric)
    {
      best_metric = v;
      best_angle = angle;
      best_shift = translate->GetOffset() + shift;
      // best_overlap = overlap;

#ifdef DEBUG_EVERYTHING
      if (! fn_debug.empty())
      {
        save_rgb<image_t>(fn_dbg_pfx.str() + "better.png",

                          remap_min_max<image_t>(a),
                          remap_min_max<image_t>(b_warped),

                          translate,

                          ma,
                          mb_warped,
                          true);
      }
#endif
    }
    // TODO: this needs to be a state instead
//    set_major_progress(0.94 * (k+1)/(num_orientations+1) + 0.05);
  }

  // TODO: this needs to be a state instead
//  set_major_progress(0.99);
}

bool
ActionSliceToSliceBruteFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

//----------------------------------------------------------------
// main
//
bool
ActionSliceToSliceBruteFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(6);
    std::cout.setf(std::ios::scientific);

    track_progress(true);

    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);

    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);

    bfs::path fn_load[2] = { this->input_fixed_, this->input_moving_ };
  //  fn_load[0] = this->input_fixed_;
  //  fn_load[1] = this->input_moving_;

    bfs::path fn_mask[2];
    if ( (this->mask_fixed_ != "<none>") && (this->mask_moving_ != "<none>") )
    {
      fn_mask[0] = this->mask_fixed_;
      fn_mask[1] = this->mask_moving_;
    }

    bool flip[2] = { this->flip_fixed_, this->flip_moving_ };

    bfs::path image_dirs[2];
    if ( (this->image_dir_fixed_ != "<none>") && (this->image_dir_moving_ != "<none>") )
    {
      image_dirs[0] = this->image_dir_fixed_;
      image_dirs[1] = this->image_dir_moving_;
    }

    bfs::path fn_save(this->output_stos_);
    if (! bfs::is_directory( fn_save.parent_path() ) )
    {
      CORE_LOG_DEBUG(std::string("Creating parent path to ") + this->output_stos_);
      if (! boost::filesystem::create_directories(fn_save.parent_path()))
      {
        std::ostringstream oss;
        oss << "Could not create missing directory " << fn_save.parent_path() << " required to create output mosaic.";
        context->report_error(oss.str());
        return false;
      }
    }

    bool brute_force_rotation = true;
    if (this->best_angle_ != 0)
    {
      brute_force_rotation = false;
      // convert degrees to radians:
      this->best_angle_ = this->best_angle_ * TWO_PI / 360.0;
    }

    bool brute_force_translation = true;
    vec2d_t best_shift = vec2d(0, 0);
    if (this->best_shift_x_ != 0 && this->best_shift_y_ != 0)
    {
      brute_force_translation = false;
      best_shift = vec2d(this->best_shift_x_, this->best_shift_y_);
    }

    // unsigned int num_orientations = 120;
    unsigned int num_orientations = 360;
    unsigned int step_size = 2;

    // TODO: expose?
    bool verbose = false;

    if (fn_load[0].empty() || fn_load[1].empty())
    {
      context->report_error("must specify 2 files with the -load option");
      return false;
    }

    if (fn_save.empty())
    {
      context->report_error("must specify a file to open with the -save option");
      return false;
    }

    // dump the status:
    std::cout << "shrink factor: " << this->shrink_factor_ << std::endl
    << "search for rotation: " << brute_force_rotation << std::endl
    << "search for translation: " << brute_force_translation << std::endl
    << std::endl;

    bfs::path fn_debug;
#ifdef DEBUG_EVERYTHING
    fn_debug = fn_save.string() + ".FIXME-";
#endif

    // load the images, assemble the pyramids, match them:
    image_t::Pointer mosaic[2];
    mask_t::Pointer mosaic_mask[2];
    pyramid_t pyramid[2];

    if (fn_load[0].extension() == ".mosaic")
    {
      load_slice(fn_load[0],
                 flip[0],
                 this->shrink_factor_,
                 this->clahe_slope_,
                 image_dirs[0],
                 mosaic[0],
                 mosaic_mask[0]);

      setup_pyramid(pyramid[0],
                    0, // index
                    fn_load[0],
                    mosaic[0],
                    mosaic_mask[0],
                    STOS_BRUTE_PYRAMID_KEY,
                    SCALES_PER_OCTAVE,
                    DEFAULT_KEY_GENERATE_SETTING, // generate descriptors?
                    fn_debug);
    }
    else if (fn_load[0].extension() == ".pyramid")
    {
      // load the pyramid:
      load_pyramid(fn_load[0], pyramid[0], mosaic[0], mosaic_mask[0]);
    }
    else
    {
      // load first slice:
      mosaic[0] = std_tile<image_t>(fn_load[0],
                                    this->shrink_factor_,
                                    this->pixel_spacing_);

      if (!fn_mask[0].empty())
      {
        // load slice mask:
        mosaic_mask[0] = std_tile<mask_t>(fn_mask[0],
                                          this->shrink_factor_,
                                          this->pixel_spacing_);
      }
      else
      {
        mosaic_mask[0] = std_mask<image_t>(mosaic[0], this->use_standard_mask_);
      }

      setup_pyramid(pyramid[0],
                    0, // index
                    fn_load[0],
                    mosaic[0],
                    mosaic_mask[0],
                    STOS_BRUTE_PYRAMID_KEY,
                    SCALES_PER_OCTAVE,
                    DEFAULT_KEY_GENERATE_SETTING, // generate descriptors?
                    fn_debug);
    }

    if (fn_load[1].extension() == ".mosaic")
    {
      load_slice(fn_load[1],
                 flip[1],
                 this->shrink_factor_,
                 this->clahe_slope_,
                 image_dirs[1],
                 mosaic[1],
                 mosaic_mask[1]);

      setup_pyramid(pyramid[1],
                    1, // index
                    fn_load[1],
                    mosaic[1],
                    mosaic_mask[1],
                    STOS_BRUTE_PYRAMID_KEY,
                    SCALES_PER_OCTAVE,
                    DEFAULT_KEY_GENERATE_SETTING, // generate descriptors?
                    fn_debug);
    }
    else if (fn_load[1].extension() == ".pyramid")
    {
      // load the pyramid:
      load_pyramid(fn_load[1], pyramid[1], mosaic[1], mosaic_mask[1]);
    }
    else
    {
      // load second slice:
      mosaic[1] = std_tile<image_t>(fn_load[1],
                                    this->shrink_factor_,
                                    this->pixel_spacing_);

      if (!fn_mask[1].empty())
      {
        // load slice mask:
        mosaic_mask[1] = std_tile<mask_t>(fn_mask[1],
                                          this->shrink_factor_,
                                          this->pixel_spacing_);
      }
      else
      {
        mosaic_mask[1] = std_mask<image_t>(mosaic[1], this->use_standard_mask_);
      }

      setup_pyramid(pyramid[1],
                    1, // index
                    fn_load[1],
                    mosaic[1],
                    mosaic_mask[1],
                    STOS_BRUTE_PYRAMID_KEY,
                    SCALES_PER_OCTAVE,
                    DEFAULT_KEY_GENERATE_SETTING, // generate descriptors?
                    fn_debug);
    }

    const int num_levels = std::min(pyramid[0].octaves(), pyramid[1].octaves());

    // FIXME:
//#if 0
//  for (unsigned int i = 0; i < 2; i++)
//  {
//    for (unsigned int j = 0; j < num_levels; j++)
//    {
//      the_text_t fn =
//      the_text_t::number(i, 2, '0') + "-" +
//      the_text_t::number(j, 2, '0') + ".png";
//
//      save<native_image_t>
//      (cast<image_t, native_image_t>(pyramid[i].octave_[j].L_[0]),
//       fn);
//    }
//  }
//#endif

    // brute force registration:
    brute_force(brute_force_rotation,
                brute_force_translation,
                0.3, // minimum overlap
                this->best_overlap_,
                this->best_angle_,
                best_shift,
                fn_debug,
                pyramid[0].octave_[num_levels - 1].L_[0],
                pyramid[1].octave_[num_levels - 1].L_[0],
                pyramid[0].octave_[num_levels - 1].mask_,
                pyramid[1].octave_[num_levels - 1].mask_,

                // start/finish rotation angles:
                0.0,
                TWO_PI * (static_cast<double>(num_orientations - 1) /
                          static_cast<double>(num_orientations)),
                num_orientations,
                step_size,
                verbose);

    std::cout << "BEST SHIFT:   " << best_shift << std::endl
    << "BEST ANGLE:   " << this->best_angle_ * 360.0 / TWO_PI<< std::endl
    << "BEST OVERLAP: " << this->best_overlap_ << std::endl;

    coarse_transform_t::Pointer t_coarse = coarse_transform_t::New();
    t_coarse->SetTranslation(best_shift);
    t_coarse->SetCenterOfRotationComponent
    (image_center<image_t>(pyramid[1].octave_[num_levels - 1].L_[0]));
    t_coarse->Rotate2D(this->best_angle_);

    base_transform_t::ConstPointer t_final;
    if (!this->use_refinement_)
    {
      t_final = t_coarse.GetPointer();
    }
    else
    {
      // calculate the octave range for initial transform estimation
      // (low-order, or constrained high-order):
      int octave_start = std::min(pyramid[0].octaves(),
                                  pyramid[1].octaves()) - 1;
      // int octave_end = 0;
      int octave_end = std::max(octave_start + 1 -
                                std::min(3, static_cast<int>(std::min(pyramid[0].octaves(),
                                                         pyramid[1].octaves()))),
                                0);

      if (! this->use_cubic_)
      {
        order2_transform_t::Pointer t_order2;
        refine_one_pair(octave_start,
                        octave_start - octave_end + 1,
                        pyramid[0],
                        pyramid[1],
                        t_coarse,
                        t_order2,
                        DEFAULT_ITERATIONS,
                        DEFAULT_MIN_STEP,
                        DEFAULT_MAX_STEP, // FIXME: 1e-6
                        fn_debug);

        t_final = t_order2.GetPointer();
      }
      else
      {
        order4_transform_t::Pointer t_order4;
        refine_one_pair(octave_start,
                        octave_start - octave_end + 1,
                        pyramid[0],
                        pyramid[1],
                        this->regularize_,
                        t_coarse,
                        t_order4,
                        DEFAULT_ITERATIONS,
                        DEFAULT_MIN_STEP,
                        DEFAULT_MAX_STEP, // FIXME: 1e-6
                        fn_debug);

        if (octave_end > 0)
        {
          itk::Array<double> scales( t_order4->GetNumberOfParameters() );
          scales.Fill(1e-0);

          // encourage translation:
          scales[order4_transform_t::index_a(0, 0)] = 1e-1;
          scales[order4_transform_t::index_b(0, 0)] = 1e-1;

          // Avoid ruining the previous transform results due to image
          // artifacts in the low resolution octaves of the pyramid.
          // Start with the next unprocessed octave and complete the process
          // to the highest resolution:
          refine_one_pair<order4_transform_t>(octave_end - 1,
                                              octave_end,
                                              pyramid[0],
                                              pyramid[1],
                                              t_order4,
                                              DEFAULT_ITERATIONS,
                                              DEFAULT_MIN_STEP,
                                              DEFAULT_MAX_STEP, // FIXME: 1e-6
                                              scales,
                                              fn_debug.string() + "final-");
        }

        t_final = t_order4.GetPointer();
      }
    }

    save_stos<image_t>(fn_save,
                       fn_load[0],
                       fn_load[1],
                       fn_mask[0],
                       fn_mask[1],
                       flip[0],
                       flip[1],
                       mosaic[0],
                       mosaic[1],
                       this->shrink_factor_,
                       t_final,
                       true);

    // TODO: this needs to be a state instead
//    set_major_progress(1.0);

    CORE_LOG_SUCCESS("ir-stos-brute done");

    // done:
    return true;
  }
  catch (bfs::filesystem_error &err)
  {
    context->report_error(err.what());
  }
  catch (itk::ExceptionObject &err)
  {
    context->report_error(err.GetDescription());
  }
  catch (Core::Exception &err)
  {
    context->report_error(err.what());
    context->report_error(err.message());
  }
  catch (std::exception &err)
  {
    context->report_error(err.what());
  }
  catch (...)
  {
    context->report_error("Unknown exception type caught.");
  }
  return false;
}

void
ActionSliceToSliceBruteFilter::Dispatch(Core::ActionContextHandle context,
                                        std::string target_layer,
                                        unsigned int shrink_factor,
                                        double pixel_spacing,
                                        double clahe_slope,
                                        double best_shift_x,
                                        double best_shift_y,
                                        double best_angle,
                                        double best_overlap,
                                        bool use_standard_mask,
                                        bool use_refinement,
                                        bool use_cubic,
                                        bool regularize,
                                        bool flip_fixed,
                                        bool flip_moving,
                                        std::string input_fixed,
                                        std::string input_moving,
                                        std::string output_stos,
                                        std::string mask_fixed,
                                        std::string mask_moving,
                                        std::string image_dir_fixed,
                                        std::string image_dir_moving)
{
  // Create a new action
  ActionSliceToSliceBruteFilter* action = new ActionSliceToSliceBruteFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->shrink_factor_ = shrink_factor;
  action->pixel_spacing_ = pixel_spacing;
  action->clahe_slope_ = clahe_slope;
  action->best_shift_x_ = best_shift_x;
  action->best_shift_y_ = best_shift_y;
  action->best_angle_ = best_angle;
  action->best_overlap_ = best_overlap;
  action->use_standard_mask_ = use_standard_mask;
  action->use_refinement_ = use_refinement;
  action->use_cubic_ = use_cubic;
  action->regularize_ = regularize;
  action->flip_fixed_ = flip_fixed;
  action->flip_moving_ = flip_moving;
  action->input_fixed_ = input_fixed;
  action->input_moving_ = input_moving;
  action->output_stos_ = output_stos;
  action->mask_fixed_ = mask_fixed;
  action->mask_moving_ = mask_moving;
  action->image_dir_fixed_ = image_dir_fixed;
  action->image_dir_moving_ = image_dir_moving;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
