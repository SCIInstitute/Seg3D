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

#include <Core/Math/MathFunctions.h>
#include <Core/Volume/VolumeSlice.h>
#include <Core/RenderResources/RenderResources.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class VolumeSlicePrivate
//////////////////////////////////////////////////////////////////////////

class VolumeSlicePrivate
{
public:
  VolumeSlicePrivate( const VolumeHandle& volume, 
    VolumeSliceType type, size_t slice_num ) :
    slice_changed_( true ), 
    size_changed_( true ),
    out_of_boundary_( false ),
    volume_( volume ), 
    grid_transform_( volume->get_grid_transform() ),
    inverse_transform_( volume->get_inverse_transform() ),
    grid_nx_( volume->get_nx() ),
    grid_ny_( volume->get_ny() ),
    grid_nz_( volume->get_nz() ),
    slice_type_( type ), 
    slice_number_ ( slice_num )
  {
  }

  VolumeSlicePrivate( const VolumeSlicePrivate& copy ) :
    slice_changed_( copy.slice_changed_ ),
    size_changed_( copy.size_changed_ ),
    nx_( copy.nx_ ),
    ny_( copy.ny_ ),
    number_of_slices_( copy.number_of_slices_ ),
    out_of_boundary_( copy.out_of_boundary_ ),
    left_( copy.left_ ), right_( copy.right_ ), 
    bottom_( copy.bottom_ ), top_( copy.top_ ),
    depth_( copy.depth_ ),
    bottom_left_( copy.bottom_left_ ),
    bottom_right_( copy.bottom_right_ ),
    top_left_( copy.top_left_ ),
    top_right_( copy.top_right_ ),
    texture_( copy.texture_ ),
    volume_( copy.volume_ ),
    grid_transform_( copy.grid_transform_ ),
    inverse_transform_( copy.inverse_transform_ ),
    grid_nx_( copy.grid_nx_ ),
    grid_ny_( copy.grid_ny_ ),
    grid_nz_( copy.grid_nz_ ),
    slice_type_( copy.slice_type_ ),
    slice_number_( copy.slice_number_ )
  {
  }

  // UPDATE_DIMENSION:
  // Update the dimension of the slice.
  void update_dimension();

  // UPDATE_POSITION:
  // Update the position of the slice.
  void update_position();

  size_t to_index( size_t x, size_t y, size_t z )
  {
//    assert( x < this->grid_nx_ && y < this->grid_ny_ && z < this->grid_nz_ );
    return z * this->grid_ny_ * this->grid_nx_ + y * this->grid_nx_ + x;
  }

  bool slice_changed_;
  bool size_changed_;
  size_t nx_;
  size_t ny_;
  size_t number_of_slices_;
  bool out_of_boundary_;

  double left_;
  double right_;
  double bottom_;
  double top_;
  double depth_;
  Point bottom_left_;
  Point bottom_right_;
  Point top_left_;
  Point top_right_;

  Texture2DHandle texture_;

  VolumeHandle volume_;
  
  const GridTransform grid_transform_;
  const Transform inverse_transform_;
  const size_t grid_nx_;
  const size_t grid_ny_;
  const size_t grid_nz_;

  VolumeSliceType slice_type_;
  size_t slice_number_;
  VolumeSlice* slice_;
};

void VolumeSlicePrivate::update_dimension()
{
  switch( this->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    this->nx_ = this->grid_nx_;
    this->ny_ = this->grid_ny_;
    this->number_of_slices_ = this->grid_nz_;
    break;
  case VolumeSliceType::CORONAL_E:
    this->nx_ = this->grid_nx_;
    this->ny_ = this->grid_nz_;
    this->number_of_slices_ = this->grid_ny_;
    break;
  case VolumeSliceType::SAGITTAL_E:
    this->nx_ = this->grid_ny_;
    this->ny_ = this->grid_nz_;
    this->number_of_slices_ = this->grid_nx_;
    break;
  default:
    assert( false );
    break;
  }
}

