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

#include <Core/Action/ActionFactory.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Geometry/Path.h>

#include <Core/ITKSpeedLine/itkSpeedFunctionToPathFilter.h>
#include <Core/ITKSpeedLine/itkArrivalFunctionToPathFilter.h>

#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionSpeedline.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Tools/SpeedlineTool.h>

//ITK Includes
#include <itkRescaleIntensityImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkPathIterator.h>
#include <itkIndex.h>
#include <itkImageBase.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

CORE_REGISTER_ACTION( Seg3D, Speedline )

namespace Seg3D
{

class ActionSpeedlineAlgo : public  ITKFilter
{
public:
  static const unsigned int DIMENSION_C = 2;
  typedef double pixel_type;
  typedef itk::Image< pixel_type, DIMENSION_C > image_type;
  typedef itk::PolyLineParametricPath< DIMENSION_C > path_type;
  typedef itk::SpeedFunctionToPathFilter< image_type, path_type > path_filter_type;
  typedef path_filter_type::CostFunctionType::CoordRepType coord_rep_type; // double

  image_type::Pointer speed_image_2D_;
  Core::Path itk_paths_;
  Core::Path world_paths_;

public:
  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector< Core::Point > vertices_;
  int current_vertex_index_;
  
  DataLayerHandle target_layer_;

  int iterations_;
  double termination_;
  bool update_all_paths_;
  std::string itk_path_state_id_;
  std::string world_path_state_id_;

public:
  void compute_path ( int start_index, int end_index )
  {
    // Create interpolator

    typedef itk::LinearInterpolateImageFunction< image_type, coord_rep_type >
      interpolator_type;
    interpolator_type::Pointer interp = interpolator_type::New();

    // Create cost function

    path_filter_type::CostFunctionType::Pointer cost = path_filter_type::CostFunctionType::New();
    cost->SetInterpolator( interp );

    // Create optimizer

    image_type::SpacingType spacing = this->speed_image_2D_->GetSpacing();
    double min_spacing = ( spacing[ 0 ] < spacing[ 1 ] ) ? spacing[ 0 ] : spacing[ 1 ];
    double max_step = 0.5 * min_spacing;
    double min_step = 0.001 * min_spacing;
    
    typedef itk::RegularStepGradientDescentOptimizer optimizer_type;
    optimizer_type::Pointer optimizer = optimizer_type::New();
    optimizer->SetNumberOfIterations( this->iterations_ );
    optimizer->SetMaximumStepLength( max_step );
    optimizer->SetMinimumStepLength( min_step );
    optimizer->SetRelaxationFactor( 0.6 );

    // Create path filter

    path_filter_type::Pointer path_filter = path_filter_type::New();
    
    path_filter->SetCostFunction( cost );
    path_filter->SetOptimizer( optimizer );
    path_filter->SetTerminationValue( this->termination_ * min_spacing );
    path_filter->SetInput( this->speed_image_2D_ );

    if ( !this->update_all_paths_ )
    {
      // Add the seed points.
      path_filter_type::PathInfo  info;
      path_filter_type::PointType pnt;

      if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
      {
        pnt[0] = this->vertices_[ start_index ].y();
        pnt[1] = this->vertices_[ start_index ].z();

      }
      else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
      {
        pnt[0] = this->vertices_[ start_index ].x();
        pnt[1] = this->vertices_[ start_index ].z();

      }
      else
      {
        pnt[0] = this->vertices_[ start_index ].x();
        pnt[1] = this->vertices_[ start_index ].y();
      }

      info.SetStartPoint( pnt );

      if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
      {
        pnt[0] = this->vertices_[ end_index ].y();
        pnt[1] = this->vertices_[ end_index ].z();

      }
      else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
      {
        pnt[0] = this->vertices_[ end_index ].x();
        pnt[1] = this->vertices_[ end_index ].z();

      }
      else
      {
        pnt[0] = this->vertices_[ end_index ].x();
        pnt[1] = this->vertices_[ end_index ].y();
      }

      info.SetEndPoint( pnt );

      path_filter->AddPathInfo( info );
      path_filter->Update();

      size_t num_of_outputs = path_filter->GetNumberOfOutputs(); 
      assert( num_of_outputs == 1);

      Core::SinglePath new_path( this->vertices_[ start_index ], this->vertices_[ end_index ] );

      path_type::Pointer itk_path = path_filter->GetOutput( 0 );
      size_t vertext_list_size = itk_path->GetVertexList()->Size();

      for ( unsigned int k = 0; k < vertext_list_size; k++ )
      {
        const double x = itk_path->GetVertexList()->ElementAt( k )[0];
        const double y = itk_path->GetVertexList()->ElementAt( k )[1];

        Core::Point ipnt;

        if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E ) // SAGITTAL_E = 2
        {
          ipnt[0] = this->slice_number_;
          ipnt[1] = x;
          ipnt[2] = y;
        }
        else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E ) // CORONAL_E = 1
        {
          ipnt[0] = x;
          ipnt[1] = this->slice_number_;
          ipnt[2] = y;
        }
        else
        {
          ipnt[0] = x;
          ipnt[1] = y;
          ipnt[2] = this->slice_number_;
        }
        new_path.add_a_point( ipnt );
      }

