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

#include <Core/DataBlock/DataBlockManager.h>

namespace Core 
{

CORE_SINGLETON_IMPLEMENTATION( DataBlockManager );
  
class DataBlockManagerPrivate
{
public:
  typedef std::map< DataBlock::generation_type, DataBlockWeakHandle > generation_map_type;
  generation_map_type generation_map_;
  
  DataBlock::generation_type generation_;
};
  
DataBlockManager::DataBlockManager() :
  private_( new DataBlockManagerPrivate )
{
  this->private_->generation_ = 0;
}
  
void DataBlockManager::register_datablock( DataBlockHandle data_block, 
  DataBlock::generation_type generation )
{
  lock_type lock( this->get_mutex() );

  assert( data_block->get_generation() == -1 );

  if ( generation == -1 )
  {
    generation = this->private_->generation_; 
  }
  else if ( this->private_->generation_ < generation )
  {
    this->private_->generation_ = generation;
  }

  data_block->set_generation( generation );
  this->private_->generation_++;
  
  assert( this->private_->generation_map_.count( data_block->get_generation() ) == 0 );
  this->private_->generation_map_[ data_block->get_generation() ] = data_block;
}

bool DataBlockManager::find_datablock(  DataBlock::generation_type generation, 
  DataBlockHandle& datablock )
{
  lock_type lock( this->get_mutex() );

  datablock.reset();
  DataBlockManagerPrivate::generation_map_type::const_iterator it = 
    this->private_->generation_map_.find( generation );
  if ( it == this->private_->generation_map_.end() ) return false;

  datablock = ( *it ).second.lock();
  
  if ( datablock ) return true;
  return false;
}

void DataBlockManager::unregister_datablock( DataBlock::generation_type generation )
{
  lock_type lock( this->get_mutex() );
  DataBlockHandle datablock;
  if ( this->find_datablock( generation, datablock ) )
  {
    datablock->set_generation( -1 );
  }
  
  this->private_->generation_map_.erase( generation );
}

DataBlock::generation_type DataBlockManager::increase_generation( 
  DataBlock::generation_type old_generation )
{
  lock_type lock( this->get_mutex() );

  // Only increase the generation number if the old generation number is valid
  DataBlockHandle data_block;
  if ( this->find_datablock( old_generation, data_block ) )
  {
    this->private_->generation_map_.erase( old_generation );
    DataBlock::generation_type new_generation = this->private_->generation_++;
    this->private_->generation_map_[ new_generation ] = data_block;
    return new_generation;
  }
  
  return -1;
}

DataBlock::generation_type DataBlockManager::get_generation_count()
{
  lock_type lock( this->get_mutex() );

  return this->private_->generation_;
}

void DataBlockManager::set_generation_count( DataBlock::generation_type generation )
{
  lock_type lock( this->get_mutex() );

  this->private_->generation_ = generation;
}

void DataBlockManager::clear()
{
  lock_type lock( this->get_mutex() );
  DataBlockManagerPrivate::generation_map_type::iterator it =
    this->private_->generation_map_.begin();
  while ( it != this->private_->generation_map_.end() )
  {
    DataBlockHandle datablock = ( *it ).second.lock();
    if ( datablock )
    {
      datablock->set_generation( -1 );
    }
    ++it;
  }
  this->private_->generation_map_.clear();  
  this->private_->generation_ = 0;
}

} // end namespace Core