void VolumeSlicePrivate::update_position()
{
  Point index;

  switch( this->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    this->slice_->to_index( 0, 0, index );
    this->bottom_left_ = this->grid_transform_ * index;
    this->slice_->to_index( this->nx_ - 1, 0, index );
    this->bottom_right_ = this->grid_transform_ * index;
    this->slice_->to_index( this->nx_ - 1, this->ny_ - 1, index );
    this->top_right_ = this->grid_transform_ * index;
    this->slice_->to_index( 0, this->ny_ - 1, index );
    this->top_left_ = this->grid_transform_ * index;
    this->left_ = this->bottom_left_.x();
    this->right_ = this->top_right_.x();
    this->bottom_ = this->bottom_left_.y();
    this->top_ = this->top_right_.y();
    this->depth_ = this->bottom_left_.z();
    break;
  case VolumeSliceType::CORONAL_E:
    this->slice_->to_index( 0, 0, index );
    this->bottom_left_ = this->grid_transform_ * index;
    this->slice_->to_index( this->nx_ - 1, 0, index );
    this->bottom_right_ = this->grid_transform_ * index;
    this->slice_->to_index( this->nx_ - 1, this->ny_ - 1, index );
    this->top_right_ = this->grid_transform_ * index;
    this->slice_->to_index( 0, this->ny_ - 1, index );
    this->top_left_ = this->grid_transform_ * index;
    this->left_ = this->bottom_left_.x();
    this->right_ = this->top_right_.x();
    this->bottom_ = this->bottom_left_.z();
    this->top_ = this->top_right_.z();
    this->depth_ = this->bottom_left_.y();
    break;
  case VolumeSliceType::SAGITTAL_E:
    this->slice_->to_index( 0, 0, index );
    this->bottom_left_ = this->grid_transform_ * index;
    this->slice_->to_index( this->nx_ - 1, 0, index );
    this->bottom_right_ = this->grid_transform_ * index;
    this->slice_->to_index( this->nx_ - 1, this->ny_ - 1, index );
    this->top_right_ = this->grid_transform_ * index;
    this->slice_->to_index( 0, this->ny_ - 1, index );
    this->top_left_ = this->grid_transform_ * index;
    this->left_ = this->bottom_left_.y();
    this->right_ = this->top_right_.y();
    this->bottom_ = this->bottom_left_.z();
    this->top_ = this->top_right_.z();
    this->depth_ = this->bottom_left_.x();
    break;
  default:
    assert( false );
    break;
  }
}

//////////////////////////////////////////////////////////////////////////
// Class VolumeSlice
//////////////////////////////////////////////////////////////////////////

VolumeSlice::VolumeSlice( const VolumeHandle& volume, 
             VolumeSliceType type, size_t slice_num ) :
  private_( new VolumeSlicePrivate( volume, type, slice_num ) )
{
  this->private_->slice_ = this;
  this->private_->update_dimension();
  this->private_->slice_number_ = Min( this->private_->slice_number_, 
    this->private_->number_of_slices_ - 1 );
  this->private_->update_position();
}

VolumeSlice::VolumeSlice( const VolumeSlice& copy ) :
  private_( new VolumeSlicePrivate( *copy.private_ ) )
{
  this->private_->slice_ = this;
}

VolumeSlice::~VolumeSlice()
{
  this->disconnect_all();
}

void VolumeSlice::set_slice_type( VolumeSliceType type )
{
  if ( this->private_->slice_type_ != type )
  {
    this->private_->slice_changed_ = true;
    this->private_->size_changed_ = true;
    this->private_->slice_type_ = type;

    this->private_->update_dimension();
    this->private_->slice_number_ = Min( this->private_->slice_number_, 
      this->private_->number_of_slices_ - 1 );
    this->private_->update_position();
  }
}

VolumeSliceType VolumeSlice::get_slice_type() const
{
  return this->private_->slice_type_;
}

void VolumeSlice::set_slice_number( size_t slice_num )
{
  slice_num = Min( slice_num, this->private_->number_of_slices_ - 1 );
  this->private_->out_of_boundary_ = false;
  if ( this->private_->slice_number_ != slice_num )
  {
    this->private_->slice_number_ = slice_num;
    this->private_->slice_changed_ = true;
    this->private_->update_position();
  }
}

size_t VolumeSlice::to_index( size_t i, size_t j ) const
{
  switch ( this->private_->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    return this->private_->to_index( i, j, this->private_->slice_number_ );
  case VolumeSliceType::CORONAL_E:
    return this->private_->to_index( i, this->private_->slice_number_, j );
  default:
    return this->private_->to_index( this->private_->slice_number_, i, j );
  }
}

void VolumeSlice::to_index( size_t i, size_t j, Point& index ) const
{
  switch ( this->private_->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    index[ 0 ] = static_cast<double>( i );
    index[ 1 ] = static_cast<double>( j );
    index[ 2 ] = static_cast<double>( this->private_->slice_number_ );
    break;
  case VolumeSliceType::CORONAL_E:
    index[ 0 ] = static_cast<double>( i );
    index[ 1 ] = static_cast<double>( this->private_->slice_number_ );
    index[ 2 ] = static_cast<double>( j );
    break;
  default:
    index[ 0 ] = static_cast<double>( this->private_->slice_number_ );
    index[ 1 ] = static_cast<double>( i );
    index[ 2 ] = static_cast<double>( j );
  }
}

