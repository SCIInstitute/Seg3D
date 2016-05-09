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

#include <limits>
#include <vector>

#include <boost/unordered_map.hpp>

#include <Core/DataBlock/DataBlock.h>
#include <Core/Graphics/PixelBufferObject.h>
#include <Core/RenderResources/RenderResources.h>
#include <Core/Volume/LargeVolumeBrickSlice.h>
#include <Core/Utils/Exception.h>
#include <Core/LargeVolume/LargeVolumeCache.h>

namespace Core
{
class LargeVolumeBrickSlicePrivate
{
public:
  Texture2DHandle texture_;
  VolumeSliceType texture_slice_type_;
  int texture_slice_number_;
  int texture_width_;
  int texture_height_;
  LargeVolumeHandle volume_;
  LargeVolumeSchemaHandle lv_schema_;
  BrickInfo bi_;

  BBox inner_;
  BBox outer_;

  LargeVolumeBrickSlicePrivate( LargeVolumeHandle volume, LargeVolumeSchemaHandle schema, const BrickInfo& bi ) :
    texture_slice_type_(VolumeSliceType::AXIAL_E), volume_(volume), lv_schema_(schema), bi_(bi)
  {
    this->texture_width_ = -1;
    this->texture_height_ = -1;

    GridTransform total_trans = volume->get_grid_transform();
    GridTransform brick_trans = schema->get_brick_grid_transform( bi );
    double overlap = static_cast<double>( schema->get_overlap() );
    Vector spacing =  schema->get_level_spacing( bi.level_ );

    BBox total = BBox( total_trans.get_origin() - total_trans.project( Vector( 0.5, 0.5, 0.5 ) ) , total_trans.project( Point (
      static_cast<double>(total_trans.get_nx()), 
      static_cast<double>(total_trans.get_ny()), 
      static_cast<double>(total_trans.get_nz()) ) ) - total_trans.project( Vector( 0.5, 0.5, 0.5 ) ) );

    this->outer_ = BBox( brick_trans.get_origin() - brick_trans.project( Vector( 0.5, 0.5, 0.5 ) ) , brick_trans.project( Point (
      static_cast<double>(brick_trans.get_nx()), 
      static_cast<double>(brick_trans.get_ny()), 
      static_cast<double>(brick_trans.get_nz()) ) ) - brick_trans.project( Vector( 0.5, 0.5, 0.5 ) ) );

    Point inner_min = outer_.min() + spacing * overlap;
    Point inner_max = outer_.max() - spacing * overlap;
    Point total_min = total.min();
    Point total_max = total.max();

    this->inner_ = BBox(  Max( inner_min, total_min ), Min( inner_max, total_max ) );
  }

  template<class T>
  void copy_slice_data(T* data, int slice_data_start, int width, int height, int h_stride, int v_stride, double value_min, double value_max )
  {
    const double numeric_min = static_cast<double>( std::numeric_limits< unsigned short >::min() );
    const double numeric_max = static_cast<double>( std::numeric_limits< unsigned short >::max() );
    const double value_range = value_max - value_min;
    const double inv_value_range = ( numeric_max - numeric_min ) / value_range;

    const T typed_value_min = static_cast<T>( value_min );


    // Step 1. copy the data in the slice to a pixel unpack buffer
    PixelBufferObjectHandle pixel_buffer(new PixelUnpackBuffer);
    pixel_buffer->bind();
    pixel_buffer->set_buffer_data(sizeof(unsigned short) * width * height,
      NULL, GL_STREAM_DRAW);
    unsigned short* buffer = reinterpret_cast< unsigned short* >( pixel_buffer->map_buffer( GL_WRITE_ONLY ) );

    int row_start = slice_data_start;
    int current_index;
    for (int j = 0; j < height; j++)
    {
      current_index = row_start;
      for (int i = 0; i < width; i++)
      {
        buffer[ j * width + i ] = static_cast<unsigned short>(  
        ( data[ current_index ] - typed_value_min ) * inv_value_range );

        current_index += h_stride;
      }
      row_start += v_stride;
    }

    // Step 2. copy from the pixel buffer to texture
    pixel_buffer->unmap_buffer();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    this->texture_->set_sub_image(0, 0, width,
      height, NULL, GL_LUMINANCE, GL_UNSIGNED_SHORT);
    this->texture_->unbind();

    // Step 3. release the pixel unpack buffer
    // NOTE: The texture streaming will still succeed even if the PBO is deleted.
    pixel_buffer->unbind();

    // Use glFinish here to solve synchronization issue when the slice is used in multiple views
    glFinish();

    CORE_CHECK_OPENGL_ERROR();
  }

