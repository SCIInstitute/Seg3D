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

//Core Includes
#include <Core/Math/MathFunctions.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>
#include <Core/Volume/LargeVolume.h>
#include <Core/DataBlock/DataBlockManager.h>
#include <Core/Geometry/BBox.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class LargeVolumePrivate
//////////////////////////////////////////////////////////////////////////

class LargeVolumePrivate : public SharedLockable
{
public:
  LargeVolume* volume_;
  LargeVolumeSchemaHandle lv_schema_;
  bool is_cropped_;
};


//////////////////////////////////////////////////////////////////////////
// Class LargeVolume
//////////////////////////////////////////////////////////////////////////

LargeVolume::LargeVolume( const LargeVolumeSchemaHandle& lv_schema ) :
  Volume( lv_schema->get_grid_transform() ), 
  private_( new LargeVolumePrivate )
{
  this->private_->volume_ = this;
  this->private_->lv_schema_ = lv_schema;
  this->private_->is_cropped_ = false;
}

LargeVolume::LargeVolume( const LargeVolumeSchemaHandle& lv_schema, const GridTransform& cropped_trans ) :
  Volume( cropped_trans ),
  private_( new LargeVolumePrivate )
{
  this->private_->volume_ = this;
  this->private_->lv_schema_ = lv_schema;
  this->private_->is_cropped_ = false;
}

LargeVolume::~LargeVolume()
{
}

DataType LargeVolume::get_data_type() const
{
  if ( this->private_->lv_schema_ )
  {
    return this->private_->lv_schema_->get_data_type();
  }
  else
  {
    return DataType::UNKNOWN_E;
  }
}

double LargeVolume::get_min() const
{
  if (this->private_->lv_schema_)
  {
    return this->private_->lv_schema_->get_min();
  }
  else
  {
    return 0.0;
  }
}

double LargeVolume::get_max() const
{
  if (this->private_->lv_schema_)
  {
    return this->private_->lv_schema_->get_max();
  }
  else
  {
    return 0.0;
  }
}

double LargeVolume::get_cum_value( double fraction ) const
{
  return 0.0;
}

VolumeType LargeVolume::get_type() const
{
  return VolumeType::LARGE_DATA_E;
}

NrrdDataHandle LargeVolume::convert_to_nrrd()
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolume does not implement convert_to_nrrd.");
}

DataBlock::generation_type LargeVolume::get_generation() const
{ 
  return -1;
}

bool LargeVolume::is_valid() const
{
  return (bool)this->private_->lv_schema_;
}

LargeVolume::mutex_type& LargeVolume::get_mutex()
{
  return this->private_->get_mutex();
}

DataBlock::generation_type LargeVolume::register_data( DataBlock::generation_type generation )
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolume does not implement register_data.");
}

void LargeVolume::unregister_data()
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolume does not implement unregister_data.");
}

size_t LargeVolume::get_byte_size() const
{
  CORE_THROW_NOTIMPLEMENTEDERROR("LargeVolume does not implement get_byte_size.");
}

LargeVolumeSchemaHandle LargeVolume::get_schema()
{
  return this->private_->lv_schema_;
}

bool LargeVolume::is_cropped() const
{
  return this->private_->is_cropped_;
}

} // end namespace Core
