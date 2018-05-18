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

//ITK Includes
#include <itkTranslationTransform.h>
#include <itkEuler3DTransform.h>
#include <itkRigid3DTransform.h>
#include <itkVersorTransform.h>
#include <itkEuclideanDistancePointMetric.h>
#include <itkLevenbergMarquardtOptimizer.h>
#include <itkPointSetToPointSetRegistrationMethod.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkPointSetToImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkCommand.h>


// Core includes
#include <Core/Math/MathFunctions.h>
#include <Core/Geometry/Point.h>
#include <Core/State/StateVectorHelper.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionPointSetRegisterFilter.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/PointSetFilter.h>

#include <boost/lexical_cast.hpp>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, PointSetRegisterFilter )

namespace Seg3D
{

bool ActionPointSetRegisterFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, Core::VolumeType::MASK_E, 
    context, this->sandbox_ ) ) return false;

  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, false, 
    context, this->sandbox_ ) ) return false;
  
  // Check for layer existence and type information mask layer
  if ( ! LayerManager::CheckLayerExistenceAndType( this->mask_layer_, Core::VolumeType::MASK_E,
    context, this->sandbox_ ) ) return false;

  if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_, 
    false, context, this->sandbox_ ) ) return false;
  
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->iterations_ < 1 )
  {
    context->report_error( "The number of iterations needs to be larger than zero." );
    return false;
  }

  // Validation successful
  return true;
}


template <class TScalarType>
class Rigid3DTransformSurrogate : public itk::Rigid3DTransform < TScalarType >
{
public:
  /** Standard class typedefs. */
  typedef Rigid3DTransformSurrogate        Self;
  typedef itk::Rigid3DTransform< TScalarType >  Superclass;
  typedef itk::SmartPointer< Self >             Pointer;
  typedef itk::SmartPointer< const Self >       ConstPointer;

  itkNewMacro(Self);

private:
  Rigid3DTransformSurrogate() {}
  ~Rigid3DTransformSurrogate() {}

};

typedef itk::Euler3DTransform< double > transform_type;

//Calculate Transformation Matrix
std::vector<double> calculateTransformationMatrix(transform_type::Pointer transform)
{
	std::vector<double> transformation(16, 0.0);
	transformation[15] = 1.0;

	//Fill matrix
	int inx = 0;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{ 
			transformation[inx] = transform->GetMatrix()[i][j];
			inx++;
		}
		transformation[inx] = transform->GetTranslation()[i];
		inx++;
	}
	return transformation;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

//class PointSetFilterAlgo : public LayerFilter, ITKFilter
class PointSetFilterAlgo : public  ITKFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

  int iterations_;
  std::string transform_state_id_, complete_transform_state_id_;

