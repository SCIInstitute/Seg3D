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

#include <list>
#include <queue>

#include <boost/unordered_map.hpp>

#include <Core/Application/Application.h>
#include <Core/Utils/ConnectionHandler.h>
#include <Core/DataBlock/DataBlock.h>
#include <Core/LargeVolume/LargeVolumeCache.h>

namespace Core
{
CORE_SINGLETON_IMPLEMENTATION( LargeVolumeCache );

class LargeVolumeCachePrivate : ConnectionHandler, Lockable, public EventHandler
{
  typedef std::list<std::string> cache_access_list_type;

  struct CacheEntry
  {
    DataBlockHandle data_block_;
    cache_access_list_type::iterator access_record_;
  };

  struct LoadJob
  {
    LoadJob( LargeVolumeSchemaHandle schema, BrickInfo bi ) :
      schema_( schema ), bi_( bi )
    {
    };

    LargeVolumeSchemaHandle schema_;
    BrickInfo bi_;
  };

  typedef boost::unordered_map<std::string, CacheEntry> cache_map_type;

public:
  long long cache_capacity_;
  long long cache_size_;
  cache_access_list_type cache_access_list_;
  cache_map_type cache_map_;

  LargeVolumeCache* instance_;

  boost::unordered_map<std::string,std::queue<LoadJob> > jobs_;

  LargeVolumeCachePrivate()
  {
    if (sizeof( void * ) == 4)
    {
      // For 32bit systems, do not exceed 1 GB of data usage.
      // On Windows, addressable space is 2 GB, hence this leaves enough space for the program itself.
      this->cache_capacity_ = static_cast<long long>( 1 ) << 30;
    }
    else
    {
      // For 64bit systems, try to get a lot of space.
      // Get total memory size and subtract 2 GB (for running program and operating system).
      long long mem_size = Core::Application::Instance()->get_total_physical_memory();
      mem_size -= static_cast<long long>( 2 ) << 30;
      mem_size = static_cast<long long>( 0.5 * mem_size );

      // If less than 1 GB, use 1 GB.
      if (mem_size < ( static_cast<long long>( 1 ) << 30 )) mem_size = static_cast<long long>( 1 ) << 30;

      // Do not use more than 32 GB, unless explicity requested.
      this->cache_capacity_ = Core::Min( static_cast<long long>( 32 ) << 30, mem_size );
    }

    this->cache_size_ = 0;

    this->add_connection( Application::Instance()->reset_signal_.connect(
      boost::bind( &LargeVolumeCachePrivate::clear_cache, this ) ) );
  }

  ~LargeVolumeCachePrivate()
  {
    this->disconnect_all();
  }

  void add_entry(const std::string& brick_name, DataBlockHandle data_block)
  {
    lock_type lock( this->get_mutex() );

    this->cache_access_list_.push_front( brick_name );
    this->cache_size_ += data_block->get_byte_size();

    CacheEntry entry;
    entry.data_block_ = data_block;
    entry.access_record_ = this->cache_access_list_.begin();
    this->cache_map_[ brick_name ] = entry;

    this->constraint_cache_size();
  }

  void constraint_cache_size()
  {
    while (this->cache_size_ > this->cache_capacity_)
    {
      std::string brick_name = this->cache_access_list_.back();
      cache_map_type::iterator it = this->cache_map_.find( brick_name );
      this->cache_size_ -= it->second.data_block_->get_byte_size();
      this->cache_access_list_.pop_back();
      this->cache_map_.erase( it );
    }
  }

  bool get_entry( const std::string& brick_name, DataBlockHandle& data_block )
  {
    lock_type lock( this->get_mutex() );

    cache_map_type::iterator it = this->cache_map_.find( brick_name );
    if (it == this->cache_map_.end()) 
      return false;

    this->cache_access_list_.erase( it->second.access_record_ );
    this->cache_access_list_.push_front( brick_name );
    it->second.access_record_ = this->cache_access_list_.begin();
    data_block = it->second.data_block_;

    return true;
  }

  void clear_cache()
  {
    lock_type lock( this->get_mutex() );

    this->cache_access_list_.clear();
    this->cache_map_.clear();
    this->cache_size_ = 0;
  }

  void load_brick( LargeVolumeSchemaHandle schema, BrickInfo bi, std::string load_key )
  {
    this->jobs_[ load_key ].push( LoadJob( schema, bi ) );

    this->post_event( boost::bind( &LargeVolumeCachePrivate::process_load_job, this, load_key ) );
  }

  void process_load_job( std::string load_key )
  {
    if (! this->jobs_[ load_key ].empty() )
    {
      LoadJob lj = this->jobs_[ load_key ].front();
      this->jobs_[ load_key ].pop();

      std::string brick_name = lj.schema_->get_brick_file_name( lj.bi_ ).string();
      DataBlockHandle data_block;
      if (! this->get_entry( brick_name, data_block )) 
      {
        std::string error;
        lj.schema_->read_brick( data_block, lj.bi_, error );
        this->add_entry( brick_name, data_block );
        this->instance_->brick_loaded_signal_();
      }
    }

    if (! this->jobs_[ load_key ].empty() )
    {
      this->post_event( boost::bind( &LargeVolumeCachePrivate::process_load_job, this, load_key ) );  
    }
  }

  void clear_load_queue( std::string load_key )
  {
    this->jobs_[ load_key ] = std::queue<LoadJob>();
  }
};

LargeVolumeCache::LargeVolumeCache() : private_( new LargeVolumeCachePrivate )
{
  this->private_->instance_ = this;
  this->private_->start_eventhandler();
}

LargeVolumeCache::~LargeVolumeCache()
{

}

bool LargeVolumeCache::mark_brick( LargeVolumeSchemaHandle schema, const BrickInfo& bi )
{
  std::string brick_name = schema->get_brick_file_name( bi ).string();

  DataBlockHandle data_block;
  return this->private_->get_entry( brick_name, data_block );
}

bool LargeVolumeCache::get_brick( LargeVolumeSchemaHandle schema, const BrickInfo& bi, 
  const std::string& load_key, DataBlockHandle& data_block )
{
  std::string brick_name = schema->get_brick_file_name( bi ).string();

  if (this->private_->get_entry( brick_name, data_block ))
  {
    return true;
  }

  this->private_->post_event( boost::bind( &LargeVolumeCachePrivate::load_brick, this->private_, schema, bi, load_key ) );

  return false;
}

void LargeVolumeCache::load_brick( LargeVolumeSchemaHandle schema, const BrickInfo& bi, const std::string& load_key )
{
  DataBlockHandle data_block;
  this->get_brick( schema, bi, load_key, data_block ); 
}

void LargeVolumeCache::clear_load_queue( const std::string& load_key )
{
  this->private_->post_event( boost::bind( &LargeVolumeCachePrivate::clear_load_queue, this->private_, load_key ) );
}

} // end namespace
