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

//Core Includes
#include <Core/Utils/Log.h>
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Volume/DataVolume.h>

namespace Core
{

DataVolume::DataVolume( const GridTransform& grid_transform, 
             const DataBlockHandle& data_block ) :
  Volume( grid_transform ), 
  data_block_( data_block )
{
}

DataVolume::~DataVolume()
{
}

DataBlockHandle DataVolume::get_data_block() const
{
  return this->data_block_;
}

DataType DataVolume::get_data_type() const
{
  if ( this->data_block_ )
  {
    return this->data_block_->get_data_type();
  }
  else
  {
    return DataType::UNKNOWN_E;
  }
}

double DataVolume::get_min() const
{
  if ( this->data_block_ )
  {
    return this->data_block_->get_min();
  }
  else
  {
    return 0.0;
  }
}

double DataVolume::get_max() const
{
  if ( this->data_block_ )
  {
    return this->data_block_->get_max();
  }
  else
  {
    return 0.0;
  }
}

VolumeType DataVolume::get_type() const
{
  return VolumeType::DATA_E;
}

NrrdDataHandle DataVolume::convert_to_nrrd()
{
  if ( this->data_block_ )
  {
    NrrdDataHandle nrrd_data( new NrrdData( this->data_block_, get_grid_transform().transform() ) );
    return nrrd_data;
  }
  else
  {
       NrrdDataHandle handle;
    return handle;
  }
}

DataBlock::generation_type DataVolume::get_generation() const
{ 
  if ( this->data_block_ )
  {
    return this->data_block_->get_generation();
  }
  else
  {
    return -1;
  }
}

DataVolume::mutex_type& DataVolume::get_mutex()
{
  if ( this->data_block_ )
  {
    return this->data_block_->get_mutex();
  }
  else
  {
    return this->invalid_mutex_;
  }
}

bool DataVolume::LoadDataVolume( const boost::filesystem::path& filename, 
                DataVolumeHandle& volume, std::string& error )
{
  volume.reset();
  
  NrrdDataHandle nrrd;
  if ( ! ( NrrdData::LoadNrrd( filename.string(), nrrd, error ) ) ) return false;
  
  DataBlock::generation_type generation;
  Core::ImportFromString( filename.stem(), generation );
  Core::DataBlockHandle datablock( Core::NrrdDataBlock::New( nrrd, generation ) );
  datablock->set_histogram( nrrd->get_histogram( true ) );

  volume = DataVolumeHandle( new DataVolume( nrrd->get_grid_transform(), datablock ) );
  return true;
}

bool DataVolume::SaveDataVolume( const boost::filesystem::path& filepath, DataVolumeHandle& volume, std::string& error )
{
  if( !boost::filesystem::exists( filepath ) )
  {
    NrrdDataHandle nrrd = NrrdDataHandle( new NrrdData( 
      volume->data_block_, volume->get_grid_transform() ) );

    nrrd->set_histogram( volume->data_block_->get_histogram() );
    if ( ! ( NrrdData::SaveNrrd( filepath.string(), nrrd, error ) ) ) 
    {
      CORE_LOG_ERROR( error );
      return false;
    }
  }
  
  return true;
}

bool DataVolume::CreateEmptyData( GridTransform grid_transform, 
  DataType data_type, DataVolumeHandle& data )
{
  DataBlockHandle data_block = StdDataBlock::New( grid_transform, data_type ) ;
  data_block->clear();
  
  data = DataVolumeHandle( new DataVolume( grid_transform, data_block ) );
  return true;
}

bool DataVolume::CreateInvalidData( GridTransform grid_transform, DataVolumeHandle& data )
{
  DataBlockHandle data_block;

  data = DataVolumeHandle( new DataVolume( grid_transform, data_block ) );
  return true;
}

void DataVolume::ConvertToCanonicalVolume( const DataVolumeHandle& src_volume, 
                      DataVolumeHandle& dst_volume )
{
  const GridTransform& src_transform = src_volume->get_grid_transform();
  std::vector< int > permutation;
  GridTransform dst_transform;
  GridTransform::AlignToCanonicalCoordinates( src_transform, permutation, dst_transform );
  DataBlockHandle src_data_block = src_volume->get_data_block();
  DataBlockHandle dst_data_block;
  assert( permutation.size() == 3 );
  if ( permutation[ 0 ] != 1 || permutation[ 1 ] != 2 || permutation[ 2 ] != 3)
  {
    DataBlock::PermuteData( src_data_block, dst_data_block, permutation );
  }
  else
  {
    dst_data_block = src_data_block;
  }

  dst_volume.reset( new DataVolume( dst_transform, dst_data_block ) );
}


} // end namespace Core
