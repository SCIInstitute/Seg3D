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

//Core Includes
#include <Core/Math/MathFunctions.h>
#include <Core/Utils/Log.h>
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Volume/DataVolume.h>
#include <Core/DataBlock/DataBlockManager.h>
#include <Core/Geometry/BBox.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Graphics/PixelBufferObject.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class DataVolume
//////////////////////////////////////////////////////////////////////////

class DataVolumePrivate : public Lockable
{
public:
  bool generate_bricks();

  template< class DST_TYPE >
  void copy_data( DST_TYPE* buffer, size_t width, size_t height, size_t depth, size_t x_start, 
    size_t x_end, size_t y_start, size_t y_end, size_t z_start, size_t z_end );

  template< class DST_TYPE, class SRC_TYPE >
  void copy_typed_data( DST_TYPE* buffer, size_t width, size_t height, size_t depth, 
    size_t x_start, size_t x_end, size_t y_start, size_t y_end, size_t z_start, size_t z_end );

  // Handle to where the volume data is really stored
  DataBlockHandle data_block_;

  bool bricks_generated_;
  std::vector< DataVolumeBrickHandle > bricks_;
  DataVolume* volume_;

public:
  const static unsigned int BRICK_SIZE_C;
  const static unsigned int OVERLAP_SIZE_C;
};

const unsigned int DataVolumePrivate::BRICK_SIZE_C = 256;
const unsigned int DataVolumePrivate::OVERLAP_SIZE_C = 2;

template< class DST_TYPE, class SRC_TYPE >
void DataVolumePrivate::copy_typed_data( DST_TYPE* buffer, size_t width, size_t height, 
    size_t depth, size_t x_start, size_t x_end, size_t y_start, size_t y_end, 
    size_t z_start, size_t z_end )
{
  const double numeric_min = static_cast<double>( std::numeric_limits< DST_TYPE >::min() );
  const double numeric_max = static_cast<double>( std::numeric_limits< DST_TYPE >::max() );
  const double value_min = this->data_block_->get_min();
  const double value_max = this->data_block_->get_max();
  const double value_range = value_max - value_min;
  const double inv_value_range = ( numeric_max - numeric_min ) / value_range;
  const SRC_TYPE typed_value_min = static_cast< SRC_TYPE >( value_min );
  const SRC_TYPE* src_data = static_cast< SRC_TYPE* >( this->data_block_->get_data() );

  size_t current_index;
  size_t dst_index = 0;
  size_t texture_stride_z = width * height;
  for ( size_t z = z_start; z <= z_end; ++z )
  {
    for ( size_t y = y_start; y <= y_end; ++y )
    {
      current_index = this->data_block_->to_index( x_start, y, z );
      for ( size_t x = x_start; x <= x_end; ++x )
      {
        // NOTE: removed unnecessary addition for unsigned texture types
        buffer[ dst_index++ ] = static_cast< DST_TYPE >(
          ( src_data[ current_index++ ] - typed_value_min ) * inv_value_range );
      }

      // Pad the texture in X-direction with boundary values
      for ( size_t x = x_end - x_start + 2; x <= width; ++x )
      {
        buffer[ dst_index ] = buffer[ dst_index - 1 ];
        ++dst_index;
    }
  }

    // Pad the texture in Y-direction with boundary values
    for ( size_t y = y_end - y_start + 2; y <= height; ++y )
    {
      memcpy( buffer + dst_index, buffer + dst_index - width, sizeof( DST_TYPE ) * width );
      dst_index += width;
}
  }

  // Pad the texture in Z-direction with boundary values
  for ( size_t z = z_end - z_start + 2; z <= depth; ++z )
  {
    memcpy( buffer + dst_index, buffer + dst_index - texture_stride_z, 
      sizeof( DST_TYPE ) * texture_stride_z );
    dst_index += texture_stride_z;
  } 
}