public:
  // RUN:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.

  bool check_abort()
  {
    return false;
  }

  SCI_BEGIN_TYPED_ITK_RUN( this->src_layer_->get_data_type() )
  {
    MaskLayerHandle input_fixed_layer = 
      boost::dynamic_pointer_cast<MaskLayer>( this->src_layer_ );
    MaskLayerHandle input_moving_layer = 
      boost::dynamic_pointer_cast<MaskLayer>( this->mask_layer_ );
    
    double quality_factor = 1.0;
    Core::IsosurfaceHandle fixed_iso;
    Core::IsosurfaceHandle moving_iso;
    std::vector< Core::PointF > fixed_iso_points_set;
    std::vector< Core::PointF > moving_iso_points_set;

    bool sample_ready = false;

    while ( !sample_ready )
    {
      fixed_iso.reset( new Core::Isosurface( input_fixed_layer->get_mask_volume() ) );
      fixed_iso->compute( quality_factor, false, boost::bind( &PointSetFilterAlgo::check_abort, this ) );

      moving_iso.reset( new Core::Isosurface( input_moving_layer->get_mask_volume() ) );
      moving_iso->compute( quality_factor, false, boost::bind( &PointSetFilterAlgo::check_abort, this ) );


      fixed_iso_points_set = fixed_iso->get_points();
      moving_iso_points_set = moving_iso->get_points();

      unsigned long fixed_point_set_size = fixed_iso_points_set.size();
      unsigned long moving_point_set_size = moving_iso_points_set.size();

      if ( fixed_point_set_size >= 2000 && moving_point_set_size >= 2000 )
      {
        if ( quality_factor == 0.125 )
        {
          sample_ready = true;
          break;
        }
        else
        {
          quality_factor *= 0.5;
        }
      }
      else
      {
        sample_ready = true;
      }
    }

    
    //-----------------------------------------------------------
    // Set up  the Metric
    //-----------------------------------------------------------
    const unsigned int DIMENSION_C = 3;
    typedef itk::PointSet< float, DIMENSION_C >  point_set_type;

    point_set_type::Pointer fixed_point_set  = point_set_type::New();
    point_set_type::Pointer moving_point_set = point_set_type::New();

    typedef point_set_type::PointType     point_type;
    typedef point_set_type::PointsContainer  points_container_type;

    points_container_type::Pointer fixed_point_container  = points_container_type::New();
    points_container_type::Pointer moving_point_container = points_container_type::New();


    unsigned int point_id = 0;

    std::vector< Core::PointF >::const_iterator cii;
    for( cii=fixed_iso_points_set.begin(); 
      cii!=fixed_iso_points_set.end(); cii++ )
    {
      point_type fixed_point;
      fixed_point[0] = (*cii).x();
      fixed_point[1] = (*cii).y();
      fixed_point[2] = (*cii).z();

      fixed_point_container->InsertElement( point_id++, fixed_point );
    }

    fixed_point_set->SetPoints( fixed_point_container );
    unsigned long fixed_points_num = fixed_point_set->GetNumberOfPoints(); 
    CORE_LOG_MESSAGE( std::string("Number of fixed Points = ") + 
      boost::lexical_cast<std::string>(fixed_points_num) );

    point_id = 0;

    for( cii=moving_iso_points_set.begin(); cii!=moving_iso_points_set.end(); cii++ )
    {
      point_type moving_point;
      moving_point[0] = (*cii).x();
      moving_point[1] = (*cii).y();
      moving_point[2] = (*cii).z();

      moving_point_container->InsertElement( point_id++, moving_point );
    }

    moving_point_set->SetPoints( moving_point_container );
    unsigned long moving_points_num = moving_point_set->GetNumberOfPoints(); 
    CORE_LOG_MESSAGE( std::string("Number of moving Points = ") + 
      boost::lexical_cast< std::string >( moving_points_num ) );

    typedef itk::EuclideanDistancePointMetric< point_set_type, point_set_type > metric_type;
    typedef metric_type::TransformType                transform_base_type;
    typedef transform_base_type::ParametersType       parameters_type;
    typedef transform_base_type::JacobianType         jacobian_type;

    metric_type::Pointer  metric = metric_type::New();

    //-----------------------------------------------------------
    // Set up a Transform
    //-----------------------------------------------------------

    transform_type::Pointer transform = transform_type::New();

    // Optimizer Type
    typedef itk::LevenbergMarquardtOptimizer optimizer_type;

    optimizer_type::Pointer      optimizer     = optimizer_type::New();
    optimizer->SetUseCostFunctionGradient(false);


    // Registration Method
    typedef itk::PointSetToPointSetRegistrationMethod<
      point_set_type,
      point_set_type >
      registration_type;


    registration_type::Pointer   registration  = registration_type::New();
    this->forward_abort_to_filter( registration, this->dst_layer_ );

    // Scale the translation components of the Transform in the Optimizer
    optimizer_type::ScalesType scales( transform->GetNumberOfParameters() );
    
    const double TRANSLATION_SCALE_C = 1000.0; // dynamic range of translations
    const double ROTATION_SCALE_C = 1.0; // dynamic range of rotations
    scales[0] = 1.0 / ROTATION_SCALE_C;
    scales[1] = 1.0 / ROTATION_SCALE_C;
    scales[2] = 1.0 / ROTATION_SCALE_C;
    scales[3] = 1.0 / TRANSLATION_SCALE_C;
    scales[4] = 1.0 / TRANSLATION_SCALE_C;
    scales[5] = 1.0 / TRANSLATION_SCALE_C;

    unsigned long   number_of_iterations = this->iterations_;
    double          gradient_tolerance  =  1e-5;    // convergence criterion
    double          value_tolerance     =  1e-5;    // convergence criterion
    double          epsilon_function    =  1e-6;   // convergence criterion


    optimizer->SetScales( scales );
    optimizer->SetNumberOfIterations( number_of_iterations );
    optimizer->SetValueTolerance( value_tolerance );
    optimizer->SetGradientTolerance( gradient_tolerance );
    optimizer->SetEpsilonFunction( epsilon_function );

    // Start from an Identity transform (in a normal case, the user
    // can probably provide a better guess than the identity...
    transform->SetIdentity();

    registration->SetInitialTransformParameters( transform->GetParameters() );

    //------------------------------------------------------
    // Connect all the components required for Registration
    //------------------------------------------------------
    registration->SetMetric(        metric        );
    registration->SetOptimizer(     optimizer     );
    registration->SetTransform(     transform     );
    registration->SetFixedPointSet( fixed_point_set );
    registration->SetMovingPointSet( moving_point_set );

    // Retrieve the image as an itk image from the underlying data structure
    // NOTE: This only does wrapping and does not regenerate the data.
    
    this->forward_abort_to_filter( registration, this->dst_layer_ );

    double progress_unit =  1.0 / number_of_iterations ;
    this->observe_itk_iterations( optimizer, boost::bind( 
      &PointSetFilterAlgo::update_iteration, this, _1, progress_unit ) );

    this->progress_ = 0.0;
  
    // Ensure we will have some threads left for doing something else
    this->limit_number_of_itk_threads( registration );

    // Run the actual ITK filter.
    // This needs to be in a try/catch statement as certain filters throw exceptions when they
    // are aborted. In that case we will relay a message to the status bar for information.
    try 
    { 
      registration->Update(); 
    } 
    catch ( ... ) 
    {
      if ( this->check_abort() )
      {
        this->report_error( "Filter was aborted." );
        return;
      }

      this->report_error( "ITK filter failed to complete." );
      return;
    }

    // As ITK filters generate an inconsistent abort behavior, we record our own abort flag
    // This one is set when the abort button is pressed and an abort is sent to ITK.
    if ( this->check_abort() ) return;

    registration_type::ParametersType final_parameters = 
      registration->GetLastTransformParameters();

    std::string solution = "[" + boost::lexical_cast<std::string>(final_parameters[0]) +
      "," + boost::lexical_cast<std::string>(final_parameters[1]) +"," +
      boost::lexical_cast<std::string>(final_parameters[2]) + "," + 
      boost::lexical_cast<std::string>(final_parameters[3]) + "," +
      boost::lexical_cast<std::string>(final_parameters[4]) + "," +
      boost::lexical_cast<std::string>(final_parameters[5]) + "]"; 
    CORE_LOG_MESSAGE( std::string("Solution = ") + 
      solution );

	std::vector< double > matrix_entries(final_parameters.begin(), final_parameters.end());

    transform_type::Pointer final_transform = transform_type::New();
    final_transform->SetParameters( final_parameters );

    typedef itk:: LinearInterpolateImageFunction<
      TYPED_IMAGE_TYPE,
      double          >    interpolator_type;

    typedef itk::ResampleImageFilter<
      TYPED_IMAGE_TYPE,
      TYPED_IMAGE_TYPE > resample_filter_type;

    typename resample_filter_type::Pointer resampler = resample_filter_type::New();
    typename Core::ITKImageDataT<VALUE_TYPE>::Handle moving_image; 
    this->get_itk_image_from_layer<VALUE_TYPE>( this->mask_layer_, moving_image );
    resampler->SetInput( moving_image->get_image() );
  
    final_transform->GetInverse( final_transform );
    resampler->SetTransform( final_transform );

    typename Core::ITKImageDataT<VALUE_TYPE>::Handle fixed_image;
    this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, fixed_image );
    resampler->SetSize( fixed_image->get_image()->GetLargestPossibleRegion().GetSize() );
    resampler->SetOutputOrigin(  fixed_image->get_image()->GetOrigin() );
    resampler->SetOutputSpacing( fixed_image->get_image()->GetSpacing() );
    resampler->SetOutputDirection( fixed_image->get_image()->GetDirection() );
    resampler->SetDefaultPixelValue( 0 );

    resampler->Update();

    if ( resampler->GetOutput() != NULL )
    {
      this->insert_itk_image_into_layer( this->dst_layer_, resampler->GetOutput() );

      // Only output the state variable value when not in a sandbox
      if ( this->get_sandbox() == -1 )
      {
		  Core::setStateVector(transform_state_id_, matrix_entries);

		  auto transformation = calculateTransformationMatrix(transform);
		  Core::setStateVector(complete_transform_state_id_, transformation);
      }
    }
  }
  SCI_END_TYPED_ITK_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "PointSet Register Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "PointSetRegister";  
  }

