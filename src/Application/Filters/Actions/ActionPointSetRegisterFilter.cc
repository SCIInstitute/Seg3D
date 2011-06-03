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

  // Application includes
  #include <Application/LayerManager/LayerManager.h>
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

  //class PointSetFilterAlgo : public LayerFilter, ITKFilter
  class PointSetFilterAlgo : public  ITKFilter
  {

  public:
    LayerHandle src_layer_;
    LayerHandle mask_layer_;
    LayerHandle dst_layer_;

    int iterations_;
    std::string pointset_tool_id_;

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

      //Estimate the quality factor from the fraction from bounding box
      /*double target_layer_size  =
         input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_size();

      size_t target_bounding_box_size;
      {
        Core::MaskDataBlock::shared_lock_type lock( 
          input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_mutex() );

        size_t min_x = 0; 
        size_t max_x = 0;
        size_t min_y = 0; 
        size_t max_y = 0;
        size_t min_z = 0; 
        size_t max_z = 0;

        size_t nx = 
          input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_nx();

        size_t ny = 
          input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_ny();

        size_t nz = 
          input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_nz();

        for( size_t x = 0; x < nx; ++x )
        {
          for ( size_t y = 0; y < ny; ++y )
          {
            for ( size_t z = 0; z < nz; ++z )
            {
              if( input_fixed_layer->get_mask_volume()->get_mask_data_block()->get_mask_at( x, y, z ) )
              {
                if ( x < min_x )
                {
                  min_x = x;
                }

                if ( y < min_y )
                {
                  min_y = y;
                }

                if ( z < min_z )
                {
                  min_z = z;
                }

                if ( x > max_x )
                {
                  max_x = x;
                }

                if ( y > max_y )
                {
                  max_y = y;
                }

                if ( z > max_z )
                {
                  max_z = z;
                }
              }
            }
          }
        }

        target_bounding_box_size =
          ( max_x - min_x ) * ( max_y - min_y ) * ( max_z - min_z );
      }

      double fraction_target_layer = 
        target_bounding_box_size / target_layer_size;

      size_t moving_bounding_box_size;
      {
        Core::MaskDataBlock::shared_lock_type lock( 
          input_moving_layer->get_mask_volume()->get_mask_data_block()->get_mutex() );

        size_t min_x = 0; 
        size_t max_x = 0;
        size_t min_y = 0; 
        size_t max_y = 0;
        size_t min_z = 0; 
        size_t max_z = 0;

        size_t nx = 
          input_moving_layer->get_mask_volume()->get_mask_data_block()->get_nx();

        size_t ny = 
          input_moving_layer->get_mask_volume()->get_mask_data_block()->get_ny();

        size_t nz = 
          input_moving_layer->get_mask_volume()->get_mask_data_block()->get_nz();

        for( size_t x = 0; x < nx; ++x )
        {
          for ( size_t y = 0; y < ny; ++y )
          {
            for ( size_t z = 0; z < nz; ++z )
            {
              if( input_moving_layer->get_mask_volume()->get_mask_data_block()->get_mask_at( x, y, z ) )
              {
                if ( x < min_x )
                {
                  min_x = x;
                }

                if ( y < min_y )
                {
                  min_y = y;
                }

                if ( z < min_z )
                {
                  min_z = z;
                }

                if ( x > max_x )
                {
                  max_x = x;
                }

                if ( y > max_y )
                {
                  max_y = y;
                }

                if ( z > max_z )
                {
                  max_z = z;
                }
              }
            }
          }
        }

        moving_bounding_box_size =
          ( max_x - min_x ) * ( max_y - min_y ) * ( max_z - min_z );
      }

      double moving_layer_size  =
        input_moving_layer->get_mask_volume()->get_mask_data_block()->get_size();

      double fraction_moving_layer = moving_bounding_box_size / moving_layer_size;

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

        //sample_ready = true;
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

      //
      // First map the Fixed Points into a binary image.
      // This is needed because the DanielssonDistance
      // filter expects an image as input.
      //
      //-------------------------------------------------
      //typedef itk::Image< unsigned char, DIMENSION_C > binary_image_type;
      //typedef itk::PointSetToImageFilter<
      //  point_set_type,
      //  binary_image_type > points_to_image_filter_type;
      //points_to_image_filter_type::Pointer points_to_image_filter = 
      //  points_to_image_filter_type::New();
      //points_to_image_filter->SetInput( fixed_point_set );

      //binary_image_type::SpacingType spacing;
      //spacing.Fill( 1.0 );
      //binary_image_type::PointType origin;
      //origin.Fill( 0.0 );
      //points_to_image_filter->SetSpacing( spacing );
      //points_to_image_filter->SetOrigin( origin );
      //points_to_image_filter->Update();
      //binary_image_type::Pointer binary_image = points_to_image_filter->GetOutput();
      //typedef itk::Image< unsigned short, DIMENSION_C > distance_image_type;
      //typedef itk::DanielssonDistanceMapImageFilter<
      //  binary_image_type,
      //  distance_image_type > distance_filter_type;
      //distance_filter_type::Pointer distance_filter = distance_filter_type::New();

      //distance_filter->SetInput( binary_image );
      //distance_filter->Update();
      //metric->SetDistanceMap( distance_filter->GetOutput() );

      //-----------------------------------------------------------
      // Set up a Transform
      //-----------------------------------------------------------

      //typedef itk::TranslationTransform< double, Dimension >      TransformType;
      //typedef Seg3D::Rigid3DTransformSurrogate<double>  TransformType;
      typedef itk::Euler3DTransform< double > transform_type;
      //typedef itk::VersorTransform< double > TransformType;

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
      //CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();

      //observer->SetProgressStart( 0.0 );
      //observer->SetProgressUnit( progress_unit );
      //observer->SetLayer(dst_layer_);
      //optimizer->AddObserver( itk::IterationEvent(), observer );
    

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

      //convert Euler transformation parameters into 4x4 matrix
      std::vector< double > matrix_entries;
      //matrix_entries.resize( 16 );
      //std::fill( matrix_entries.begin(), matrix_entries.end(), 0 );

      for ( unsigned int i = 0; i < final_parameters.size(); ++i )
      {
        matrix_entries.push_back( final_parameters[i] );
      }

      double  rotation[3];
      double  translation[3];

      rotation[0] = final_parameters[0]; 
      rotation[1] = final_parameters[1]; 
      rotation[2] = final_parameters[2]; 

      translation[0] = final_parameters[3]; 
      translation[1] = final_parameters[4]; 
      translation[2] = final_parameters[5]; 

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
      //resampler->SetTransform( registration->GetOutput()->Get() );
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
        PointSetFilterHandle pointset_tool = 
          boost::dynamic_pointer_cast<PointSetFilter> (ToolManager::Instance()->get_tool(this->pointset_tool_id_));

        Core::Application::PostEvent( boost::bind( &Core::StateBool::set,
          pointset_tool->registration_ready_state_, true, Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDoubleVector::set,
          pointset_tool->transform_matrix_, matrix_entries, Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDouble::set,
          pointset_tool->translation_state_[0], translation[0] , Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDouble::set,
          pointset_tool->translation_state_[1], translation[1] , Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDouble::set,
          pointset_tool->translation_state_[2], translation[2] , Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDouble::set,
          pointset_tool->rotation_state_[0], rotation[0] , Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDouble::set,
          pointset_tool->rotation_state_[1], rotation[1] , Core::ActionSource::NONE_E ) );

        Core::Application::PostEvent( boost::bind( &Core::StateDouble::set,
          pointset_tool->rotation_state_[2], rotation[2] , Core::ActionSource::NONE_E ) );
      }

    }
    SCI_END_TYPED_ITK_RUN()
    
    // GET_FITLER_NAME:
    // The name of the filter, this information is used for generating new layer labels.
    virtual std::string get_filter_name() const
    {
      //return "And Filter";
      return "PointSet Register Filter";
    }

    // GET_LAYER_PREFIX:
    // This function returns the name of the filter. The latter is prepended to the new layer name, 
    // when a new layer is generated. 
    virtual std::string get_layer_prefix() const
    {
      //return "AND"; 
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

      //typedef itk::LevenbergMarquardtOptimizer OptimizerType;
      //typedef   const OptimizerType *              OptimizerPointer;

      //OptimizerPointer optimizer =
      //  dynamic_cast< OptimizerPointer >( itk_object );

      //CORE_LOG_MESSAGE( std::string("value = ") + 
      //  boost::lexical_cast<std::string>( optimizer->GetValue() ));

      //CORE_LOG_MESSAGE( std::string("params = ") + 
      //  boost::lexical_cast<std::string>( optimizer->GetCurrentPosition() ));
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
    algo->src_layer_ = LayerManager::FindLayer( this->target_layer_ );
    algo->mask_layer_ = LayerManager::FindLayer( this->mask_layer_ );
    algo->iterations_ = this->iterations_;
    algo->pointset_tool_id_ = this->pointset_tool_id_;
    
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
    algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

    // Start the filter.
    Core::Runnable::Start( algo );

    return true;
  }

  //void ActionPointSetRegisterFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  //  std::string mask_layer,  int iterations, bool replace )
  //void ActionPointSetRegisterFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  //  std::string mask_layer,  int iterations, bool replace, std::string toolid )

  void ActionPointSetRegisterFilter::Dispatch( Core::ActionContextHandle context,
    std::string target_layer, std::string mask_layer,  int iterations,  std::string toolid )
  { 
    // Create a new action
    ActionPointSetRegisterFilter* action = new ActionPointSetRegisterFilter;

    // Setup the parameters
    action->target_layer_ = target_layer;
    action->mask_layer_ = mask_layer; 
    action->iterations_ = iterations;
    action->pointset_tool_id_ = toolid;

    // Dispatch action to underlying engine
    Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
  }


    
  } // end namespace Seg3D
