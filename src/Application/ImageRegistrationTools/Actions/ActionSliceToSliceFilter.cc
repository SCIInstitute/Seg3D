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

#include <Application/ImageRegistrationTools/Actions/ActionSliceToSliceFilter.h>

// local includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Filtering/itkNormalizeImageFilterWithMask.h>
#include <Core/ITKCommon/Optimizers/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/STOS/stos.hxx>
#include <Core/ITKCommon/STOS/stos_common.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegistrationMethod.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>

// system includes:
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <sstream>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

CORE_REGISTER_ACTION( Seg3D, SliceToSliceFilter )

namespace Seg3D
{

bool
ActionSliceToSliceFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

//----------------------------------------------------------------
// DEBUG_EVERYTHING
//
// #define DEBUG_EVERYTHING


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
typedef itk::ImageRegistrationMethod<image_t, image_t> registration_t;


//----------------------------------------------------------------
// optimizer_observer_t
//
class optimizer_observer_t : public itk::Command
{
public:
  typedef optimizer_observer_t Self;
  typedef itk::Command Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  itkNewMacro(Self);

  void Execute(itk::Object *caller, const itk::EventObject & event)
  { Execute((const itk::Object *)(caller), event); }

  void Execute(const itk::Object * object, const itk::EventObject & event) override
  {
    if (typeid(event) != typeid(itk::IterationEvent)) return;

    const optimizer_t * optimizer = dynamic_cast<const optimizer_t *>(object);
    std::cout << static_cast<unsigned int>(optimizer->GetCurrentIteration()) << '\t'
    << optimizer->GetValue() << '\t'
    << optimizer->GetCurrentPosition() << std::endl;
  }

protected:
  optimizer_observer_t() {}
};

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
  optimizer->AddObserver(itk::IterationEvent(), optimizer_observer_t::New());
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
  assert(metric_before != std::numeric_limits<double>::max());

  typename transform_t::ParametersType params_before =
  t01->GetParameters();

