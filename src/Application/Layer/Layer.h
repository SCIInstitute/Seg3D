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

// Core includes
#include <Core/Action/Action.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/State/State.h>
#include <Core/Volume/Volume.h>

// Application includes
#include <Application/Layer/LayerFWD.h>

namespace Seg3D
{

// CLASS Layer
// This is the main class for collecting state information on a layer

// Class definition
class Layer : public Core::StateHandler
{

  // -- Constructor/destructor --
protected:
  // NOTE: Use the derived class to build the layer
  Layer( const std::string& name );
  Layer( const std::string& name, const std::string& state_id );

  virtual ~Layer();

private:
  // INITIALIZE_STATES:
  // This function sets the initial states of the layer, and returns true if successful
  void initialize_states( const std::string& name );

  // -- Layer properties --
public:

  // TYPE
  // Get the type of the layer
  virtual Core::VolumeType type() const = 0;

  // GRID_TRANSFORM
  // Get the transform of the layer
  virtual const Core::GridTransform get_grid_transform() const = 0;

  // -- layer progress signals --
public:
  typedef boost::signals2::signal< void ( bool ) > lock_signal_type;
  
  typedef boost::signals2::signal< void (double) > update_progress_signal_type;

  // DATA_LOCK_SIGNAL:
  // This signal is triggered when the data contained in the layer is locked from being
  // modified (bool = true), or when the data is unlocked (bool = false). This also means 
  // that the layer cannot be used in any filter.
  lock_signal_type data_lock_signal_;

  // UPDATE_PROGRESS:
  // When new information on progress is available this signal is triggered. If this signal is 
  // triggered it should end with a value 1.0 indicating that progress reporting has finised.
  // Progress is measured between 0.0 and 1.0.
  update_progress_signal_type update_progress_signal_;

  // -- layer updated signal --
public:
  typedef boost::signals2::signal< void () > layer_updated_signal_type;
  // LAYER_UPDATED_SIGNAL:
  // Triggered when the layer data has been modified.
  layer_updated_signal_type layer_updated_signal_;
  
  // -- layer filter abort support --
public:
  // RAISE_ABORT:
  // Abort the filter that is working on this layer
  void raise_abort();
  
  // CHECK_ABORT:
  // Check whether the abort flag was raised, it was reset by start_progress
  bool check_abort();
  
private:
  bool abort_;
  
  // -- State variables --
public:
  
  // The name of the layer
  Core::StateNameHandle name_state_;

  // Per viewer state of whether this layer is visible
  std::vector< Core::StateBoolHandle > visible_state_;

  // State indicating whether the layer is locked
  Core::StateBoolHandle visual_lock_state_;

  // State that describes the opacity with which the layer is displayed
  Core::StateRangedDoubleHandle opacity_state_;

  // State of the checkbox that records which layer needs to be processed in
  // the group
  Core::StateBoolHandle selected_state_;

  // State that describes which menu is currently shown
  Core::StateOptionHandle menu_state_;

  // State that stores the current layer state
  Core::StateOptionHandle data_state_;

protected:
  // State that stores the generation of its datablock
  Core::StateLongLongHandle generation_state_;
  
  // State that stores the last action that was played
  Core::StateStringHandle last_action_state_;

  // -- Accessors --
public:

  // GET_LAYER_GROUP:
  // Get a handle to the group this layer belongs to
  LayerGroupHandle get_layer_group();
  
  // SET_LAYER_GROUP:
  // Set the group this layer is contained within
  void set_layer_group( LayerGroupWeakHandle layer_group );
  
  // GET_LAYER_ID:
  // Get the id of this layer
  std::string get_layer_id() const;

  // GET_LAYER_NAME:
  // Get the name of the layer
  std::string get_layer_name() const;
  
  // GET_GENERATION:
  // Get the generation of the current data block
  Core::DataBlock::generation_type get_generation() const;
  
protected:
  virtual bool post_save_states( Core::StateIO& state_io );

private:  
  // Handle to the layer group (this one needs to be weak to ensure objects are not persistent
  // due to a circular dependency)
  LayerGroupWeakHandle layer_group_;

  // -- Locking system --
public:
  // This code just aligns the layer locking with the StateEngine locking
  typedef Core::StateEngine::mutex_type mutex_type;
  typedef Core::StateEngine::lock_type lock_type;
  
  // GETMUTEX:
  // Get the mutex of the state engine
  static mutex_type& GetMutex();

public:

  // Options for the state of the data inside of the layer
  const static std::string CREATING_C;
  const static std::string PROCESSING_C;
  const static std::string AVAILABLE_C;
  const static std::string IN_USE_C;
  
  // Options for the state of the menus that are open in the layer manager
  const static std::string NO_MENU_C;
  const static std::string OPACITY_MENU_C;
  const static std::string COLOR_MENU_C;
  const static std::string CONTRAST_MENU_C;
  const static std::string APPEARANCE_MENU_C;
};

} // end namespace Seg3D

#endif
