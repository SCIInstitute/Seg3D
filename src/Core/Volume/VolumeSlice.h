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

#ifndef CORE_VOLUME_VOLUMESLICE_H
#define CORE_VOLUME_VOLUMESLICE_H

#include <boost/shared_ptr.hpp>

#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/EnumClass.h>
#include <Core/Geometry/Point.h>
#include <Core/Graphics/Texture.h>
#include <Core/Volume/Volume.h>

namespace Core
{

CORE_ENUM_CLASS
(
  VolumeSliceType,
  AXIAL_E = 0,
  CORONAL_E = 1,
  SAGITTAL_E = 2
)

// Forward declaration
class VolumeSlice;
typedef boost::shared_ptr< VolumeSlice > VolumeSliceHandle;

// CLASS VolumeSlice
// A helper class for accessing data in a slice of a volume.
class VolumeSlice : protected ConnectionHandler
{
public:
  typedef Volume::mutex_type mutex_type;
  typedef Volume::lock_type lock_type;
  typedef Volume::shared_lock_type shared_lock_type;

protected:
  VolumeSlice( const VolumeHandle& volume, VolumeSliceType type, size_t slice_num );

  // Copy Constructor
  // NOTE: This is provided for the purpose of taking a snapshot of current status of a 
  // VolumeSlice, which will then be used for rendering. The copy constructor shouldn't
  // connect to any signals so its state won't be changed after construction.
  // NOTE: The copy-constructed object will share the same texture object with the old one.
  VolumeSlice( const VolumeSlice& copy );

  virtual ~VolumeSlice();

public:

  VolumeHandle get_volume() const
  {
    return this->volume_;
  }

  VolumeType volume_type() const
  {
    return this->volume_->get_type();
  }

  void set_slice_type( VolumeSliceType type );

  VolumeSliceType get_slice_type() const;

  void set_slice_number( size_t slice_num );

  size_t get_slice_number() const
  {
    return this->slice_number_;
  }

  // Get the index of the point in the volume
  void to_index( size_t i, size_t j, Point& index ) const;

  // Returns the linear index of the point in the volume
  size_t to_index( size_t i, size_t j ) const;

  // Get the index of the point that's closest to the given position in world space
  // NOTE: the indices returned can be out of the slice boundary.
  void world_to_index( double i_pos, double j_pos, int& i, int& j ) const;

  // Get the 3D world coordinate of the point
  void get_world_coord( double i_pos, double j_pos, Point& world_coord ) const;

  // Project a point in 3D space onto the plane of the slice.
  void project_onto_slice( const Point& pt, double& i_pos, double& j_pos ) const;

  // Return the slice number that's closest to the point in 3D space.
  int get_closest_slice( const Point& pt ) const;

  // Move the slice to closely match the given point in world space.
  // Returns true if slice is moved, otherwise false.
  void move_slice_to( const Point& pos, bool fail_safe = false );

  // Move the slice to the specified depth in world space.
  // Returns true if the slice is moved successfully, otherwise false.
  void move_slice_to( double depth, bool fail_safe = false );

  size_t nx() const { return this->nx_; }
  size_t ny() const { return this->ny_; }
  size_t number_of_slices() const { return this->number_of_slices_; }
  bool out_of_boundary() const { return this->out_of_boundary_; }

  double left() const { return this->left_; }
  double right() const { return this->right_; }
  double bottom() const { return this->bottom_; }
  double top() const { return this->top_; }
  double depth() const { return this->depth_; }

  const Point& bottom_left() const { return this->bottom_left_; }
  const Point& bottom_right() const { return this->bottom_right_; }
  const Point& top_left() const { return this->top_left_; }
  const Point& top_right() const { return this->top_right_; }

  void handle_volume_updated()
  {
    this->slice_changed_ = true;
  }

  Point apply_grid_transform( const Point& pt ) const
  {
    return this->volume_->apply_grid_transform( pt );
  }

  Point apply_inverse_grid_transform( const Point& pt ) const
  {
    return this->volume_->apply_inverse_grid_transform( pt );
  }
  
  mutex_type& get_mutex() const
  {
    return this->volume_->get_mutex();
  }

  // Create the texture object
  virtual void initialize_texture();

  // Upload the volume slice to texture.
  // NOTE: This function allocates resources on the GPU, so the caller should
  // acquire a lock on the RenderResources before calling this function.
  virtual void upload_texture() = 0;

  // CLONE:
  // Make a copy of the slice, which will share texture object with the original one.
  virtual VolumeSliceHandle clone() = 0;

  TextureHandle get_texture()
  {
    return this->texture_;
  }

private:
  // UPDATE_DIMENSION:
  // Update the dimension of the slice.
  void update_dimension();

  // UPDATE_POSITION:
  // Update the position of the slice.
  void update_position();

protected:

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

  // Mutex for thread-safe access on member variables. 
  // NOTE: Member variables are completely independent of the underlying volume, so it's
  // better to have a separate mutex.
  typedef boost::recursive_mutex internal_mutex_type;
  typedef boost::unique_lock< internal_mutex_type > internal_lock_type;
  mutable internal_mutex_type internal_mutex_;

private:
  VolumeHandle volume_;
  VolumeSliceType slice_type_;
  size_t slice_number_;

};

} // end namespace Core

#endif