template< class DST_TYPE >
void DataVolumePrivate::copy_data( DST_TYPE* buffer, size_t width, size_t height, size_t depth,
      size_t x_start, size_t x_end, size_t y_start, size_t y_end, size_t z_start, size_t z_end )
{
  switch ( this->data_block_->get_data_type() )
  {
  case DataType::CHAR_E:
    this->copy_typed_data< DST_TYPE, signed char >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::UCHAR_E:
    this->copy_typed_data< DST_TYPE, unsigned char >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::SHORT_E:
    this->copy_typed_data< DST_TYPE, short >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::USHORT_E:
    this->copy_typed_data< DST_TYPE, unsigned short >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::INT_E:
    this->copy_typed_data< DST_TYPE, int >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::UINT_E:
    this->copy_typed_data< DST_TYPE, unsigned int >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::FLOAT_E:
    this->copy_typed_data< DST_TYPE, float >( buffer, width, height, depth,
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  case DataType::DOUBLE_E:
    this->copy_typed_data< DST_TYPE, double >( buffer, width, height, depth, 
      x_start, x_end, y_start, y_end, z_start, z_end );
    break;
  }
}

bool DataVolumePrivate::generate_bricks()
{
  this->bricks_.clear();

  // Lock the render resources as we are going to create new OpenGL objects
  RenderResources::lock_type rr_lock( RenderResources::GetMutex() );

  // Lock the data block
  DataBlock::shared_lock_type data_lock( this->data_block_->get_mutex() );

  // Set pixel unpack alignment to 1
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  PixelBufferObjectHandle pixel_buffer( new PixelUnpackBuffer );
  pixel_buffer->bind();

  size_t nx = this->data_block_->get_nx();
  size_t ny = this->data_block_->get_ny();
  size_t nz = this->data_block_->get_nz();
  GridTransform grid_trans = this->volume_->get_grid_transform();

  // Make sure that brick size is power of 2
  assert( IsPowerOf2( BRICK_SIZE_C ) );
  BOOST_STATIC_ASSERT( BRICK_SIZE_C > OVERLAP_SIZE_C * 2 );

  size_t brick_x_start, brick_x_end, brick_y_start, brick_y_end, brick_z_start, brick_z_end;
  for ( size_t z = 0; z < nz; z = brick_z_end + 1 )
  {
    // The boundary of the brick texture ( padded to power of 2 ) in index space
    size_t data_z_start = z > OVERLAP_SIZE_C ? ( z - OVERLAP_SIZE_C ) : z;
    size_t texture_depth = BRICK_SIZE_C;
    if ( data_z_start + texture_depth > nz )
    {
      texture_depth = NextPowerOf2( nz - data_z_start );
    }
    size_t data_z_end = Core::Min( data_z_start + texture_depth - 1, nz - 1 );

    // The effective boundary of the brick in index space relative to the original volume.
    brick_z_start = z;
    brick_z_end = data_z_end < nz - 1 ? ( data_z_end - OVERLAP_SIZE_C ) : ( nz - 1 );

    for ( size_t y = 0; y < ny; y = brick_y_end + 1 )
    {
      size_t data_y_start = y > OVERLAP_SIZE_C ? ( y - OVERLAP_SIZE_C ) : y;
      size_t texture_height = BRICK_SIZE_C;
      if ( data_y_start + texture_height > ny )
      {
        texture_height = NextPowerOf2( ny - data_y_start );
      }
      size_t data_y_end = Core::Min( data_y_start + texture_height - 1, ny - 1 );

      // The effective boundary of the brick in index space relative to the original volume.
      brick_y_start = y;
      brick_y_end = data_y_end < ny - 1 ? ( data_y_end - OVERLAP_SIZE_C ) : ( ny - 1 );

      for ( size_t x = 0; x < nx; x = brick_x_end + 1 )
      {
        size_t data_x_start = x > OVERLAP_SIZE_C ? ( x - OVERLAP_SIZE_C ) : x;
        size_t texture_width = BRICK_SIZE_C;
        if ( data_x_start + texture_width > nx )
        {
          texture_width = NextPowerOf2( nx - data_x_start );
        }
        size_t data_x_end = Core::Min( data_x_start + texture_width - 1, nx - 1 );

        // The effective boundary of the brick in index space relative to the original volume.
        brick_x_start = x;
        brick_x_end = data_x_end < nx - 1 ? ( data_x_end - OVERLAP_SIZE_C ) : ( nx - 1 );

        Core::Point brick_bbox_min( static_cast< double >( brick_x_start ) - 0.5,
          static_cast< double >( brick_y_start ) - 0.5, static_cast< double >( brick_z_start ) - 0.5 );
        brick_bbox_min = grid_trans * brick_bbox_min;
        Core::Point brick_bbox_max( static_cast< double >( brick_x_end ) + 0.5,
          static_cast< double >( brick_y_end ) + 0.5, static_cast< double >( brick_z_end ) + 0.5 );
        brick_bbox_max = grid_trans * brick_bbox_max;
        Core::Point tex_bbox_min( static_cast< double >( data_x_start ) - 0.5,
          static_cast< double >( data_y_start ) - 0.5, static_cast< double >( data_z_start ) - 0.5 );
        tex_bbox_min = grid_trans * tex_bbox_min;
        Core::Point tex_bbox_max( static_cast< double >( data_x_start + texture_width - 1.0 ) + 0.5,
          static_cast< double >( data_y_start + texture_height - 1.0 ) + 0.5, 
          static_cast< double >( data_z_start + texture_depth - 1.0 ) + 0.5 );
        tex_bbox_max = grid_trans * tex_bbox_max;

        // Brick bounding box in world space (excluding overlapped regions)
        BBox brick_bbox( brick_bbox_min, brick_bbox_max );
        // Brick texture bounding box in world space (including overlapped regions)
        BBox texture_bbox( tex_bbox_min, tex_bbox_max );
        // Texel size in texture space
        Vector texel_size( 1.0 / texture_width, 1.0 / texture_height, 1.0 / texture_depth );
        
        pixel_buffer->set_buffer_data( sizeof( DataVolumeBrick::data_type ) * 
          texture_width * texture_height * texture_depth, NULL, GL_STREAM_DRAW );
        DataVolumeBrick::data_type* buffer = reinterpret_cast< DataVolumeBrick::data_type* >(
          pixel_buffer->map_buffer( GL_WRITE_ONLY ) );
        if ( buffer == 0 )
        {
          CORE_LOG_ERROR( "Failed to map OpenGL buffer." );
          pixel_buffer->unbind();
          return false;
        }
        this->copy_data( buffer, texture_width, texture_height, texture_depth, data_x_start, 
          data_x_end, data_y_start, data_y_end, data_z_start, data_z_end );
        pixel_buffer->unmap_buffer();
        Texture3DHandle tex( new Texture3D );
        tex->bind();
        tex->set_mag_filter( GL_LINEAR );
        tex->set_min_filter( GL_LINEAR );
        tex->set_wrap_s( GL_CLAMP_TO_EDGE );
        tex->set_wrap_t( GL_CLAMP_TO_EDGE );
        tex->set_wrap_r( GL_CLAMP_TO_EDGE );
        tex->set_image( static_cast< int >( texture_width ), static_cast< int >( texture_height ), 
          static_cast< int >( texture_depth ), DataVolumeBrick::TEXTURE_FORMAT_C,
          0, GL_ALPHA, DataVolumeBrick::TEXTURE_DATA_TYPE_C );
        tex->unbind();

        DataVolumeBrickHandle brick( new DataVolumeBrick( brick_bbox, 
          texture_bbox, texel_size, tex ) );
        this->bricks_.push_back( brick );
      }
    }
  }

  pixel_buffer->unbind();

  // NOTE: Wait for all the GL operations to finish before returning, because the bricks
  // may be shared by multiple rendering threads later.
  glFinish();

  return true;
}

//////////////////////////////////////////////////////////////////////////
// Class DataVolumePrivate
//////////////////////////////////////////////////////////////////////////

DataVolume::DataVolume( const GridTransform& grid_transform, 
             const DataBlockHandle& data_block ) :
  Volume( grid_transform ), 
  private_( new DataVolumePrivate )
{
  this->private_->data_block_ = data_block;
  this->private_->bricks_generated_ = false;
  this->private_->volume_ = this;
}

DataVolume::~DataVolume()
{
}

DataBlockHandle DataVolume::get_data_block() const
{
  return this->private_->data_block_;
}

DataType DataVolume::get_data_type() const
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->get_data_type();
  }
  else
  {
    return DataType::UNKNOWN_E;
  }
}

