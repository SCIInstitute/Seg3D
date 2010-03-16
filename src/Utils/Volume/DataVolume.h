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

#ifndef UTILS_VOLUME_DATAVOLUME_H
#define UTILS_VOLUME_DATAVOLUME_H

#include <Utils/DataBlock/DataBlock.h>
#include <Utils/DataBlock/NrrdData.h>
#include <Utils/Geometry/GridTransform.h>
#include <Utils/Volume/Volume.h>

namespace Utils
{

class DataVolume;
typedef boost::shared_ptr< DataVolume > DataVolumeHandle;
typedef boost::weak_ptr< DataVolume > DataVolumeWeakHandle;

class DataVolume : public Volume
{
public:
  DataVolume( const GridTransform& grid_transform, const DataBlockHandle& data_block );
  virtual ~DataVolume();

  virtual VolumeType type() const
  {
    return VolumeType::DATA_E;
  }

  // DATA_BLOCK:
  // Get the data block that contains the volume data
  DataBlockHandle data_block()
  {
    return this->data_block_;
  }

  virtual mutex_type& get_mutex()
  {
    return this->data_block_->get_mutex();
  }

private:
  // Handle to where the volume data is really stored
  DataBlockHandle data_block_;

};

} // end namespace Utils

#endif