      this->itk_paths_.add_one_path( new_path );
    }
    else
    {
      // update each path in new slice
      Core::Path updated_paths;

      this->itk_paths_.delete_all_paths();
      size_t vertices_num = this->vertices_.size();
      for ( unsigned int i = 0; i < vertices_num; ++i )
      {
        Core::Point p0, p1;
        p0 = this->vertices_[ i ];
        p1 = this->vertices_[ ( i + 1 ) % vertices_num ];

        path_filter_type::PathInfo  info;
        path_filter_type::PointType pnt;

        if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
        {
          pnt[0] = p0.y();
          pnt[1] = p0.z();

        }
        else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
        {
          pnt[0] = p0.x();
          pnt[1] = p0.z();

        }
        else
        {
          pnt[0] = p0.x();
          pnt[1] = p0.y();
        }

        info.SetStartPoint( pnt );

        if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
        {
          pnt[0] = p1.y();
          pnt[1] = p1.z();

        }
        else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
        {
          pnt[0] = p1.x();
          pnt[1] = p1.z();

        }
        else
        {
          pnt[0] = p1.x();
          pnt[1] = p1.y();
        }

        info.SetEndPoint( pnt );

        path_filter->AddPathInfo( info );
      }

      path_filter->Update();

      size_t num_of_outputs = path_filter->GetNumberOfOutputs(); 
      for ( unsigned int j = 0; j < num_of_outputs; ++j )
      {
        Core::Point p0, p1;

        p0 = this->vertices_[ j ];
        p1 = this->vertices_[ ( j + 1 ) % vertices_num ];
        Core::SinglePath new_path( p0, p1 );

        path_type::Pointer itk_path = path_filter->GetOutput( j );
        size_t vertext_list_size = itk_path->GetVertexList()->Size();

        for ( unsigned int k = 0; k < vertext_list_size; ++k )
        {
          const double x = itk_path->GetVertexList()->ElementAt( k )[0];
          const double y = itk_path->GetVertexList()->ElementAt( k )[1];

          Core::Point ipnt;

          if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E ) // SAGITTAL_E = 2
          {
            ipnt[0] = this->slice_number_;
            ipnt[1] = x;
            ipnt[2] = y;

          }
          else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E ) // CORONAL_E = 1
          {
            ipnt[0] = x;
            ipnt[1] = this->slice_number_;
            ipnt[2] = y;
          }
          else
          {
            ipnt[0] = x;
            ipnt[1] = y;
            ipnt[2] = this->slice_number_;
          }
          new_path.add_a_point( ipnt );
        }