  bool upload_texture(VolumeSliceType slice_type, int slice_number, const std::string& load_key )
  {
    if (this->texture_ && this->texture_slice_type_ == slice_type && this->texture_slice_number_ == slice_number)
      return true;

    DataBlockHandle data_block;
    if (!LargeVolumeCache::Instance()->get_brick( this->lv_schema_, this->bi_, load_key, data_block ))
      return false;

    int nx = static_cast<int>(data_block->get_nx());
    int ny = static_cast<int>(data_block->get_ny());
    int nz = static_cast<int>(data_block->get_nz());
    int width, height, h_stride, v_stride, slice_data_start;

    switch (slice_type)
    {
    case SliceType::AXIAL_E:
      width = nx;
      height = ny;
      h_stride = 1;
      v_stride = nx;
      slice_data_start = slice_number * nx * ny;
      break;
    case SliceType::CORONAL_E:
      width = nx;
      height = nz;
      h_stride = 1;
      v_stride = nx * ny;
      slice_data_start = slice_number * nx;
      break;
    case SliceType::SAGITTAL_E:
      width = ny;
      height = nz;
      h_stride = nx;
      v_stride = nx * ny;
      slice_data_start = slice_number;
      break;
    }

    if (!this->texture_)
    {
      this->texture_.reset(new Texture2D);
      this->texture_->set_mag_filter(GL_NEAREST);
      this->texture_->set_min_filter(GL_NEAREST);
    }

    this->texture_->bind();
    if (this->texture_width_ != width || this->texture_height_ != height)
    {
      this->texture_->set_image(width, height, GL_LUMINANCE16);
      this->texture_width_ = width;
      this->texture_height_ = height;
    }

    void* data = data_block->get_data();
    double min_val = this->lv_schema_->get_min();
    double max_val = this->lv_schema_->get_max();

    switch (data_block->get_data_type())
    {
    case DataType::UCHAR_E:
      copy_slice_data(reinterpret_cast<unsigned char*>(data), slice_data_start, 
        width, height, h_stride, v_stride, min_val, max_val );
      break;
    case DataType::CHAR_E:
      copy_slice_data(reinterpret_cast<signed char*>(data), slice_data_start, 
        width, height, h_stride, v_stride, min_val, max_val );
      break;
    case DataType::USHORT_E:
      copy_slice_data(reinterpret_cast<unsigned short*>(data), slice_data_start,
        width, height, h_stride, v_stride, min_val, max_val );
      break;
    case DataType::SHORT_E:
      copy_slice_data(reinterpret_cast<short*>(data), slice_data_start,
        width, height, h_stride, v_stride, min_val, max_val );
      break;
    case DataType::UINT_E:
      copy_slice_data(reinterpret_cast<unsigned int*>(data), slice_data_start,
        width, height, h_stride, v_stride, min_val, max_val );
      break;
    case DataType::INT_E:
      copy_slice_data(reinterpret_cast<int*>(data), slice_data_start,
        width, height, h_stride, v_stride, min_val, max_val );
      break;    
    case DataType::FLOAT_E:
      copy_slice_data(reinterpret_cast<float*>(data), slice_data_start,
        width, height, h_stride, v_stride, min_val, max_val );
      break;
    case DataType::DOUBLE_E:
      copy_slice_data(reinterpret_cast<double*>(data), slice_data_start,
        width, height, h_stride, v_stride, min_val, max_val );
      break;    
    default:
      break;
    }

    this->texture_slice_type_ = slice_type;
    this->texture_slice_number_ = slice_number;
    return true;
  }
};

LargeVolumeBrickSlice::LargeVolumeBrickSlice( LargeVolumeHandle volume, 
  LargeVolumeSchemaHandle schema, const BrickInfo& bi ) :
  private_(new LargeVolumeBrickSlicePrivate(volume, schema, bi))
{
}

LargeVolumeBrickSlice::~LargeVolumeBrickSlice()
{

}

Texture2DHandle LargeVolumeBrickSlice::get_texture( VolumeSliceType slice_type, double slice_pos, int& width, int& height,const std::string& load_key )
{
  // TODO: convert slice_pos to slice number
  GridTransform gt = this->private_->lv_schema_->get_brick_grid_transform( this->private_->bi_ );

  int slice_number = 0;
  switch ( slice_type )
  {
    case SliceType::SAGITTAL_E:
      slice_number =  Clamp( Floor( ( slice_pos - gt.project(Point(0.0,0.0,0.0)).x() ) / gt.spacing_x() ), 0, static_cast<int>(gt.get_nx() - 1));
    break;

    case SliceType::CORONAL_E:
      slice_number =  Clamp( Floor( ( slice_pos - gt.project(Point(0.0,0.0,0.0)).y() ) / gt.spacing_y() ), 0, static_cast<int>(gt.get_ny() - 1));
    break;

    case SliceType::AXIAL_E:
      slice_number =  Clamp( Floor( ( slice_pos - gt.project(Point(0.0,0.0,0.0)).z() ) / gt.spacing_z() ), 0, static_cast<int>(gt.get_nz() - 1));
    break;
  }

  if (this->private_->upload_texture( slice_type, slice_number, load_key ))
  {
    width = this->private_->texture_width_;
    height = this->private_->texture_height_;
    return this->private_->texture_;
  }

  return Texture2DHandle();
}

BBox LargeVolumeBrickSlice::get_inner_brick_bbox() const
{
  return this->private_->inner_;
}

BBox LargeVolumeBrickSlice::get_outer_brick_bbox() const
{
  return this->private_->outer_;
}

} // end namespace Core
