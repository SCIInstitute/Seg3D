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

#ifndef UTILS_VOLUME_MASKVOLUME_H
#define UTILS_VOLUME_MASKVOLUME_H

#include <Utils/DataBlock/MaskDataBlock.h>
#include <Utils/DataBlock/NrrdData.h>
#include <Utils/Volume/Volume.h>

namespace Utils
{

class MaskVolume;
typedef boost::shared_ptr< MaskVolume > MaskVolumeHandle;

class MaskVolume : public Volume
{
public:
  MaskVolume( const GridTransform& grid_transform, const MaskDataBlockHandle& mask_data_block );
  virtual ~MaskVolume() {}

  virtual VolumeType type() const
  {
    return VolumeType::MASK_E;
  }

  // MASK_DATA_BLOCK:
  // Get the datablock that contains the mask
  MaskDataBlockHandle mask_data_block()
  {
    return this->mask_data_block_;
  }

  virtual mutex_type& get_mutex()
  {
    return this->mask_data_block_->get_mutex();
  }

private:
  // Handle to where the mask volume is really stored
  MaskDataBlockHandle mask_data_block_;

public:
  // CREATEMASKVOLUMEFROMNRRD:
  // Create a data volume from a nrrd
  static VolumeHandle CreateMaskVolumeFromNrrd( NrrdDataHandle& nrrddata );
};

} // end namespace Utils

#endif