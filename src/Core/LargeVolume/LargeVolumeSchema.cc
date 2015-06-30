/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <zlib.h>

#include <limits>
#include <fstream>
#include <set>
#include <queue>

// test
#include <iostream>
// test

#include <Core/Utils/FilesystemUtil.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Math/MathFunctions.h>
#include <Core/DataBlock/StdDataBlock.h>

#include <Core/LargeVolume/LargeVolumeSchema.h>
#include <Core/LargeVolume/LargeVolumeCache.h>

namespace bfs=boost::filesystem;

namespace Core
{

class LargeVolumeSchemaPrivate {

public:
  // -- types --
  typedef LargeVolumeSchema::index_type index_type;

  // -- constructor --
public:
  LargeVolumeSchemaPrivate() :
    brick_size_( 256, 256, 256 ),
    effective_brick_size_( 256, 256, 256 ),
    overlap_(0),
    data_type_(DataType::UNKNOWN_E),
    compression_(false),
    little_endian_(DataBlock::IsLittleEndian()),
    downsample_x_( true ),
    downsample_y_( true ),
    downsample_z_( true ),
    min_( 0.0 ),
    max_( 0.0 )
  {
  }

  IndexVector compute_brick_layout( const IndexVector& size ) const
  {
    const IndexVector& effective_brick_size = this->effective_brick_size_;

    return IndexVector( Ceil( size.xd() / effective_brick_size.x() ), 
      Ceil( size.yd() / effective_brick_size.y() ), Ceil( size.zd() / effective_brick_size.z() ) );
  }

  IndexVector compute_level_size( index_type level ) const
  {
    const IndexVector& size = this->size_;
    const IndexVector& ratio = this->levels_[ level ];

    return IndexVector( Ceil( size.xd() / ratio.x() ) , Ceil( size.yd() / ratio.y() ), Ceil( size.zd() / ratio.z() ) );
  }

  Vector compute_level_spacing( index_type level ) const
  {
    const Vector& spacing = this->spacing_;
    const IndexVector& ratio = this->levels_[ level ];

    return Vector( spacing.x() * ratio.x(), spacing.y() * ratio.y(), spacing.z() * ratio.z() );
  }

  IndexVector compute_brick_index_vector( const IndexVector& layout, index_type brick_index )
  {
    const index_type nxy = layout.x() * layout.y();
    const index_type z = brick_index / nxy;
    const index_type y = (brick_index -  z * nxy ) / layout.x();
    const index_type x = (brick_index -  z * nxy - y * layout.x() );

    return IndexVector( x, y, z );
  }

  size_t compute_level_num_bricks( index_type level ) const
  {
    const IndexVector& size = this->compute_level_size( level );
    const IndexVector& effective_brick_size = this->effective_brick_size_;

    return Ceil( size.xd() / effective_brick_size.x() ) * Ceil( size.yd() / effective_brick_size.y() ) * Ceil( size.zd() / effective_brick_size.z() );
  }
  
  IndexVector compute_remainder_brick( const BrickInfo& bi,
                                  const IndexVector& index )
  {
    const IndexVector& layout = this->level_layout_[ bi.level_ ];
    const IndexVector& size = this->level_size_[ bi.level_ ];
    const size_t overlap = this->overlap_;
    IndexVector remainder_brick_size = this->brick_size_;
    
    if ( index.x() == layout.x() - 1 ) remainder_brick_size.x( size.x() - ( this->effective_brick_size_.x() * index.x()) + 2 * overlap );
    if ( index.y() == layout.y() - 1 ) remainder_brick_size.y( size.y() - ( this->effective_brick_size_.y() * index.y()) + 2 * overlap );
    if ( index.z() == layout.z() - 1 ) remainder_brick_size.z( size.z() - ( this->effective_brick_size_.z() * index.z()) + 2 * overlap );
    return remainder_brick_size;
  }

  bfs::path get_brick_file_name( BrickInfo bi )
  {
    // 65 = A in ASCII char table
    std::string filename = std::string() + static_cast<char>( 65 + static_cast<int>( bi.level_ ) ) +
      ExportToString( bi.index_ ) + ".raw";
    return this->dir_ / filename;
  }


  void compute_cached_level_info() 
  {
    this->level_spacing_.clear();
    this->level_size_.clear();
    this->level_layout_.clear();

    for ( size_t j = 0; j < this->levels_.size(); j++ )
    {
      this->level_spacing_.push_back( this->compute_level_spacing( j ) );
      this->level_size_.push_back( this->compute_level_size( j ) );
      this->level_layout_.push_back( this->compute_brick_layout( this->level_size_[ j ] ) );
    }
  }


  template<class T>
  bool insert_brick_internals( DataBlockHandle volume, DataBlockHandle brick,
                               const IndexVector& offset,
                               const IndexVector& clip_start,
                               const IndexVector& clip_end );

  template<class T>
  bool insert_brick_internals( DataBlockHandle volume, DataBlockHandle brick,
                               const IndexVector& offset);

  bool insert_brick( DataBlockHandle volume, DataBlockHandle brick,
                     const IndexVector& offset,
                     const IndexVector& clip_start,
                     const IndexVector& clip_end );

  void load_and_substitue_missing_bricks( std::vector<BrickInfo>& want_to_render, SliceType slice, 
    double depth, const std::string& load_key, std::vector<BrickInfo>& current_render );

  // -- contents in the text header --
public:
  IndexVector size_;

  Point origin_;
  Vector spacing_;

  IndexVector brick_size_;
  IndexVector effective_brick_size_;
  size_t overlap_;

  DataType data_type_;

  bool compression_;
  bool little_endian_;

  bool downsample_x_;
  bool downsample_y_;
  bool downsample_z_;

  std::vector< IndexVector > levels_;
  std::vector< Vector > level_spacing_;
  std::vector< IndexVector > level_size_;
  std::vector< IndexVector > level_layout_;

  double min_;
  double max_;
  
