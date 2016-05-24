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

#ifndef CORE_LARGEVOLUME_LARGEVOLUMECACHE_H
#define CORE_LARGEVOLUME_LARGEVOLUMECACHE_H

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <Core/LargeVolume/LargeVolumeSchema.h>
#include <Core/Utils/Singleton.h>

namespace Core
{

class LargeVolumeCache;
typedef boost::shared_ptr< LargeVolumeCache > LargeVolumeCacheHandle;

class LargeVolumeCachePrivate;
typedef boost::shared_ptr< LargeVolumeCachePrivate > LargeVolumeCachePrivateHandle;

class LargeVolumeCache
{
  CORE_SINGLETON( LargeVolumeCache );
  // -- constructor --
private:
  LargeVolumeCache();
  ~LargeVolumeCache();

  // -- header --
public:
  bool mark_brick( LargeVolumeSchemaHandle schema, const BrickInfo& bi );

  bool get_brick( LargeVolumeSchemaHandle schema, const BrickInfo& bi, 
    const std::string& load_key, DataBlockHandle& data_block );

  void load_brick( LargeVolumeSchemaHandle schema, const BrickInfo& bi, 
    const std::string& load_key );

  void clear_load_queue( const std::string& load_key );

  boost::signals2::signal<void()> brick_loaded_signal_;

private:
  LargeVolumeCachePrivateHandle private_;
};

} // end namespace Core

#endif
