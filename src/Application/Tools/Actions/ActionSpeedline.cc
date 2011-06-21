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
#include <Core/ITKSpeedLine/itkIterateNeighborhoodOptimizer.h>

#include <Application/Provenance/Provenance.h>
#include <Application/Provenance/ProvenanceStep.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionSpeedline.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Tools/SpeedlineTool.h>

//ITK Includes
#include <itkRescaleIntensityImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkPathIterator.h>
#include <itkGradientDescentOptimizer.h>
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
  bool erase_;
  std::vector< Core::Point > vertices_;
  int current_vertex_index_;
  
  DataLayerHandle target_layer_;
  Core::DataVolumeSliceHandle vol_slice_;

  int iterations_;
  double termination_;
  bool update_all_paths_;
  std::string speedline_tool_id_;

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

    typedef itk::RegularStepGradientDescentOptimizer optimizer_type;
    optimizer_type::Pointer optimizer = optimizer_type::New();
    optimizer->SetNumberOfIterations( this->iterations_ );
    optimizer->SetMaximumStepLength( 0.5 );
    optimizer->SetMinimumStepLength( 0.001 );
    optimizer->SetRelaxationFactor( 0.5 );

    //typedef itk::IterateNeighborhoodOptimizer optimizer_type;
    //optimizer_type::Pointer optimizer = optimizer_type::New();
    //optimizer->MinimizeOn();
    //optimizer->FullyConnectedOn();
    //optimizer_type::NeighborhoodSizeType size( DIMENSION_C );
    //double StepLengthFactorValue = 1.0;

    //for(size_t i = 0; i < DIMENSION_C; i++)
    //{
    //  size[i] = this->speed_image_2D_->GetSpacing()[ (unsigned int) i] * StepLengthFactorValue;
    //}

    //optimizer->SetNeighborhoodSize( size );

    //typedef itk::GradientDescentOptimizer optimizer_type;
    //optimizer_type::Pointer optimizer = optimizer_type::New();
    //optimizer->SetNumberOfIterations( this->iterations_ );

    // Create path filter
    path_filter_type::Pointer path_filter = path_filter_type::New();
    
    path_filter->SetCostFunction( cost );
    path_filter->SetOptimizer( optimizer );
    path_filter->SetTerminationValue( this->termination_ );
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

      for ( unsigned int i = 0; i < this->itk_paths_.get_path_num(); ++i )
      {
        Core::SinglePath old_path = this->itk_paths_.get_one_path( i );
        Core::Point p0, p1;
        old_path.get_point_on_ends( 0, p0 );
        old_path.get_point_on_ends( 1, p1 );

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

        Core::SinglePath old_path = this->itk_paths_.get_one_path( j );
        Core::Point p0, p1;
        old_path.get_point_on_ends( 0, p0 );
        old_path.get_point_on_ends( 1, p1 );
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
  void clean_paths( )
  {
    //for ( unsigned int i = 0; i < paths_num; ++i )
    //{
    //  Core::SinglePath single_path = this->paths_.get_one_path( i );

    //  Core::Point p0, p1;
    //  single_path.get_point_on_ends( 0, p0 );
    //  single_path.get_point_on_ends( 1, p1 );

    //  std::vector< Core::Point >::iterator it0  = 
    //    std::find( this->vertices_.begin(), this->vertices_.end(), p0 );

    //  std::vector< Core::Point >::iterator it1  = 
    //    std::find( this->vertices_.begin(), this->vertices_.end(), p1 );

    //  if ( it0 == this->vertices_.end() || it1 == this->vertices_.end() )
    //  {
    //    //bool is_deleted = this->paths_.delete_one_path( p0, p1 );
    //  }
    //  else
    //  {
    //    tmp_paths.push_back( single_path );
    //  }
    //}

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
        //bool is_deleted = this->paths_.delete_one_path( p0, p1 );
      }
      else
      {
        tmp_paths.push_back( single_path );
      }
    }

    this->itk_paths_.delete_all_paths();
    for ( std::vector< Core::SinglePath >::iterator it = tmp_paths.begin(); it != tmp_paths.end(); ++it )
    {
      this->itk_paths_.add_one_path( *it );
    }

    //std::vector< Core::SinglePath > paths = this->paths_.get_all_paths();
    //for ( std::vector< Core::SinglePath >::iterator it = paths.begin(); it != paths.end(); ++it )
    //{
    //  Core::SinglePath single_path = *it;
    //  Core::Point p0, p1;
    //  single_path.get_point_on_ends( 0, p0 );
    //  single_path.get_point_on_ends( 1, p1 );

    //  std::vector< Core::Point >::iterator it0  = 
    //    std::find( this->vertices_.begin(), this->vertices_.end(), p0 );

    //  std::vector< Core::Point >::iterator it1  = 
    //    std::find( this->vertices_.begin(), this->vertices_.end(), p1 );

    //  if ( it0 == this->vertices_.end() || it1 == this->vertices_.end() )
    //  {
    //    //paths.erase( it );
    //  }
    //}
    
  }

  SCI_BEGIN_TYPED_ITK_RUN( this->target_layer_->get_data_type() )
  {

    SpeedlineToolHandle speedline_tool = 
      boost::dynamic_pointer_cast< SpeedlineTool > ( ToolManager::Instance()->get_tool( this->speedline_tool_id_ ) );
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      this->world_paths_ = speedline_tool->path_state_->get();
    }

    this->world_paths_.delete_all_paths();
    if ( this->current_vertex_index_ == -1 )
    {
      clean_paths();

      Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
        speedline_tool->itk_path_state_, this->itk_paths_, Core::ActionSource::NONE_E ) );

      Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
        speedline_tool->path_state_, this->world_paths_, Core::ActionSource::NONE_E ) );
      return;
    }

    typename Core::ITKImageDataT< VALUE_TYPE >::Handle speed_image_3D;
    this->get_itk_image_from_layer< VALUE_TYPE >( this->target_layer_, speed_image_3D );

    //double epsilon = 1.0e-7;
    //double max_value = 1.0;
    //typedef itk::RescaleIntensityImageFilter < TYPED_IMAGE_TYPE, TYPED_IMAGE_TYPE > rescale_filter_type;

    //rescale_filter_type::Pointer rescale_filter = rescale_filter_type::New();
    //rescale_filter->SetInput( speed_image_3D->get_image() );
    //rescale_filter->SetOutputMinimum( epsilon );
    //rescale_filter->SetOutputMaximum( max_value );
    //rescale_filter->Update();

    // Create 2D itk image
    typedef itk::ExtractImageFilter< TYPED_IMAGE_TYPE, image_type >  extract_filter_type;
    typename extract_filter_type::Pointer extract_filter = extract_filter_type::New();
    
    TYPED_IMAGE_TYPE::RegionType input_region = speed_image_3D->get_image()->GetLargestPossibleRegion();
    //TYPED_IMAGE_TYPE::RegionType input_region = ( rescale_filter->GetOutput() )->GetLargestPossibleRegion();
    TYPED_IMAGE_TYPE::SizeType size = input_region.GetSize();
    size[2] = 0;
    typename TYPED_IMAGE_TYPE::IndexType start = input_region.GetIndex();
    start[2] = this->slice_number_;
    typename TYPED_IMAGE_TYPE::RegionType desired_region;
    desired_region.SetSize( size );
    desired_region.SetIndex( start );

    extract_filter->SetExtractionRegion( desired_region );
    extract_filter->SetInput( speed_image_3D->get_image() );
    //extract_filter->SetInput( rescale_filter->GetOutput() );
    extract_filter->Update();

    this->speed_image_2D_ = extract_filter->GetOutput();

    size_t num_of_vertices = this->vertices_.size();
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
      clean_paths();

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

          if ( num_of_vertices > this->itk_paths_.get_path_num() )  //new vertex
          {
            int deleted_start_index = num_of_vertices - 2;
            int deleted_end_index = 0;
            Core::Point p0, p1;

            p0 = this->vertices_[ deleted_start_index ];
            p1 = this->vertices_[ deleted_end_index ];
            bool is_deleted = this->itk_paths_.delete_one_path( p0, p1 );
          }

        }
        else
        {
          start_index = this->current_vertex_index_ - 1;
          end_index = this->current_vertex_index_ + 1;
        
        }
        compute_path( start_index, this->current_vertex_index_ );
        compute_path( this->current_vertex_index_, end_index );
      }
    }
  
    //Core::Path full_path;
    //full_path.set_start_point( this->itk_paths_.get_start_point() );
    //full_path.set_end_point( this->itk_paths_.get_end_point() );

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
      
      //full_path.add_one_path( new_path );
      this->world_paths_.add_one_path( new_path );
    }

    Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
      speedline_tool->itk_path_state_, this->itk_paths_, Core::ActionSource::NONE_E ) );

    //Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
    //  speedline_tool->path_state_, full_path, Core::ActionSource::NONE_E ) );

    Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
      speedline_tool->path_state_, this->world_paths_, Core::ActionSource::NONE_E ) );
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

