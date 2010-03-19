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

#ifndef UTILS_VOLUME_VOLUMESLICE_H
#define UTILS_VOLUME_VOLUMESLICE_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <Utils/Core/ConnectionHandler.h>
#include <Utils/Core/EnumClass.h>
#include <Utils/Geometry/Point.h>
#include <Utils/Graphics/Texture.h>
#include <Utils/Volume/Volume.h>

namespace Utils
{

SCI_ENUM_CLASS
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
  typedef Point full_index_type;
  typedef Volume::mutex_type mutex_type;
  typedef Volume::lock_type lock_type;

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

  void set_slice_type( VolumeSliceType type );

  void set_slice_number( size_t slice_num );

  // Returns the 3D index of the point in the volume
  inline void to_index( size_t i, size_t j, full_index_type& index ) const
  {
    this->full_index_func_( i, j, index );
  }

  // Returns the linear index of the point in the volume
  inline size_t to_index( size_t i, size_t j ) const
  {
    return this->index_func_( i, j );
  }

  inline int to_slice_number( int x, int y, int z ) const
  {
    return this->extract_slice_number_func_( x, y, z );
  }

  inline size_t width() const
  {
    return this->width_;
  }

  inline size_t height() const
  {
    return this->height_;
  }

  inline size_t number_of_slices() const
  {
    return this->number_of_slices_;
  }

  bool slice_changed() const
  {
    return this->slice_changed_;
  }

  inline void volume_updated_slot()
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

  void get_world_space_boundary_2d( double& left, double& right, 
    double& bottom, double& top  ) const;

  void get_world_space_boundary_3d( Point& bottom_left, Point& bottom_right, 
    Point& top_right, Point& top_left  ) const;
  
  inline mutex_type& get_mutex()
  {
    return this->volume_->get_mutex();
  }

  // Create the texture object
  virtual void initialize_texture() = 0;

  // Upload the volume slice to texture.
  // NOTE: This function allocates resources on the GPU, so the caller should
  // acquire a lock on the RenderResources before calling this function.
  virtual void upload_texture() = 0;

  inline TextureHandle get_texture()
  {
    return this->texture_;
  }

private:
  // Boost functions for converting the position of a point in the 2D slice to its
  // position in the 3D volume. By taking advantage of parameter reordering capability
  // of boost bind, these functions provide a faster and more convenient approach than
  // using "switch...case" statements.

  // Convert a 2D coordinate to a 1D index in the volume data.
  boost::function<size_t ( size_t, size_t )> index_func_;

  // Convert a 2D coordinate in the slice to 3D coordinate in the volume.
  boost::function<void ( size_t, size_t, full_index_type& )> full_index_func_;

  // Extract the slice number from a 3D coordinate.
  boost::function<int ( int, int, int )> extract_slice_number_func_;

private:

  inline size_t slice_number() const
  {
    return this->slice_number_;
  }

  void update();

  inline void make_full_index( size_t x, size_t y, size_t z, full_index_type& index ) const
  {
    index[0] = static_cast<double>( x );
    index[1] = static_cast<double>( y );
    index[2] = static_cast<double>( z );
  }

  inline int extract_slice_number( int num ) const
  {
    return num;
  }

protected:

  bool slice_changed_;
  bool size_changed_;
  size_t width_;
  size_t height_;
  size_t number_of_slices_;

  Texture2DHandle texture_;

  // Mutex for thread-safe access on member variables. 
  // NOTE: Member variables are completely independent of the underlying volume, so it's
  // better to have a separate mutex.
  typedef boost::recursive_mutex internal_mutex_type;
  typedef boost::unique_lock< internal_mutex_type > internal_lock_type;
  boost::recursive_mutex internal_mutex_;

private:
  VolumeHandle volume_;
  VolumeSliceType slice_type_;
  size_t slice_number_;

};

} // end namespace Utils

#endif