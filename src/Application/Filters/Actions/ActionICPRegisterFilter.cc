  /*
   For more information, please see: http://software.sci.utah.edu
   
   The MIT License
   
   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
  #include <itkEuclideanDistancePointMetric.h>
  #include <itkLevenbergMarquardtOptimizer.h>
  #include <itkPointSetToPointSetRegistrationMethod.h>
  #include <itkResampleImageFilter.h>

  // Core includes
  #include <Core/Math/MathFunctions.h>
  #include <Core/Geometry/Point.h>

  // Application includes
  #include <Application/LayerManager/LayerManager.h>
  #include <Application/StatusBar/StatusBar.h>
  #include <Application/Filters/LayerFilter.h>
  #include <Application/Filters/Actions/ActionICPRegisterFilter.h>
  #include <Application/Filters/ITKFilter.h>
  #include <Application/ToolManager/ToolManager.h>
  #include <Application/Tools/ICPFilter.h>

  #include <boost/lexical_cast.hpp>

  // REGISTER ACTION:
  // Define a function that registers the action. The action also needs to be
  // registered in the CMake file.
  // NOTE: Registration needs to be done outside of any namespace
  CORE_REGISTER_ACTION( Seg3D, ICPRegisterFilter )

  namespace Seg3D
  {

  bool ActionICPRegisterFilter::validate( Core::ActionContextHandle& context )
  {
    // Check for layer existance and type information
    if ( ! LayerManager::CheckLayerExistanceAndType( this->target_layer_, Core::VolumeType::MASK_E, 
      context ) ) return false;
    
    // Check for layer availability 
    //if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, this->replace_, 
    //  context ) ) return false;

    if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, false, 
      context ) ) return false;
    
    // Check for layer existance and type information mask layer
    if ( ! LayerManager::CheckLayerExistanceAndType( this->mask_layer_, Core::VolumeType::MASK_E,
      context ) ) return false;

    // Check whether mask and data have the same size
    //if ( ! LayerManager::CheckLayerSize( this->mask_layer_, this->target_layer_,
    //  context ) ) return false;
      
    // Check for layer availability mask layer
    //if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_, 
    //  this->replace_, context ) ) return false;


    if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_, 
      false, context ) ) return false;
    
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

  // ALGORITHM CLASS
  // This class does the actual work and is run on a separate thread.
  // NOTE: The separation of the algorithm into a private class is for the purpose of running the
  // filter on a separate thread.

  //class ICPFilterAlgo : public LayerFilter, ITKFilter
  class ICPFilterAlgo : public  ITKFilter
  {

  public:
    LayerHandle src_layer_;
    LayerHandle mask_layer_;
    LayerHandle dst_layer_;

    int iterations_;
    std::string icp_tool_id_;

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

      //Estimate the quality factor from the fraction from 1/0
      /*double target_layer_size  =
         input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_size();

      double voxel_count = 0;
      {
        Core::MaskDataBlock::shared_lock_type lock( 
          input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_mutex() );

        
        for( size_t j = 0; 
          j < input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_size(); ++j )
        {
          if( input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_mask_at( j ) )
          {
            voxel_count++;
          }
        }
      }

      double fraction_target_layer = voxel_count / target_layer_size;


      double moving_layer_size  =
        input_moving_layer->get_mask_volume()->get_mask_data_block()->get_size();

      voxel_count = 0;
      {
        Core::MaskDataBlock::shared_lock_type lock( 
          input_moving_layer->get_mask_volume()->get_mask_data_block()->get_mutex() );


        for( size_t j = 0; 
          j < input_moving_layer->get_mask_volume()->get_mask_data_block()->get_size(); ++j )
        {
          if( input_moving_layer->get_mask_volume()->get_mask_data_block()->get_mask_at( j ) )
          {
            voxel_count++;
          }
        }
      }
      
      double fraction_moving_layer = voxel_count / moving_layer_size;

      double fraction = (fraction_target_layer > fraction_moving_layer) ? fraction_target_layer : fraction_moving_layer;

      double quality_factor = 0.125;

      if ( fraction < 0.125 )
      {
        quality_factor = 1.0;
      }
      else if ( fraction > 0.125 && fraction < 0.5 )
      {
        quality_factor = 0.5;
      }
      else if ( fraction > 0.5 && fraction < 0.75 )
      {
        quality_factor = 0.25;
      }
      else
      {
        quality_factor = 0.125;
      }*/


      //Core::IsosurfaceHandle fixed_iso = 
      //  input_fixed_layer->get_isosurface();

      double quality_factor = 0.125;
      
      Core::IsosurfaceHandle fixed_iso;
      fixed_iso.reset( new Core::Isosurface( input_fixed_layer->get_mask_volume() ) );
      fixed_iso->compute( quality_factor, false, boost::bind( &ICPFilterAlgo::check_abort, this ) );


      //Core::IsosurfaceHandle moving_iso = 
      //  input_moving_layer->get_isosurface();

      Core::IsosurfaceHandle moving_iso;
      moving_iso.reset( new Core::Isosurface( input_moving_layer->get_mask_volume() ) );
      moving_iso->compute( quality_factor, false, boost::bind( &ICPFilterAlgo::check_abort, this ) );


      std::vector< Core::PointF > fixed_iso_points_set = 
        fixed_iso->get_points();

      std::vector< Core::PointF > moving_iso_points_set = 
        moving_iso->get_points();
      //-----------------------------------------------------------
      // Set up  the Metric
      //-----------------------------------------------------------
      const unsigned int Dimension = 3;
      typedef itk::PointSet< float, Dimension >   PointSetType;

      PointSetType::Pointer fixedPointSet  = PointSetType::New();
      PointSetType::Pointer movingPointSet = PointSetType::New();

      typedef PointSetType::PointType     PointType;
      typedef PointSetType::PointsContainer  PointsContainer;

      PointsContainer::Pointer fixedPointContainer  = PointsContainer::New();
      PointsContainer::Pointer movingPointContainer = PointsContainer::New();


      unsigned int pointId = 0;

      std::vector<Core::PointF>::const_iterator cii;
      for(cii=fixed_iso_points_set.begin(); 
        cii!=fixed_iso_points_set.end(); cii++)
      {
        std::cout << *cii << std::endl;
        PointType fixedPoint;
        fixedPoint[0] = (*cii).x();
        fixedPoint[1] = (*cii).y();
        fixedPoint[2] = (*cii).z();

        fixedPointContainer->InsertElement( pointId++, fixedPoint );
      }

      fixedPointSet->SetPoints( fixedPointContainer );
      unsigned long fixed_points_num = fixedPointSet->GetNumberOfPoints(); 
      CORE_LOG_MESSAGE( std::string("Number of fixed Points = ") + 
        boost::lexical_cast<std::string>(fixed_points_num) );
      //std::cout << "Number of fixed Points = "
      //  << fixedPointSet->GetNumberOfPoints() << std::endl;

      pointId = 0;

      for(cii=moving_iso_points_set.begin(); 
        cii!=moving_iso_points_set.end(); cii++)
      {
        std::cout << *cii << std::endl;

        PointType movingPoint;
        movingPoint[0] = (*cii).x();
        movingPoint[1] = (*cii).y();
        movingPoint[2] = (*cii).z();

        movingPointContainer->InsertElement( pointId++, movingPoint );
      }

      movingPointSet->SetPoints( movingPointContainer );
      unsigned long moving_points_num =
        movingPointSet->GetNumberOfPoints(); 
      CORE_LOG_MESSAGE( std::string("Number of moving Points = ") + 
        boost::lexical_cast<std::string>(moving_points_num));
      //std::cout << "Number of moving Points = "
      //  << movingPointSet->GetNumberOfPoints() << std::endl;


      typedef itk::EuclideanDistancePointMetric<
        PointSetType,
        PointSetType>
        MetricType;

      typedef MetricType::TransformType                 TransformBaseType;
      typedef TransformBaseType::ParametersType         ParametersType;
      typedef TransformBaseType::JacobianType           JacobianType;

      MetricType::Pointer  metric = MetricType::New();


      //-----------------------------------------------------------
      // Set up a Transform
      //-----------------------------------------------------------

      //typedef itk::TranslationTransform< double, Dimension >      TransformType;
      //typedef Seg3D::Rigid3DTransformSurrogate<double>  TransformType;
      typedef itk::Euler3DTransform< double > TransformType;

      TransformType::Pointer transform = TransformType::New();


      // Optimizer Type
      typedef itk::LevenbergMarquardtOptimizer OptimizerType;

      OptimizerType::Pointer      optimizer     = OptimizerType::New();
      optimizer->SetUseCostFunctionGradient(false);


      // Registration Method
      typedef itk::PointSetToPointSetRegistrationMethod<
        PointSetType,
        PointSetType >
        RegistrationType;


      RegistrationType::Pointer   registration  = RegistrationType::New();

      this->forward_abort_to_filter( registration, this->dst_layer_ );
      this->observe_itk_progress( registration, this->dst_layer_ );

      // Scale the translation components of the Transform in the Optimizer
      OptimizerType::ScalesType scales( transform->GetNumberOfParameters() );
      //scales.Fill( 0.01 );
      const double translationScale = 500.0; // dynamic range of translations
      const double rotationScale = 1.0; // dynamic range of rotations
      scales[0] = 1.0 / rotationScale;
      scales[1] = 1.0 / rotationScale;
      scales[2] = 1.0 / rotationScale;
      scales[3] = 1.0 / translationScale;
      scales[4] = 1.0 / translationScale;
      scales[5] = 1.0 / translationScale;

      unsigned long   numberOfIterations = this->iterations_;
      double          gradientTolerance  =  1e-5;    // convergence criterion
      double          valueTolerance     =  1e-5;    // convergence criterion
      double          epsilonFunction    =  1e-6;   // convergence criterion


      optimizer->SetScales( scales );
      optimizer->SetNumberOfIterations( numberOfIterations );
      optimizer->SetValueTolerance( valueTolerance );
      optimizer->SetGradientTolerance( gradientTolerance );
      optimizer->SetEpsilonFunction( epsilonFunction );

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
      registration->SetFixedPointSet( fixedPointSet );
      registration->SetMovingPointSet(   movingPointSet   );

      // Retrieve the image as an itk image from the underlying data structure
      // NOTE: This only does wrapping and does not regenerate the data.
      
          

      // Ensure we will have some threads left for doing something else
      //this->limit_number_of_itk_threads( registration );

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

      //std::cout << "Solution = " << transform->GetParameters() << std::endl;

      RegistrationType::ParametersType finalParameters = 
        registration->GetLastTransformParameters();

      std::string solution = "[" + boost::lexical_cast<std::string>(finalParameters[0]) +
        "," + boost::lexical_cast<std::string>(finalParameters[1]) +"," + boost::lexical_cast<std::string>(finalParameters[2]) + "]"; 
      CORE_LOG_MESSAGE( std::string("Solution = ") + 
        solution );

      //convert Euler transformation parameters into 4x4 matrix
      std::vector<double> matrix_entries;
      //matrix_entries.resize( 16 );
      //std::fill( matrix_entries.begin(), matrix_entries.end(), 0 );

      for ( unsigned int i = 0; i < finalParameters.size(); ++i )
      {
        matrix_entries.push_back( finalParameters[i] );
      }

      //itk::Matrix matrix = transform->GetMatrix();
      //itk::OutputVectorType  translation = transform->GetTranslation();
      //for ( unsigned row = 0; row < Dimension; ++row )
      //{
      //  for ( unsigned col = 0; col < Dimension; ++col )
      //  {
      //    matrix_entries.push_back(matrix[row][col]);
      //  }
      //  matrix_entries.push_back( translation[row] );
      //}



      TransformType::Pointer finalTransform = TransformType::New();
      finalTransform->SetParameters( finalParameters );

      typedef itk:: LinearInterpolateImageFunction<
        TYPED_IMAGE_TYPE,
        double          >    InterpolatorType;

      typedef itk::ResampleImageFilter<
        TYPED_IMAGE_TYPE,
        TYPED_IMAGE_TYPE > ResampleFilterType;

      typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
      typename Core::ITKImageDataT<VALUE_TYPE>::Handle moving_image; 
      this->get_itk_image_from_layer<VALUE_TYPE>( this->mask_layer_, moving_image );
      resampler->SetInput( moving_image->get_image() );
      //resampler->SetTransform( registration->GetOutput()->Get() );
      finalTransform->GetInverse(finalTransform);
      resampler->SetTransform( finalTransform );

      typename Core::ITKImageDataT<VALUE_TYPE>::Handle fixed_image;
      this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, fixed_image );
      resampler->SetSize( fixed_image->get_image()->GetLargestPossibleRegion().GetSize() );
      resampler->SetOutputOrigin(  fixed_image->get_image()->GetOrigin() );
      resampler->SetOutputSpacing( fixed_image->get_image()->GetSpacing() );
      resampler->SetOutputDirection( fixed_image->get_image()->GetDirection() );
      resampler->SetDefaultPixelValue( 0 );

      resampler->Update();
      //this->dispatch_insert_mask_volume_into_layer( this->dst_layer_, mask_volume );
      if (resampler->GetOutput() != NULL )
      {
        this->insert_itk_image_into_layer( this->dst_layer_, resampler->GetOutput() );  
        ICPFilterHandle icp_tool = 
          boost::dynamic_pointer_cast<ICPFilter> (ToolManager::Instance()->get_tool(this->icp_tool_id_));

        Core::Application::PostEvent( boost::bind( &Core::StateBool::set,
          icp_tool->registration_ready_state_, true, Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDoubleVector::set,
          icp_tool->transform_matrix_, matrix_entries, Core::ActionSource::NONE_E ) );

        //const Core::ITKImageDataT<VALUE_TYPE>::image_type::SpacingType& target_image_spacing = 
        //  fixed_image->get_image()->GetSpacing();

        //std::vector<double> spacing;
        //spacing[0] = target_image_spacing[0];
        //spacing[1] = target_image_spacing[1];
        //spacing[2] = target_image_spacing[2];

        //Core::Application::PostEvent( boost::bind( &Core::StateDoubleVector::set,
        //  icp_tool->spacing_, spacing, Core::ActionSource::NONE_E ) );

      }

    }
    SCI_END_TYPED_ITK_RUN()
    
    // GET_FITLER_NAME:
    // The name of the filter, this information is used for generating new layer labels.
    virtual std::string get_filter_name() const
    {
      //return "And Filter";
      return "ICP Register Filter";
    }

    // GET_LAYER_PREFIX:
    // This function returns the name of the filter. The latter is prepended to the new layer name, 
    // when a new layer is generated. 
    virtual std::string get_layer_prefix() const
    {
      //return "AND"; 
      return "ICPRegister"; 
    }
  };


  bool ActionICPRegisterFilter::run( Core::ActionContextHandle& context, 
    Core::ActionResultHandle& result )
  {
    // Create algorithm
    boost::shared_ptr<ICPFilterAlgo> algo( new ICPFilterAlgo );

    // Find the handle to the layer
    algo->src_layer_ = LayerManager::FindLayer( this->target_layer_ );
    algo->mask_layer_ = LayerManager::FindLayer( this->mask_layer_ );
    algo->iterations_ = this->iterations_;
    algo->icp_tool_id_ = this->icp_tool_id_;
    
    // Check whether the source layer was found
    if ( !algo->src_layer_ || !algo->mask_layer_ ) return false;
    
    // Lock the mask layer, so no other layer can access it
    algo->lock_for_use( algo->mask_layer_ );

    //if ( this->replace_ )
    //{
    //  // Copy the handles as destination and source will be the same
    //  algo->dst_layer_ = algo->src_layer_;
  
    //  // Mark the layer for processing.
    //  algo->lock_for_processing( algo->dst_layer_ );  
    //}
    //else
    //{
    //  //// Lock the src layer, so it cannot be used else where
    //  //algo->lock_for_use( algo->src_layer_ );
    //  //
    //  //// Create the destination layer, which will show progress
    //  //algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );

    //  switch ( algo->src_layer_->get_type() )
    //  { 
    //  case Core::VolumeType::DATA_E:
    //    algo->create_and_lock_data_layer_from_layer( 
    //      algo->src_layer_, algo->dst_layer_  );
    //    break;
    //  case Core::VolumeType::MASK_E:
    //    algo->create_and_lock_mask_layer_from_layer( 
    //      algo->src_layer_, algo->dst_layer_ , algo->mask_layer_->get_layer_name() );
    //    break;
    //  default:
    //    assert( false );
    //  }
    //}

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

    // Build the undo-redo record for this action
    algo->create_undo_redo_record( context, this->shared_from_this() );

    // Build the provenance record for this action
    algo->create_provenance_record( context, this->shared_from_this() );

    // Start the filter.
    Core::Runnable::Start( algo );

    return true;
  }

  //void ActionICPRegisterFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  //  std::string mask_layer,  int iterations, bool replace )
  //void ActionICPRegisterFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  //  std::string mask_layer,  int iterations, bool replace, std::string toolid )

  void ActionICPRegisterFilter::Dispatch( Core::ActionContextHandle context,
    std::string target_layer, std::string mask_layer,  int iterations,  std::string toolid )
  { 
    // Create a new action
    ActionICPRegisterFilter* action = new ActionICPRegisterFilter;

    // Setup the parameters
    action->target_layer_ = target_layer;
    action->mask_layer_ = mask_layer; 
    //action->replace_ = replace;
    action->iterations_ = iterations;
    action->icp_tool_id_ = toolid;

    // Dispatch action to underlying engine
    Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
  }


    
  } // end namespace Seg3D