void VolumeSlice::world_to_index( double x_pos, double y_pos, int& i, int& j ) const
{
  Point pos;
  switch ( this->private_->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    pos = this->private_->inverse_transform_ * Point( x_pos, y_pos, 0 );
    i = Round( pos.x() );
    j = Round( pos.y() );
    break;
  case VolumeSliceType::CORONAL_E:
    pos = this->private_->inverse_transform_ * Point( x_pos, 0, y_pos );
    i = Round( pos.x() );
    j = Round( pos.z() );
    break;
  case VolumeSliceType::SAGITTAL_E:
    pos = this->private_->inverse_transform_ * Point( 0, x_pos, y_pos );
    i = Round( pos.y() );
    j = Round( pos.z() );
    break;
  default:
    break;
  }
}

void VolumeSlice::index_to_world( int i, int j, double& world_i, double& world_j ) const
{
  Point pos;
  double i_pos = static_cast< double >(i);
  double j_pos = static_cast< double >(j);
  switch ( this->private_->slice_type_ )
  {
    case VolumeSliceType::AXIAL_E:
      pos = this->private_->grid_transform_ * Point( i_pos, j_pos, 0 );
      world_i = pos.x();
      world_j = pos.y();
      break;
    case VolumeSliceType::CORONAL_E:
      pos = this->private_->grid_transform_ * Point( i_pos, 0, j_pos );
      world_i = pos.x();
      world_j = pos.z();
      break;
    case VolumeSliceType::SAGITTAL_E:
      pos = this->private_->grid_transform_ * Point( 0, i_pos, j_pos );
      world_i = pos.y();
      world_j = pos.z();
      break;
    default:
      break;
  }
}

void VolumeSlice::get_world_coord( double i_pos, double j_pos, Point& world_coord ) const
{
  switch( this->private_->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    world_coord[ 0 ] = i_pos;
    world_coord[ 1 ] = j_pos;
    world_coord[ 2 ] = this->private_->depth_;
    break;
  case VolumeSliceType::CORONAL_E:
    world_coord[ 0 ] = i_pos;
    world_coord[ 2 ] = j_pos;
    world_coord[ 1 ] = this->private_->depth_;
    break;
  case VolumeSliceType::SAGITTAL_E:
    world_coord[ 1 ] = i_pos;
    world_coord[ 2 ] = j_pos;
    world_coord[ 0 ] = this->private_->depth_;
    break;
  default:
    assert( false );
    break;
  }
}

void VolumeSlice::project_onto_slice( const Point& pt, double& i_pos, double& j_pos ) const
{
  VolumeSlice::ProjectOntoSlice( this->private_->slice_type_, pt, i_pos, j_pos );
}

void VolumeSlice::project_onto_slice( const Point& pt, double& i_pos, double& j_pos, double& depth ) const
{
  VolumeSlice::ProjectOntoSlice( this->private_->slice_type_, pt, i_pos, j_pos, depth );
}

void VolumeSlice::move_slice_to( const Point& pos, bool fail_safe )
{
  int slice_num = this->get_closest_slice( pos );
  if ( ( slice_num < 0 || 
    slice_num >= static_cast< int >( this->private_->number_of_slices_ ) ) && 
     !fail_safe )
  {
    this->private_->out_of_boundary_ = true;
    return;
  }

  slice_num = Max( 0, slice_num );
  slice_num = Min( slice_num, static_cast< int >( this->private_->number_of_slices_ - 1 ) );

  this->set_slice_number( static_cast< size_t >( slice_num ) );
}

void VolumeSlice::move_slice_to( double depth, bool fail_safe )
{
  Point pos;
  switch ( this->private_->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    pos = Point( 0, 0, depth );
    break;
  case VolumeSliceType::CORONAL_E:
    pos = Point( 0, depth, 0 );
    break;
  case VolumeSliceType::SAGITTAL_E:
    pos = Point( depth, 0, 0 );
    break;
  }

  this->move_slice_to( pos, fail_safe );
}

int VolumeSlice::get_closest_slice( const Point& pt ) const
{
  Point index = this->private_->inverse_transform_ * pt;
  int slice_num = -1;
  switch ( this->private_->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    slice_num = Round( index.z() );
    break;
  case VolumeSliceType::CORONAL_E:
    slice_num = Round( index.y() );
    break;
  case VolumeSliceType::SAGITTAL_E:
    slice_num = Round( index.x() );
    break;
  }

  return slice_num;
}

