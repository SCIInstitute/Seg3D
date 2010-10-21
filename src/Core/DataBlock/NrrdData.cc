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
 
// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Exception.h>
#include <Core/Math/MathFunctions.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/DataBlock/NrrdData.h>

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


NrrdData::NrrdData( DataBlockHandle data_block, Transform transform ) :
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

    set_transform( transform );
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

Transform NrrdData::get_transform() const
{
  Transform transform;
  transform.load_identity();

  if ( this->private_->nrrd_ == 0 ) return transform;

  size_t dim = this->private_->nrrd_->dim;

  // Ensure min and max are of the proper size and are initialized
  std::vector< double > min( dim, 0.0 ), max( dim, 0.0 );
  std::vector< size_t > size( dim, 0 );

  // Extract info of oldest versions of nrrds
  for ( size_t p = 0; p < dim; p++ )
  {
    size[ p ] = this->private_->nrrd_->axis[ p ].size;

    if ( IsFinite( this->private_->nrrd_->axis[ p ].min ) )
    {
      min[ p ] = this->private_->nrrd_->axis[ p ].min;
    }
    else
    {
      min[ p ] = 0.0;
    }

    if ( IsFinite( this->private_->nrrd_->axis[ p ].spacing ) )
    {
      max[ p ] = this->private_->nrrd_->axis[ p ].spacing * size[ p ];
    }
    else
    {
      if ( IsFinite( this->private_->nrrd_->axis[ p ].max ) )
      {
        max[ p ] = this->private_->nrrd_->axis[ p ].max;
      }
      else
      {
        max[ p ] = static_cast< double > ( size[ p ] - 1 );
      }
    }
  }

  // Remove empty dimensions
  size_t k = 0;
  for ( size_t p = 0; p < dim; p++ )
  {
    if ( size[ p ] == 1 ) continue;
    k++;
  }
  size_t rdim = k;

  std::vector< double > rmin( rdim ), rmax( rdim );
  std::vector< int > rsize( rdim );

  k = 0;
  for ( size_t p = 0; p < dim; p++ )
  {
    if ( size[ p ] == 1 ) continue;
    rmin[ k ] = min[ p ];
    rmax[ k ] = max[ p ];
    rsize[ k ] = static_cast<int>( size[ p ] );
    k++;
  }

  if ( rdim == 1 )
  {
    Vector v0, v1, v2;
    v0 = Point( rmax[ 0 ], 0.0, 0.0 ) - Point( rmin[ 0 ], 0.0, 0.0 );
    v0.find_orthogonal( v1, v2 );

    transform.load_basis( Point( rmin[ 0 ], 0.0, 0.0 ), v0, v1, v2 );
    transform.post_scale( Vector( 1.0 / static_cast< double > ( rsize[ 0 ] - 1 ), 1.0, 1.0 ) );
  }
  else if ( rdim == 2 )
  {
    Vector v0, v1, v2;
    v0 = Point( rmax[ 0 ], 0.0, 0.0 ) - Point( rmin[ 0 ], 0.0, 0.0 );
    v1 = Point( 0.0, rmax[ 1 ], 0.0 ) - Point( 0.0, rmin[ 1 ], 0.0 );
    v2 = Cross( v0, v1 );
    v2.normalize();

    transform.load_basis( Point( rmin[ 0 ], rmin[ 1 ], 0.0 ), v0, v1, v2 );
    transform.post_scale( Vector( 1.0 / static_cast< double > ( rsize[ 0 ] - 1 ), 1.0
        / static_cast< double > ( rsize[ 1 ] - 1 ), 1.0 ) );
  }
  else if ( rdim > 2 )
  {
    Vector v0, v1, v2;
    v0 = Point( rmax[ 0 ], 0.0, 0.0 ) - Point( rmin[ 0 ], 0.0, 0.0 );
    v1 = Point( 0.0, rmax[ 1 ], 0.0 ) - Point( 0.0, rmin[ 1 ], 0.0 );
    v2 = Point( 0.0, 0.0, rmax[ 2 ] ) - Point( 0.0, 0.0, rmin[ 2 ] );

    transform.load_basis( Point( rmin[ 0 ], rmin[ 1 ], rmin[ 2 ] ), v0, v1, v2 );
    transform.post_scale( Vector( 1.0 / static_cast< double > ( rsize[ 0 ] - 1 ), 1.0
        / static_cast< double > ( rsize[ 1 ] - 1 ), 1.0 / static_cast< double > ( rsize[ 2 ] - 1 ) ) );
  }

  if ( this->private_->nrrd_->spaceDim > 0 )
  {
    Vector Origin;
    std::vector< Vector > SpaceDir( 3 );

    for ( size_t p = 0; p < rdim; p++ )
    {
      rmin[ p ] = 0.0;
      rmax[ p ] = static_cast< double > ( rsize[ p ] );
//      if ( nrrd_->axis[ p ].center == nrrdCenterCell )
//      {
//        double cor = ( rmax[ p ] - rmin[ p ] ) / ( 2 * rsize[ p ] );
//        rmin[ p ] -= cor;
//        rmax[ p ] -= cor;
//      }
    }

    if ( this->private_->nrrd_->spaceDim > 0 )
    {
      if ( IsFinite( this->private_->nrrd_->spaceOrigin[ 0 ] ) )
      {
        Origin.x( this->private_->nrrd_->spaceOrigin[ 0 ] );
      }

      for ( size_t p = 0; p < rdim && p < 3; p++ )
      {
        if ( IsFinite( this->private_->nrrd_->axis[ p ].spaceDirection[ 0 ] ) )
        {
          SpaceDir[ p ].x( this->private_->nrrd_->axis[ p ].spaceDirection[ 0 ] );
        }
      }
    }

    if ( this->private_->nrrd_->spaceDim > 1 )
    {
      if ( IsFinite( this->private_->nrrd_->spaceOrigin[ 1 ] ) )
      {
        Origin.y( this->private_->nrrd_->spaceOrigin[ 1 ] );
      }
      for ( size_t p = 0; p < rdim && p < 3; p++ )
      {
        if ( IsFinite( this->private_->nrrd_->axis[ p ].spaceDirection[ 1 ] ) )
        {
          SpaceDir[ p ].y( this->private_->nrrd_->axis[ p ].spaceDirection[ 1 ] );
        }
      }
    }

    if ( this->private_->nrrd_->spaceDim > 2 )
    {
      if ( IsFinite( this->private_->nrrd_->spaceOrigin[ 2 ] ) )
      {
        Origin.z( this->private_->nrrd_->spaceOrigin[ 2 ] );
      }
      for ( size_t p = 0; p < rdim && p < 3; p++ )
      {
        if ( IsFinite( this->private_->nrrd_->axis[ p ].spaceDirection[ 2 ] ) )
        {
          SpaceDir[ p ].z( this->private_->nrrd_->axis[ p ].spaceDirection[ 2 ] );
        }
      }
    }

    if ( dim == 1 )
    {
      SpaceDir[ 0 ].find_orthogonal( SpaceDir[ 1 ], SpaceDir[ 2 ] );
    }
    else if ( dim == 2 )
    {
      SpaceDir[ 2 ] = Cross( SpaceDir[ 0 ], SpaceDir[ 1 ] );
    }

    Transform space_transform;
    space_transform.load_basis( Point( Origin ), SpaceDir[ 0 ], SpaceDir[ 1 ], SpaceDir[ 2 ] );
    transform.pre_transform( space_transform );
  }

  return transform;
}


