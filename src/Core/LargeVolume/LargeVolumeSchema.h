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

#ifndef CORE_LARGEVOLUME_LARGEVOLUMESCHEMA_H
#define CORE_LARGEVOLUME_LARGEVOLUMESCHEMA_H

// Geometry
#include <Core/Geometry/IndexVector.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/DataBlock.h>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

namespace Core
{

// TODO: brief blurb about computing downsampling ratios
// TODO: brief blurb about remainder brick dimensions

/// Brick Schema:
///
/// The schema is organized in levels from full to coarsest resolution
/// (containing only 1 brick), with brick layout in row-major order.
/// Brick naming convention is A=level 0 (full resolution), B=level 1, ...
/// Brick sizes are computed in the XY plane.
///
/// For example, if a 5000 x 5000 x n dataset is divided into 20
/// 256x256x256 bricks at full resolution with n layers in the z axis, the schema is:
///
/// level 0 (20, 20, n):
/// A0   (0 0 0)  | A1  (1 0 0)   | A2  (2 0 0)   | ... | A19 (19 0 0)
/// A20  (0 1 0)  | A21 (1 1 0)   | A22 (2 1 0)   | ... | A39 (19 1 0)
/// ...
/// A380 (0 19 0) | A381 (1 19 0) | A382 (2 19 0) | ... | A399 (19 19 0)
///
/// level 1 (10, 10, n):
/// B0  (0 0 0) | B1  (1 0 0) | B2  (2 0 0)  | ... | B9 (9 0 0)
/// B10 (0 1 0) | B11 (1 1 0) | B12 (2 1 0)  | ... | B19 (9 1 0)
/// ...
/// B90 (0 9 0) | B91 (1 9 0) | B92 (2 9 0)  | ... | B99 (9 9 0)
///
/// level 2 (5, 5, n):
/// C0 (0 0 0)  | C1 (1 0 0)  | C2 (2 0 0)  | C3 (3 0 0)  | C4 (4 0 0)
/// C5 (0 1 0)  | C6 (1 1 0)  | C7 (2 1 0)  | C8 (3 1 0)  | C9 (4 1 0)
/// ...
/// C20 (0 4 0) | C21 (1 4 0) | C22 (2 4 0) | C23 (3 4 0) | C24 (4 4 0)
///
/// level 3 (3, 3, n):
/// D0 (0 0 0) | D1 (1 0 0) | D2 (2 0 0)
/// D3 (0 1 0) | D4 (1 1 0) | D5 (2 1 0)
/// D6 (0 2 0) | D7 (1 2 0) | D8 (2 2 0)
///
/// level 4 (2, 2, n):
/// E0 (0 0 0) | E1 (1 0 0)
/// E2 (0 1 0) | E3 (1 1 0)
///
/// level 5 (1, 1, n):
/// F0 (0 0 0)

// Internals are separated from the interface
class LargeVolumeSchemaPrivate;
typedef boost::shared_ptr< LargeVolumeSchemaPrivate > LargeVolumeSchemaPrivateHandle;

class BrickInfo {
public:
  typedef IndexVector::index_type index_type;

  index_type index_;
  index_type level_;

  BrickInfo( index_type index, index_type level ) :
    index_( index ), level_ ( level )
  {}

  bool operator==(const Core::BrickInfo rhs) const
  {
    return this->level_ == rhs.level_ && this->index_ == rhs.index_;
  }

  bool operator<(const Core::BrickInfo rhs) const
  {
    return this->level_ != rhs.level_ ? this->level_ > rhs.level_ : this->index_ < rhs.index_;
  }
};


class LargeVolumeSchema;
typedef boost::shared_ptr< LargeVolumeSchema > LargeVolumeSchemaHandle;

class LargeVolumeSchema : public boost::enable_shared_from_this<LargeVolumeSchema>
{

  // -- types --
public:
  typedef IndexVector::index_type index_type;

  // -- constructor --
public:
  LargeVolumeSchema();


  // -- io functions --
public:
  /// LOAD
  /// Load a large volume text file
  bool load( std::string& error );


  /// SAVE
  /// Save a large volume text file
  bool save( std::string& error) const;

  // -- access functions --
public:

  /// GET_NX
  size_t get_nx() const;

  /// GET_NY
  size_t get_ny() const;

  /// GET_NZ
  size_t get_nz() const;

  // GET_DIR
  boost::filesystem::path get_dir() const;

  // GET_SIZE
  // Get size as an index
  const IndexVector& get_size() const;

  /// GET_ORIGIN
  /// Get origin location of the data (we assume node centered grid)
  const Point& get_origin() const;

  /// GET_SPACING
  /// Get the spacing of the data at highest resolution
  const Vector& get_spacing() const;

  /// GET_BRICK_SIZE
  /// Get the bricksize of the data
  const IndexVector& get_brick_size() const;

  /// GET_EFFECTIVE_BRICK_SIZE
  /// Get the bricksize of the data minus overlap
  const IndexVector& get_effective_brick_size() const;

  /// GET_OVERLAP
  /// Get the overlap between the bricks, .i.e each brick has a border of this size surrounding
  /// the full brick
  size_t get_overlap() const;

