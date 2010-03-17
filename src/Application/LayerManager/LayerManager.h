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

#ifndef APPLICATION_LAYERMANAGER_LAYERMANAGER_H
#define APPLICATION_LAYERMANAGER_LAYERMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 



// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/StringUtil.h>
#include <Utils/Singleton/Singleton.h>
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerScene.h>
#include <Application/State/StateHandler.h>

namespace Seg3D
{

// Forward declaration
class LayerManager;

// Class definition
class LayerManager : public StateHandler, public Utils::Singleton< LayerManager >
{

  // -- Constructor/Destructor --

private:
  friend class Utils::Singleton< LayerManager >;
  LayerManager();
  virtual ~LayerManager();

  
public:
  typedef std::list < LayerGroupHandle > group_handle_list_type;
  group_handle_list_type group_handle_list_;
  LayerHandle active_layer_;
  void return_group_vector( std::vector< LayerGroupHandle > &vector_of_groups );
  LayerGroupHandle check_for_group( std::string group_id );
  
  
public:
  bool insert_layer( LayerHandle layer );
  bool insert_layer( LayerGroupHandle group );
  
  void insert_layer_top( LayerHandle layer );
  void set_active_layer( LayerHandle layer );
  LayerHandle get_active_layer();
  LayerGroupWeakHandle get_active_group();
  void delete_layer( LayerHandle layer );
  
  friend class ActionInsertLayer;

public:
  // Take an atomic snapshot of visual properties of layers for rendering in the specified viewer
  LayerSceneHandle compose_layer_scene( size_t viewer_id );
  
public: 
  typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock< mutex_type > lock_type;
  
  // GET_MUTEX:
  // Get the mutex, so it can be locked by the interface that is built
  // on top of this
  mutex_type& get_mutex();

public:
  // -- Signal/Slots --
  typedef boost::signals2::signal< void( LayerHandle ) > layer_signal_type;
  typedef boost::signals2::signal< void( LayerGroupHandle ) > group_signal_type;
  
  // ACTIVE_LAYER_CHANGED_SIGNAL:
  // This signal is triggered after the active layer is changed
  layer_signal_type active_layer_changed_signal_; 
  
  // LAYER_INSERTED_SIGNAL:
  // This signal is triggered after a layer has been inserted
  layer_signal_type layer_inserted_signal_;
  
  layer_signal_type layer_deleted_signal_;
  
  // This signal is triggered when the groups layers have changed
  group_signal_type group_layers_changed_signal_;
  
  //group_signal_type layer_inserted_signal_;
  group_signal_type add_group_signal_;
  group_signal_type delete_group_signal_;
  
private:
  
  // mutex for the group_handle_list
  LayerGroupHandle create_group( const Utils::GridTransform& ) const;
  mutex_type group_handle_list_mutex_; 
    
};

} // end namespace seg3D

#endif