        updated_paths.add_one_path( new_path );
      }

      this->itk_paths_.delete_all_paths();

      for ( unsigned int i = 0; i < updated_paths.get_path_num(); ++i )
      {
        this->itk_paths_.add_one_path( updated_paths.get_one_path( i ) );
      }
    }
    
  }

  // When mouse is moved, clean the temporary paths
  bool clean_paths( )
  {
    bool delete_outdated_path = false;
    std::vector< Core::SinglePath > tmp_paths;

    size_t paths_num = this->itk_paths_.get_path_num();
    for ( unsigned int i = 0; i < paths_num; ++i )
    {
      Core::SinglePath single_path = this->itk_paths_.get_one_path( i );

      Core::Point p0, p1;
      single_path.get_point_on_ends( 0, p0 );
      single_path.get_point_on_ends( 1, p1 );

      std::vector< Core::Point >::iterator it0  = 
        std::find( this->vertices_.begin(), this->vertices_.end(), p0 );

      std::vector< Core::Point >::iterator it1  = 
        std::find( this->vertices_.begin(), this->vertices_.end(), p1 );

      if ( it0 == this->vertices_.end() || it1 == this->vertices_.end() )
      {
        delete_outdated_path = true;
      }
      else
      {
        tmp_paths.push_back( single_path );
      }
    }

    //this->itk_paths_.delete_all_paths();
    //for ( std::vector< Core::SinglePath >::iterator it = tmp_paths.begin(); it != tmp_paths.end(); ++it )
    //{
    //  this->itk_paths_.add_one_path( *it );
    //}

    this->itk_paths_.set_all_paths( tmp_paths );

    return delete_outdated_path;  
  }

  SCI_BEGIN_TYPED_ITK_RUN( this->target_layer_->get_data_type() )
  {
    Core::StateSpeedlinePathHandle world_path_state;
    Core::StateSpeedlinePathHandle itk_path_state;
    
    Core::StateBaseHandle state_var_1;
    Core::StateBaseHandle state_var_2;

    if ( Core::StateEngine::Instance()->get_state( 
      this->world_path_state_id_, state_var_1 ) )
    {
      world_path_state = boost::dynamic_pointer_cast<
        Core::StateSpeedlinePath > ( state_var_1 );
      if ( world_path_state )
      {
        this->world_paths_ = world_path_state->get();
      }
    }

    if ( Core::StateEngine::Instance()->get_state( 
      this->itk_path_state_id_, state_var_2 ) )
    {
      itk_path_state = boost::dynamic_pointer_cast<
        Core::StateSpeedlinePath > ( state_var_2 );
      if ( itk_path_state )
      {
        this->itk_paths_ = itk_path_state->get();
      }
    }

    this->world_paths_.delete_all_paths();
    if ( this->current_vertex_index_ == -1 )
    {
      clean_paths();
      Core::Point pstart ( DBL_MAX, DBL_MAX, DBL_MAX );
      Core::Point pend ( DBL_MIN, DBL_MIN, DBL_MIN );
      this->itk_paths_.set_start_point( pstart );
      this->itk_paths_.set_end_point( pend );

      Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
          itk_path_state, this->itk_paths_, Core::ActionSource::NONE_E ) );
  
      Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
          world_path_state, this->world_paths_, Core::ActionSource::NONE_E ) );

      return;
    }

    size_t num_of_vertices = this->vertices_.size();

    if ( num_of_vertices == 0 )
      return;

    typename Core::ITKImageDataT< VALUE_TYPE >::Handle speed_image_3D;
    this->get_itk_image_from_layer< VALUE_TYPE >( this->target_layer_, speed_image_3D );

    // Create 2D itk image
    typedef itk::ExtractImageFilter< TYPED_IMAGE_TYPE, image_type >  extract_filter_type;
    typename extract_filter_type::Pointer extract_filter = extract_filter_type::New();
    
    typename TYPED_IMAGE_TYPE::RegionType input_region = 
      speed_image_3D->get_image()->GetLargestPossibleRegion();
    typename TYPED_IMAGE_TYPE::SizeType size = input_region.GetSize();
    size[2] = 0;
    typename TYPED_IMAGE_TYPE::IndexType start = input_region.GetIndex();
    start[2] = this->slice_number_;
    typename TYPED_IMAGE_TYPE::RegionType desired_region;
    desired_region.SetSize( size );
    desired_region.SetIndex( start );

    extract_filter->SetExtractionRegion( desired_region );
    extract_filter->SetInput( speed_image_3D->get_image() );
    extract_filter->Update();

    this->speed_image_2D_ = extract_filter->GetOutput();

    this->itk_paths_.set_start_point( this->vertices_[0] );
    this->itk_paths_.set_end_point( this->vertices_[ this->vertices_.size() - 1 ] );

    // NOTE: ITK filter uses continueIndex instead of real physical point
    // So we need to convert the point position to continue index
    typedef itk::ContinuousIndex< double, 3 >   continuous_index_type;
    typedef itk::Point< double, 3 > itk_point_type;

    int start_index;
    int end_index;

    if ( this->update_all_paths_ )
    {
      start_index = 0;
      end_index = num_of_vertices - 1;

      compute_path( start_index, end_index );
    }
    
    else
    {
      // clean out-dated paths
      bool deleted_paths = clean_paths();

      if ( num_of_vertices == 1)
      {
        start_index = 0;
        end_index = 0;
      }
      
      else if ( num_of_vertices == 2 )
      {
        start_index = 0;
        end_index = 1;

        this->itk_paths_.delete_all_paths();  //2 points just redraw

        compute_path( start_index, end_index );
      }
      else
      { 
        if ( this->current_vertex_index_ == 0 )
        {
          start_index = num_of_vertices - 1;
          end_index = this->current_vertex_index_ + 1;
        }
        else if ( this->current_vertex_index_ == num_of_vertices - 1)
        { 
          start_index = num_of_vertices - 2;
          end_index = 0;  
        }
        else
        {
          start_index = this->current_vertex_index_ - 1;
          end_index = this->current_vertex_index_ + 1;
        }
 
        // Add a new vertex
        if ( num_of_vertices > this->itk_paths_.get_path_num() && !deleted_paths ) 
        {
          Core::Point p0, p1;

          p0 = this->vertices_[start_index ];
          p1 = this->vertices_[ end_index ];
          bool is_deleted = this->itk_paths_.delete_one_path( p0, p1 );
        }

        Core::Point p0, p1;

        p0 = this->vertices_[ start_index ];
        p1 = this->vertices_[ this->current_vertex_index_ ];
        bool is_deleted = this->itk_paths_.delete_one_path( p0, p1 );

        p0 = this->vertices_[ this->current_vertex_index_ ];
        p1 = this->vertices_[ end_index ];
        is_deleted = this->itk_paths_.delete_one_path( p0, p1 );

        compute_path( start_index, this->current_vertex_index_ );
        compute_path( this->current_vertex_index_, end_index );
      }
    }

    this->world_paths_.set_start_point( this->itk_paths_.get_start_point() );
    this->world_paths_.set_end_point( this->itk_paths_.get_end_point() );

    size_t path_num = this->itk_paths_.get_path_num();
    for ( unsigned int i = 0; i < path_num; ++i )
    {
      Core::SinglePath new_path;
      Core::Point p0, p1;
      this->itk_paths_.get_one_path( i ).get_point_on_ends( 0, p0 );
      this->itk_paths_.get_one_path( i ).get_point_on_ends( 1, p1 );

      new_path.set_point_on_ends( 0, p0 );
      new_path.set_point_on_ends( 1, p1 );

      size_t num_points_on_path = this->itk_paths_.get_one_path( i ).get_points_num_on_path();
      Core::SinglePath cpath = this->itk_paths_.get_one_path( i );

      new_path.add_a_point( p1 );

      for ( unsigned int j = 0; j < num_points_on_path; ++j )
      {
        continuous_index_type cindex;
        itk_point_type itk_point;

        cindex[ 0 ] =  cpath.get_a_point( j ).x();
        cindex[ 1 ] =  cpath.get_a_point( j ).y();
        cindex[ 2 ] =  cpath.get_a_point( j ).z();

        speed_image_3D->get_image()->TransformContinuousIndexToPhysicalPoint( cindex, itk_point );  
        Core::Point cpoint( itk_point[ 0 ], itk_point[ 1 ], itk_point[ 2 ] );
        new_path.add_a_point( cpoint );
      }
      new_path.add_a_point( p0 );
      this->world_paths_.add_one_path( new_path );
    }

    Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
      itk_path_state, this->itk_paths_, Core::ActionSource::NONE_E ) );

    Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
      world_path_state, this->world_paths_, Core::ActionSource::NONE_E ) );
  }
  SCI_END_TYPED_ITK_RUN()

  virtual std::string get_filter_name() const
  {
    return "SpeedLine Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Speed"; 
  }
};