  if (! fn_prefix.empty())
  {
    try
    {
      // try to save the failed mosaic -- sometimes the resulting image
      // may not fit in memory, so an exception will be thrown:

      save_rgb<image_t>(fn_prefix.string() + "v0.tif",
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
  catch (itk::ExceptionObject & exception)
  {
    std::cerr << "image registration threw an exception: "
    << std::endl << exception << std::endl;

    // FIXME:
//#if 1
#ifdef DEBUG_EVERYTHING
    try
    {
      // try to save the failed mosaic -- sometimes the resulting image
      // may not fit in memory, so an exception will be thrown:

      save_rgb<image_t>(fn_prefix.string() + "exception.tif",
                        remap_min_max<image_t>(fi),
                        remap_min_max<image_t>(mi),
                        t01, fi_mask, mi_mask);
    }
    catch (itk::ExceptionObject &err)
    {
      CORE_LOG_ERROR(err.GetDescription());
    }
//#endif
#endif // DEBUG_EVERYTHING
    throw;
  }
  t01->SetParameters(optimizer->GetBestParams());

  // evaluate the metric after the registration:
  double metric_after =
  eval_metric<metric_t, interpolator_t>(t01, fi, mi, fi_mask, mi_mask);

  std::cout << "BEFORE: " << metric_before << std::endl
  << "AFTER:  " << metric_after << std::endl;

  if (! fn_prefix.empty())
  {
    try
    {
      // try to save the failed mosaic -- sometimes the resulting image
      // may not fit in memory, so an exception will be thrown:

      save_rgb<image_t>(fn_prefix.string() + "v1.tif",
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
    std::cout << "NOTE: minimization failed, ignoring registration results..."
    << std::endl;
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
    assert(last_scale + 1 == p1.octave_[i].scales());

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
                                      fn_prefix);
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

  return refine_one_pair<transform_t>
  (p0, p1, t01, iterations, min_step, max_step, scales, fn_prefix);
}

//----------------------------------------------------------------
// refine_one_pair
//
double
refine_one_pair(int octave_start,
                int num_octaves,
                const pyramid_t & p0,
                const pyramid_t & p1,
                const base_transform_t * t_coarse,
                order4_transform_t::Pointer & t_order4,

                const unsigned int iterations,
                const double & min_step,
                const double & max_step,

                const bfs::path & fn_prefix = "")
{
  // encourage translation:
  itk::Array<double> scales2( order2_transform_t::New()->GetNumberOfParameters() );
  scales2.Fill(1e-0);
  scales2[order2_transform_t::index_a(0, 0)] = 1e-1;
  scales2[order2_transform_t::index_b(0, 0)] = 1e-1;

  // encourage translation:
  itk::Array<double> scales4( order4_transform_t::New()->GetNumberOfParameters() );
  scales4.Fill(1e-0);
  scales4[order4_transform_t::index_a(0, 0)] = 1e-1;
  scales4[order4_transform_t::index_b(0, 0)] = 1e-1;

  base_transform_t::ConstPointer t_previous = t_coarse;
  double metric = std::numeric_limits<double>::max();

  int octave_end = octave_start + 1 - num_octaves;
  for (int i = octave_start; i >= octave_end; i--)
  {
    const unsigned int last_scale = p0.octave_[i].scales() - 1;
    assert(last_scale + 1 == p1.octave_[i].scales());

    for (int j = last_scale; j >= 0; j--)
    {
      const image_t * a = p0.octave_[i].L_[j];
      const image_t * b = p1.octave_[i].L_[j];
      const mask_t * ma = p0.octave_[i].mask_;
      const mask_t * mb = p1.octave_[i].mask_;

      // bootstrap the low-order transform:
      order2_transform_t::Pointer
      t_order2 = setup_transform<order2_transform_t, image_t>(b);
      solve_for_transform<image_t, order2_transform_t>
      (b, mb, t_previous, t_order2, 16, 2, true);

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
      t_previous = t_order2;

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

bool
ActionSliceToSliceFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(6);
    std::cout.setf(std::ios::scientific);

    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);

    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);

    // parameters:
    bfs::path fn_load[2] = { this->input_fixed_, this->input_moving_ };
    //  bfs::path fn_load_directory[2];

    bfs::path fn_mask[2];
    if ( (this->mask_fixed_ != "<none>") && (this->mask_moving_ != "<none>") )
    {
      fn_mask[0] = this->mask_fixed_;
      fn_mask[1] = this->mask_moving_;
    }

    bool flip[2] = { this->flip_fixed_, this->flip_moving_ };
    //  bool flip[2] = { false, false };

    bfs::path image_dirs[2];
    if ( (this->image_dir_fixed_ != "<none>") && (this->image_dir_moving_ != "<none>") )
    {
      image_dirs[0] = this->image_dir_fixed_;
      image_dirs[1] = this->image_dir_moving_;
    }

    bfs::path fn_save(this->output_stos_);

    if (fn_load[0].empty() || fn_load[1].empty())
    {
      CORE_LOG_ERROR("must specify 2 files with the -load option");
      return false;
    }

    if (fn_save.empty())
    {
      CORE_LOG_ERROR("must specify a file to open with the -save option");
      return false;
    }

    // dump the status:
    std::cout << "shrink factor: " << this->shrink_factor_ << std::endl
    //  << "transform order: " << this->transform_order_ << std::endl
    << "descriptor: v" << this->descriptor_version_ << std::endl
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
                    0,
                    fn_load[0],
                    mosaic[0],
                    mosaic_mask[0],
                    this->descriptor_version_,
                    3,
                    true,
                    fn_debug);
    }
    else if (fn_load[0].extension() == ".pyramid")
    {
      // load the pyramid:
      load_pyramid(fn_load[0], pyramid[0], mosaic[0], mosaic_mask[0]);

      std::cout << "    generating keys: ";
      pyramid[0].generate_keys();
      std::cout << pyramid[0].count_keys() << std::endl;

      std::cout << "    generating descriptors" << std::endl;
      pyramid[0].generate_descriptors(this->descriptor_version_);
    }
    else
    {
      // load first slice:
      mosaic[0] = std_tile<image_t>(fn_load[0],
                                    this->shrink_factor_,
                                    this->pixel_spacing_);

      if (! fn_mask[0].empty())
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
                    0,
                    fn_load[0],
                    mosaic[0],
                    mosaic_mask[0],
                    this->descriptor_version_,
                    3,
                    true, // generate descriptors?
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
                    1,
                    fn_load[1],
                    mosaic[1],
                    mosaic_mask[1],
                    this->descriptor_version_,
                    3,
                    true,
                    fn_debug);
    }
    else if (fn_load[1].extension() == ".pyramid")
    {
      // load the pyramid:
      load_pyramid(fn_load[1], pyramid[1], mosaic[1], mosaic_mask[1]);

      std::cout << "    generating keys: ";
      pyramid[1].generate_keys();
      std::cout << pyramid[1].count_keys() << std::endl;

      std::cout << "    generating descriptors" << std::endl;
      pyramid[1].generate_descriptors(this->descriptor_version_);
    }
    else
    {
      // load second slice:
      mosaic[1] = std_tile<image_t>(fn_load[1],
                                    this->shrink_factor_,
                                    this->pixel_spacing_);

      if (! fn_mask[1].empty())
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
                    1,
                    fn_load[1],
                    mosaic[1],
                    mosaic_mask[1],
                    this->descriptor_version_,
                    3,
                    true, // generate descriptors?
                    fn_debug);
    }

    pyramid[0].debug(fn_debug.string() + "p0-");
    pyramid[1].debug(fn_debug.string() + "p1-");

    // solve for the transform mapping between the adjacent slices:
    order2_transform_t::Pointer t_coarse =
    match<order2_transform_t>(order2_transform_t::Degree + 1,
                              // std::min(2u, transform_order),
                              pyramid[0],
                              pyramid[1],
                              fn_debug);

    // calculate the octave range for initial transform estimation
    // (low-order, or constrained high-order):
    int octave_start = std::min(pyramid[0].octaves(),
                                pyramid[1].octaves()) - 1;
    // int octave_end = 0;
    int octave_end = std::max(octave_start + 1 -
                              std::min(3, static_cast<int>(std::min(pyramid[0].octaves(),
                                                                    pyramid[1].octaves()))),
                              0);

    // try to minimize the mismatch via energy minimization:
    order4_transform_t::Pointer t_order4;
    refine_one_pair(octave_start,
                    octave_start - octave_end + 1,
                    pyramid[0],
                    pyramid[1],
                    t_coarse,
                    t_order4,
                    500,
                    1e-12,
                    1e-3, // FIXME: 1e-6
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
                                          500,
                                          1e-12,
                                          1e-3, // FIXME: 1e-6
                                          scales,
                                          fn_debug.string() + "final-");
    }