ActionSpeedline::ActionSpeedline() 
{
  this->add_layer_id(  this->target_layer_id_ );
  this->add_parameter( this->slice_type_ );
  this->add_parameter( this->slice_number_ );
  this->add_parameter( this->vertices_ );
  this->add_parameter( this->iterations_ );
  this->add_parameter( this->termination_ );
}

bool ActionSpeedline::validate( Core::ActionContextHandle& context )
{
  // Check whether the target layer exists
  DataLayerHandle target_layer = boost::dynamic_pointer_cast< DataLayer >( 
    LayerManager::Instance()->get_layer_by_id( this->target_layer_id_ ) );
  if ( !target_layer )
  {
    context->report_error( "Layer '" + this->target_layer_id_ +
      "' is not a valid mask layer." );
    return false;
  }

  // Check whether the target layer can be used for processing
  Core::NotifierHandle notifier;
  if ( !LayerManager::Instance()->CheckLayerAvailabilityForProcessing(
    this->target_layer_id_, context ) ) return false;
  
  this->target_layer_ = LayerManager::FindDataLayer( 
    this->target_layer_id_ );
  
  if ( this->slice_type_ != Core::VolumeSliceType::AXIAL_E &&
    this->slice_type_ != Core::VolumeSliceType::CORONAL_E &&
    this->slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->slice_type_ );
  Core::DataVolumeSliceHandle volume_slice( new Core::DataVolumeSlice(
    this->target_layer_->get_data_volume(), slice_type ) );
  if ( this->slice_number_ >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }
  
  volume_slice->set_slice_number( this->slice_number_ );
  this->vol_slice_ = volume_slice;
  
  const std::vector< Core::Point >& vertices = this->vertices_;

  //if ( vertices.size() < 2 )
  //{
  //  context->report_error( "The Speedline has less than 2 vertices." );
  //  return false;
  //}
  
  return true;
}