GridTransform NrrdData::get_grid_transform() const
{
  Core::GridTransform grid_transform( get_nx(), get_ny(), get_nz(), get_transform() );
  return grid_transform;
}

void NrrdData::set_transform( Transform& transform )
{
  if ( !this->private_->nrrd_ ) return;
  int centerdata[ NRRD_DIM_MAX ];
  for ( int p = 0; p < NRRD_DIM_MAX; p++ )
  {
    centerdata[ p ] = nrrdCenterCell;
  }

  nrrdAxisInfoSet_nva( this->private_->nrrd_, nrrdAxisInfoCenter, centerdata );
  int kind[ NRRD_DIM_MAX ];
  for ( int p = 0; p < NRRD_DIM_MAX; p++ )
  {
    kind[ p ] = nrrdKindSpace;
  }
  nrrdAxisInfoSet_nva( this->private_->nrrd_, nrrdAxisInfoKind, kind );

  this->private_->nrrd_->spaceDim = 3;

  double Trans[ 16 ];

  transform.get( Trans );
  for ( int p = 0; p < 3; p++ )
  {
    this->private_->nrrd_->spaceOrigin[ p ] = Trans[ 12 + p ];
    for ( size_t q = 0; q < this->private_->nrrd_->dim; q++ )
    {
      this->private_->nrrd_->axis[ q ].spaceDirection[ p ] = Trans[ q + 4 * p ];
    }
  }

  for ( int p = 0; p < 3; p++ )
    for ( int q = 0; q < 3; q++ )
    {
      if ( p == q ) this->private_->nrrd_->measurementFrame[ p ][ q ] = 1.0;
      else this->private_->nrrd_->measurementFrame[ p ][ q ] = 0.0;
    }

  this->private_->nrrd_->space = nrrdSpace3DRightHanded;

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

bool NrrdData::LoadNrrd( const std::string& filename, NrrdDataHandle& nrrddata, std::string& error )
{
  // Lock down the Teem library
  lock_type lock( GetMutex() );

  Nrrd* nrrd = nrrdNew();
  if ( nrrdLoad( nrrd, filename.c_str(), 0 ) )
  {
    char *err = biffGet( NRRD );
    error = std::string( "Could not open file: " ) + filename + " : " + std::string( err );
    free( err );
    biffDone( NRRD );

    nrrddata.reset();
    return false;
  }

  error = "";
  nrrddata = NrrdDataHandle( new NrrdData( nrrd ) );
  return true;
}

bool NrrdData::SaveNrrd( const std::string& filename, NrrdDataHandle nrrddata, std::string& error, 
  bool compress, int level )
{
  // Lock down the Teem library
  lock_type lock( GetMutex() );
  // TODO:
  // Fix filename handling so it always ends in .nrrd or .nhdr

  if ( !( nrrddata.get() ) )
  {
    error = "Error writing file: " + filename + " : no data volume available";
    return false;
  }

  // Check whether the NRRD0005 format is needed for export, if not downgrade nrrd
  Nrrd* nrrd = nrrddata->nrrd();

  if ( nrrd->spaceDim > 0 && nrrd->spaceDim == nrrd->dim )
  {
    bool is_aligned_nrrd = true;
    for ( size_t j = 0; j < static_cast<size_t>( nrrd->spaceDim ); j++ )
    {
      for ( size_t i = 0; i < static_cast<size_t>( nrrd->dim ); i++ )
      {
        if ( i != j && ( nrrd->axis[ i ].spaceDirection[ j ] != 0.0 &&
          ! IsNan( nrrd->axis[ i ].spaceDirection[ j ] ) ) )
        {
          is_aligned_nrrd = false;
        }
      }
    }

    for ( size_t j = 0; j < static_cast<size_t>( nrrd->spaceDim ); j++ )
    {
      for ( size_t i = 0; i < static_cast<size_t>( nrrd->spaceDim ); i++ )
      {
        if ( i != j && ( nrrd->measurementFrame[ i ][ j ] != 0.0 &&
          ! IsNan( nrrd->measurementFrame[ i ][ j ] ) ) )
        {
          is_aligned_nrrd = false;
        }
      }
    }

    if ( is_aligned_nrrd )
    {
      // Down grade nrrd
      nrrdOrientationReduce( nrrd, nrrd, 1 );
      for ( size_t i = 0; i < static_cast<size_t>( nrrd->dim ); i++ )
      {
        nrrd->axis[ i ].kind = nrrdKindUnknown; 
      }
    }
  }

  NrrdIoState* nio = nrrdIoStateNew();
  nrrdIoStateSet( nio,  nrrdIoStateZlibLevel, level );

  // Turn on compression if the user wants it.
  if( compress )
  { 
    nrrdIoStateEncodingSet( nio, nrrdEncodingGzip );
  }
  
  if ( nrrdSave( filename.c_str(), nrrd, nio ) )
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