private:
  double progress_;

  // UPDATE_ITERATION:
  // At regular intervals update the results to the user
  void update_iteration( itk::Object* itk_object, double progress_unit )
  {
    progress_ += progress_unit;
    this->dst_layer_->update_progress_signal_( progress_ );
  }

  class CommandIterationUpdate : public itk::Command
  {
  public:
    typedef  CommandIterationUpdate   Self;
    typedef  itk::Command             Superclass;
    typedef itk::SmartPointer<Self>   Pointer;
    itkNewMacro( Self );
  protected:
    CommandIterationUpdate() {};

  private:
     LayerHandle layer_;
     double progress_start_;
     double progress_unit_;

  public:
    typedef itk::LevenbergMarquardtOptimizer OptimizerType;
    typedef   const OptimizerType *              OptimizerPointer;

    void SetLayer( LayerHandle layer )
    {
      this->layer_ = layer;
    }

    void SetProgressUnit( double progress_unit )
    {
      this->progress_unit_ = progress_unit;
    }

    void SetProgressStart( double progress_start )
    {
      this->progress_start_ = progress_start;
    }

    void Execute(itk::Object *caller, const itk::EventObject & event)
    {
      Execute( (const itk::Object *)caller, event);
    }

    void Execute(const itk::Object * object, const itk::EventObject & event)
    {
      OptimizerPointer optimizer =
        dynamic_cast< OptimizerPointer >( object );

      if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
        return;
      }

      progress_start_ += progress_unit_;
      layer_->update_progress_signal_( progress_start_ );


      CORE_LOG_MESSAGE( std::string("value = ") + 
        boost::lexical_cast<std::string>( optimizer->GetValue() ));

      CORE_LOG_MESSAGE( std::string("params = ") + 
        boost::lexical_cast<std::string>( optimizer->GetCurrentPosition() ));

    }
  };

};