    // save the slice-to-slice transform:
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
                       t_order4,
                       true);

    CORE_LOG_SUCCESS("ir-stos done");

    // done:
    return true;
  }
  catch (bfs::filesystem_error &err)
  {
    CORE_LOG_ERROR(err.what());
  }
  catch (itk::ExceptionObject &err)
  {
    CORE_LOG_ERROR(err.GetDescription());
  }
  catch (Core::Exception &err)
  {
    CORE_LOG_ERROR(err.what());
    CORE_LOG_ERROR(err.message());
  }
  catch (std::exception &err)
  {
    CORE_LOG_ERROR(err.what());
  }
  catch (...)
  {
    CORE_LOG_ERROR("Unknown exception type caught.");
  }
  return false;
}


void
ActionSliceToSliceFilter::Dispatch(Core::ActionContextHandle context,
                                   std::string target_layer,
                                   unsigned int shrink_factor,
                                   unsigned int descriptor_version,
                                   double pixel_spacing,
                                   double clahe_slope,
                                   bool use_standard_mask,
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
  ActionSliceToSliceFilter* action = new ActionSliceToSliceFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->shrink_factor_ = shrink_factor;
  action->pixel_spacing_ = pixel_spacing;
  action->clahe_slope_ = clahe_slope;
  action->use_standard_mask_ = use_standard_mask;
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
