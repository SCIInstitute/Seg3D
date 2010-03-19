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

#ifndef UTILS_VOLUME_MASKVOLUMESLICE_H
#define UTILS_VOLUME_MASKVOLUMESLICE_H

#include <Utils/Graphics/PixelBufferObject.h>
#include <Utils/Volume/MaskVolume.h>
#include <Utils/Volume/VolumeSlice.h>

namespace Utils
{

class MaskVolumeSlice;
typedef boost::shared_ptr< MaskVolumeSlice > MaskVolumeSliceHandle;

class MaskVolumeSlice : public VolumeSlice
{
public:
  MaskVolumeSlice( const MaskVolumeHandle& mask_volume, 
    VolumeSliceType type = VolumeSliceType::AXIAL_E, size_t slice_num = 0 );
  MaskVolumeSlice( const MaskVolumeSlice& copy );
  virtual ~MaskVolumeSlice() {}

  inline bool get_mask_at( size_t i, size_t j ) const
  {
    return this->mask_data_block_->get_mask_at( this->to_index( i, j ) );
  }

  void set_mask_at( size_t i, size_t j )
  {
    this->mask_data_block_->set_mask_at( this->to_index( i, j ) );
  }

  void clear_mask_at( size_t i, size_t j )
  {
    this->mask_data_block_->clear_mask_at( this->to_index( i, j ) );
  }

  // Create the texture object
  virtual void initialize_texture();

  // Upload the mask slice to graphics texture.
  // NOTE: This function allocates resources on the GPU, so the caller should
  // acquire a lock on the RenderResources before calling this function. 
  virtual void upload_texture();

private:
  // Pointer to the mask data block. The base class keeps a handle of the volume,
  // so it's safe to use a pointer here.
  MaskDataBlock* mask_data_block_;

  PixelBufferObjectHandle pixel_buffer_;

};

} // end namespace Utils

#endif