double DataVolume::get_min() const
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->get_min();
  }
  else
  {
    return 0.0;
  }
}

double DataVolume::get_max() const
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->get_max();
  }
  else
  {
    return 0.0;
  }
}

double DataVolume::get_cum_value( double fraction ) const
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->get_histogram().get_cum_value( fraction );
  }
  else
  {
    return 0.0;
  }
}

VolumeType DataVolume::get_type() const
{
  return VolumeType::DATA_E;
}

NrrdDataHandle DataVolume::convert_to_nrrd()
{
  if ( this->private_->data_block_ )
  {
    NrrdDataHandle nrrd_data( new NrrdData( this->private_->data_block_, get_grid_transform() ) );
    return nrrd_data;
  }
  else
  {
       NrrdDataHandle handle;
    return handle;
  }
}

DataBlock::generation_type DataVolume::get_generation() const
{ 
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->get_generation();
  }
  else
  {
    return -1;
  }
}

bool DataVolume::is_valid() const
{
  return this->private_->data_block_.get() != 0;
}

DataVolume::mutex_type& DataVolume::get_mutex()
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->get_mutex();
  }
  else
  {
    return this->invalid_mutex_;
  }
}

DataBlock::generation_type DataVolume::register_data( DataBlock::generation_type generation )
{
  if ( this->private_->data_block_ )
  {
    Core::DataBlockManager::Instance()->register_datablock( this->private_->data_block_, generation );
    return this->private_->data_block_->get_generation();
  }
  return -1;
}