bool ActionSpeedline::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  Core::DataVolumeSliceHandle volume_slice = this->vol_slice_;

  {
    // Get the layer on which this action operates
    LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
      this->target_layer_id_ );

    // Create a provenance record
    ProvenanceStepHandle provenance_step( new ProvenanceStep );

    // Get the input provenance ids from the translate step
    provenance_step->set_input_provenance_ids( this->get_input_provenance_ids() );

    // Get the output and replace provenance ids from the analysis above
    provenance_step->set_output_provenance_ids(  this->get_output_provenance_ids( 1 )  );

    ProvenanceIDList deleted_provenance_ids( 1, layer->provenance_id_state_->get() );
    provenance_step->set_replaced_provenance_ids( deleted_provenance_ids );

    provenance_step->set_action( this->export_to_provenance_string() );   

    ProvenanceStepID step_id = ProjectManager::Instance()->get_current_project()->
      add_provenance_record( provenance_step );   

    // Build the undo/redo for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Speedline" ) );

    // Get the axis along which the flood fill works
    Core::SliceType slice_type = static_cast< Core::SliceType::enum_type>(
      this->slice_type_ );

    // Get the slice number
    size_t slice_number = this->slice_number_;

    // Create a check point of the slice on which the flood fill will operate
    LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice_type, slice_number ) );

    // The redo action is the current one
    item->set_redo_action( this->shared_from_this() );

    // Tell the item which provenance record to delete when undone
    item->set_provenance_step_id( step_id );

    // Tell the item which layer to restore with which check point for the undo action
    item->add_layer_to_restore( layer, check_point );

    // Now add the undo/redo action to undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );

    // Add provenance id to output layer
    layer->provenance_id_state_->set( this->get_output_provenance_id( 0 ) );
  }

  // Create algorithm
  typedef boost::shared_ptr< ActionSpeedlineAlgo > ActionSpeedlineAlgoHandle;
  ActionSpeedlineAlgoHandle algo( new ActionSpeedlineAlgo );

  algo->target_layer_id_ = this->target_layer_id_ ;
  algo->slice_type_ = this->slice_type_ ;
  algo->slice_number_ = this->slice_number_;
  algo->vertices_ = this->vertices_;
  algo->current_vertex_index_ = this->current_vertex_index_;
  algo->itk_paths_ = this->itk_paths_;
  algo->iterations_ = this->iterations_;
  algo->termination_ = this->termination_;
  algo->target_layer_ = LayerManager::FindDataLayer( 
    this->target_layer_id_ );
  algo->update_all_paths_ = this->update_all_paths_;
  algo->speedline_tool_id_ = this->speedline_tool_id_;

  Core::Runnable::Start( algo );

  return true;
}

void ActionSpeedline::Dispatch( Core::ActionContextHandle context, 
                const std::string& layer_id, Core::VolumeSliceType slice_type, 
                size_t slice_number,
                const std::vector< Core::Point >& vertices,
                int current_vertex_index,
                const Core::Path& itk_paths,
                int iterations, double termination, 
                bool update_all_paths,
                std::string toolid )
{
  ActionSpeedline* action = new ActionSpeedline;

  action->target_layer_id_ = layer_id;
  action->slice_type_ = slice_type;
  action->slice_number_ = slice_number;
  action->vertices_ = vertices;
  action->current_vertex_index_ = current_vertex_index;
  action->itk_paths_ = itk_paths;
  action->iterations_ = iterations;
  action->termination_ = termination;
  action->update_all_paths_ = update_all_paths;
  action->speedline_tool_id_ = toolid;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
