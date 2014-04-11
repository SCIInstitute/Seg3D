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

#include <Application/ImageRegistrationTools/Actions/ActionRefineGridFilter.h>

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/mosaic_refinement_common.hxx>
#include <Core/ITKCommon/Optimizers/itkImageMosaicVarianceMetric.h>
#include <Core/ITKCommon/Optimizers/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/Transform/itkRadialDistortionTransform.h>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Transform/itkMeshTransform.h>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>
#include <Core/ITKCommon/the_dynamic_array.hxx>

// system includes:
#include <math.h>
#include <sstream>

// ITK includes:
#include <itkDiscreteGaussianImageFilter.h>
#include <itkShrinkImageFilter.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;


CORE_REGISTER_ACTION( Seg3D, RefineGridFilter )

namespace Seg3D
{
  

//----------------------------------------------------------------
// SAVE_INTERMEDIATE_MOSAIC
//
#define SAVE_INTERMEDIATE_MOSAIC


//----------------------------------------------------------------
// optimizer_t
//
typedef itk::RegularStepGradientDescentOptimizer2 optimizer_t;


//----------------------------------------------------------------
// optimization_observer_t
//
template<class mosaic_metric_t>
class optimization_observer_t : public itk::Command
{
public:
  typedef optimization_observer_t Self;
  typedef itk::Command Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  
  itkNewMacro(Self);
  
  void Execute(itk::Object *caller, const itk::EventObject & event)
  { Execute((const itk::Object *)(caller), event); }
  
  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    if (typeid(event) != typeid(itk::IterationEvent)) return;
    
    const optimizer_t * optimizer = dynamic_cast<const optimizer_t *>(object);
    std::cout << static_cast<unsigned int>(optimizer->GetCurrentIteration()) << '\t'
    << optimizer->GetValue() << '\t'
    << metric_->GetNumberOfPixelsCounted() << '\t'
    << optimizer->GetCurrentPosition() << std::endl;
    
    // FIXME: 2005/11/09
#ifdef SAVE_INTERMEDIATE_MOSAIC
    {
      static unsigned int counter = 0;

      std::ostringstream fn;
      fn << prefix_ << the_text_t::number(counter, 3, '0')  << ".mosaic";
      std::fstream fout;
      fout.open(fn.str().c_str(), ios::out);
      save_mosaic<base_transform_t>(fout,
                                    this->pixel_spacing_,
                                    this->use_standard_mask_,
                                    in_,
                                    transform_);
      fout.close();
      counter++;
    }
#endif
  }
  
  // FIXME: 2005/11/09
  typename mosaic_metric_t::Pointer metric_;
  std::list<bfs::path> in_;
  double pixel_spacing_;
  bool use_standard_mask_;
  std::vector<base_transform_t::Pointer> transform_;
  bfs::path prefix_;

protected:
  optimization_observer_t() {}
};


//----------------------------------------------------------------
// radial_distortion_t
//
typedef itk::RadialDistortionTransform<double, 2>
radial_distortion_t;

//----------------------------------------------------------------
// affine_transform_t
//
typedef itk::LegendrePolynomialTransform<double, 1>
affine_transform_t;

//----------------------------------------------------------------
// cubic_transform_t
//
typedef itk::LegendrePolynomialTransform<double, 3>
cubic_transform_t;

//----------------------------------------------------------------
// image_interpolator_t
//
typedef itk::LinearInterpolateImageFunction<image_t, double> image_interpolator_t;