void DataVolume::unregister_data()
{
  if ( this->private_->data_block_ && this->private_->data_block_->get_generation() != -1 )
  {
    Core::DataBlockManager::Instance()->unregister_datablock( 
      this->private_->data_block_->get_generation() );
  }
}

bool DataVolume::LoadDataVolume( const boost::filesystem::path& filename, 
                DataVolumeHandle& volume, std::string& error )
{
  volume.reset();
  
  NrrdDataHandle nrrd;
  if ( ! ( NrrdData::LoadNrrd( filename.string(), nrrd, error ) ) ) return false;
  
  Core::DataBlockHandle datablock( Core::NrrdDataBlock::New( nrrd ) );
  
  // Load the datablock and trust any histogram recorded inside of it.
  datablock->set_histogram( nrrd->get_histogram( true ) );

  volume = DataVolumeHandle( new DataVolume( nrrd->get_grid_transform(), datablock ) );
  return true;
}

bool DataVolume::SaveDataVolume( const boost::filesystem::path& filepath, 
                DataVolumeHandle& volume, std::string& error, 
                bool compress, int level )
{
  NrrdDataHandle nrrd = NrrdDataHandle( new NrrdData( 
    volume->private_->data_block_, volume->get_grid_transform() ) );

  nrrd->set_histogram( volume->private_->data_block_->get_histogram() );
  
  DataBlock::shared_lock_type slock( volume->private_->data_block_->get_mutex() );
  if ( ! ( NrrdData::SaveNrrd( filepath.string(), nrrd, error, compress, level ) ) ) 
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

bool DataVolume::CreateEmptyData( GridTransform grid_transform, 
  DataType data_type, DataVolumeHandle& data )
{
  DataBlockHandle data_block = StdDataBlock::New( grid_transform, data_type ) ;
  data_block->clear();
  
  data = DataVolumeHandle( new DataVolume( grid_transform, data_block ) );
  return true;
}

bool DataVolume::CreateInvalidData( GridTransform grid_transform, DataVolumeHandle& data )
{
  DataBlockHandle data_block;

  data = DataVolumeHandle( new DataVolume( grid_transform, data_block ) );
  return true;
}

bool DataVolume::ConvertToCanonicalVolume( const DataVolumeHandle& src_volume, 
                      DataVolumeHandle& dst_volume )
{
  const GridTransform& src_transform = src_volume->get_grid_transform();
  std::vector< int > permutation;
  GridTransform dst_transform;
  GridTransform::AlignToCanonicalCoordinates( src_transform, permutation, dst_transform );
  DataBlockHandle src_data_block = src_volume->get_data_block();
  DataBlockHandle dst_data_block;
  assert( permutation.size() == 3 );
  if ( permutation[ 0 ] != 1 || permutation[ 1 ] != 2 || permutation[ 2 ] != 3 )
  {
    DataBlock::PermuteData( src_data_block, dst_data_block, permutation );
  }
  else
  {
    dst_data_block = src_data_block;
  }

  dst_volume.reset( new DataVolume( dst_transform, dst_data_block ) );
  
  return true;
}

bool DataVolume::DuplicateVolume( const DataVolumeHandle& src_data_volume, 
    DataVolumeHandle& dst_data_volume )
{
  if ( !src_data_volume ) return false;
  
  DataBlockHandle dst_data_block;
  
  if ( !( DataBlock::Duplicate( src_data_volume->get_data_block(), dst_data_block ) ) )
  {
    return false;
  }
  
  dst_data_volume = DataVolumeHandle( new DataVolume( 
    src_data_volume->get_grid_transform(), dst_data_block ) );
  
  if ( !dst_data_volume ) return false;
  return true;
}

size_t DataVolume::get_byte_size() const
{
  return this->get_data_block()->get_byte_size();
}

bool DataVolume::insert_slice( const DataSliceHandle slice )
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->insert_slice( slice );
  }
  return false;
}

bool DataVolume::extract_slice( SliceType type, DataBlock::index_type index, DataSliceHandle& slice )
{
  if ( this->private_->data_block_ )
  {
    return this->private_->data_block_->extract_slice( type, index, slice );
  }
  return false;
}

void DataVolume::get_bricks( std::vector< DataVolumeBrickHandle >& bricks )
{
  bricks.clear();

  if ( !this->private_->data_block_ )
  {
    return;
  }
  
  {
    DataVolumePrivate::lock_type lock( this->private_->get_mutex() );
    if ( !this->private_->bricks_generated_ )
    {
      if ( !this->private_->generate_bricks() )
      {
        this->private_->bricks_.clear();
        return;
      }
      this->private_->bricks_generated_ = true;
    }
  }

  bricks = this->private_->bricks_;
}

} // end namespace Core