void VolumeSlice::initialize_texture()
{
  lock_type lock( this->get_mutex() );
  if ( !this->private_->texture_ )
  {
    RenderResources::lock_type rr_lock( RenderResources::GetMutex() );
    this->private_->texture_.reset( new Texture2D );
    this->private_->texture_->set_mag_filter( GL_NEAREST );
    this->private_->texture_->set_min_filter( GL_NEAREST );
  }
}

void VolumeSlice::set_volume( const VolumeHandle& volume )
{
  lock_type lock( this->get_mutex() );
  this->disconnect_all();
  this->private_->volume_ = volume;
  this->private_->slice_changed_ = true;
}

VolumeHandle VolumeSlice::get_volume() const
{
  lock_type lock( this->get_mutex() );
  return this->private_->volume_;
}

VolumeType VolumeSlice::volume_type() const
{
  lock_type lock( this->get_mutex() );
  return this->private_->volume_->get_type();
}

void VolumeSlice::handle_volume_updated()
{
  lock_type lock( this->get_mutex() );
  this->private_->slice_changed_ = true;
}

Point VolumeSlice::apply_grid_transform( const Point& pt ) const
{
  return this->private_->grid_transform_ * pt;
}

Point VolumeSlice::apply_inverse_grid_transform( const Point& pt ) const
{
  return this->private_->inverse_transform_ * pt;
}

Core::Texture2DHandle VolumeSlice::get_texture()
{
  lock_type lock( this->get_mutex() );
  return this->private_->texture_;
}

size_t VolumeSlice::get_slice_number() const
{
  return this->private_->slice_number_;
}

size_t VolumeSlice::nx() const
{
  return this->private_->nx_;
}

size_t VolumeSlice::ny() const
{
  return this->private_->ny_;
}

size_t VolumeSlice::number_of_slices() const
{
  return this->private_->number_of_slices_;
}

bool VolumeSlice::out_of_boundary() const
{
  return this->private_->out_of_boundary_;
}

double VolumeSlice::left() const
{
  return this->private_->left_;
}

double VolumeSlice::right() const
{
  return this->private_->right_;
}

double VolumeSlice::bottom() const
{
  return this->private_->bottom_;
}

double VolumeSlice::top() const
{
  return this->private_->top_;
}

double VolumeSlice::depth() const
{
  return this->private_->depth_;
}

const Point& VolumeSlice::bottom_left() const
{
  return this->private_->bottom_left_;
}

const Point& VolumeSlice::bottom_right() const
{
  return this->private_->bottom_right_;
}

const Point& VolumeSlice::top_left() const
{
  return this->private_->top_left_;
}

const Point& VolumeSlice::top_right() const
{
  return this->private_->top_right_;
}

bool VolumeSlice::get_slice_changed()
{
  return this->private_->slice_changed_;
}

void VolumeSlice::set_slice_changed( bool changed )
{
  this->private_->slice_changed_ = changed;
}

bool VolumeSlice::get_size_changed()
{
  return this->private_->size_changed_;
}

void VolumeSlice::set_size_changed( bool changed )
{
  this->private_->size_changed_ = changed;
}

bool VolumeSlice::is_valid() const
{
  lock_type lock( this->get_mutex() );
  return ( !this->private_->out_of_boundary_ && 
    this->private_->volume_ && 
    this->private_->volume_->is_valid() );
}

void VolumeSlice::ProjectOntoSlice( VolumeSliceType slice_type, const Point& pt, 
                   double& i_pos, double& j_pos )
{
  double depth;
  ProjectOntoSlice( slice_type, pt, i_pos, j_pos, depth );
}

void VolumeSlice::ProjectOntoSlice( VolumeSliceType slice_type, const Point& pt, 
                   double& i_pos, double& j_pos, double& depth )
{
  switch ( slice_type )
  {
  case VolumeSliceType::AXIAL_E:
    i_pos = pt[ 0 ];
    j_pos = pt[ 1 ];
    depth = pt[ 2 ];
    break;
  case VolumeSliceType::CORONAL_E:
    i_pos = pt[ 0 ];
    j_pos = pt[ 2 ];
    depth = pt[ 1 ];
    break;
  case VolumeSliceType::SAGITTAL_E:
    i_pos = pt[ 1 ];
    j_pos = pt[ 2 ];
    depth = pt[ 0 ];
    break;
  default:
    assert( false );
  }
}

} // end namespace Core