bool
ActionRefineGridFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionRefineGridFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(3);
    std::cout.setf(std::ios::scientific);
    
    track_progress(true);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);
    
    bool pixel_spacing_user_override = false;
    if (this->pixel_spacing_ != 1.0) // TODO: improve this check
    {
      pixel_spacing_user_override = true;
    }
    bfs::path fn_results(this->output_mosaic_file_);
    
    bfs::path fn_in(this->input_mosaic_file_);
    bfs::path image_dir(this->directory_);
    
    // TODO: expose?
    bool verbose = true;
    
    // by default run as many threads as there are cores:
    //  unsigned int num_threads = boost::thread::hardware_concurrency();
    if (this->num_threads_ == 0)
    {
      this->num_threads_ = boost::thread::hardware_concurrency();
    }
    
    fftwf_init_threads();
    itk_fft::set_num_fftw_threads(1);
    
    std::list<bfs::path> in;
    std::vector<base_transform_t::Pointer> tbase;
    std::vector<image_t::Pointer> image;
    std::vector<mask_t::ConstPointer> mask;
    
    std::fstream fin;
    fin.open(fn_in.c_str(), ios::in);
    if (!fin.is_open())
    {
      std::ostringstream oss;
      oss << "Could not open " << fn_in << " for reading.";
      CORE_LOG_ERROR(oss.str());
      return false;
    }
    
    double mosaic_pixel_spacing = 0;
    load_mosaic<base_transform_t>(fin,
                                  mosaic_pixel_spacing,
                                  this->use_standard_mask_,
                                  in,
                                  tbase,
                                  image_dir);
    if (! pixel_spacing_user_override)
    {
      this->pixel_spacing_ = mosaic_pixel_spacing;
    }
    
    unsigned int num_images = tbase.size();
  //  assert(pixel_spacing != 0);
    if (this->pixel_spacing_ == 0)
    {
      CORE_LOG_ERROR("Pixel spacing is zero");
      return false;
    }
    
    image.resize(num_images);
    mask.resize(num_images);
    
    unsigned int i = 0;
    for (std::list<bfs::path>::const_iterator iter = in.begin();
         iter != in.end(); ++iter, ++i)
    {
      const bfs::path & fn_image = *iter;
      
      // read the image:
      std::cout << std::setw(3) << i << " ";
      image[i] = std_tile<image_t>(fn_image, this->shrink_factor_, this->pixel_spacing_);
      // TODO: Anytime we do anything with itk we really should be at least
      //       catching their itk::ExceptionObject.  Heck we could even
      //       wrap the entire tool in a try catch block, it's better than
      //       nothing.
      
      if (this->use_standard_mask_)
      {
        mask[i] = std_mask<image_t>(image[i]);
      }
    }
    
    fin.close();
    
    set_major_progress(0.05);
    
    // a minimum of two images are required for the registration:
    num_images = in.size();
    if (num_images == 0)
    {
      std::cout << "No images passed the test, nothing to save out." << std::endl;
    }
    
    if (this->cell_size_ == 0 && (this->mesh_rows_ == 0 || this->mesh_cols_ == 0))
    {
  //    usage("you must specify either the -cell or -mesh parameter");
      CORE_LOG_ERROR("Set either cell_size or mesh parameters (mesh_rows and mesh_cols).");
      return false;
    }
    else if (this->cell_size_ == 0)
    {
      // calculate the cell size from the rows/cols:
      image_t::SizeType sz = image[0]->GetLargestPossibleRegion().GetSize();
      unsigned int nx = 3 * sz[0] / (this->mesh_cols_ - 1);
      unsigned int ny = 3 * sz[1] / (this->mesh_rows_ - 1);
      this->cell_size_ = std::max(nx, ny);
    }
    else if (this->mesh_rows_ == 0 || this->mesh_cols_ == 0)
    {
      // calculate the cell size from the rows/cols:
      image_t::SizeType sz = image[0]->GetLargestPossibleRegion().GetSize();
      this->mesh_cols_ = 1 + 3 * sz[0] / this->cell_size_;
      this->mesh_rows_ = 1 + 3 * sz[1] / this->cell_size_;
    }
    
    // get the image bounding boxes:
    std::vector<pnt2d_t> tile_min(num_images);
    std::vector<pnt2d_t> tile_max(num_images);
    calc_image_bboxes<image_t::Pointer>(image, tile_min, tile_max);
    
    // setup the mesh transforms:
    std::vector<itk::GridTransform::Pointer> transform(num_images);
    for (unsigned int i = 0; i < num_images; i++)
    {
      transform[i] = itk::GridTransform::New();
      bool ok = setup_grid_transform(transform[i]->transform_,
                                     this->mesh_rows_ - 1, // grid rows
                                     this->mesh_cols_ - 1, // grid cols
                                     tile_min[i],
                                     tile_max[i],
                                     mask[i].GetPointer(),
                                     tbase[i].GetPointer(),
                                     MAX_ITERATIONS,
                                     MIN_STEP_SCALE,
                                     MIN_ERROR_SQ,
                                     PICKUP_PACE_STEPS);
      if (!ok)
      {
  //      assert(false);
        CORE_LOG_ERROR("setup_grid_transform failed");
        return false;
      }
      transform[i]->setup(transform[i]->transform_);
      
      set_minor_progress(static_cast<double>(i)/static_cast<double>(num_images), 0.15);
    }
    
    set_major_progress(0.15);
    
    std::cout << "shrink factor: " << this->shrink_factor_ << std::endl
    << "pixel spacing: " << this->pixel_spacing_ << std::endl
    << "iterations:    " << this->iterations_ << std::endl
    << "neighborhood:  " << this->cell_size_ << std::endl
    << "mesh rows:     " << this->mesh_rows_ << std::endl
    << "mesh cols:     " << this->mesh_cols_ << std::endl
    << "threads:       " << this->num_threads_ << std::endl
    << std::endl;
    
    image_t::SizeType image_sz = image[0]->GetLargestPossibleRegion().GetSize();
    
    refine_mosaic_mt<image_t::Pointer, mask_t::ConstPointer>(transform,
                                                             image,
                                                             mask,
                                                             this->cell_size_,
                                                             PREWARP_TILES,
                                                             MIN_OVERLAP,
                                                             DEFAULT_MEDIAN_FILTER_RADIUS,
                                                             this->iterations_, // number of refinement passes
                                                             MOVE_ALL_TILES,
                                                             this->displacement_threshold_,
                                                             this->num_threads_); // number of threads to launch
    
    set_major_progress(0.95);
    
    // save the results:
    std::fstream fout;
    fout.open(fn_results.c_str(), ios::out);
    
    if (!fout.is_open())
    {
      std::ostringstream oss;
      oss << "Could not open " << fn_results << " for writing.";
      CORE_LOG_ERROR(oss.str());
      return false;
    }
    
    save_mosaic<itk::GridTransform>(fout,
                                    this->pixel_spacing_,
                                    this->use_standard_mask_,
                                    in,
                                    transform);
    
    fout.close();
    
    set_major_progress(1.0);
    
    CORE_LOG_SUCCESS("ir-refine-grid done");
    
    // done:
    return true;
  }
  catch (...)
  {
    CORE_LOG_ERROR("Exception caught");
    return false;
  }
}


void
ActionRefineGridFilter::Dispatch(Core::ActionContextHandle context,
                                 std::string target_layer,
                                 unsigned int shrink_factor,
                                 unsigned int num_threads,
                                 unsigned int iterations,
                                 unsigned int cell_size,
                                 unsigned int mesh_rows,
                                 unsigned int mesh_cols,
                                 double pixel_spacing,
                                 double displacement_threshold,
                                 bool use_standard_mask,
                                 std::string input_mosaic_file,
                                 std::string output_mosaic_file,
                                 std::string directory)
{
  // Create a new action
  ActionRefineGridFilter* action = new ActionRefineGridFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->input_mosaic_file_ = input_mosaic_file;
  action->output_mosaic_file_ = output_mosaic_file;
  action->directory_ = directory;
  action->shrink_factor_ = shrink_factor;
  action->num_threads_ = num_threads;
  action->iterations_ = iterations;
  action->cell_size_ = cell_size;
  action->mesh_rows_ = mesh_rows;
  action->mesh_cols_ = mesh_cols;
  action->pixel_spacing_ = pixel_spacing;
  action->displacement_threshold_ = displacement_threshold;
  action->use_standard_mask_ = use_standard_mask;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

  
}