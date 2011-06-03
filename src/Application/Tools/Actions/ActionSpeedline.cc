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
#include <itkExtractImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkPathIterator.h>
#include <itkGradientDescentOptimizer.h>
#include <itkRegularStepGradientDescentOptimizer.h>
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
  typedef float pixel_type;
  typedef itk::Image< pixel_type, DIMENSION_C > image_type;
  typedef itk::PolyLineParametricPath< DIMENSION_C > path_type;
  typedef itk::SpeedFunctionToPathFilter< image_type, path_type > path_filter_type;
  typedef path_filter_type::CostFunctionType::CoordRepType coord_rep_type;

  image_type::Pointer speed_image_2D_;
  std::vector< std::vector< Core::Point > > paths_;

public:
  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  bool erase_;
  std::vector< ActionSpeedline::VertexCoord > vertices_;

  DataLayerHandle target_layer_;
  Core::DataVolumeSliceHandle vol_slice_;

  int iterations_;
  double termination_;
  std::string speedline_tool_id_;



public:
  void compute_path ( int start_index, int end_index, int num_of_vertices, bool is_edge = false )
  {
    // Create interpolator
    typedef itk::LinearInterpolateImageFunction<image_type, coord_rep_type>
      interpolator_type;
    interpolator_type::Pointer interp = interpolator_type::New();

    // Create cost function
    path_filter_type::CostFunctionType::Pointer cost = path_filter_type::CostFunctionType::New();
    cost->SetInterpolator( interp );

    // Create optimizer
    typedef itk::GradientDescentOptimizer optimizer_type;
    //typedef itk::RegularStepGradientDescentOptimizer optimizer_type;
    optimizer_type::Pointer optimizer = optimizer_type::New();
    optimizer->SetNumberOfIterations( this->iterations_ );
    //optimizer->SetMaximumStepLength( 0.5 );
    //optimizer->SetMinimumStepLength( 0.1 );
    //optimizer->SetRelaxationFactor( 0.5 );

    // Create path filter
    path_filter_type::Pointer path_filter = path_filter_type::New();
    
    path_filter->SetCostFunction( cost );
    path_filter->SetOptimizer( optimizer );
    path_filter->SetTerminationValue( this->termination_ );
    path_filter->SetInput( this->speed_image_2D_ );

    for ( size_t i = start_index; i < end_index; ++i )
    {
      // Add the seed points.
      path_filter_type::PathInfo info;
      path_filter_type::PointType pnt;

      if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
      {
        pnt[0] = this->vertices_[ i ].y();
        pnt[1] = this->vertices_[ i ].z();
      }
      else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
      {
        pnt[0] = this->vertices_[ i ].x();
        pnt[1] = this->vertices_[ i ].z();
      }
      else
      {
        pnt[0] = this->vertices_[ i ].x();
        pnt[1] = this->vertices_[ i ].y();
      }

      info.SetEndPoint( pnt );

      if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
      {
        if ( i == num_of_vertices - 1 )
        {
          pnt[0] = this->vertices_[ 0 ].y();
          pnt[1] = this->vertices_[ 0 ].z();
        }
        else
        {
          pnt[0] = this->vertices_[ i + 1 ].y();
          pnt[1] = this->vertices_[ i + 1 ].z();
        }

      }
      else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
      {
        if ( i == num_of_vertices - 1 )
        {
          pnt[0] = this->vertices_[ 0 ].x();
          pnt[1] = this->vertices_[ 0 ].z();

        }
        else
        {
          pnt[0] = this->vertices_[ i + 1 ].x();
          pnt[1] = this->vertices_[ i + 1 ].z();
        }

      }
      else
      {
        if ( i == num_of_vertices - 1 )
        {
          pnt[0] = this->vertices_[ 0 ].x();
          pnt[1] = this->vertices_[ 0 ].y();
        }
        else
        {
          pnt[0] = this->vertices_[ i + 1 ].x();
          pnt[1] = this->vertices_[ i + 1 ].y();
        }

      }

      info.SetStartPoint( pnt );

      path_filter->AddPathInfo( info );
    }

    path_filter->Update();

    size_t num_of_outputs = path_filter->GetNumberOfOutputs();
    for ( size_t j = 0; j < num_of_outputs; j++ )
    {
      this->paths_[ j + start_index ].clear();
      path_type::Pointer path = path_filter->GetOutput( j );

      size_t vertext_list_size = path->GetVertexList()->Size();
      for ( unsigned int k = 0; k < vertext_list_size; k++ )
      {
        const float x = path->GetVertexList()->ElementAt( k )[0];
        const float y = path->GetVertexList()->ElementAt( k )[1];

        Core::Point ipnt;

        if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E ) // SAGITTAL_E = 2
        {
          ipnt[0] = this->slice_number_;
          ipnt[2] = x;
          ipnt[3] = y;
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
        this->paths_[ j + start_index ].push_back( ipnt );
      }
    }
  }

  SCI_BEGIN_TYPED_ITK_RUN( this->target_layer_->get_data_type() )
  {
    //-----------------------------------------------------------
    // Set up ITK SpeedLine filter
    //-----------------------------------------------------------
    //const unsigned int DIMENSION_C = 2;
    //typedef float pixel_type;
    //typedef itk::Image< pixel_type, DIMENSION_C > image_type;
    //typedef itk::PolyLineParametricPath< DIMENSION_C > path_type;
    //typedef itk::SpeedFunctionToPathFilter< image_type, path_type > path_filter_type;
    //typedef path_filter_type::CostFunctionType::CoordRepType coord_rep_type;

    typename Core::ITKImageDataT<VALUE_TYPE>::Handle speed_image_3D;
    this->get_itk_image_from_layer<VALUE_TYPE>( this->target_layer_, speed_image_3D );

    // Create 2D itk image
    typedef itk::ExtractImageFilter<TYPED_IMAGE_TYPE, image_type >  extract_filter_type;
    extract_filter_type::Pointer extract_filter = extract_filter_type::New();
    
    TYPED_IMAGE_TYPE::RegionType input_region = speed_image_3D->get_image()->GetLargestPossibleRegion();
    TYPED_IMAGE_TYPE::SizeType size = input_region.GetSize();
    size[2] = 0;
    TYPED_IMAGE_TYPE::IndexType start = input_region.GetIndex();
    start[2] = this->slice_number_;
    TYPED_IMAGE_TYPE::RegionType desired_region;
    desired_region.SetSize( size );
    desired_region.SetIndex( start );

    extract_filter->SetExtractionRegion( desired_region );
    extract_filter->SetInput( speed_image_3D->get_image() );

    // typename image_type::Pointer speed_image_2D = extract_filter->GetOutput();
    this->speed_image_2D_ = extract_filter->GetOutput();

    size_t num_of_vertices = this->vertices_.size();

    // NOTE: ITK filter uses continueIndex instead of real physical point
    // So we need to convert the point position to continue index
    typedef itk::ContinuousIndex< double, 3 >   continuous_index_type;
    typedef itk::Point< double, 3 > itk_point_type;

    // std::vector< Core::Point > cindices;

    //for ( size_t i = 0; i < num_of_vertices; ++i )
    //{
    //  continuous_index_type cindex;
    //  itk_point_type itk_point;
    //  itk_point[ 0 ] = this->vertices_[ i ].x();
    //  itk_point[ 1 ] = this->vertices_[ i ].y();
    //  itk_point[ 2 ] = this->vertices_[ i ].z();

    //  speed_image_3D->get_image()->TransformPhysicalPointToContinuousIndex( itk_point, cindex );
    //    Core::Point cpoint( cindex[ 0 ], cindex[ 1 ], cindex[ 2 ] );
    //  cindices.push_back( cpoint );
    //}

    //std::vector< std::vector< path_filter_type::PointType > > paths;
    //std::vector< std::vector< Core::Point > > paths;

    //if ( paths.size() != num_of_vertices )
    //{
    //  paths.resize( num_of_vertices );
    //} 
    
    SpeedlineToolHandle speedline_tool = 
      boost::dynamic_pointer_cast<SpeedlineTool> (ToolManager::Instance()->get_tool(this->speedline_tool_id_));
    int current_vertex_index;
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      current_vertex_index = speedline_tool->current_vertex_index_state_->get();
    }

    // for ( size_t i = 0; i < num_of_vertices - 1; ++i )
    int start_index;
    int end_index;

    if ( num_of_vertices < 2 )
    {
      return;
    }
    else if ( num_of_vertices == 2 )
    {
      start_index = 0;
      end_index = 1;
      this->paths_.resize( num_of_vertices );
      compute_path( start_index, end_index, num_of_vertices );
    }
    else
    { 
      if ( this->paths_.size() != num_of_vertices  )
      {       
        this->paths_.resize( num_of_vertices );
        start_index = 0;
        end_index = num_of_vertices - 1;
        compute_path( start_index, end_index, num_of_vertices );
      }
      else
      {
        start_index = 0;
        end_index = num_of_vertices - 1;
        compute_path( start_index, end_index, num_of_vertices );
      }
    }

    Core::Path full_path;
    for ( unsigned int i = 0; i < this->paths_.size(); ++i )
    {
      std::vector< Core::Point > pt;
      for ( unsigned int j = 0; j < this->paths_[i].size(); ++j )
      {
        continuous_index_type cindex;
        itk_point_type itk_point;

        cindex[ 0 ] =  this->paths_[i][j].x();
        cindex[ 1 ] =  this->paths_[i][j].y();
        cindex[ 2 ] =  this->paths_[i][j].z();

        speed_image_3D->get_image()->TransformContinuousIndexToPhysicalPoint( cindex, itk_point );  
        Core::Point cpoint( itk_point[ 0 ], itk_point[ 1 ], itk_point[ 2 ] );
        pt.push_back( cpoint );
      }
      full_path.add_one_path( pt );
    }
  
    //Core::Application::PostEvent( boost::bind( &Core::StatePointVector::set,
    //  speedline_tool->path_state_, single_path, Core::ActionSource::NONE_E ) );

    Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
      speedline_tool->path_state_, full_path, Core::ActionSource::NONE_E ) );
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
  this->add_parameter( this->erase_ );
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
  
  const std::vector< VertexCoord >& vertices = this->vertices_;

  if ( vertices.size() < 2 )
  {
    context->report_error( "The Speedline has less than 2 vertices." );
    return false;
  }
  
  return true;
}

