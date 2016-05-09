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

#ifndef CORE_VOLUME_VOLUMESLICE_H
#define CORE_VOLUME_VOLUMESLICE_H

#include <boost/shared_ptr.hpp>

#include <Core/DataBlock/SliceType.h>
#include <Core/Geometry/Point.h>
#include <Core/Graphics/Texture.h>
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Lockable.h>
#include <Core/Volume/Volume.h>

namespace Core
{

typedef SliceType VolumeSliceType;

// Forward declaration
class VolumeSlice;
typedef boost::shared_ptr< VolumeSlice > VolumeSliceHandle;

class VolumeSlicePrivate;
typedef boost::shared_ptr< VolumeSlicePrivate > VolumeSlicePrivateHandle;

// CLASS VolumeSlice
/// A helper class for accessing data in a slice of a volume.
class VolumeSlice : protected ConnectionHandler, public RecursiveLockable
{

protected:
  VolumeSlice( const VolumeHandle& volume, VolumeSliceType type, size_t slice_num );

  // Copy Constructor
  /// NOTE: This is provided for the purpose of taking a snapshot of current status of a 
  /// VolumeSlice, which will then be used for rendering. The copy constructor shouldn't
  /// connect to any signals so its state won't be changed after construction.
  /// NOTE: The copy-constructed object will share the same texture object with the old one.
  VolumeSlice( const VolumeSlice& copy );

  virtual ~VolumeSlice();

public:

  // IS_VALID:
  /// Returns true if the underlying volume is valid and the slice is not out of boundary.
  bool is_valid() const;

  VolumeHandle get_volume() const;

  VolumeType volume_type() const;

  void set_slice_type( VolumeSliceType type );

  VolumeSliceType get_slice_type() const;

  void set_slice_number( size_t slice_num );

  size_t get_slice_number() const;

  /// Get the index of the point in the volume
  void to_index( size_t i, size_t j, Point& index ) const;

  /// Returns the linear index of the point in the volume
  size_t to_index( size_t i, size_t j ) const;

  /// Get the index of the point that's closest to the given position in world space
  /// NOTE: the indices returned can be out of the slice boundary.
  void world_to_index( double i_pos, double j_pos, int& i, int& j ) const;

  void index_to_world( int i, int j, double& world_i, double& world_j ) const;

  /// Get the 3D world coordinate of the point
  void get_world_coord( double i_pos, double j_pos, Point& world_coord ) const;

  /// Project a point in 3D space onto the plane of the slice.
  void project_onto_slice( const Point& pt, double& i_pos, double& j_pos ) const;

  /// Project a point in 3D space onto the plane of the slice.  Return depth by reference.
  void project_onto_slice( const Point& pt, double& i_pos, double& j_pos, double& depth ) const;

  /// Return the slice number that's closest to the point in 3D space.
  int get_closest_slice( const Point& pt ) const;

  // Move the slice to closely match the given point in world space.
  // Returns true if slice is moved, otherwise false.
  void move_slice_to( const Point& pos, bool fail_safe = false );

  /// Move the slice to the specified depth in world space.
  /// Returns true if the slice is moved successfully, otherwise false.
  void move_slice_to( double depth, bool fail_safe = false );

  size_t nx() const;
  size_t ny() const;
  size_t number_of_slices() const;
  bool out_of_boundary() const;

  double left() const;
  double right() const;
  double bottom() const;
  double top() const;
  double depth() const;

  const Point& bottom_left() const;
  const Point& bottom_right() const;
  const Point& top_left() const;
  const Point& top_right() const;

  void handle_volume_updated();

  Point apply_grid_transform( const Point& pt ) const;

  Point apply_inverse_grid_transform( const Point& pt ) const;
  
  // Create the texture object
  virtual void initialize_texture();

  /// Upload the volume slice to texture.
  /// NOTE: This function allocates resources on the GPU, so the caller should
  /// acquire a lock on the RenderResources before calling this function.
  virtual void upload_texture() = 0;

  // CLONE:
  /// Make a copy of the slice, which will share texture object with the original one.
  virtual VolumeSliceHandle clone() = 0;

  Texture2DHandle get_texture();

  // SET_VOLUME:
  /// Set the volume out of which the slice will be taken.
  virtual void set_volume( const VolumeHandle& volume ) = 0;

protected:
  bool get_slice_changed();
  void set_slice_changed( bool );
  bool get_size_changed();
  void set_size_changed( bool );

private:
  friend class VolumeSlicePrivate;
  VolumeSlicePrivateHandle private_;

public:
  static void ProjectOntoSlice( VolumeSliceType slice_type, const Point& pt,
    double& i_pos, double& j_pos );

  static void ProjectOntoSlice( VolumeSliceType slice_type, const Point& pt,
    double& i_pos, double& j_pos, double& depth );
};

} // end namespace Core

#endif
