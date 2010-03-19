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

#include <Utils/Math/MathFunctions.h>
#include <Utils/Volume/VolumeSlice.h>

namespace Utils
{

VolumeSlice::VolumeSlice( const VolumeHandle& volume, 
             VolumeSliceType type, size_t slice_num ) :
  slice_changed_( true ), 
  size_changed_( true ),
  volume_( volume ), 
  slice_type_( type ), 
  slice_number_ ( slice_num )
{
  this->update();
  this->slice_number_ = Min( this->slice_number_, this->number_of_slices_ - 1 );
}

VolumeSlice::VolumeSlice( const VolumeSlice& copy ) :
  slice_changed_( copy.slice_changed_ ),
  size_changed_( copy.size_changed_ ),
  width_( copy.width_ ),
  height_( copy.height_ ),
  number_of_slices_( copy.number_of_slices_ ),
  texture_( copy.texture_ ),
  volume_( copy.volume_ ),
  slice_type_( copy.slice_type_ ),
  slice_number_( copy.slice_number_ )
{
  this->update();
}

VolumeSlice::~VolumeSlice()
{
  this->disconnect_all();
}

void VolumeSlice::set_slice_type( VolumeSliceType type )
{
  if ( this->slice_type_ != type )
  {
    this->slice_changed_ = true;
    this->size_changed_ = true;
    this->slice_type_ = type;

    this->update();
    this->slice_number_ = Min( this->slice_number_, this->number_of_slices_ - 1 );
  }
}

void VolumeSlice::set_slice_number( size_t slice_num )
{
  slice_num = Min( slice_num, this->number_of_slices_ - 1 );
  if ( this->slice_number_ != slice_num )
  {
    this->slice_number_ = slice_num;
    this->slice_changed_ = true;
  }
}

void VolumeSlice::get_world_space_boundary_3d( Point& bottom_left, Point& bottom_right, 
  Point& top_left, Point& top_right  ) const
{
  full_index_type index;
  this->to_index( 0, 0, index );
  bottom_left = this->volume_->apply_grid_transform( Point( index[0], index[1], index[2] ) );
  this->to_index( this->width_ - 1, 0, index );
  bottom_right = this->volume_->apply_grid_transform( Point( index[0], index[1], index[2] ) );
  this->to_index( this->width_ - 1, this->height_ - 1, index );
  top_right = this->volume_->apply_grid_transform( Point( index[0], index[1], index[2] ) );
  this->to_index( 0, this->height_ - 1, index );
  top_left = this->volume_->apply_grid_transform( Point( index[0], index[1], index[2] ) );
}

void VolumeSlice::get_world_space_boundary_2d(double& left, double& right, 
    double& bottom, double& top  ) const
{
  full_index_type index;
  this->to_index( 0, 0, index );
  Point bottom_left = this->volume_->apply_grid_transform( Point( index[0], index[1], index[2] ) );
  this->to_index( this->width_ - 1, this->height_ - 1, index );
  Point top_right = this->volume_->apply_grid_transform( Point( index[0], index[1], index[2] ) );

  switch ( this->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    left = bottom_left.x();
    right = top_right.x();
    bottom = bottom_left.y();
    top = top_right.y();
    break;
  case VolumeSliceType::CORONAL_E:
    left = bottom_left.z();
    right = top_right.z();
    bottom = bottom_left.x();
    top = top_right.x();
    break;
  case VolumeSliceType::SAGITTAL_E:
    left = bottom_left.y();
    right = top_right.y();
    bottom = bottom_left.z();
    top = top_right.z();
    break;
  }
}

void VolumeSlice::update()
{
  switch( this->slice_type_ )
  {
  case VolumeSliceType::AXIAL_E:
    this->width_ = this->volume_->nx();
    this->height_ = this->volume_->ny();
    this->number_of_slices_ = this->volume_->nz();
    this->index_func_ = boost::bind( &Volume::to_index, this->volume_, 
      _1, _2, boost::bind( &VolumeSlice::slice_number, this ) );
    this->full_index_func_ = boost::bind( &VolumeSlice::make_full_index, this,
      _1, _2, boost::bind( &VolumeSlice::slice_number, this ), _3 );
    this->extract_slice_number_func_ = boost::bind( 
      &VolumeSlice::extract_slice_number, this, _3 );
    break;
  case VolumeSliceType::CORONAL_E:
    this->width_ = this->volume_->nz();
    this->height_ = this->volume_->nx();
    this->number_of_slices_ = this->volume_->ny();
    this->index_func_ = boost::bind( &Volume::to_index, this->volume_,
      _2, boost::bind( &VolumeSlice::slice_number, this ), _1 );
    this->full_index_func_ = boost::bind( &VolumeSlice::make_full_index, this,
      _2, boost::bind( &VolumeSlice::slice_number, this ), _1, _3 );
    this->extract_slice_number_func_ = boost::bind( 
      &VolumeSlice::extract_slice_number, this, _2 );
    break;
  case VolumeSliceType::SAGITTAL_E:
    this->width_ = this->volume_->ny();
    this->height_ = this->volume_->nz();
    this->number_of_slices_ = this->volume_->nx();
    this->index_func_ = boost::bind( &Volume::to_index, this->volume_,
      boost::bind( &VolumeSlice::slice_number, this ), _1, _2 );
    this->full_index_func_ = boost::bind( &VolumeSlice::make_full_index, this,
      boost::bind( &VolumeSlice::slice_number, this ), _1, _2, _3 );
    this->extract_slice_number_func_ = boost::bind( 
      &VolumeSlice::extract_slice_number, this, _1 );
    break;
  default:
    assert( false );
    break;
  }

  // TODO: remove this. It's for testing only
  this->slice_number_ = this->number_of_slices_ / 2;
}

} // end namespace Utils