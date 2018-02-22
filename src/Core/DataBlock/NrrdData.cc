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

#include <locale>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Exception.h>
#include <Core/Math/MathFunctions.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/DataBlock/NrrdData.h>

// Boost includes
#include <boost/filesystem.hpp>

namespace Core
{

class NrrdDataPrivate
{
public:
  // Location where the original nrrd is stored
  Nrrd* nrrd_;

  // Do we need to clear the nrrd when done
  bool own_data_;

  // Location of the data block from which this nrrd was derived
  DataBlockHandle data_block_;
};


NrrdData::NrrdData( Nrrd* nrrd, bool own_data ) :
  private_( new  NrrdDataPrivate )
{
  this->private_->nrrd_ = nrrd;
  this->private_->own_data_ = own_data;
}

NrrdData::NrrdData( DataBlockHandle data_block ) :
  private_( new  NrrdDataPrivate )
{
  if ( !data_block )
  {
    CORE_THROW_LOGICERROR( "Cannot create NrrdData from empty object" );
  }

  this->private_->own_data_ = false;
  this->private_->data_block_ = data_block;
  this->private_->nrrd_ = nrrdNew();

  if ( this->private_->nrrd_ )
  {
    nrrdWrap_va( this->private_->nrrd_, data_block->get_data(),
      GetNrrdDataType( data_block->get_data_type() ), 3,
      data_block->get_nx(), data_block->get_ny(),
      data_block->get_nz() );
  }
}


NrrdData::NrrdData( DataBlockHandle data_block, GridTransform transform, bool no_downgrade ) :
  private_( new  NrrdDataPrivate )
{
  if ( !data_block )
  {
    CORE_THROW_LOGICERROR( "Cannot create NrrdData from empty object" );
  }

  this->private_->own_data_ = false;
  this->private_->data_block_ = data_block;
  this->private_->nrrd_ = nrrdNew();

  if ( this->private_->nrrd_ )
  {
    nrrdWrap_va( this->private_->nrrd_, data_block->get_data(),
      GetNrrdDataType( data_block->get_data_type() ), 3,
      data_block->get_nx(), data_block->get_ny(),
      data_block->get_nz() );

    set_transform( transform, no_downgrade );
  }
}

NrrdData::~NrrdData()
{
  // If we own the data, clear the nrrd
  if ( this->private_->own_data_ )
  {
    nrrdNuke( this->private_->nrrd_ );
  }
  else
  {
    nrrdNix( this->private_->nrrd_ );
  }

  this->private_->data_block_.reset();
}

Nrrd* NrrdData::nrrd() const
{
  return this->private_->nrrd_;
}

void* NrrdData::get_data() const
{
  if ( this->private_->nrrd_ ) return this->private_->nrrd_->data;
  else return 0;
}

bool NrrdData::own_data() const
{
  return this->private_->own_data_;
}

// Represents grid transform using Teem's concept of space origin and space direction.  This
// representation is independent of the cell- versus node-centering of the data
// (http://teem.sourceforge.net/nrrd/format.html#spaceorigin).  However, the the
// cell- versus node-centering of the data does affect the values used for space
// origin and space direction in the case where axis mins and/or axis maxs are used
// (http://teem.sourceforge.net/nrrd/format.html#centers)
Transform NrrdData::get_transform() const
{
  Transform transform;
  transform.load_identity();

  // Step 1: Find domain axes
  // Note: nrrdDomainAxesGet is based on the per-axis "kind" field.  If this field is not set then
  // the axis is assumed to be a domain axis, even if it is in fact a 3-component color value,
  // for example.
  unsigned int axis_idx_array[ NRRD_DIM_MAX ];
  unsigned int axis_idx_num = nrrdDomainAxesGet( this->private_->nrrd_, axis_idx_array );
  // We assume at most 3 axes in the following code
  const unsigned int max_dim = 3;
  axis_idx_num = Min( axis_idx_num, max_dim );
  unsigned int space_dim = Min( this->private_->nrrd_->spaceDim, max_dim );

  // Step 2: Calculate space directions for all axes
  std::vector< Vector > space_directions; // Space direction per axis
  space_directions.push_back( Vector( 1, 0, 0 ) );
  space_directions.push_back( Vector( 0, 1, 0 ) );
  space_directions.push_back( Vector( 0, 0, 1 ) );

  if( space_dim > 0 ) // We have the space direction info already
  {

    // For space direction for each axis
    for( size_t axis_lookup = 0; axis_lookup < axis_idx_num; axis_lookup++ )
    {
      for( size_t space_dir_idx = 0; space_dir_idx < space_dim; space_dir_idx++ )
      {
        size_t axis_idx = axis_idx_array[ axis_lookup ];
        space_directions[ axis_lookup ][ space_dir_idx ] =
          this->private_->nrrd_->axis[ axis_idx ].spaceDirection[ space_dir_idx ];
      }
    }

    if( axis_idx_num == 2 && space_dim == 3 ) // Special case: 2D nrrd with 3D space dimensions
    {
      // Calculate third space direction as the cross product of the first two
      space_directions[ 2 ] = Cross( space_directions[ 0 ], space_directions[ 1 ] );
    }
  }
  else // If possible, calculate spacing from mins, maxs, spacing
  {
    // For each axis
    for( size_t axis_lookup = 0; axis_lookup < axis_idx_num; axis_lookup++ )
    {
      size_t axis_idx = axis_idx_array[ axis_lookup ];
      NrrdAxisInfo nrrd_axis_info = this->private_->nrrd_->axis[ axis_idx ];
      double spacing = nrrdDefaultSpacing;
      if( IsFinite( nrrd_axis_info.spacing ) ) // Spacing
      {
        spacing = nrrd_axis_info.spacing;
      }
      else if( IsFinite( nrrd_axis_info.min ) && IsFinite( nrrd_axis_info.max ) ) // Min & max
      {
        spacing = NRRD_SPACING( nrrd_axis_info.center, nrrd_axis_info.min,
          nrrd_axis_info.max, nrrd_axis_info.size );
      }
      space_directions[ axis_lookup ] *= spacing;
    }
  }

  // Step 3: Calculate space origin
  Point space_origin( 0, 0, 0 );
  if( space_dim > 0 ) // We have the space origin info already
  {
    for( size_t space_origin_idx = 0; space_origin_idx < space_dim; space_origin_idx++ )
    {
      space_origin[ space_origin_idx ] =
        this->private_->nrrd_->spaceOrigin[ space_origin_idx ];
    }
  }
  else // If possible, calculate space origin from mins, maxs, spacing
  {
    double origin_array[ NRRD_DIM_MAX ];
    unsigned int ret_val = nrrdOriginCalculate( this->private_->nrrd_, axis_idx_array,
      axis_idx_num, nrrdCenterNode, origin_array );
    if( ret_val == nrrdOriginStatusOkay )
    {
      for( size_t axis_lookup = 0; axis_lookup < axis_idx_num; axis_lookup++ )
      {
        space_origin[ axis_lookup ] = origin_array[ axis_lookup ];
      }
    }
    else
    {
      // There is no origin information, so just put min at (0, 0, 0)
      for( size_t axis_lookup = 0; axis_lookup < axis_idx_num; axis_lookup++ )
      {
        size_t axis_idx = axis_idx_array[ axis_lookup ];
        if( this->private_->nrrd_->axis[ axis_idx ].center == nrrdCenterCell )
        {
          // Assume that if no space origin was specified, then no space directions were
          // specified and we are axis-aligned.
          double spacing = space_directions[ axis_lookup ].length();
          space_origin[ axis_lookup ] = spacing / 2.0;
        }
        else // Node-centered or unknown
        {
          space_origin[ axis_lookup ] = 0.0;
        }
      }
    }
  }

  // Step 4: Build transform from space origin and space directions
  transform.load_basis( space_origin, space_directions[ 0 ], space_directions[ 1 ],
    space_directions[ 2 ] );

  return transform;
}


GridTransform NrrdData::get_grid_transform() const
{
  Core::GridTransform grid_transform( this->get_nx(), this->get_ny(), this->get_nz(),
    this->get_transform(), this->get_originally_node_centered() );
  return grid_transform;
}

void NrrdData::set_transform( GridTransform& transform, bool no_downgrade )
{
  if ( !this->private_->nrrd_ ) return;

  // Restore original centering
  int centerdata[ NRRD_DIM_MAX ];
  unsigned int centering =
    transform.get_originally_node_centered() ? nrrdCenterNode : nrrdCenterCell;
  for ( int p = 0; p < NRRD_DIM_MAX; p++ )
  {
    centerdata[ p ] = centering;
  }

  nrrdAxisInfoSet_nva( this->private_->nrrd_, nrrdAxisInfoCenter, centerdata );
  int kind[ NRRD_DIM_MAX ];
  for ( int p = 0; p < NRRD_DIM_MAX; p++ )
  {
    kind[ p ] = nrrdKindSpace;
  }
  nrrdAxisInfoSet_nva( this->private_->nrrd_, nrrdAxisInfoKind, kind );

  // Set space origin and space direction
  this->private_->nrrd_->spaceDim = 3;

  double *transformArray = new double[ Transform::TRANSFORM_LENGTH ];
  transform.get( transformArray );
  for ( int p = 0; p < 3; p++ )
  {
    this->private_->nrrd_->spaceOrigin[ p ] = transformArray[ 12 + p ];
    for ( size_t q = 0; q < this->private_->nrrd_->dim; q++ )
    {
      this->private_->nrrd_->axis[ q ].spaceDirection[ p ] = transformArray[ q + 4 * p ];
    }
  }
  delete [] transformArray;

  for ( int p = 0; p < 3; p++ )
    {
    for ( int q = 0; q < 3; q++ )
    {
      if ( p == q ) this->private_->nrrd_->measurementFrame[ p ][ q ] = 1.0;
      else this->private_->nrrd_->measurementFrame[ p ][ q ] = 0.0;
    }
  }

  this->private_->nrrd_->space = nrrdSpace3DRightHanded;

  // Check whether the NRRD0005 format is needed for export, if not downgrade nrrd.
  // Downgrade nrrd in case where grid transform is axis-aligned.
  // Downgraded nrrd uses axis mins and spacing instead of space origin and space direction.
  Nrrd* nrrd = this->private_->nrrd_;

  if ( nrrd->spaceDim > 0 && nrrd->spaceDim == nrrd->dim )
  {
    bool is_aligned_nrrd = true;
    if ( no_downgrade )
    {
      is_aligned_nrrd = false;
    }
    else
    {
      for ( size_t j = 0; j < static_cast<size_t>( nrrd->spaceDim ); j++ )
      {
        for ( size_t i = 0; i < static_cast<size_t>( nrrd->dim ); i++ )
        {
          if ( (i != j) &&
               ( (nrrd->axis[ i ].spaceDirection[ j ] != 0.0) &&
                 (! IsNan(nrrd->axis[ i ].spaceDirection[ j ])) ) )
          {
            is_aligned_nrrd = false;
          }
        }
      }

      for ( size_t j = 0; j < static_cast<size_t>( nrrd->spaceDim ); j++ )
      {
        for ( size_t i = 0; i < static_cast<size_t>( nrrd->spaceDim ); i++ )
        {
          if ( (i != j) &&
               ( (nrrd->measurementFrame[ i ][ j ] != 0.0) &&
                 (! IsNan( nrrd->measurementFrame[ i ][ j ])) ) )
          {
            is_aligned_nrrd = false;
          }
        }
      }
    }

    if ( is_aligned_nrrd )
    {
      // Down grade nrrd
      nrrdOrientationReduce( nrrd, nrrd, 1 );

      // Get domain axes
      unsigned int axis_idx_array[ NRRD_DIM_MAX ];
      unsigned int axis_idx_num = nrrdDomainAxesGet( nrrd, axis_idx_array );

      // nrrdOrientationReduce doesn't handle centering properly (or at all).  It puts the
      // min at the origin regardless of centering.  If cell-centered, need to adjust min.
      for( size_t axis_lookup = 0; axis_lookup < axis_idx_num; axis_lookup++ )
      {
        size_t axis_idx = axis_idx_array[ axis_lookup ];
        if( nrrd->axis[ axis_idx ].center == nrrdCenterCell )
        {
          if( IsFinite( nrrd->axis[ axis_idx ].min ) &&
            IsFinite( nrrd->axis[ axis_idx ].spacing ) )
          {
            nrrd->axis[ axis_idx ].min -= ( nrrd->axis[ axis_idx ].spacing / 2.0 );
          }
        }
      }

      for ( size_t i = 0; i < static_cast<size_t>( nrrd->dim ); i++ )
      {
        nrrd->axis[ i ].kind = nrrdKindUnknown;
      }
    }
  }
}

void NrrdData::set_histogram( const Histogram& histogram )
{
  if ( this->private_->nrrd_ )
  {
    nrrdKeyValueAdd( this->private_->nrrd_, "seg3d-histogram",
      ExportToString( histogram ).c_str() );
  }
}

Histogram NrrdData::get_histogram( bool trust_meta_data )
{
  Histogram result;
  if ( trust_meta_data )
  {
    char* value = nrrdKeyValueGet( this->private_->nrrd_, "seg3d-histogram" );
    if ( value )
    {
    if ( ImportFromString( value, result ) )
    {
      free( value );
      return result;
    }
    free ( value );
    }
  }

  switch ( this->get_data_type() )
  {
    case Core::DataType::CHAR_E:
      result.compute( reinterpret_cast<signed char *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::UCHAR_E:
      result.compute( reinterpret_cast<unsigned char *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::SHORT_E:
      result.compute( reinterpret_cast<short *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::USHORT_E:
      result.compute( reinterpret_cast<unsigned short *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::INT_E:
      result.compute( reinterpret_cast<int *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::UINT_E:
      result.compute( reinterpret_cast<unsigned int *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::FLOAT_E:
      result.compute( reinterpret_cast<float *>( this->get_data() ), this->get_size() );
      break;
    case Core::DataType::DOUBLE_E:
      result.compute( reinterpret_cast<double *>( this->get_data() ), this->get_size() );
      break;
  }
  return result;
}

size_t NrrdData::get_nx() const
{
  if ( this->private_->nrrd_ && this->private_->nrrd_->dim > 0 )
  {
    return this->private_->nrrd_->axis[ 0 ].size;
  }
  return 1;
}

size_t NrrdData::get_ny() const
{
  if ( this->private_->nrrd_ && this->private_->nrrd_->dim > 1 )
  {
    return this->private_->nrrd_->axis[ 1 ].size;
  }
  return 1;
}

size_t NrrdData::get_nz() const
{
  if ( this->private_->nrrd_ && this->private_->nrrd_->dim > 2 )
  {
    return this->private_->nrrd_->axis[ 2 ].size;
  }
  return 1;
}

size_t NrrdData::get_size() const
{
  return get_nx() * get_ny() * get_nz();
}

DataType NrrdData::get_data_type() const
{
  if ( ! ( this->private_->nrrd_ ) ) return DataType::UNKNOWN_E;
  switch ( this->private_->nrrd_->type )
  {
    case nrrdTypeChar: return DataType::CHAR_E;
    case nrrdTypeUChar: return DataType::UCHAR_E;
    case nrrdTypeShort: return DataType::SHORT_E;
    case nrrdTypeUShort: return DataType::USHORT_E;
    case nrrdTypeInt: return DataType::INT_E;
    case nrrdTypeUInt: return DataType::UINT_E;
    case nrrdTypeLLong: return DataType::LONGLONG_E;
    case nrrdTypeULLong: return DataType::ULONGLONG_E;
    case nrrdTypeFloat: return DataType::FLOAT_E;
    case nrrdTypeDouble: return DataType::DOUBLE_E;
    default: return DataType::UNKNOWN_E;
  }
}

bool NrrdData::get_originally_node_centered() const
{
  if ( this->private_->nrrd_ && this->private_->nrrd_->dim > 0 )
  {
    // unknown = node-centered to match unu resample
    return ( this->private_->nrrd_->axis[ 0 ].center != nrrdCenterCell );
  }
  return true;
}


bool NrrdData::LoadNrrd( const std::string& filename, NrrdDataHandle& nrrddata, std::string& error )
{
  // Lock down the Teem library
  lock_type lock( GetMutex() );

  Nrrd* nrrd = nrrdNew();
    boost::filesystem::path nrrd_path = boost::filesystem::path( filename ).parent_path();
    std::string filename_only = boost::filesystem::path( filename ).filename().string();

    boost::system::error_code ec;
    boost::filesystem::path current_path = boost::filesystem::current_path( ec );
    if ( ec )
    {
    error = std::string( "Could not open file: " ) + filename + " : Could not get current directory.";
        return false;
    }
    boost::filesystem::current_path( nrrd_path, ec );
    if ( ec )
    {
    error = std::string( "Could not open file: " ) + filename + " : Could not access path.";
        return false;
    }

  if ( nrrdLoad( nrrd, filename_only.c_str(), 0 ) )
  {
    char *err = biffGet( NRRD );
    error = std::string( "Could not open file: " ) + filename + " : " + std::string( err );
    free( err );
    biffDone( NRRD );
    nrrdNuke( nrrd );
    nrrddata.reset();
        boost::filesystem::current_path( current_path, ec );
    return false;
  }
    boost::filesystem::current_path( current_path, ec );

  if ( nrrd->dim < 2 )
  {
    error = "Currently only 2D or 3D nrrd files are supported.";
    nrrdNuke( nrrd );
    nrrddata.reset();
    return false;
  }

  if ( nrrd->dim == 2 )
  {
    nrrd->dim = 3;
    nrrd->axis[ 2 ].size = 1;
    nrrd->axis[ 2 ].spacing = 1.0;
    nrrd->axis[ 2 ].min = 0.0;
    nrrd->axis[ 2 ].max = 1.0;
    nrrd->axis[ 2 ].center = nrrd->axis[ 1].center;
    nrrd->axis[ 2 ].kind = nrrd->axis[ 1].kind;
    nrrd->axis[ 2 ].label = 0;
    nrrd->axis[ 2 ].units = 0;

    if ( nrrd->spaceDim == 2 )
    {
      nrrd->spaceDim = 3;
      nrrd->axis[ 0 ].spaceDirection[ 2 ] = 0.0;
      nrrd->axis[ 1 ].spaceDirection[ 2 ] = 0.0;
      nrrd->axis[ 2 ].spaceDirection[ 0 ] = 0.0;
      nrrd->axis[ 2 ].spaceDirection[ 1 ] = 0.0;
      nrrd->axis[ 2 ].spaceDirection[ 2 ] = 1.0;

      nrrd->spaceUnits[ 2 ] = 0;
      nrrd->spaceOrigin[ 2 ] = 0.0;
      nrrd->measurementFrame[ 0 ][ 2 ] = 0.0;
      nrrd->measurementFrame[ 1 ][ 2 ] = 0.0;
      nrrd->measurementFrame[ 2 ][ 2 ] = 0.0;
      nrrd->measurementFrame[ 2 ][ 1 ] = 0.0;
      nrrd->measurementFrame[ 2 ][ 0 ] = 0.0;
    }
    else if ( nrrd->spaceDim == 3 )
    {
      // Build two vectors, take cross product to find third space direction
      Vector space_dir_0( nrrd->axis[ 0 ].spaceDirection[ 0 ],
        nrrd->axis[ 0 ].spaceDirection[ 1 ], nrrd->axis[ 0 ].spaceDirection[ 2 ] );
      Vector space_dir_1( nrrd->axis[ 1 ].spaceDirection[ 0 ],
        nrrd->axis[ 1 ].spaceDirection[ 1 ], nrrd->axis[ 1 ].spaceDirection[ 2 ] );
      Vector space_dir_2 = Cross( space_dir_0, space_dir_1 );

      nrrd->axis[ 2 ].spaceDirection[ 0 ] = space_dir_2.x();
      nrrd->axis[ 2 ].spaceDirection[ 1 ] = space_dir_2.y();
      nrrd->axis[ 2 ].spaceDirection[ 2 ] = space_dir_2.z();
    }
  }

  // Fix a problem with nrrds with stub axes
  // In the old Seg3D this was once fashionable to add a dormant axis that would tell that the
  // data is scalar. Of course none of this made sense, but we need to handle it for case that
  // were saved in the past using this feature.
  if ( nrrd->dim > 3 && nrrd->spaceDim == 0 )
  {
    // Squeeze empty dimensions
    for ( unsigned int j = 0; j < nrrd->dim; j++ )
    {
      if ( nrrd->axis[ j ].size == 1 )
      {
        if ( nrrd->axis[ j ].label )
        {
          free( nrrd->axis[ j ].label );
        }
        if ( nrrd->axis[ j ].units )
        {
          free( nrrd->axis[ j ].units );
        }

        for ( unsigned int k = j + 1; k < nrrd->dim; k++ )
        {
          nrrd->axis[ k - 1 ] = nrrd->axis[ k ];
        }
        nrrd->dim--;
      }
    }
  }

  error = "";
  nrrddata = NrrdDataHandle( new NrrdData( nrrd ) );
  return true;
}

bool NrrdData::SaveNrrd( const std::string& filename,
                         NrrdDataHandle nrrddata,
                         std::string& error,
                         bool compress,
                         int level )
{
  // Lock down the Teem library
  lock_type lock( GetMutex() );

  if ( ! nrrddata.get() )
  {
    error = "Error writing file: " + filename + " : no data volume available";
    return false;
  }

  NrrdIoState* nio = nrrdIoStateNew();

  // Turn on compression if the user wants it.
  if ( compress )
  {
    nrrdIoStateEncodingSet( nio, nrrdEncodingGzip );
    nrrdIoStateSet( nio,  nrrdIoStateZlibLevel, level );
  }
  else
  {
    // guarantees consistent compression settings
    nrrdIoStateSet( nio,  nrrdIoStateZlibLevel, 0 );
  }

  // teem library should check for valid nrrd (including file extension?)
  if ( nrrdSave( filename.c_str(), nrrddata->nrrd(), nio ) )
  {
    char *err = biffGet( NRRD );
    error = "Error writing file: " + filename + " : " + std::string( err );
    free( err );
    biffDone( NRRD );

    return false;
  }

  nio = nrrdIoStateNix( nio );

  error = "";
  return true;
}

NrrdData::mutex_type& NrrdData::GetMutex()
{
  // Mutex protecting Teem calls like nrrdLoad and nrrdSave that are known
  // to be not thread safe

  static mutex_type teem_mutex;
  return teem_mutex;
}

} // end namespace Core
