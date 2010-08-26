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

#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionPolyline.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>

CORE_REGISTER_ACTION( Seg3D, Polyline )

namespace Seg3D
{

class ActionPolylinePrivate
{
public:
  Core::ActionParameter< std::string > target_layer_id_;
  Core::ActionParameter< int > slice_type_;
  Core::ActionParameter< size_t > slice_number_;
  Core::ActionParameter< bool > erase_;
  Core::ActionParameter< std::vector< ActionPolyline::VertexCoord > > vertices_;

  Core::ActionCachedHandle< Core::MaskLayerHandle > target_layer_;
  Core::ActionCachedHandle< Core::MaskVolumeSliceHandle > vol_slice_;
};

ActionPolyline::ActionPolyline() :
  private_( new ActionPolylinePrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->slice_number_ );
  this->add_argument( this->private_->erase_ );
  this->add_argument( this->private_->vertices_ );
  this->add_cachedhandle( this->private_->target_layer_ );
  this->add_cachedhandle( this->private_->vol_slice_ );
}

ActionPolyline::~ActionPolyline()
{
}

bool ActionPolyline::validate( Core::ActionContextHandle& context )
{
  if ( !this->cache_mask_layer_handle( context, this->private_->target_layer_id_,
    this->private_->target_layer_ ) )
  {
    return false;
  }

  if ( !this->private_->target_layer_.handle()->is_valid() )
  {
    context->report_error( "Mask layer '" + this->private_->target_layer_id_.value() + 
      "' is invalid." );
    return false;
  }
  
  if ( this->private_->slice_type_.value() != Core::VolumeSliceType::AXIAL_E &&
    this->private_->slice_type_.value() != Core::VolumeSliceType::CORONAL_E &&
    this->private_->slice_type_.value() != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_.value() );
  Core::MaskVolumeSliceHandle volume_slice( new Core::MaskVolumeSlice(
    this->private_->target_layer_.handle()->get_mask_volume(), slice_type ) );
  if ( this->private_->slice_number_.value() >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }
  volume_slice->set_slice_number( this->private_->slice_number_.value() );
  this->private_->vol_slice_.handle() = volume_slice;
  
  const std::vector< VertexCoord >& vertices = this->private_->vertices_.value();
  if ( vertices.size() <= 2 )
  {
    context->report_error( "The polyline has less than 3 vertices." );
    return false;
  }
  
  return true;
}

inline void ComputeIntersection( const ActionPolyline::VertexCoord& start, 
  const ActionPolyline::VertexCoord& end, int y, 
  std::vector< int >& intersections, std::vector< int >& horizontal_edges )
{
  // If the edge is horizontal, treat it as no intersection
  if ( end[ 1 ] == start[ 1 ] )
  {
    if ( start[ 1 ] == y )
    {
      if ( start[ 0 ] < end[ 0 ] )
      {
        horizontal_edges.push_back( static_cast< int >( start[ 0 ] ) );
        horizontal_edges.push_back( static_cast< int >( end[ 0 ] ) );
      }
      else
      {
        horizontal_edges.push_back( static_cast< int >( end[ 0 ] ) );
        horizontal_edges.push_back( static_cast< int >( start[ 0 ] ) );
      }
    }
    
    return;
  }

  // If the scanline falls on one of the endpoints of the edge, then only
  // count in the intersection if the edge is below the scanline
  if ( start[ 1 ] == y )
  {
    if ( end[ 1 ] < y )
    {
      intersections.push_back( static_cast< int >( start[ 0 ] ) );
    }
    else
    {
      horizontal_edges.push_back( static_cast< int >( start[ 0 ] ) );
      horizontal_edges.push_back( static_cast< int >( start[ 0 ] ) );
    }
    return;
  }

  if ( end[ 1 ] == y )
  {
    if ( start[ 1 ] < y )
    {
      intersections.push_back( static_cast< int >( end[ 0 ] ) );
    }
    else
    {
      horizontal_edges.push_back( static_cast< int >( end[ 0 ] ) );
      horizontal_edges.push_back( static_cast< int >( end[ 0 ] ) );
    }
    return;
  }

  float t = ( y -start[ 1 ] ) / ( end[ 1 ] - start[ 1 ] );
  if ( t <= 0.0f || t >= 1.0f )
  {
    return;
  }
  intersections.push_back( Core::Round( ( 1 - t ) * start[ 0 ] + t * end[ 0 ] ) );
}

