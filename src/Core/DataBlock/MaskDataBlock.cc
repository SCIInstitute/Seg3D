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

#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

namespace Core
{

MaskDataBlock::MaskDataBlock( DataBlockHandle data_block, unsigned int mask_bit ) :
  nx_( data_block->get_nx() ),
  ny_( data_block->get_ny() ),
  nz_( data_block->get_nz() ),
  data_block_( data_block ),
  mask_bit_( mask_bit ),
  mask_value_( 1 << mask_bit ),
  not_mask_value_( ~( 1 << mask_bit ) ) 
{
  this->data_ = reinterpret_cast<unsigned char*>( this->data_block_->get_data() );
}

MaskDataBlock::~MaskDataBlock()
{
  MaskDataBlockManager::Instance()->release( data_block_, mask_bit_ );
}

DataBlockHandle MaskDataBlock::get_data_block()
{
  return this->data_block_;
}

DataBlock::generation_type MaskDataBlock::get_generation() const
{
  return  this->data_block_->get_generation();
}

void MaskDataBlock::increase_generation()
{
  this->data_block_->increase_generation();
}

bool MaskDataBlock::extract_slice( SliceType type, 
  index_type index, MaskDataSliceHandle& slice  )
{
  size_t nx = this->get_nx();
  size_t ny = this->get_ny();
  size_t nz = this->get_nz();
    
  MaskDataBlockHandle slice_mask_data_block;
  
  slice.reset();
  
  switch( type )
  {
    // SAGITTAL SLICE
    case SliceType::SAGITTAL_E:
    {
      if ( index < 0 || index >= static_cast<index_type>( nx ) ) return false;
    
      if ( !MaskDataBlockManager::Instance()->create( GridTransform( 1, ny, nz ),
        slice_mask_data_block ) )
      {
        return false;
      }

      MaskDataBlock::lock_type lock( slice_mask_data_block->get_mutex() );
      MaskDataBlock::shared_lock_type slock;

      if ( this->get_data_block() != slice_mask_data_block->get_data_block() )
      {
        // Need a read lock for the slice mask
        shared_lock_type read_lock( this->get_mutex() );
        slock.swap( read_lock );
      }

      unsigned char volume_mask_value = this->get_mask_value();         
      unsigned char slice_mask_value = slice_mask_data_block->get_mask_value();         
      unsigned char* volume_ptr = this->get_mask_data();
      unsigned char* slice_ptr = slice_mask_data_block->get_mask_data();
      
      size_t nxy = nx * ny;
      size_t ny8 = RemoveRemainder8( ny );

      for ( size_t z = 0; z < nz; z++ )
      {
        size_t y = 0;
        for ( ; y < ny8; y += 8 )
        {
          size_t a = y + z * ny; 
          size_t b = index + y * nx + z * nxy;
          
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b+= nx;            
        }
        for ( ; y < ny; y++ )
        {
          size_t a = y + z * ny; 
          size_t b = index + y * nx + z * nxy;
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;           
        }
      }
      
      slice = MaskDataSliceHandle( new MaskDataSlice( slice_mask_data_block, type, index ) );
      return true;
    }
    // CORONAL SLICE
    case SliceType::CORONAL_E:
    {
      if ( index < 0 || index >= static_cast<index_type>( ny ) ) return false;
    
      if ( !MaskDataBlockManager::Instance()->create( GridTransform( nx, 1, nz ),
        slice_mask_data_block ) )
      {
        return false;
      }

      MaskDataBlock::lock_type lock( slice_mask_data_block->get_mutex() );
      MaskDataBlock::shared_lock_type slock;

      if ( this->get_data_block() != slice_mask_data_block->get_data_block() )
      {
        // Need a read lock for the slice mask
        shared_lock_type read_lock( this->get_mutex() );
        slock.swap( read_lock );
      }


      unsigned char volume_mask_value = this->get_mask_value();         
      unsigned char slice_mask_value = slice_mask_data_block->get_mask_value();         
      unsigned char* volume_ptr = this->get_mask_data();
      unsigned char* slice_ptr = slice_mask_data_block->get_mask_data();
      
      size_t nxy = nx * ny;
      size_t nx8 = RemoveRemainder8( nx );

      for ( size_t z = 0; z < nz; z++ )
      {
        size_t x = 0;
        for ( ; x < nx8; x += 8 )
        {
          size_t a = x + z * nx; 
          size_t b = x + index * nx + z * nxy;
          
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
        }
        for ( ; x < nx; x++ )
        {
          size_t a = x + z * nx; 
          size_t b = x + index * nx + z * nxy;
          
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
        }
      }

      slice = MaskDataSliceHandle( new MaskDataSlice( slice_mask_data_block, type, index ) );
      return true;
    }
    // AXIAL SLICE
    case SliceType::AXIAL_E:
    {
      if ( index < 0 || index >= static_cast<index_type>( nz ) ) return false;
    
      if ( !MaskDataBlockManager::Instance()->create( GridTransform( nx, ny, 1 ),
        slice_mask_data_block ) )
      {
        return false;
      }

      MaskDataBlock::lock_type lock( slice_mask_data_block->get_mutex() );
      MaskDataBlock::shared_lock_type slock;

      if ( this->get_data_block() != slice_mask_data_block->get_data_block() )
      {
        // Need a read lock for the slice mask
        shared_lock_type read_lock( this->get_mutex() );
        slock.swap( read_lock );
      }

      unsigned char volume_mask_value = this->get_mask_value();         
      unsigned char slice_mask_value = slice_mask_data_block->get_mask_value();         
      unsigned char* volume_ptr = this->get_mask_data();
      unsigned char* slice_ptr = slice_mask_data_block->get_mask_data();
      
      size_t nxy = nx * ny;
      size_t nx8 = RemoveRemainder8( nx );

      for ( size_t y = 0; y < ny; y++ )
      {
        size_t x = 0;
        for ( ; x < nx8; x += 8 )
        {
          size_t a = x + y * nx; 
          size_t b = x + y * nx + index * nxy;
          
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;           
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;
          a++; b++;
        }
        for ( ; x < nx; x++ )
        {
          size_t a = x + y * nx; 
          size_t b = x + y * nx + index * nxy;
          
          if (  volume_ptr[ b ] & volume_mask_value ) slice_ptr[ a ] |= slice_mask_value;         
        }
      }

      slice = MaskDataSliceHandle( new MaskDataSlice( slice_mask_data_block, type, index ) );
      return true;
    }
    default:
    {
      return false;
    }
  }
}

bool MaskDataBlock::insert_slice( const MaskDataSliceHandle slice )
{
  // Check whether the mask datablock pointers are valid
  if ( !slice ) return false;

  MaskDataBlockHandle slice_mask_data_block = slice->get_mask_data_block();

  // Get the dimensions
  size_t nx = this->get_nx();
  size_t ny = this->get_ny();
  size_t nz = this->get_nz();
    

  // Need a write lock for the destination mask
  lock_type lock( this->get_mutex() );
  shared_lock_type slock;

  if ( this->get_data_block() != slice_mask_data_block->get_data_block() )
  {
    // Need a read lock for the slice mask
    shared_lock_type read_lock( slice_mask_data_block->get_mutex() );
    slock.swap( read_lock );
  }
  
  index_type index = slice->get_index();

  // For each orientation we have an optimized implementation 
  switch( slice->get_slice_type() )
  {
    // SAGITTAL SLICE
    case SliceType::SAGITTAL_E:
    {
      // Check range of the slice numbers
      if ( index < 0 || index >= static_cast<index_type>( nx ) ) return false;
    
      // Get the values of the bits that are set in both masks, so that one does not need
      // a shift operator each time. 
      unsigned char volume_mask_value = this->get_mask_value();         
      unsigned char slice_mask_value = slice_mask_data_block->get_mask_value();         
      unsigned char volume_not_mask_value = ~( this->get_mask_value() );
      
      // Get the direct pointers to the data
      unsigned char* volume_ptr = this->get_mask_data();
      unsigned char* slice_ptr = slice_mask_data_block->get_mask_data();
      
      size_t nxy = nx * ny;

      // Setup loop unrolement
      size_t ny8 = RemoveRemainder8( ny );

      for ( size_t z = 0; z < nz; z++ )
      {
        // Loop unrolement
        size_t y = 0;
        for ( ; y < ny8; y += 8 )
        {
          size_t a = y + z * ny; 
          size_t b = index + y * nx + z * nxy;
          
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b+= nx;           
        }
        // Finish loop unrolement
        for ( ; y < ny; y++ )
        {
          size_t a = y + z * ny; 
          size_t b = index + y * nx + z * nxy;
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value;          
        }
      }

      // Generate a new generation number for the new volume
      this->increase_generation();

      return true;
    }
    // CORONAL SLICE
    case SliceType::CORONAL_E:
    {
      // Check range of the slice numbers
      if ( index < 0 || index >= static_cast<index_type>( ny ) ) return false;
    
      // Get the values of the bits that are set in both masks, so that one does not need
      // a shift operator each time. 
      unsigned char volume_mask_value = this->get_mask_value();         
      unsigned char slice_mask_value = slice_mask_data_block->get_mask_value();         
      unsigned char volume_not_mask_value = ~( this->get_mask_value() );  
      
      // Get the direct pointers to the data
      unsigned char* volume_ptr = this->get_mask_data();
      unsigned char* slice_ptr = slice_mask_data_block->get_mask_data();
      
      size_t nxy = nx * ny;
      // Setup loop unrolement
      size_t nx8 = RemoveRemainder8( nx );

      for ( size_t z = 0; z < nz; z++ )
      {
        size_t x = 0;
        // Loop unrolement
        for ( ; x < nx8; x += 8 )
        {
          // Copy data back into the mask
          size_t a = x + z * nx; 
          size_t b = x + index * nx + z * nxy;
          
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
        }
        // Finish loop unrolement
        for ( ; x < nx; x++ )
        {
          size_t a = x + z * nx; 
          size_t b = x + index * nx + z * nxy;
          
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value;            
        }
      }

      // Generate a new generation number for the new volume
      this->increase_generation();

      return true;
    }
    // AXIAL SLICE
    case SliceType::AXIAL_E:
    {
      // Check range of the slice numbers
      if ( index < 0 || index >= static_cast<index_type>( nz ) ) return false;
    
      // Get the values of the bits that are set in both masks, so that one does not need
      // a shift operator each time. 
      unsigned char volume_mask_value = this->get_mask_value();         
      unsigned char slice_mask_value = slice_mask_data_block->get_mask_value();         
      unsigned char volume_not_mask_value = ~( this->get_mask_value() );          

      // Get the direct pointers to the data
      unsigned char* volume_ptr = this->get_mask_data();
      unsigned char* slice_ptr = slice_mask_data_block->get_mask_data();
      
      size_t nxy = nx * ny;

      // Setup loop unrolement
      size_t nx8 = RemoveRemainder8( nx );

      for ( size_t y = 0; y < ny; y++ )
      {
        // Loop unrolement
        size_t x = 0;
        for ( ; x < nx8; x += 8 )
        {
          size_t a = x + y * nx; 
          size_t b = x + y * nx + index * nxy;
          
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
        }
        // Finish loop unrolement
        for ( ; x < nx; x++ )
        {
          size_t a = x + y * nx; 
          size_t b = x + y * nx + index * nxy;
          
          if (  slice_ptr[ a ] & slice_mask_value ) volume_ptr[ b ] |= volume_mask_value;
          else volume_ptr[ b ] &= volume_not_mask_value; a++; b++;            
        }
      }
      
      // Generate a new generation number for the new volume
      this->increase_generation();

      return true;
    }
    default:
    {
      return false;
    }
  }
}

} // end namespace Core
