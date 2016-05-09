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
 
#ifndef APPLICATION_LAYER_LAYERCHECKPOINT_H 
#define APPLICATION_LAYER_LAYERCHECKPOINT_H 

// Boost includes
#include <boost/smart_ptr.hpp> 
#include <boost/utility.hpp> 
 
// Core includes
#include <Core/Volume/VolumeSlice.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>

namespace Seg3D
{

class LayerCheckPoint;
class LayerCheckPointPrivate;

typedef boost::shared_ptr<LayerCheckPoint> LayerCheckPointHandle;
typedef boost::shared_ptr<LayerCheckPointPrivate> LayerCheckPointPrivateHandle;

class LayerCheckPoint : public boost::noncopyable
{
  // -- constructor / destructor -- 
public:
  /// Create a volume check point
  LayerCheckPoint( LayerHandle layer );

  /// Create a slice check point
  LayerCheckPoint( LayerHandle layer, Core::SliceType type,
    Core::DataBlock::index_type index );

  /// Create a slice check point
  LayerCheckPoint( LayerHandle layer, Core::SliceType type,
    Core::DataBlock::index_type start, Core::DataBlock::index_type end );

  // destructor
  virtual ~LayerCheckPoint();
  
  // -- retrieving check points --
public:
  /// APPLY:
  /// Applies a check point to a layer
  bool apply( LayerHandle layer ) const;
  
  // -- making check points --
public:
  /// CREATE_VOLUME:
  /// Check point the full volume
  bool create_volume( LayerHandle layer );
  
  /// CREATE_SLICE:
  /// Check point a slice check point
  bool create_slice( LayerHandle layer, Core::SliceType type,
    Core::DataBlock::index_type index );

  /// CREATE_SLICE:
  /// Check point a slice check point
  bool create_slice( LayerHandle layer, Core::SliceType type,
    Core::DataBlock::index_type start, Core::DataBlock::index_type end );
  
  /// GET_BYTE_SIZE:
  /// Get the size of the check point
  size_t get_byte_size() const;
  
        // -- internals --
private:
  LayerCheckPointPrivateHandle private_;  
};

} // end namespace Seg3D

#endif