  /// GET_DATA_TYPE
  /// Get the type of the underlying data
  DataType get_data_type() const;

  /// IS_COMPRESSED
  /// Check whether the data is compressed
  bool is_compressed() const;

  /// IS_LITTLE_ENDIAN
  /// Check whether data is little endian
  bool is_little_endian() const;

  /// GET_LEVEL_DOWNSAMPLE_RATIO
  /// Get the downsample ratio for each direction per level
  const IndexVector& get_level_downsample_ratio( size_t idx ) const;

  /// GET_NUM_LEVELS
  /// Get the number of resolution levels in the data
  size_t get_num_levels() const;

  /// GET_MIN
  /// Get minimum value of dataset
  double get_min() const;

  /// GET_MAX
  /// Get minimum value of dataset
  double get_max() const;

  /// SET_DIR
  /// Set large volume dir
  void set_dir( const boost::filesystem::path& dir );

  /// SET_PARAMETERS
  /// Set bricking parameters
  void set_parameters( const IndexVector& size, const Vector& spacing, const Point& origin, const IndexVector& brick_size, size_t overlap, DataType datatype );

  /// SET_COMPRESSION
  /// Set whether data is compressed
  void set_compression( bool compression );

  /// SET_MIN_MAX
  /// Set min and max values for the dataset
  void set_min_max( double min, double max ) const;

  /// ENABLE_DOWNSAMPLE
  /// Enable down sample in certain directions only
  void enable_downsample( bool downsample_x, bool downsample_y, bool downsample_z );

  // -- schema computations --
public:

  /// COMPUTE LEVELS
  /// Compute the levels and downsample ratios based on size, spacing, brick size and overlap
  void compute_levels();

  /// COMPUTE_LEVEL_SIZE
  /// Get the size of a level (without overlaps)
  IndexVector get_level_size( index_type level ) const;

  /// COMPUTE_LEVEL_SPACING
  /// Get the spacing of each level
  Vector get_level_spacing( index_type level ) const;

  /// COMPUTE_LEVEL_LAYOUT
  /// Get the loayout of each level
  IndexVector get_level_layout( index_type level ) const;

  /// COMPUTE_LEVEL_NUM_BRICKS
  /// Compute how many bricks are located at a certassin level
  size_t compute_level_num_bricks( index_type level ) const;

  /// GET_GRID_TRANSFORM
  /// Total rtansform of the complete volume
  GridTransform get_grid_transform() const;

  // -- brick function --
public:
  /// Get the file name of the brick
  boost::filesystem::path get_brick_file_name( const BrickInfo& bi );

  /// GET_BRICK_SIZE
  /// Get the size + overlap of the brick
  IndexVector get_brick_size( const BrickInfo& bi ) const;

  /// GET_GRID_TRANSFORM
  GridTransform get_brick_grid_transform( const BrickInfo& bi ) const;

  /// GET_BRICKS_FOR_REGION
  /// Get bricks for a certain region
  std::vector<BrickInfo> get_bricks_for_region( const BBox& region, 
    double pixel_size, SliceType slice, const std::string& load_key );

  std::vector<BrickInfo> get_bricks_for_volume( Transform world_viewport, 
  int width, int height, SliceType slice, const BBox& effective_bbox, double depth, 
  const std::string& load_key );

  /// GET_PARENT
  bool  get_parent( const BrickInfo& bi, BrickInfo& parent );

  /// GET+CHILDREN
  bool get_children( const BrickInfo& bi, SliceType slice, double depth,
    std::vector<BrickInfo>& children );

  // -- level functions --
public:

  /// GET_LEVEL_START_AND_END
  bool get_level_start_and_end( const GridTransform& region, int level,
    IndexVector& start, IndexVector& end, GridTransform& gt );

  /// INSERT_BRICK
  /// Insert a brick while doing clipping
  bool insert_brick( DataBlockHandle volume, DataBlockHandle brick, IndexVector offset,
    IndexVector clip_start, IndexVector clip_end );

  /// INSERT_BRICK
  /// Insert a brick into datavolume
  bool insert_brick( DataBlockHandle volume, DataBlockHandle brick, IndexVector offset );

  // -- reading/writing bricks --
public:

  /// READ_BRICK
  /// Read in a brick from disk
  bool read_brick( DataBlockHandle& data_block, const BrickInfo& bi,
    std::string& error ) const;

  /// WRITE_BRICK
  /// Write a brick to disk
  bool write_brick( DataBlockHandle data_block, 
    const BrickInfo& bi, std::string& error ) const;

  /// REPROCESS_BRICK
  /// As bricks are written in parallel, they may end up all over the drive
  bool reprocess_brick( const BrickInfo& bi,
    std::string& error ) const;

  /// APPEND_BRICK_BUFFER
  /// Append data to a brick to disk
  bool append_brick_buffer( DataBlockHandle data_block, size_t z_start, size_t z_end, const size_t offset,
    const BrickInfo& bi, std::string& error ) const;

  // -- internals --
private:
  LargeVolumeSchemaPrivateHandle private_;
  const boost::filesystem::path VOLUME_FILE_NAME_;
};

} // end namespace Core

#endif