bool ActionPolyline::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  Core::MaskVolumeSliceHandle volume_slice = this->private_->vol_slice_.handle();
  size_t nx = volume_slice->nx();
  size_t ny = volume_slice->ny();

  // Compute the bounding box of the polyline
  int min_x = std::numeric_limits< int >::max();
  int max_x = std::numeric_limits< int >::min();
  int min_y = min_x;
  int max_y = max_x;
  const std::vector< VertexCoord >& vertices = this->private_->vertices_.value();
  size_t num_of_vertices = vertices.size();
  for ( size_t i = 0; i < num_of_vertices; ++i )
  {
    min_x = Core::Min( min_x, static_cast< int >( vertices[ i ][ 0 ] ) );
    max_x = Core::Max( max_x, static_cast< int >( vertices[ i ][ 0 ] ) );
    min_y = Core::Min( min_y, static_cast< int >( vertices[ i ][ 1 ] ) );
    max_y = Core::Max( max_y, static_cast< int >( vertices[ i ][ 1 ] ) );
  }
  
  // If the polyline doesn't overlap the slice, no need to proceed
  if ( min_x >= static_cast< int >( nx ) ||
    max_x < 0 || max_y < 0 ||
    min_y >= static_cast< int >( ny ) )
  {
    return false;
  }
  
  // Otherwise, do a scanline fill in the overlapped region

  min_y = Core::Max( min_y, 0 );
  max_y = Core::Min( max_y, static_cast< int >( ny - 1 ) );
  Core::MaskDataBlockHandle mask_data_block = volume_slice->get_mask_data_block();

  // Lock the mask data block
  Core::MaskDataBlock::lock_type mask_data_lock( mask_data_block->get_mutex() );
  
  unsigned char* mask_data = mask_data_block->get_mask_data();
  unsigned char mask_value = mask_data_block->get_mask_value();
  unsigned char not_mask_value = ~mask_value;
  bool erase = this->private_->erase_.value();
  const size_t x_stride = volume_slice->to_index( 1, 0 ) - volume_slice->to_index( 0, 0 );
  for ( int y = min_y; y <= max_y; ++y )
  {
    std::vector< int > intersections;
    std::vector< int > horizontal_edges;

    // Compute the intersections of the scanline with each edge
    for ( size_t i = 0; i < num_of_vertices - 1; ++i )
    {
      ComputeIntersection( vertices[ i ], vertices[ i + 1 ], y, 
        intersections, horizontal_edges );
    }
    ComputeIntersection( vertices[ num_of_vertices - 1 ], vertices[ 0 ], y, 
      intersections, horizontal_edges );

    // Sort the intersections in left-to-right order
    std::sort( intersections.begin(), intersections.end() );

    // Fill the pixels between each pair of intersection points
    intersections.insert( intersections.end(), horizontal_edges.begin(), 
      horizontal_edges.end() );
    assert( intersections.size() % 2 == 0 );
    for ( size_t i = 0; i < intersections.size(); i += 2 )
    {
      int x0 = Core::Max( intersections[ i ], 0 );
      int x1 = Core::Min( intersections[ i + 1 ], static_cast< int >( nx - 1 ) );
      if ( x0 >= static_cast< int >( nx ) || x1 < 0 )
      {
        continue;
      }
      
      size_t start = volume_slice->to_index( static_cast< size_t >( x0 ), 
        static_cast< size_t >( y ) );
      size_t end = volume_slice->to_index( static_cast< size_t >( x1 ),
        static_cast< size_t >( y ) );
      for ( size_t index = start; index <= end; index += x_stride )
      {
        if ( erase )
        {
          mask_data[ index ] &= not_mask_value;
        }
        else
        {
          mask_data[ index ] |= mask_value;
        }
      }
    }
  }
  
  mask_data_lock.unlock();
  mask_data_block->increase_generation();
  mask_data_block->mask_updated_signal_();

  return true;
}

void ActionPolyline::Dispatch( Core::ActionContextHandle context, 
                const std::string& layer_id, Core::VolumeSliceType slice_type, 
                size_t slice_number, bool erase, 
                const std::vector< VertexCoord >& vertices )
{
  ActionPolyline* action = new ActionPolyline;
  action->private_->target_layer_id_.value() = layer_id;
  action->private_->slice_type_.value() = slice_type;
  action->private_->slice_number_.value() = slice_number;
  action->private_->erase_.value() = erase;
  action->private_->vertices_.value() = vertices;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D