  bfs::path dir_;
  LargeVolumeSchema* schema_;
};

template<class T>
bool LargeVolumeSchemaPrivate::insert_brick_internals( DataBlockHandle volume, DataBlockHandle brick,
                                                       const IndexVector& offset,
                                                       const IndexVector& clip_start,
                                                       const IndexVector& clip_end )
{
  // TODO: too much repeated code
  const IndexVector::index_type overlap = static_cast<IndexVector::index_type>( this->overlap_ );

  const IndexVector::index_type bnx = static_cast<IndexVector::index_type>( brick->get_nx() );
  const IndexVector::index_type bny = static_cast<IndexVector::index_type>( brick->get_ny() );
  const IndexVector::index_type bnz = static_cast<IndexVector::index_type>( brick->get_nz() );
  const IndexVector::index_type bnxy = bnx * bny;

  const IndexVector::index_type bxstart = overlap + clip_start.x();
  const IndexVector::index_type bystart = overlap + clip_start.y();
  const IndexVector::index_type bzstart = overlap + clip_start.z();

  const IndexVector::index_type bxend = overlap + clip_end.x();
  const IndexVector::index_type byend = overlap + clip_end.y();
  const IndexVector::index_type bzend = overlap + clip_end.z();

  const IndexVector::index_type bxstride = ( bnx - ( bxend - bxstart ) );
  const IndexVector::index_type bystride = ( bny - ( byend - bystart ) ) * bnx;

  const IndexVector::index_type vnx = static_cast<IndexVector::index_type>( volume->get_nx() );
  const IndexVector::index_type vny = static_cast<IndexVector::index_type>( volume->get_ny() );
  const IndexVector::index_type vnz = static_cast<IndexVector::index_type>( volume->get_nz() );
  const IndexVector::index_type vnxy = vnx * vny;

  const IndexVector::index_type vxstride = vnx - ( bxend - bxstart  );
  const IndexVector::index_type vystride = ( vny - ( byend - bystart ) ) * vnx;
  
  // same code from here to return
  T* src = reinterpret_cast<T*>( brick->get_data() );
  T* dst = reinterpret_cast<T*>( volume->get_data() );

  src += bzstart * bnxy + bystart * bnx + bxstart;
  dst += offset.z() * vnxy + offset.y() * vnx  + offset.x();

  for ( IndexVector::index_type z = bzstart; z < bzend; z++, src += bystride, dst += vystride )
  {
    for ( IndexVector::index_type y = bystart; y < byend; y++, src += bxstride, dst += vxstride )
    {
      for ( IndexVector::index_type x = bxstart; x < bxend; x++ , src++, dst++ )
      {
        *dst = *src;
      }     
    } 
  }

  return true;
}

template<class T>
bool LargeVolumeSchemaPrivate::insert_brick_internals( DataBlockHandle volume, DataBlockHandle brick,
                                                       const IndexVector& offset )
{
  // TODO: too much repeated code
  const IndexVector::index_type overlap = static_cast<IndexVector::index_type>( this->overlap_ );

  const IndexVector::index_type bnx = static_cast<IndexVector::index_type>( brick->get_nx() );
  const IndexVector::index_type bny = static_cast<IndexVector::index_type>( brick->get_ny() );
  const IndexVector::index_type bnz = static_cast<IndexVector::index_type>( brick->get_nz() );
  const IndexVector::index_type bnxy = bnx * bny;

  const IndexVector::index_type bxstart = overlap;
  const IndexVector::index_type bystart = overlap;
  const IndexVector::index_type bzstart = overlap;
  
  const IndexVector::index_type bxend = bnx - overlap;
  const IndexVector::index_type byend = bny - overlap;
  const IndexVector::index_type bzend = bnz - overlap;

  const IndexVector::index_type bxstride = ( bnx - ( bxend - bxstart ) );
  const IndexVector::index_type bystride = ( bny - ( byend - bystart ) ) * bnx;

  const IndexVector::index_type vnx = static_cast<IndexVector::index_type>( volume->get_nx() );
  const IndexVector::index_type vny = static_cast<IndexVector::index_type>( volume->get_ny() );
  const IndexVector::index_type vnz = static_cast<IndexVector::index_type>( volume->get_nz() );
  const IndexVector::index_type vnxy = vnx * vny;

  const IndexVector::index_type vxstride = vnx - ( bnx - 2 * overlap );
  const IndexVector::index_type vystride = ( vny - ( bny - 2 * overlap ) ) * vnx;
  
  // same code from here to return
  T* src = reinterpret_cast<T*>( brick->get_data() );
  T* dst = reinterpret_cast<T*>( volume->get_data() );

  src += bzstart * bnxy + bystart * bnx + bxstart;
  dst += offset.z() * vnxy + offset.y() * vnx  + offset.x();

  for ( IndexVector::index_type z = bzstart; z < bzend; z++, src += bystride, dst += vystride )
  {
    for ( IndexVector::index_type y = bystart; y < byend; y++, src += bxstride, dst += vxstride )
    {
      for ( IndexVector::index_type x = bxstart; x < bxend; x++ , src++, dst++ )
      {
        *dst = *src;
      }     
    } 
  }

  return true;
}

LargeVolumeSchema::LargeVolumeSchema() :
  private_(new LargeVolumeSchemaPrivate),
  VOLUME_FILE_NAME_("volume.txt")
{
  this->private_->schema_ = this;
}


bool LargeVolumeSchema::load( std::string& error)
{
  bfs::path filename = this->private_->dir_ / VOLUME_FILE_NAME_;

  // Check if file exists
  if ( ! bfs::exists( filename ) )
  {
    error = "Could not open volume file '" + filename.string() + "'.";
    return false;
  }

  try
  {
    std::ifstream file_text( filename.string().c_str() );
    std::string line;
    std::map<std::string,std::string> values;
    
    // read text file

    while( !file_text.eof() )
    {
      // Grab the next line
      std::getline( file_text, line );
    
      std::vector<std::string> key_value = SplitString( line, ":" );
      if ( key_value.size() == 2 )
      {
        std::string val = key_value[1];
        StripSurroundingSpaces( val );
        values[key_value[0]] = val;
      }
    }

    // Read fields

    if (values.find("size") == values.end())
    {
      error = "Volume file does not contain a field called 'size'.";
      return false;
    }
    
    if (! ImportFromString( values[ "size" ], this->private_->size_ ) )
    {
      error = "Could not read size field.";
      return false;
    }

    if ( values.find( "origin" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'origin'.";
      return false;
    }
    
    if ( !ImportFromString( values[ "origin" ], this->private_->origin_ ) )
    {
      error = "Could not read origin field.";
      return false;
    }

    if ( values.find( "spacing" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'spacing'.";
      return false;
    }
    
    if ( !ImportFromString( values["spacing" ], this->private_->spacing_ ) )
    {
      error = "Could not read spacing field.";
      return false;
    }


    if ( values.find( "bricksize" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'bricksize'.";
      return false;
    }
    
    if (! ImportFromString( values[ "bricksize" ], this->private_->brick_size_ ) )
    {
      IndexVector::index_type single_brick_size;
      if (! ImportFromString( values[ "bricksize" ], single_brick_size ) )
      {
        error = "Could not read bricksize field.";
        return false;
      }

      this->private_->brick_size_ = IndexVector( single_brick_size, single_brick_size, single_brick_size );
    }


    if ( values.find( "overlap" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'overlap'.";
      return false;
    }
    
    if (! ImportFromString( values[ "overlap" ], this->private_->overlap_ ) )
    {
      error = "Could not read overlap field.";
      return false;
    }

    this->private_->effective_brick_size_.x( this->private_->brick_size_.x() -  2 * this->private_->overlap_ );
    this->private_->effective_brick_size_.y( this->private_->brick_size_.y() -  2 * this->private_->overlap_ );
    this->private_->effective_brick_size_.z( this->private_->brick_size_.z() -  2 * this->private_->overlap_ );

    if ( values.find( "datatype" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'datatype'.";
      return false;
    }
    
    if (! ImportFromString( values[ "datatype" ], this->private_->data_type_ ) )
    {
      error = "Could not read datatype field.";
      return false;
    }

    if ( values.find( "endian" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'endian'.";
      return false;
    }
    
    if ( values[ "endian" ] == "little" )
    {
      this->private_->little_endian_ = true;
    }
    else
    {
      this->private_->little_endian_ = false;
    }

    if ( values.find( "min" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'min'.";
      return false;
    }

    if (! ImportFromString( values[ "min" ], this->private_->min_ ) )
    {
      error = "Could not read min field.";
      return false;
    }


    if ( values.find( "max" ) == values.end() )
    {
      error = "Volume file does not contain a field called 'max'.";
      return false;
    }

    if (! ImportFromString( values[ "max" ], this->private_->max_ ) )
    {
      error = "Could not read min field.";
      return false;
    }

    size_t level = 0;
    
    while ( values.find( "level" + ExportToString(level)) != values.end() )
    {
      std::string level_string = values[ "level" + ExportToString(level) ];
      
      IndexVector ratios;
      if (! ImportFromString( level_string, ratios ))
      {
        error = "Could not read level " + ExportToString( level );
        return false;
      }

      this->private_->levels_.push_back( ratios );
      
      level++;
    }
    
    this->private_->compute_cached_level_info();
  }
  catch (...)
  {
    error = "Could not read volume file '" + filename.string() + "'.";
    return false;
  }

  return true;
}


bool LargeVolumeSchema::save( std::string& error ) const
{
  if ( !CreateOrIgnoreDirectory( this->private_->dir_ ) )
  {
    error = "Could not create directory '" + this->private_->dir_.string() + "'.";
    return false;
  }

  bfs::path filename = this->private_->dir_ / VOLUME_FILE_NAME_;
  
  try {
    std::ofstream text_file( filename.string().c_str() );
  
    text_file << "size: " << ExportToString( this->private_->size_ ) << std::endl;
    text_file << "origin: " << ExportToString( this->private_->origin_ ) << std::endl;
    text_file << "spacing: " << ExportToString( this->private_->spacing_ ) << std::endl;
    text_file << "overlap: " << ExportToString( this->private_->overlap_ ) << std::endl;
    text_file << "bricksize: " << ExportToString( this->private_->brick_size_ ) << std::endl;

    text_file << "datatype: " << ExportToString( this->private_->data_type_ ) << std::endl;
    text_file << "endian: " << ( this->private_->little_endian_ ? "little" : "big" ) << std::endl;
    text_file << "min: " << ExportToString( this->private_->min_ ) << std::endl;
    text_file << "max: " << ExportToString( this->private_->max_ ) << std::endl;
    
    for (size_t j = 0 ; j < this->private_->levels_.size(); j++ )
    {    
      text_file << "level" << j << ": " << ExportToString( this->private_->levels_[j] ) << std::endl;
    }
  }
  catch (...)
  {
    return false;
  }

  return true;
}


size_t LargeVolumeSchema::get_nx() const
{
  return this->private_->size_[0];
}

size_t LargeVolumeSchema::get_ny() const
{
  return this->private_->size_[1];
}

size_t LargeVolumeSchema::get_nz() const
{
  return this->private_->size_[2];
}

const IndexVector& LargeVolumeSchema::get_size() const
{
  return this->private_->size_;
}

const Point& LargeVolumeSchema::get_origin() const
{
  return this->private_->origin_;
}

const Vector& LargeVolumeSchema::get_spacing() const
{
  return this->private_->spacing_;
}

const IndexVector& LargeVolumeSchema::get_brick_size() const
{
  return this->private_->brick_size_;
}

const IndexVector& LargeVolumeSchema::get_effective_brick_size() const
{
  return this->private_->effective_brick_size_;
}

size_t LargeVolumeSchema::get_overlap() const
{
  return this->private_->overlap_;
}

DataType LargeVolumeSchema::get_data_type() const
{
  return this->private_->data_type_;
}

double LargeVolumeSchema::get_min() const
{
  return this->private_->min_;
}

double LargeVolumeSchema::get_max() const
{
  return this->private_->max_;
}

bfs::path LargeVolumeSchema::get_dir() const
{
  return this->private_->dir_;
}


GridTransform LargeVolumeSchema::get_grid_transform() const
{
  return GridTransform( this->private_->size_[ 0 ], this->private_->size_[ 1 ], this->private_->size_[ 2 ],
                        this->private_->origin_,
                        this->private_->spacing_[ 0 ] * GridTransform::X_AXIS,
                        this->private_->spacing_[ 1 ] * GridTransform::Y_AXIS,
                        this->private_->spacing_[ 2 ] * GridTransform::Z_AXIS );
}

GridTransform LargeVolumeSchema::get_brick_grid_transform( const BrickInfo& bi ) const
{
  const IndexVector& layout = this->get_level_layout( bi.level_ );
  const IndexVector& index = this->private_->compute_brick_index_vector( layout, bi.index_ );
  const Vector spacing = this->get_level_spacing( bi.level_ );

  const Point origin = this->get_origin();
  const IndexVector& effective_brick_size = this->private_->effective_brick_size_;
  const size_t overlap = this->private_->overlap_;

  Vector offset = Vector( index.x() * effective_brick_size.x() * spacing.x(),
                          index.y() * effective_brick_size.y() * spacing.y(),
                          index.z() * effective_brick_size.z() * spacing.z() )
                    - spacing * static_cast<double>( overlap );
  IndexVector bs = this->private_->compute_remainder_brick(bi, index);

  return GridTransform( bs.x(), bs.y(), bs.z(),
                        origin + offset,
                        spacing.x() * GridTransform::X_AXIS,
                        spacing.y() * GridTransform::Y_AXIS,
                        spacing.z() * GridTransform::Z_AXIS );
}

void LargeVolumeSchema::set_min_max( double min, double max ) const
{
  this->private_->min_ = min;
  this->private_->max_ = max;
}

bool LargeVolumeSchema::is_compressed() const
{
  return this->private_->compression_;
}

bool LargeVolumeSchema::is_little_endian() const
{
  return this->private_->little_endian_;
}

const IndexVector& LargeVolumeSchema::get_level_downsample_ratio( size_t idx ) const
{
  return this->private_->levels_[idx];
}

size_t LargeVolumeSchema::get_num_levels() const
{
  return this->private_->levels_.size();
}

void LargeVolumeSchema::set_dir( const bfs::path& dir )
{
  this->private_->dir_ = dir;
}

void LargeVolumeSchema::set_parameters( const IndexVector& size, const Vector& spacing, const Point& origin, 
  const IndexVector& brick_size, size_t overlap, DataType data_type )
{
  this->private_->size_ = size;
  this->private_->spacing_ = spacing;
  this->private_->origin_ = origin;
  this->private_->brick_size_ = brick_size;
  this->private_->overlap_ =  overlap;
  this->private_->data_type_ = data_type;
  this->private_->effective_brick_size_.x( brick_size.x() - 2 * overlap );
  this->private_->effective_brick_size_.y( brick_size.y() - 2 * overlap );
  this->private_->effective_brick_size_.z( brick_size.z() - 2 * overlap );
}

void LargeVolumeSchema::set_compression( bool compression )
{
  this->private_->compression_ = compression;
}

void LargeVolumeSchema::compute_levels()
{
  // Insert level 0:
  int cur_level = 0;
  IndexVector ratio( 1, 1, 1);
  this->private_->levels_.push_back( ratio );

  if ( !this->private_->downsample_x_ && !this->private_->downsample_y_ &&
    !this->private_->downsample_z_ )
  {
    this->private_->compute_cached_level_info();
  }
  else
  {
    IndexVector size = this->private_->compute_level_size( cur_level );
    IndexVector layout  = this->private_->compute_brick_layout( size );
    while ( 
      (this->private_->downsample_x_ && layout.x() > 1) || 
      (this->private_->downsample_y_ && layout.y() > 1) ||
      (this->private_->downsample_z_ && layout.z() > 1) )
    {
      Vector spacing = this->private_->compute_level_spacing( cur_level );
      if (! this->private_->downsample_x_ ) spacing.x( std::numeric_limits<double>::infinity() );
      if (! this->private_->downsample_y_ ) spacing.y( std::numeric_limits<double>::infinity() );
      if (! this->private_->downsample_z_ ) spacing.z( std::numeric_limits<double>::infinity() );

      double min_spacing = 1.5 * Min( spacing.x(), spacing.y(), spacing.z() );

      if ( spacing.x() < min_spacing ) ratio.x( 2 * ratio.x() );
      if ( spacing.y() < min_spacing ) ratio.y( 2 * ratio.y() );
      if ( spacing.z() < min_spacing ) ratio.z( 2 * ratio.z() );
  
      this->private_->levels_.push_back( ratio );
      cur_level++;
      size = this->private_->compute_level_size( cur_level );
      layout  = this->private_->compute_brick_layout( size );
    }

    this->private_->compute_cached_level_info();
  }
}



IndexVector LargeVolumeSchema::get_level_size( index_type level ) const
{
  return this->private_->level_size_[ level ];
}

Vector LargeVolumeSchema::get_level_spacing( index_type level ) const
{
  return this->private_->level_spacing_[ level ];
}

IndexVector LargeVolumeSchema::get_level_layout( index_type level ) const
{
  return this->private_->level_layout_[ level ];
}

IndexVector LargeVolumeSchema::get_brick_size( const BrickInfo& bi ) const
{
  const IndexVector& effective_brick_size = this->private_->effective_brick_size_;
  const IndexVector& layout = this->get_level_layout( bi.level_ );
  const IndexVector& index = this->private_->compute_brick_index_vector( layout, bi.index_ );

  IndexVector result = this->private_->compute_remainder_brick(bi, index);
  return result;
}

bool LargeVolumeSchema::read_brick( DataBlockHandle& brick, const BrickInfo& bi, std::string& error ) const
{
  IndexVector size = this->get_brick_size( bi );
  brick = StdDataBlock::New( size[0], size[1], size[2], this->get_data_type() );
  
  if ( !brick )
  {
    error = "Could not allocate brick.";
    return false;
  }

  bfs::path brick_file = this->private_->get_brick_file_name( bi );

  if ( !bfs::exists(brick_file ) )
  {
    error = "Could not open brick.";
    return false; 
  }

  size_t file_size = bfs::file_size( brick_file );
  size_t brick_size = size[0] * size[1] * size[2] * GetSizeDataType( this->get_data_type() );

  if ( brick_size > file_size )
  {
    try
    {
      std::vector<char> buffer( file_size );

      std::ifstream input( brick_file.string().c_str(), std::ios_base::in | std::ios_base::binary );
      input.read( &buffer[0],  file_size);
      input.close();

      z_uLongf brick_size_ul = brick_size;
      if ( z_uncompress( reinterpret_cast<z_Bytef*>( brick->get_data() ), &brick_size_ul,
        reinterpret_cast<z_Bytef*>(&buffer[0]), file_size ) != Z_OK)
      {
        error = "Could not decompress file '" + brick_file.string() + "'.";
        return false;
      }

      if ( brick_size_ul != brick_size )
      {
        error = "Brick '" + brick_file.string() + "' contains invalid data.";
        brick->clear();
        return false;     
      }
    }
    catch ( ... )
    {
      error = "Error reading file '" + brick_file.string() + "'.";
      brick->clear();
      return false;
    }
  }
  else if ( brick_size == file_size )
  {
    try
    {
      std::ifstream input( brick_file.string().c_str(), std::ios_base::in | std::ios_base::binary );
      input.read( reinterpret_cast<char *>(brick->get_data()), brick_size );
      input.close();
    }
    catch ( ... )
    {
      error = "Error reading file '" + brick_file.string() + "'.";
      brick->clear();
      return false;
    } 
  }
  else 
  {
    error = "Brick file is too large to be a brick.";
    brick->clear();
    return false;
  }

  if ( DataBlock::IsLittleEndian() != this->private_->little_endian_ )
  {
    brick->swap_endian();
  }

  return true;
}

bool LargeVolumeSchema::append_brick_buffer( DataBlockHandle data_block, size_t z_start, size_t z_end,
    size_t offset, const BrickInfo& bi, std::string& error ) const
{
  IndexVector size = this->get_brick_size( bi );
  
  size_t slice_size = data_block->get_nx() * data_block->get_ny(); 

  if ( size[0] != data_block->get_nx() || size[1] != data_block->get_ny() )
  {
    error = "Brick is of incorrect size.";
    return false;
  }

  if ( this->private_->data_type_ != data_block->get_data_type() )
  {
    error = "Brick is incorrect data type.";
    return false;
  }

  bfs::path brick_file = this->private_->get_brick_file_name( bi );

  size_t buffer_size = slice_size * ( z_end - z_start ) * GetSizeDataType( this->get_data_type() );
  size_t buffer_offset = z_start * slice_size * GetSizeDataType( this->get_data_type() );

  try
  {
    std::ofstream output( brick_file.string().c_str(), std::ios_base::app | std::ios_base::binary | std::ios_base::out );
    
    output.seekp( offset * size[0] * size[1] * GetSizeDataType( this->get_data_type()), std::ios_base::beg );
    output.write( reinterpret_cast<char *>( data_block->get_data() ) + buffer_offset, buffer_size );
  }
  catch ( ... )
  {
    error = "Could not write to file '" + brick_file.string() + "'.";
    return false;
  } 

  return true;
}

bool LargeVolumeSchema::write_brick( DataBlockHandle data_block, const BrickInfo& bi, std::string& error ) const
{
  IndexVector size = this->get_brick_size( bi );
  
  if ( size[0] != data_block->get_nx() || size[1] != data_block->get_ny() ||
    size[2] != data_block->get_nz() )
  {
    error = "Brick is of incorrect size.";
    return false;
  }

  if ( this->private_->data_type_ != data_block->get_data_type() )
  {
    error = "Brick is incorrect data type.";
    return false;
  }

  bfs::path brick_file = this->private_->get_brick_file_name( bi );

  size_t brick_size = size[0] * size[1] * size[2] * GetSizeDataType( this->get_data_type() );

  if ( this->private_->compression_) 
  {
    std::vector<char> buffer( brick_size + 12 );
    z_uLongf brick_size_ul = brick_size + 12;

    int result = z_compress2( reinterpret_cast<z_Bytef*>( &buffer[0] ), &brick_size_ul,
      reinterpret_cast<z_Bytef*>(data_block->get_data()), brick_size, Z_DEFAULT_COMPRESSION );
    if (result != Z_OK )
    {
      error = "Could not compress file.";
      return false;
    }
  
    if ( brick_size_ul < brick_size )
    {
      // Compression succeeded
      try
      {
        std::ofstream output( brick_file.string().c_str(), std::ios_base::trunc | std::ios_base::binary | std::ios_base::out );
        output.write( &buffer[0], brick_size_ul );
      }
      catch ( ... )
      {
        error = "Could not write to file '" + brick_file.string() + "'.";
        return false;
      }

    } 
    else 
    {
      try
      {
        std::ofstream output( brick_file.string().c_str(), std::ios_base::trunc | std::ios_base::binary | std::ios_base::out );
        output.write( reinterpret_cast<char *>( data_block->get_data() ) , brick_size );
      }
      catch ( ... )
      {
        error = "Could not write to file '" + brick_file.string() + "'.";
        return false;
      }   
    }
  }
  else
  {
    try
    {
      std::ofstream output( brick_file.string().c_str(), std::ios_base::trunc | std::ios_base::binary | std::ios_base::out );
      output.write( reinterpret_cast<char *>( data_block->get_data() ) , brick_size );
    }
    catch ( ... )
    {
      error = "Could not write to file '" + brick_file.string() + "'.";
      return false;
    }   
  }

  return true;
}


bool LargeVolumeSchema::get_parent(const BrickInfo& bi, BrickInfo& parent)
{
  BrickInfo::index_type level = bi.level_;
  if (level >= this->get_num_levels() - 1 ) return false;

  IndexVector layout = this->get_level_layout( level );
  IndexVector layout_next = this->get_level_layout( level + 1 );
  IndexVector index = this->private_->compute_brick_index_vector( layout, bi.index_ );
  IndexVector ratio = this->get_level_downsample_ratio( level );
  IndexVector ratio_next = this->get_level_downsample_ratio( level + 1 );

  index.x( ( index.x() * ratio.x() ) / ratio_next.x() );
  index.y( ( index.y() * ratio.y() ) / ratio_next.y() );
  index.z( ( index.z() * ratio.z() ) / ratio_next.z() );

  parent =  BrickInfo( index.z() * layout_next.y() * layout_next.x() + index.y() * layout_next.x() + index.x(), level + 1 );
  return true;
}

bool LargeVolumeSchema::get_children( const BrickInfo& bi, SliceType slice, double depth,
  std::vector<BrickInfo>& children )
{
  children.clear();

  BrickInfo::index_type level = bi.level_;
  if (level == 0 ) return false;  

  IndexVector ratio = this->get_level_downsample_ratio( level );
  IndexVector ratio_next = this->get_level_downsample_ratio( level - 1 );

  IndexVector rat( ratio.x() / ratio_next.x(), ratio.y() / ratio_next.y(),
    ratio.z() / ratio_next.z());

  IndexVector layout = this->get_level_layout( level );
  IndexVector layout_next = this->get_level_layout( level - 1 );
  IndexVector index = this->private_->compute_brick_index_vector( layout, bi.index_ );

  Point origin = this->private_->origin_;

  Vector spacing = this->get_level_spacing( level );
  IndexVector eb = this->private_->effective_brick_size_;
  
  switch ( slice )
  {
  case SliceType::SAGITTAL_E:
    {
      IndexVector::index_type x_index = index.x();

      if ( rat.x() == 2 )
      {
        if ( spacing.x() * eb.xd() * ( index.xd() + 0.5 ) > depth - origin.x() )
        {
          x_index = 2 * x_index;
        } 
        else 
        {
          x_index = 2 * x_index + 1; 
        }
      } 

      x_index = Min( x_index, layout_next.x() - 1 );

      for (IndexVector::index_type y = rat.y() * index.y(); 
          y < Min( layout_next.y(), rat.y() * ( index.y() + 1 ) ); y++ )
      {
        for (IndexVector::index_type z = rat.z() * index.z(); 
          z < Min( layout_next.z(), rat.z() * ( index.z() + 1 ) ); z++ )
        {
          children.push_back( BrickInfo( layout_next.x() * layout_next.y() * z +
            layout_next.x() * y + x_index, level - 1));
        }
      }

      return true;
    }

  case SliceType::CORONAL_E:
    {
      IndexVector::index_type y_index = index.y();

      if ( rat.y() == 2 )
      {
        if (spacing.y() * eb.yd() * ( index.yd() + 0.5 ) > depth - origin.y())
        {
          y_index = 2 * y_index;
        } 
        else 
        {
          y_index = 2 * y_index + 1; 
        }
      } 

      y_index = Min( y_index, layout_next.y() - 1 );

      for (IndexVector::index_type x = rat.x() * index.x(); 
          x < Min( layout_next.x(), rat.x() * ( index.x() + 1 ) ); x++ )
      {
        for (IndexVector::index_type z = rat.z() * index.z(); 
          z < Min( layout_next.z(), rat.z() * ( index.z() + 1 ) ); z++ )
        {
          children.push_back( BrickInfo( layout_next.x() * layout_next.y() * z +
            layout_next.x() * y_index + x, level - 1));
        }
      }
    return true;
    }

  case SliceType::AXIAL_E:
    {
      IndexVector::index_type z_index = index.z();

      if ( rat.z() == 2 )
      {
        if (spacing.z() * eb.zd() * ( index.zd() + 0.5 ) > depth - origin.z())
        {
          z_index = 2 * z_index;
        } 
        else 
        {
          z_index = 2 * z_index + 1; 
        }
      } 

      z_index = Min( z_index, layout_next.z() - 1 );

      for (IndexVector::index_type x = rat.x() * index.x(); 
          x < Min( layout_next.x(), rat.x() * ( index.x() + 1 ) ); x++ )
      {
        for (IndexVector::index_type y = rat.y() * index.y(); 
          y < Min( layout_next.y(), rat.y() * ( index.y() + 1 ) ); y++ )
        {
          children.push_back( BrickInfo( layout_next.x() * layout_next.y() * z_index +
            layout_next.x() * y + x, level - 1));
        }
      }
      return true;
    } 
  }

  return false;
}


std::vector<BrickInfo> LargeVolumeSchema::get_bricks_for_volume( Transform world_viewport, 
  int width, int height, SliceType slice, const BBox& effective_bbox, double depth,
  const std::string& load_key )
{
  std::vector<BrickInfo> result;

  int num_levels = static_cast<int>( this->get_num_levels() );
  int level = num_levels - 1;
  IndexVector layout = this->get_level_layout( level );
  std::queue<BrickInfo> bricks;

  for( size_t j = 0; j < layout.x() * layout.y() * layout.z(); j++ )
  {
    bricks.push( BrickInfo( j, level ) );
  }

  bool found_right_level = false;
  
  size_t overlap = this->get_overlap();

  BBox viewable_box( Point(-1.0, -1.0, -1.0), Point( 1.0, 1.0, 1.0 ));

  while ( !bricks.empty() )
  {
    BrickInfo bi = bricks.front();
    bricks.pop();
    Vector spacing = this->get_level_spacing(  bi.level_ );
    GridTransform trans = this->get_brick_grid_transform( bi );
    BBox bbox( trans * ( Point( overlap, overlap, overlap ) - Vector( 0.5, 0.5, 0.5 ) ),
      trans * ( Point( trans.get_nx() - overlap, trans.get_ny() - overlap,
      trans.get_nz() - overlap ) - Vector( 0.5, 0.5, 0.5 ) ) );

    switch ( slice )
    {
      case SliceType::SAGITTAL_E:
      {
        if ( !effective_bbox.overlaps(bbox) || depth <  bbox.min().x() || depth > bbox.max().x() ) continue;

        Point p1 = Point( depth, bbox.min().y(), bbox.min().z() );
        Point p2 = Point( depth, bbox.max().y(), bbox.min().z() );
        Point p3 = Point( depth, bbox.min().y(), bbox.max().z() );
        Point p4 = Point( depth, bbox.max().y(), bbox.max().z() );
        BBox region(p1, p4);

        Point proj_p1 = world_viewport.project( p1 );
        Point proj_p2 = world_viewport.project( p2 );
        Point proj_p3 = world_viewport.project( p3 );
        Point proj_p4 = world_viewport.project( p4 );

        BBox plane( proj_p1, proj_p2 );
        plane.extend( proj_p3 );
        plane.extend( proj_p4 );
        if (! viewable_box.overlaps( plane ) ) continue;

        Point q1 = p1 + Vector( 0.0, spacing.y(), spacing.z() );
        Point q2 = p2 + Vector( 0.0, -spacing.y(), spacing.z() );
        Point q3 = p3 + Vector( 0.0, spacing.y(), -spacing.z() );
        Point q4 = p4 + Vector( 0.0, -spacing.y(), -spacing.z() );

        Point proj_q1 = world_viewport.project( q1 );
        Point proj_q2 = world_viewport.project( q2 );
        Point proj_q3 = world_viewport.project( q3 );
        Point proj_q4 = world_viewport.project( q4 );

        if ( Abs( proj_p1.x() - proj_q1.x() ) * width > 2.0 || 
           Abs( proj_p1.y() - proj_q1.y() ) * height > 2.0 ||
           Abs( proj_p2.x() - proj_q2.x() ) * width > 2.0 || 
           Abs( proj_p2.y() - proj_q2.y() ) * height > 2.0 ||
           Abs( proj_p3.x() - proj_q3.x() ) * width > 2.0 || 
           Abs( proj_p3.y() - proj_q3.y() ) * height > 2.0 ||         
           Abs( proj_p4.x() - proj_q4.x() ) * width > 2.0 || 
           Abs( proj_p4.y() - proj_q4.y() ) * height > 2.0 )
        {
          std::vector<BrickInfo> children;
          if ( this->get_children( bi, slice, depth, children) )
          {
            for (size_t k = 0; k < children.size(); k++)
            bricks.push( children[ k ] );
          }
          else
          {
            result.push_back( bi );       
          }
        }
        else
        {
          result.push_back( bi );
        }
      }
      break;

    case SliceType::CORONAL_E:
      {
      if (!effective_bbox.overlaps( bbox ) || depth <  bbox.min().y() || depth > bbox.max().y()) continue;

        Point p1 = Point( bbox.min().x(), depth, bbox.min().z() );
        Point p2 = Point( bbox.max().x(), depth, bbox.min().z() );
        Point p3 = Point( bbox.min().x(), depth, bbox.max().z() );
        Point p4 = Point( bbox.max().x(), depth, bbox.max().z() );
        BBox region(p1, p4);

        Point proj_p1 = world_viewport.project( p1 );
        Point proj_p2 = world_viewport.project( p2 );
        Point proj_p3 = world_viewport.project( p3 );
        Point proj_p4 = world_viewport.project( p4 );

        BBox plane( proj_p1, proj_p2 );
        plane.extend( proj_p3 );
        plane.extend( proj_p4 );
        if (! viewable_box.overlaps( plane ) ) continue;

        Point q1 = p1 + Vector( spacing.x(), 0.0, spacing.z() );
        Point q2 = p2 + Vector( -spacing.x(), 0.0, spacing.z() );
        Point q3 = p3 + Vector( spacing.x(), 0.0, -spacing.z() );
        Point q4 = p4 + Vector( -spacing.x(), 0.0, -spacing.z() );

        Point proj_q1 = world_viewport.project( q1 );
        Point proj_q2 = world_viewport.project( q2 );
        Point proj_q3 = world_viewport.project( q3 );
        Point proj_q4 = world_viewport.project( q4 );

        if ( Abs( proj_p1.x() - proj_q1.x() ) * width > 2.0 || 
           Abs( proj_p1.y() - proj_q1.y() ) * height > 2.0 ||
           Abs( proj_p2.x() - proj_q2.x() ) * width > 2.0 || 
           Abs( proj_p2.y() - proj_q2.y() ) * height > 2.0 ||
           Abs( proj_p3.x() - proj_q3.x() ) * width > 2.0 || 
           Abs( proj_p3.y() - proj_q3.y() ) * height > 2.0 ||         
           Abs( proj_p4.x() - proj_q4.x() ) * width > 2.0 || 
           Abs( proj_p4.y() - proj_q4.y() ) * height > 2.0 )
        {
          std::vector<BrickInfo> children;
          if ( this->get_children( bi, slice, depth, children) )
          {
            for (size_t k = 0; k < children.size(); k++)
            bricks.push( children[ k ] );
          }
          else
          {
            result.push_back( bi );       
          }
        }
        else
        {
          result.push_back( bi );
        }
      }
      break;

      case SliceType::AXIAL_E:
      {
        if (!effective_bbox.overlaps( bbox ) || depth <  bbox.min().z() || depth > bbox.max().z()) continue;

        Point p1 = Point( bbox.min().x(), bbox.min().y(), depth );
        Point p2 = Point( bbox.max().x(), bbox.min().y(), depth );
        Point p3 = Point( bbox.min().x(), bbox.max().y(), depth );
        Point p4 = Point( bbox.max().x(), bbox.max().y(), depth );
        BBox region(p1, p4);

        Point proj_p1 = world_viewport.project( p1 );
        Point proj_p2 = world_viewport.project( p2 );
        Point proj_p3 = world_viewport.project( p3 );
        Point proj_p4 = world_viewport.project( p4 );

        BBox plane( proj_p1, proj_p2 );
        plane.extend( proj_p3 );
        plane.extend( proj_p4 );
        if (! viewable_box.overlaps( plane ) ) continue;

        Point q1 = p1 + Vector( spacing.x(), spacing.y(), 0.0 );
        Point q2 = p2 + Vector( -spacing.x(), spacing.y(), 0.0 );
        Point q3 = p3 + Vector( spacing.x(), -spacing.y(), 0.0 );
        Point q4 = p4 + Vector( -spacing.x(), -spacing.y(), 0.0 );

        Point proj_q1 = world_viewport.project( q1 );
        Point proj_q2 = world_viewport.project( q2 );
        Point proj_q3 = world_viewport.project( q3 );
        Point proj_q4 = world_viewport.project( q4 );

        if ( Abs( proj_p1.x() - proj_q1.x() ) * width > 2.0 || 
           Abs( proj_p1.y() - proj_q1.y() ) * height > 2.0 ||
           Abs( proj_p2.x() - proj_q2.x() ) * width > 2.0 || 
           Abs( proj_p2.y() - proj_q2.y() ) * height > 2.0 ||
           Abs( proj_p3.x() - proj_q3.x() ) * width > 2.0 || 
           Abs( proj_p3.y() - proj_q3.y() ) * height > 2.0 ||         
           Abs( proj_p4.x() - proj_q4.x() ) * width > 2.0 || 
           Abs( proj_p4.y() - proj_q4.y() ) * height > 2.0 )
        {
          std::vector<BrickInfo> children;
          if ( this->get_children( bi, slice, depth, children) )
          {
            for (size_t k = 0; k < children.size(); k++)
            bricks.push( children[ k ] );
          }
          else
          {
            result.push_back( bi );       
          }
        }
        else
        {
          result.push_back( bi );
        }
      }
      break;
    }
  }

  std::vector<BrickInfo> current_render;
  this->private_->load_and_substitue_missing_bricks( result, slice, depth, load_key, current_render );

  return current_render;
}

void LargeVolumeSchemaPrivate::load_and_substitue_missing_bricks( std::vector<BrickInfo>& want_to_render, 
  SliceType slice, double depth, const std::string& load_key, std::vector<BrickInfo>& current_render )
{
  LargeVolumeCache* cache = LargeVolumeCache::Instance();

  index_type num_levels = this->schema_->get_num_levels();
  std::vector<std::set<BrickInfo> > bricks_to_render( num_levels );
  std::vector<BrickInfo> bricks_to_load;

  for (size_t k = 0; k < want_to_render.size(); k++)
  {
    BrickInfo brick = want_to_render[ k ];
    if (!cache->mark_brick( this->schema_->shared_from_this(), brick ))
    {
      // check children
      bool have_children = false;
      std::vector<BrickInfo> children;
      if (this->schema_->get_children( brick, slice, depth, children ))
      {
        have_children = true;
        for (size_t j = 0; j < children.size(); j++)
        {
          if (!cache->mark_brick( this->schema_->shared_from_this(), children[ j ] ))
          {
            have_children = false;
            break;
          }
        }

        if (have_children)
        {
          for (size_t j = 0; j < children.size(); j++)
          {
            bricks_to_render[ children[ j ].level_ ].insert( children[ j ] );
          }
        }
      }

      if (!have_children)
      {
        // check parents
        BrickInfo parent = brick;
        while (this->schema_->get_parent( parent, parent ))
        {

          if (cache->mark_brick( this->schema_->shared_from_this(), parent ))
          {
            bricks_to_render[ parent.level_ ].insert( parent );
            break;
          }
        }
      }
      bricks_to_load.push_back( brick );
    }
    else
    {
      bricks_to_render[ brick.level_ ].insert( brick );
    }
  }

  for (index_type lev = num_levels - 1; lev >= 0; lev--)
  {
    current_render.insert( current_render.end(), bricks_to_render[ lev ].begin(), bricks_to_render[ lev ].end() );
  }

  cache->clear_load_queue( load_key );
  for (size_t k = 0; k < bricks_to_load.size(); k++)
  {
    cache->load_brick( this->schema_->shared_from_this(), bricks_to_load[ k ], load_key );
  }
}

std::vector<BrickInfo> LargeVolumeSchema::get_bricks_for_region( const BBox& region, double pixel_size, SliceType slice,
  const std::string& load_key)
{
  index_type level = 0;
  index_type num_levels = this->get_num_levels();

  LargeVolumeCache* cache = LargeVolumeCache::Instance();
  double depth;

  // Compute needed level
  switch( slice )
  {
    case SliceType::SAGITTAL_E:
      {
        depth = region.min().x();
        while (level + 1 < num_levels)
        {
          Vector spacing = this->get_level_spacing( level + 1 );
          if ( Max( spacing.y(), spacing.z() ) >= pixel_size ) break;
          level++;
        }
        break;
      }

    case SliceType::CORONAL_E:
      {
        depth = region.min().y();
        while (level + 1 < num_levels)
        {
          Vector spacing = this->get_level_spacing( level + 1 );
          if ( Max( spacing.x(), spacing.z() ) >= pixel_size ) break;
          level++;
        }
        break;
      } 
    case SliceType::AXIAL_E:
      {
        depth = region.min().z();
        while (level + 1 < num_levels)
        {
          Vector spacing = this->get_level_spacing( level + 1 );
          if ( Max( spacing.x(), spacing.y() ) >= pixel_size ) break;
          level++;
        }
        break;
      } 
  }


  Point origin = this->private_->origin_;
  Vector min = region.min() - origin;
  Vector max = region.max() - origin;

  Vector spacing = this->get_level_spacing( level );
  IndexVector layout = this->get_level_layout( level );
  IndexVector eb = this->private_->effective_brick_size_;

  index_type sx = Max( 0, Floor( min.x() / ( eb.x() * spacing.x()) ) );
  index_type ex = Min( layout.x(), static_cast<index_type>( Floor( max.x() / ( eb.x() * spacing.x()) ) + 1 ) );

  index_type sy = Max( 0, Floor( min.y() / ( eb.y() * spacing.y()) ));
  index_type ey = Min( layout.y(), static_cast<index_type>( Floor( max.y() / ( eb.y() * spacing.y()) ) + 1 ) );

  index_type sz = Max( 0, Floor( min.z() / ( eb.z() * spacing.z()) ) );
  index_type ez = Min( layout.z(), static_cast<index_type>( Floor( max.z() / ( eb.z() * spacing.z()) ) + 1 ) );

  std::vector<BrickInfo> want_to_render;

  index_type nx = layout.x();
  index_type nxy = layout.x() * layout.y();

  for (index_type z = sz; z < ez; z++ )
  {
    for (index_type y = sy; y < ey; y++ )
    {
      for (index_type x = sx; x < ex; x++ )
      {
        want_to_render.push_back(BrickInfo( x + nx * y + nxy * z, level ));
      }
    } 
  }

  std::vector<BrickInfo> current_render;
  this->private_->load_and_substitue_missing_bricks( want_to_render, slice, depth, load_key, current_render );

  return current_render;
}


bool LargeVolumeSchema::reprocess_brick( const BrickInfo& bi,
  std::string& error ) const
{
  error = "";

  // Read in uncompressed brick
  DataBlockHandle data_block;
  if (! this->read_brick( data_block, bi, error ) )
  {
    return false;
  }


  // Delete file, so that when we write the file again it will be more likely
  // to be in a continuous block, especially since we do it as one write, hence
  // a good FS should give us contiguous blocks on disk
  bfs::path brick_file = this->private_->get_brick_file_name( bi );
  if (! bfs::remove( brick_file ) )
  {
    error = "Could not remove brick file.";
    return false;
  }

  // Write the file as one entity back to disk
  if (! this->write_brick( data_block, bi, error ) )
  {
    return false;
  }

  return true;
}

void LargeVolumeSchema::enable_downsample( bool downsample_x, bool downsample_y, bool downsample_z )
{
  this->private_->downsample_x_ = downsample_x;
  this->private_->downsample_y_ = downsample_y;
  this->private_->downsample_z_ = downsample_z;
}

bfs::path LargeVolumeSchema::get_brick_file_name( const BrickInfo& bi )
{
  return this->private_->get_brick_file_name( bi );
}


bool LargeVolumeSchema::get_level_start_and_end( const GridTransform& region, int level,
  IndexVector& start, IndexVector& end, GridTransform& gt )
{
  GridTransform trans = this->get_grid_transform();

  BBox tbox( trans.project( Point( 0.0, 0.0, 0.0 ) ), trans.project(
    Point( trans.get_nx(), trans.get_ny(), trans.get_nz() ) ) );

  BBox rbox( region.project( Point( 0.0, 0.0, 0.0 ) ), region.project(
    Point( region.get_nx(), region.get_ny(), region.get_nz() )
    ) );

  Vector spacing = this->get_level_spacing( level );
  Vector data_spacing = this->get_level_spacing( 0 );

  Point origin = this->get_origin() - ( 0.5 * data_spacing ) + ( 0.5 * spacing );

  start = IndexVector(  
    Max( 0, Floor( ( rbox.min().x() - tbox.min().x() ) / spacing.x() ) ),
    Max( 0, Floor( ( rbox.min().y() - tbox.min().y() ) / spacing.y() ) ),
    Max( 0, Floor( ( rbox.min().z() - tbox.min().z() ) / spacing.z() ) ) );

  end = IndexVector(  
    Min( static_cast<int>( trans.get_nx() ), Floor( ( rbox.max().x() - tbox.min().x() ) / spacing.x() ) ),
    Min( static_cast<int>( trans.get_ny() ), Floor( ( rbox.max().y() - tbox.min().y() ) / spacing.y() ) ),
    Min( static_cast<int>( trans.get_nz() ), Floor( ( rbox.max().z() - tbox.min().z() ) / spacing.z() ) ) );

  gt = GridTransform( end.x() - start.x(), end.y() - start.y(), end.z() - start.z(),
    origin + Vector( start.xd() * spacing.x(), start.yd() * spacing.y(), start.zd() * spacing.z()), 
    spacing.x() * Vector( 1.0, 0.0, 0.0 ), spacing.y() * Vector( 0.0, 1.0, 0.0 ), spacing.z() * Vector( 0.0, 0.0, 1.0 ) );

  return true;
}

bool LargeVolumeSchema::insert_brick( DataBlockHandle volume, DataBlockHandle brick, IndexVector offset, IndexVector clip_start, IndexVector clip_end )
{
  switch ( volume->get_data_type() )
  {
  case DataType::UCHAR_E:
    return this->private_->insert_brick_internals<unsigned char>( volume, brick, offset, clip_start, clip_end );
  case DataType::CHAR_E:
    return this->private_->insert_brick_internals<signed char>( volume, brick, offset, clip_start, clip_end );
  case DataType::USHORT_E:
    return this->private_->insert_brick_internals<unsigned short>( volume, brick, offset, clip_start, clip_end );
  case DataType::SHORT_E:
    return this->private_->insert_brick_internals<short>( volume, brick, offset, clip_start, clip_end );
  case DataType::UINT_E:
    return this->private_->insert_brick_internals<unsigned int>( volume, brick, offset, clip_start, clip_end );
  case DataType::INT_E:
    return this->private_->insert_brick_internals<int>( volume, brick, offset, clip_start, clip_end );
  case DataType::FLOAT_E:
    return this->private_->insert_brick_internals<float>( volume, brick, offset, clip_start, clip_end );
  case DataType::DOUBLE_E:
    return this->private_->insert_brick_internals<double>( volume, brick, offset, clip_start, clip_end );
  }

  return false;
}

bool LargeVolumeSchema::insert_brick( DataBlockHandle volume, DataBlockHandle brick, IndexVector offset )
{
  switch ( volume->get_data_type() )
  {
  case DataType::UCHAR_E:
    return this->private_->insert_brick_internals<unsigned char>( volume, brick, offset );
  case DataType::CHAR_E:
    return this->private_->insert_brick_internals<signed char>( volume, brick, offset );
  case DataType::USHORT_E:
    return this->private_->insert_brick_internals<unsigned short>( volume, brick, offset );
  case DataType::SHORT_E:
    return this->private_->insert_brick_internals<short>( volume, brick, offset );
  case DataType::UINT_E:
    return this->private_->insert_brick_internals<unsigned int>( volume, brick, offset );
  case DataType::INT_E:
    return this->private_->insert_brick_internals<int>( volume, brick, offset );
  case DataType::FLOAT_E:
    return this->private_->insert_brick_internals<float>( volume, brick, offset );
  case DataType::DOUBLE_E:
    return this->private_->insert_brick_internals<double>( volume, brick, offset );
  }

  return false;
}


} // end namespace