class ActionSpeedlinePrivate
{
public:
  // == action parameters ==
  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector< Core::Point > vertices_;
  int iterations_;
  double termination_;
  int current_vertex_index_;
  std::string itk_path_state_id_;
  std::string world_path_state_id_;
  bool update_all_paths_;
};

ActionSpeedline::ActionSpeedline() :
  private_( new ActionSpeedlinePrivate )
{
  this->add_layer_id(  this->private_->target_layer_id_ );
  this->add_parameter( this->private_->slice_type_ );
  this->add_parameter( this->private_->slice_number_ );
  this->add_parameter( this->private_->vertices_ );
  this->add_parameter( this->private_->current_vertex_index_ );
  this->add_parameter( this->private_->iterations_ );
  this->add_parameter( this->private_->termination_ );
  this->add_parameter( this->private_->update_all_paths_ );
  this->add_parameter( this->private_->itk_path_state_id_ );
  this->add_parameter( this->private_->world_path_state_id_ );
}

bool ActionSpeedline::validate( Core::ActionContextHandle& context )
{
  // Check whether the target layer exists
  if ( !LayerManager::CheckLayerExistenceAndType( this->private_->target_layer_id_,
    Core::VolumeType::DATA_E, context ) )
  {
    return false;
  }

  // Check whether the target layer can be used for read
  if ( !LayerManager::CheckLayerAvailabilityForUse( this->private_->target_layer_id_, context ) )
    return false;
  
  if ( this->private_->slice_type_ != Core::VolumeSliceType::AXIAL_E &&
    this->private_->slice_type_ != Core::VolumeSliceType::CORONAL_E &&
    this->private_->slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  DataLayerHandle target_layer = LayerManager::FindDataLayer( this->private_->target_layer_id_ );
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_ );
  Core::DataVolumeSliceHandle volume_slice( new Core::DataVolumeSlice(
    target_layer->get_data_volume(), slice_type ) );
  if ( this->private_->slice_number_ >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }
  
  return true;
}