bool ActionSpeedline::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  Core::DataVolumeSliceHandle volume_slice = this->vol_slice_;
  //size_t nx = volume_slice->nx();
  //size_t ny = volume_slice->ny();

  //// Compute the bounding box of the Speedline
  //int min_x = std::numeric_limits< int >::max();
  //int max_x = std::numeric_limits< int >::min();
  //int min_y = min_x;
  //int max_y = max_x;
  //const std::vector< VertexCoord >& vertices = this->vertices_;
  //size_t num_of_vertices = vertices.size();
  //for ( size_t i = 0; i < num_of_vertices; ++i )
  //{
  //  min_x = Core::Min( min_x, static_cast< int >( vertices[ i ][ 0 ] ) );
  //  max_x = Core::Max( max_x, static_cast< int >( vertices[ i ][ 0 ] ) );
  //  min_y = Core::Min( min_y, static_cast< int >( vertices[ i ][ 1 ] ) );
  //  max_y = Core::Max( max_y, static_cast< int >( vertices[ i ][ 1 ] ) );
  //}

  //// If the Speedline doesn't overlap the slice, no need to proceed
  //if ( min_x >= static_cast< int >( nx ) ||
  //  max_x < 0 || max_y < 0 ||
  //  min_y >= static_cast< int >( ny ) )
  //{
  //  return false;
  //}

  {
    // Build the undo/redo for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Speedline" ) );

    // Get the axis along which the flood fill works
    Core::SliceType slice_type = static_cast< Core::SliceType::enum_type>(
      this->slice_type_ );

    // Get the slice number
    size_t slice_number = this->slice_number_;

    // Get the layer on which this action operates
    LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
      this->target_layer_id_ );
    // Create a check point of the slice on which the flood fill will operate
    LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice_type, slice_number ) );

    // The redo action is the current one
    item->set_redo_action( this->shared_from_this() );

    // Tell the item which layer to restore with which check point for the undo action
    item->add_layer_to_restore( layer, check_point );

    // Now add the undo/redo action to undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );

    // Add provenance id to output layer
    layer->provenance_id_state_->set( this->get_output_provenance_id( 0 ) );

    // Create a provenance record
    ProvenanceStepHandle provenance_step( new ProvenanceStep );

    // Get the input provenance ids from the translate step
    provenance_step->set_input_provenance_ids( this->get_input_provenance_ids() );

    // Get the output and replace provenance ids from the analysis above
    provenance_step->set_output_provenance_ids(  this->get_output_provenance_ids()  );

    ProvenanceIDList deleted_provenance_ids( 1, layer->provenance_id_state_->get() );
    provenance_step->set_deleted_provenance_ids( deleted_provenance_ids );

    provenance_step->set_action( this->export_to_provenance_string() );   

    ProjectManager::Instance()->get_current_project()->add_to_provenance_database(
      provenance_step );    
  }

  // Create algorithm
  boost::shared_ptr< ActionSpeedlineAlgo > algo( new ActionSpeedlineAlgo );

  algo->target_layer_id_ = this->target_layer_id_ ;
  algo->slice_type_ = this->slice_type_ ;
  algo->slice_number_ = this->slice_number_;
  algo->erase_  = this->erase_;
  algo->vertices_ = this->vertices_;
  algo->iterations_ = this->iterations_;
  algo->termination_ = this->termination_;
  algo->target_layer_ = LayerManager::FindDataLayer( 
    this->target_layer_id_ );
  algo->speedline_tool_id_ = this->speedline_tool_id_;

  Core::Runnable::Start( algo );

  return true;
}

//void ActionSpeedline::clear_cache()
//{
//  this->algo_->target_layer_.reset();
//  this->algo_->vol_slice_.reset();
//}


void ActionSpeedline::Dispatch( Core::ActionContextHandle context, 
                const std::string& layer_id, Core::VolumeSliceType slice_type, 
                size_t slice_number, bool erase, 
                const std::vector< VertexCoord >& vertices,
                int iterations, double termination, 
                std::string toolid )
{
  ActionSpeedline* action = new ActionSpeedline;


  action->target_layer_id_ = layer_id;
  action->slice_type_ = slice_type;
  action->slice_number_ = slice_number;
  action->erase_ = erase;
  action->vertices_ = vertices;
  action->iterations_ = iterations;
  action->termination_ = termination;
  action->speedline_tool_id_ = toolid;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
