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

#include <Core/Volume/MaskVolume.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/DataBlockManager.h>

namespace Core
{

MaskVolume::MaskVolume( const GridTransform& grid_transform, 
  const MaskDataBlockHandle& mask_data_block ) :
  Volume( grid_transform ),
  mask_data_block_( mask_data_block )
{
}

MaskVolume::~MaskVolume()
{
}

VolumeType MaskVolume::get_type() const
{
  return VolumeType::MASK_E;
}

double MaskVolume::get_min() const
{
  return 0.0;
}

double MaskVolume::get_max() const
{
  return 1.0;
}

DataBlock::generation_type MaskVolume::get_generation() const
{
  if ( this->mask_data_block_ )
  {
    return this->mask_data_block_->get_data_block()->get_generation();
  }
  else
  {
    return -1;
  }
}

MaskVolume::mutex_type& MaskVolume::get_mutex()
{
  if ( this->mask_data_block_ )
  {
    return this->mask_data_block_->get_mutex();
  }
  else
  {
    return this->invalid_mutex_;
  }
}

bool MaskVolume::is_valid() const
{
  return this->mask_data_block_.get() != 0;
}

MaskDataBlockHandle MaskVolume::get_mask_data_block() const
{
  return this->mask_data_block_;
}

DataBlock::generation_type MaskVolume::register_data( DataBlock::generation_type generation )
{
  if ( this->mask_data_block_ )
  {
    if ( this->mask_data_block_->get_data_block()->get_generation() != -1 )
    {
      this->mask_data_block_->increase_generation();
    }
    else
    {
      Core::DataBlockManager::Instance()->register_datablock( this->mask_data_block_->get_data_block() );
    }

    return this->mask_data_block_->get_generation();
  }
  
  return -1;
}

void MaskVolume::unregister_data()
{
  // Do nothing
}

size_t MaskVolume::get_byte_size() const
{
  return this->get_mask_data_block()->get_byte_size();
}


bool MaskVolume::CreateEmptyMask( GridTransform grid_transform, MaskVolumeHandle& mask )
{
  MaskDataBlockHandle mask_data_block;
    MaskDataBlockManager::Create( grid_transform, mask_data_block );
  
  mask = MaskVolumeHandle( new MaskVolume( grid_transform, mask_data_block ) );

  if ( !mask ) return false;
  return true;
}

bool MaskVolume::CreateInvalidMask( GridTransform grid_transform, MaskVolumeHandle& mask )
{
  MaskDataBlockHandle mask_data_block;

  mask = MaskVolumeHandle( new MaskVolume( grid_transform, mask_data_block ) );

  if ( !mask ) return false;
  return true;
}

} // end namespace Core