bool ActionSpeedline::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // Create algorithm
  typedef boost::shared_ptr< ActionSpeedlineAlgo > ActionSpeedlineAlgoHandle;
  ActionSpeedlineAlgoHandle algo( new ActionSpeedlineAlgo );

  algo->target_layer_id_ = this->private_->target_layer_id_ ;
  algo->slice_type_ = this->private_->slice_type_ ;
  algo->slice_number_ = this->private_->slice_number_;
  algo->vertices_ = this->private_->vertices_;
  algo->current_vertex_index_ = this->private_->current_vertex_index_;
  algo->iterations_ = this->private_->iterations_;
  algo->termination_ = this->private_->termination_;
  algo->target_layer_ = LayerManager::FindDataLayer( this->private_->target_layer_id_ );
  algo->update_all_paths_ = this->private_->update_all_paths_;
  algo->itk_path_state_id_ = this->private_->itk_path_state_id_;
  algo->world_path_state_id_ = this->private_->world_path_state_id_;

  Core::Runnable::Start( algo );

  return true;
}

void ActionSpeedline::Dispatch( Core::ActionContextHandle context, 
                const std::string& layer_id, Core::VolumeSliceType slice_type, 
                size_t slice_number,
                const std::vector< Core::Point > vertices,
                int current_vertex_index,
                int iterations, double termination, 
                bool update_all_paths,
                const std::string& itk_path_state_id,
                const std::string& world_path_state_id
                )
{
  ActionSpeedline* action = new ActionSpeedline;

  action->private_->target_layer_id_ = layer_id;
  action->private_->slice_type_ = slice_type;
  action->private_->slice_number_ = slice_number;
  action->private_->vertices_ = vertices;
  action->private_->current_vertex_index_ = current_vertex_index;
  action->private_->iterations_ = iterations;
  action->private_->termination_ = termination;
  action->private_->update_all_paths_ = update_all_paths;
  action->private_->itk_path_state_id_ = itk_path_state_id;
  action->private_->world_path_state_id_ = world_path_state_id;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
