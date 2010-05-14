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

#ifndef APPLICATION_LAYER_LAYER_H
#define APPLICATION_LAYER_LAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// Volume includes
#include <Core/Action/Action.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Volume/Volume.h>

// Application includes
#include <Core/State/State.h>


namespace Seg3D
{

// CLASS Layer
// This is the main class for collecting state information on a layer

// Forward declarations
  
class LayerGroup;
typedef boost::shared_ptr< LayerGroup > LayerGroupHandle;
typedef boost::weak_ptr< LayerGroup > LayerGroupWeakHandle;
typedef std::list < LayerGroupHandle > group_handle_list_type;
  
class Layer;
typedef boost::shared_ptr< Layer > LayerHandle;
typedef boost::weak_ptr< Layer > LayerWeakHandle;
typedef std::list< LayerHandle > layer_list_type;


// Class definition
class Layer : public Core::StateHandler
{

  // -- constructor/destructor --
protected:
  // NOTE: Use the specific class to build the layer
  Layer( const std::string& name);
  virtual ~Layer();

  // -- Layer properties --
public:

  // TYPE
  // Get the type of the layer
  virtual Core::VolumeType type() const = 0;

  // GRID_TRANSFORM
  // Get the transform of the layer
  virtual const Core::GridTransform& get_grid_transform() const = 0;
  
  // -- State variables --
public:
  
  // The name of the layer
  Core::StateStringHandle name_state_;

  // Per viewer state of whether this layer is visible
  std::vector< Core::StateBoolHandle > visible_state_;

  // State indicating whether the layer is locked
  Core::StateBoolHandle lock_state_;

  // State that describes the opacity with which the layer is displayed
  Core::StateRangedDoubleHandle opacity_state_;

  // State of the checkbox that records which layer needs to be processed in
  // the group
  Core::StateBoolHandle selected_state_;

  // State that describes which menu is currently shown
  Core::StateOptionHandle edit_mode_state_;
  
  // State that describes which fill mode is active
  Core::StateOptionHandle fill_mode_state_;
  
  // State that describes which border mode is active
  Core::StateOptionHandle border_mode_state_;

  // -- Accessors --
public:

  // GET_LAYER_GROUP:
  // Get a handle to the group this layer belongs to
  LayerGroupHandle get_layer_group();
  
  // SET_LAYER_GROUP:
  // Set the group this layer is contained within
  void set_layer_group( LayerGroupWeakHandle layer_group );
  
  // TODO: Need to check whether we really need to store whether layer is active here,
  // as it duplicates state information and allows for synchronization problems
  // --JS
  
  bool get_active();
  void set_active( bool active );
  
  // GET_LAYER_ID:
  // Get the id of this layer
  std::string get_layer_id() const;

  // GET_LAYER_NAME:
  // Get the name of the layer
  std::string get_layer_name() const;
  
private:  
  // The unique ID of the layer

  LayerGroupWeakHandle layer_group_;
  bool active_;

  // -- Locking system --
public:
  // This code just aligns the layer locking with the StateEngine locking
  typedef Core::StateEngine::mutex_type mutex_type;
  typedef Core::StateEngine::lock_type lock_type;
  
  // GETMUTEX:
  // Get the mutex of the state engine
  static mutex_type& GetMutex();
  
private:
  static std::string CreateLayerID();
};

} // end namespace Seg3D

#endif
