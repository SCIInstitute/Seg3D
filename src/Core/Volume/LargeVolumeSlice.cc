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

#include <Core/Volume/LargeVolumeSlice.h>
#include <Core/Volume/LargeVolumeBrickSlice.h>
#include <Core/Utils/Exception.h>


namespace Core
{

struct BrickInfoHash
{
  std::size_t operator()(const Core::BrickInfo& bi) const
  {
    std::stringstream ss;
    ss << 'l' << bi.level_ << 'i' << bi.index_;
    return this->hasher_(ss.str());
  }

private:
  boost::hash<std::string> hasher_;
};

class LargeVolumeSlicePrivate
{
public:
  typedef boost::unordered_map<BrickInfo, LargeVolumeBrickSliceHandle, BrickInfoHash> tile_cache_type;
  typedef boost::unordered_map<std::string, tile_cache_type> volume_view_tile_cache_type;
  tile_cache_type tiles_;
  volume_view_tile_cache_type volume_view_tiles_;
};

LargeVolumeSlice::LargeVolumeSlice( const LargeVolumeHandle& large_volume, 
                 VolumeSliceType type, size_t slice_num ) :
  VolumeSlice( large_volume, type, slice_num ),
  private_( new LargeVolumeSlicePrivate )
{
  this->lv_schema_ = large_volume->get_schema();
}

LargeVolumeSlice::LargeVolumeSlice( const LargeVolumeSlice &copy ) :
  VolumeSlice( copy ),
  private_( copy.private_ ),
  lv_schema_( copy.lv_schema_ )
{
}

LargeVolumeSlice::~LargeVolumeSlice()
{
  this->disconnect_all();
}

void LargeVolumeSlice::upload_texture()
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolumeSlice does not implement upload_texture");
}

VolumeSliceHandle LargeVolumeSlice::clone()
{
  lock_type lock( this->get_mutex() );
  return VolumeSliceHandle( new LargeVolumeSlice( *this ) );
}

void LargeVolumeSlice::set_volume( const VolumeHandle& volume )
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolumeSlice does not implement set_volume.");
}

std::vector<LargeVolumeBrickSliceHandle> LargeVolumeSlice::load_tiles(
  double left, double right, double bottom, double top, double pixel_size, const std::string& load_key)
{
  std::vector<LargeVolumeBrickSliceHandle> result;
  Point viewport_bl, viewport_tr;
  this->get_world_coord(left, bottom, viewport_bl);
  this->get_world_coord(right, top, viewport_tr);
  BBox viewport_bbox(viewport_bl, viewport_tr);

  // Intersect the viewport bbox with the volume bbox
  GridTransform total_trans = this->get_volume()->get_grid_transform();
  BBox total = BBox( total_trans.get_origin() - total_trans.project( Vector( 0.5, 0.5, 0.5 ) ), total_trans.project( Point(
    static_cast<double>( total_trans.get_nx() ),
    static_cast<double>( total_trans.get_ny() ),
    static_cast<double>( total_trans.get_nz() ) ) ) - total_trans.project( Vector( 0.5, 0.5, 0.5 ) ) );
  if (!viewport_bbox.overlaps( total ))
  {
    return result;
  }

  BBox effective_bbox( Max( viewport_bbox.min(), total.min() ), Min( viewport_bbox.max(), total.max() ) );

  std::vector<BrickInfo> bricks = this->lv_schema_->
    get_bricks_for_region( effective_bbox, pixel_size, this->get_slice_type(), load_key );

  LargeVolumeSlicePrivate::tile_cache_type current_tiles;
  LargeVolumeSlicePrivate::tile_cache_type::iterator it;
  LargeVolumeBrickSliceHandle slice;
  for (size_t i = 0; i < bricks.size(); ++i)
  {
    it = this->private_->tiles_.find(bricks[i]);
    if (it != this->private_->tiles_.end())
    {
      slice = it->second;
    }
    else
    {
      slice = LargeVolumeBrickSliceHandle( new LargeVolumeBrickSlice(
        boost::dynamic_pointer_cast< LargeVolume>( this->get_volume() ), this->lv_schema_, bricks[ i ] ) );
    }

    current_tiles[ bricks[ i ] ] = slice;
    result.push_back( slice );
  }

  this->private_->tiles_ = current_tiles;
  return result;
}

std::vector<LargeVolumeBrickSliceHandle> LargeVolumeSlice::load_3d_tiles( const Transform& mvp_trans, int width, int height, const std::string& load_key )
{
  LargeVolumeSlicePrivate::tile_cache_type& tile_cache = this->private_->volume_view_tiles_[ load_key ];
  std::vector<LargeVolumeBrickSliceHandle> result;
  GridTransform total_trans = this->get_volume()->get_grid_transform();
  BBox total = BBox( total_trans.get_origin() - total_trans.project( Vector( 0.5, 0.5, 0.5 ) ),
    total_trans.project( Point( static_cast<double>( total_trans.get_nx() ),
    static_cast<double>( total_trans.get_ny() ),
    static_cast<double>( total_trans.get_nz() ) ) ) - total_trans.project( Vector( 0.5, 0.5, 0.5 ) ) );

  std::vector<BrickInfo> bricks = this->lv_schema_->get_bricks_for_volume( mvp_trans, 
    width, height, this->get_slice_type(), total, this->depth(), load_key );

  LargeVolumeSlicePrivate::tile_cache_type current_tiles;
  LargeVolumeSlicePrivate::tile_cache_type::iterator it;
  LargeVolumeBrickSliceHandle slice;
  for (size_t i = 0; i < bricks.size(); ++i)
  {
    it = tile_cache.find( bricks[ i ] );
    if (it != tile_cache.end())
    {
      slice = it->second;
    }
    else
    {
      slice = LargeVolumeBrickSliceHandle( new LargeVolumeBrickSlice(
        boost::dynamic_pointer_cast< LargeVolume>( this->get_volume() ), this->lv_schema_, bricks[ i ] ) );
    }

    current_tiles[ bricks[ i ] ] = slice;
    result.push_back( slice );
  }

  tile_cache = current_tiles;
  return result;
}

} // end namespace Core
