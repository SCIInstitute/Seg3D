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

#ifndef CORE_VOLUME_DATAVOLUME_H
#define CORE_VOLUME_DATAVOLUME_H

#include <boost/filesystem.hpp>

#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/NrrdData.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/Volume/Volume.h>

namespace Core
{

class DataVolume;
typedef boost::shared_ptr< DataVolume > DataVolumeHandle;
typedef boost::weak_ptr< DataVolume > DataVolumeWeakHandle;

class DataVolume : public Volume
{
public:
  DataVolume( const GridTransform& grid_transform, const DataBlockHandle& data_block );
  virtual ~DataVolume();

  // GET_TYPE:
  // Get the type of the data volume
  virtual VolumeType get_type() const;

  // DATA_BLOCK:
  // Get the data block that contains the volume data
  DataBlockHandle get_data_block() const;

  // GET_DATA_TYPE:
  // Get the data type of the data in this volume
  DataType get_data_type() const;

  // GET_MUTEX:
  // Get access to the mutex protecting this DataVolume
  virtual mutex_type& get_mutex();

  // CONVERT_TO_NRRD:
  // Get the volume data as a nrrd wrapped in a NrrdData structure
  virtual NrrdDataHandle convert_to_nrrd();

  // GET_GENERATION:
  // Get the  generation number of the data volume
  virtual DataBlock::generation_type get_generation() const;

private:
  // Handle to where the volume data is really stored
  DataBlockHandle data_block_;

  // Mutex for a volume without a data block associated with it
  // NOTE: This is used to set up a new layer that is still constructing its data
  mutex_type invalid_mutex_;

  // -- functions for creating DataVolumes --
public:

  // LOADDATAVOLUME:
  // Load a DataVolume from a nrrd file
  static bool LoadDataVolume( const boost::filesystem::path& filename, DataVolumeHandle& volume,
    std::string& error );

  // SAVEDATAVOLUME:
  // Save a DataVolume to a nrrd file
  static bool SaveDataVolume( const boost::filesystem::path& filepath, DataVolumeHandle& volume, 
    std::string& error );

  // CREATEEMPTYDATA:
  // Create an empty data volume with given dimensions.
  static bool CreateEmptyData( GridTransform grid_transform, DataType data_type, 
    DataVolumeHandle& data );

  // CREATEINVALIDDATA:
  // Create a data volume with given dimensions, but no data container associated with it.
  static bool CreateInvalidData( GridTransform grid_transform, DataVolumeHandle& data );

};

} // end namespace Core

#endif