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
#include <Utils/Volume/Volume.h>

// Application includes
#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>
#include <Application/Action/Action.h>
#include <Application/State/State.h>


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
typedef std::list< LayerHandle > layer_list_type;


// Class definition
class Layer : public StateHandler
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
  virtual Utils::VolumeType type() const = 0;

  // GRID_TRANSFORM
  // Get the transform of the layer
  virtual const Utils::GridTransform& get_grid_transform() const = 0;
  
  // -- State variables --
public:
  
  // The name of the layer
  StateAliasHandle name_state_;

  // Per viewer state of whether this layer is visible
  std::vector< StateBoolHandle > visible_state_;

  // State indicating whether the layer is locked
  StateBoolHandle lock_state_;

  // State that describes the opacity with which the layer is displayed
  StateRangedDoubleHandle opacity_state_;

  // State of the checkbox that records which layer needs to be processed in
  // the group
  StateBoolHandle selected_state_;

  // State that describes which menu is currently shown
  StateOptionHandle edit_mode_state_;
  
  // The name for the layer that is defined by the user or by the imported file
  StateStringHandle user_defined_name_state_;
  
  // State that describes which fill mode is active
  StateOptionHandle fill_mode_state_;
  
  // State that describes which border mode is active
  StateOptionHandle border_mode_state_;

  // -- Locking system --
public:
  // This code just aligns the layer locking with the StateEngine locking
  typedef StateEngine::mutex_type mutex_type;
  typedef StateEngine::lock_type lock_type;


  LayerGroupHandle get_layer_group() 
  { 
    return layer_group_.lock(); 
  }
  
  inline void set_layer_group( LayerGroupWeakHandle layer_group )
  {
    layer_group_ = layer_group;
  }
  
  std::string get_layer_id() const
  {
    return stateid();
  }
  
private:  
  // The unique ID of the layer

  LayerGroupWeakHandle layer_group_;

  // GETMUTEX
public:
  // Get the mutex of the state engine
  static mutex_type& GetMutex()
  {
    return StateEngine::GetMutex();
  }
  
};

} // end namespace Seg3D

#endif
