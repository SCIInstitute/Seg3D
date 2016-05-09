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

#ifndef CORE_VOLUME_LARGEVOLUME_H
#define CORE_VOLUME_LARGEVOLUME_H


#include <Core/Geometry/GridTransform.h>
#include <Core/Volume/Volume.h>
#include <Core/LargeVolume/LargeVolumeSchema.h>

namespace Core
{

class LargeVolume;
typedef boost::shared_ptr< LargeVolume > LargeVolumeHandle;

class LargeVolumePrivate;
typedef boost::shared_ptr< LargeVolumePrivate > LargeVolumePrivateHandle;

class LargeVolume : public Volume
{
public:
  LargeVolume( const LargeVolumeSchemaHandle& lv_schema );
  LargeVolume( const LargeVolumeSchemaHandle& lv_schema, const GridTransform& cropped_trans );
  virtual ~LargeVolume();

  // GET_TYPE:
  /// Get the type of the data volume
  virtual VolumeType get_type() const;

  // GET_MIN:
  /// Get the minimum value
  virtual double get_min() const;

  // GET_MAX:
  /// Get the maximum value
  virtual double get_max() const;

  // GET_CUM_VALUE:
  /// Get the value at a specific point in the histogram
  virtual double get_cum_value( double fraction ) const;

  // IS_VALID:
  /// Check whether the volume has a valid data block
  virtual bool is_valid() const;

  // GET_DATA_TYPE:
  /// Get the data type of the data in this volume
  DataType get_data_type() const;

  // GET_MUTEX:
  /// Get access to the mutex protecting this LargeVolume
  virtual mutex_type& get_mutex();

  // CONVERT_TO_NRRD:
  /// Get the volume data as a nrrd wrapped in a NrrdData structure
  virtual NrrdDataHandle convert_to_nrrd();

  // GET_GENERATION:
  /// Get the  generation number of the data volume
  virtual DataBlock::generation_type get_generation() const;

  // REGISTER_DATA:
  /// Register the underlying data with the DataBlockManager.
  virtual DataBlock::generation_type register_data( DataBlock::generation_type generation = -1 );

  // UNREGISTER_DATA:
  /// Unregister the underlying data with DataBlockManager.
  virtual void unregister_data();

  // GET_BYTE_SIZE:
  /// Get the size of the data in bytes
  virtual size_t get_byte_size() const;

  // GET_SCHEMA:
  /// Get the underlying large volume schema.
  LargeVolumeSchemaHandle get_schema();

  /// Whether to crop the underlying large volume
  bool is_cropped() const;
    
private:
  LargeVolumePrivateHandle private_;  
};

} // end namespace Core

#endif
