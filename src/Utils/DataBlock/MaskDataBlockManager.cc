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

#include <Utils/DataBlock/MaskDataBlockManager.h>

namespace Utils {

MaskDataBlockManager::MaskDataBlockManager()
{
}

MaskDataBlockManager::~MaskDataBlockManager()
{
}

bool 
MaskDataBlockManager::create(size_t nx, size_t ny, size_t nz, 
                             MaskDataBlockHandle& mask)
{
  lock_type lock(get_mutex());
  
  DataBlockHandle data_block;
  unsigned int mask_bit = 0;
  size_t mask_entry_index = 0;
  
  for (size_t j=0;j<mask_list_.size();++)
  {
    // Find an empty location
    if ( (nx == mask_list_[j].data_block_->nx()) &&
         (ny == mask_list_[j].data_block_->ny()) &&
         (nz == mask_list_[j].data_block_->nz()) &&
         (mask_list_[j].bits_used_.count() != 8))
    {
      data_block = mask_list_[j].data_block_;
      mask_entry_index = j;
      
      for (size_t k = 0; k < 8; k++)
      {
        if (!(mask_list_[j].test(k)))
        {
          mask_bit = k;
          // drop out of for loop
          break;
        }
      }
      // drop out of for loop
      break;
    }
  }
  
  if (!(data_block.get()))
  {
    // Could not find empty position, so create a new data block
    data_block = new StdDataBlock(nx,ny,nz,DataBlock::UCHAR_E);
    mask_bit = 0;
    mask_entry_index = mask_list_.size();
    mask_list_.push_back(MaskDataBlockEntry(data_block));
  }
  
  // Generate the new mask
  mask = MaskDataBlockHandle(new MaskDataBlock(data_block, mask_bit));

  // Mark the bitplane as being used before returning the mask
  mask_list_[mask_entry_index].bits_used_[mask_bit]  = 1;
  mask_list_[mask_entry_index].data_masks_[mask_bit] = mask;
  
  return true;
}

void
MaskDataBlockManager::release(DataBlockHandle& datablock, unsigned int mask_bit)
{
  lock_type lock(get_mutex());
  
  // Remove the MaskDataBlock from the list
  for (size_t j=0;j<mask_list_.size();++)
  {      
    if ( mask_list_[j].data_block_ == datablock )
    {
      mask_list_[j].bits_used_[mask_bit] = 0;
      mask_list_[j].data_masks_[mask_bit].reset();
     
      // If the DataBlock is not used any more clear it
      if (mask_list_[j].bits_used_.count() == 0)
      {
        mask_list_.erase( mask_list_.begin() + j);
      }
    
      break;
    }
  }
}

bool
MaskDataBlockManager::compact()
{
  // TODO: Need to implement this
}

} // end namespace Utils