bool ActionPointSetRegisterFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<PointSetFilterAlgo> algo( new PointSetFilterAlgo );

  // Find the handle to the layer
  algo->set_sandbox( this->sandbox_ );
  algo->src_layer_ = LayerManager::FindLayer( this->target_layer_, this->sandbox_ );
  algo->mask_layer_ = LayerManager::FindLayer( this->mask_layer_, this->sandbox_ );
  algo->iterations_ = this->iterations_;
  algo->transform_state_id_ = this->transform_state_id_;
  algo->complete_transform_state_id_ = this->complete_transform_state_id_;

  
  // Check whether the source layer was found
  if ( !algo->src_layer_ || !algo->mask_layer_ ) return false;
  
  // Lock the mask layer, so no other layer can access it
  algo->lock_for_use( algo->mask_layer_ );
  algo->lock_for_use( algo->src_layer_ );

  switch ( algo->src_layer_->get_type() )
  { 
  case Core::VolumeType::DATA_E:
    algo->create_and_lock_data_layer_from_layer( 
      algo->src_layer_, algo->dst_layer_  );
    break;
  case Core::VolumeType::MASK_E:
    algo->create_and_lock_mask_layer_from_layer( 
      algo->src_layer_, algo->dst_layer_ , algo->mask_layer_->get_layer_name() );
    break;
  default:
    assert( false );
  }
  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record for this action
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}


void ActionPointSetRegisterFilter::Dispatch( Core::ActionContextHandle context,
  const std::string& target_layer, const std::string& mask_layer,  
  int iterations, const std::string& transform_state_id, const std::string& complete_transform_state_id)
{ 
  // Create a new action
  ActionPointSetRegisterFilter* action = new ActionPointSetRegisterFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->mask_layer_ = mask_layer;
  action->transform_state_id_ = transform_state_id;
  action->complete_transform_state_id_ = complete_transform_state_id;
  action->iterations_ = iterations;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


  
} // end namespace Seg3D
