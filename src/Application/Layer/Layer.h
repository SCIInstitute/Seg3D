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
#include <Core/State/BooleanStateGroup.h>
#include <Core/Volume/Volume.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/Layer/LayerAbstractFilter.h>

namespace Seg3D
{

class LayerPrivate;
typedef boost::shared_ptr< LayerPrivate > LayerPrivateHandle;

// CLASS Layer
// This is the main class for collecting state information on a layer

// Class definition
class Layer : public Core::StateHandler
{
  friend class LayerPrivate;

  // -- Constructor/destructor --
protected:
  // NOTE: Use the derived class to build the layer
  Layer( const std::string& name, bool creating = false );
  Layer( const std::string& name, const std::string& state_id, bool creating = false );

  virtual ~Layer();

private:
  // INITIALIZE_STATES:
  // This function sets the initial states of the layer, and returns true if successful
  void initialize_states( const std::string& name, bool creating );

  // -- Layer properties --
public:

  // GET_TYPE
  // Get the type of the layer
  virtual Core::VolumeType get_type() const = 0;

  // GET_BYTE_SIZE:
  // Get the size of the data contained in the layer in bytes
  virtual size_t get_byte_size() const = 0;

  // GET_GRID_TRANSFORM:
  // Get the transform of the layer
  virtual Core::GridTransform get_grid_transform() const = 0;

  // GET_DATA_TYPE:
  // Get the data type of the underlying data
  virtual Core::DataType get_data_type() const = 0;

  // GET_VOLUME:
  // Get the underlying volume.
  virtual Core::VolumeHandle get_volume() const = 0;

  // HAS_VALID_DATA:
  // Check whether the layer has valid data
  virtual bool has_valid_data() const = 0;
    
  // DUPLICATE_LAYER:
  // Duplicate layer
  virtual LayerHandle duplicate() const = 0;  
    
  // UPDATE_PROGRESS:
  // Update the progress bar associated with this layer
  void update_progress( double amount, double progress_start = 0.0f, double progress_amount = 1.0f ); 
  
  // IS_VISIBLE:
  // Returns true if the layer is visible in the specified viewer, otherwise false;
  bool is_visible( size_t viewer_id ) const;

  // -- layer progress signals --
public:

  // UPDATE_PROGRESS_SIGNAL:
  // When new information on progress is available this signal is triggered. If this signal is 
  // triggered it should end with a value 1.0 indicating that progress reporting has finished.
  // Progress is measured between 0.0 and 1.0.
  typedef boost::signals2::signal< void (double) > update_progress_signal_type;
  update_progress_signal_type update_progress_signal_;

  // LAYER_UPDATED_SIGNAL:
  // Triggered when the layer data has been modified.
  typedef boost::signals2::signal< void () > layer_updated_signal_type;
  layer_updated_signal_type layer_updated_signal_;
  
  // ABORT_SIGNAL:
  // Trigger an abort signal for the current filter
  typedef boost::signals2::signal< void() > abort_signal_type;
  abort_signal_type abort_signal_;
  
  // -- State variables --
public:
  
  // The name of the layer
  Core::StateNameHandle name_state_;

  // Per viewer state of whether this layer is visible
  std::vector< Core::StateBoolHandle > visible_state_;

  // Master state of the layer visibility
  Core::StateBoolHandle master_visible_state_;

  // State indicating whether the layer is locked
  Core::StateBoolHandle locked_state_;

  // State that describes the opacity with which the layer is displayed
  Core::StateRangedDoubleHandle opacity_state_;

  // State indicating whether the layer is selected for further processing
  Core::StateBoolHandle selected_state_;

  // State that stores the current layer state
  Core::StateOptionHandle data_state_;

  // -- GUI related states --
  // Whether to show the layer information
  Core::StateBoolHandle show_information_state_;
  
  // Whether to show the layer information
  Core::StateBoolHandle show_appearance_state_;
  
  // Whether to show the layer information
  Core::StateBoolHandle show_advanced_visibility_state_;

  // Whether to show the opacity control
  Core::StateBoolHandle show_opacity_state_;

  // Whether to show the progress bar
  Core::StateBoolHandle show_progress_bar_state_;

  // Whether to show the abort message
  Core::StateBoolHandle show_abort_message_state_;

protected:
  // State that stores the generation of its datablock
  Core::StateLongLongHandle generation_state_;
  
  // State that stores the last action that was played
  Core::StateStringHandle last_action_state_;
  
  // An exclusive group of boolean states that control the visibility of different parts
  Core::BooleanStateGroupHandle gui_state_group_;

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

  // -- abort handling --
public:

  void handle_abort();
  
  bool check_abort();

  void reset_abort();

protected:
  virtual bool post_save_states( Core::StateIO& state_io );

  // -- Filter keys --
public: 
  // FILTER_KEY:
  // This is a unique key that is inserted when an asynchronous filter is running. 
  // The asynchronous calls back to the layer are compared with the key, if they don't
  // match the layer is not modified, at it is assumed that the filtering was aborted.
  typedef long long filter_key_type;

  // CHECK_FILTER_KEY:
  // Check whether the current key is attached to the layer
  bool check_filter_key( filter_key_type key ) const;

  // ADD_FILTER_KEY:
  // Add a filter key to the layer
  void add_filter_key( filter_key_type key );

  // REMOVE_FILTER_KEY:
  // Reset the current key for what is being processed
  void remove_filter_key( filter_key_type key );
  
  // CLEAR_FILTER_KEYS:
  // Remove all filter keys
  void clear_filter_keys();

  // NUM_FILTER_KEYS:
  // Get the number of filter keys
  size_t num_filter_keys() const;

  // SET_FILTER_HANDLE:
  // Set the filter that is currently processing the data
  void set_filter_handle( LayerAbstractFilterHandle layer );
  
  // RESET_FILTER_HANDLE:
  // Reset the filter handle, indicating no filter it working on the data
  void reset_filter_handle();

  // GET_FITLER_HANDLE
  // Get the current filter associated with the layer
  LayerAbstractFilterHandle get_filter_handle();


  // -- internals of class --
private:  

  LayerPrivateHandle private_;

  // -- Locking system --
public:
  // This code just aligns the layer locking with the StateEngine locking
  typedef Core::StateEngine::mutex_type mutex_type;
  typedef Core::StateEngine::lock_type lock_type;
  
  // GETMUTEX:
  // Get the mutex of the state engine
  static mutex_type& GetMutex();

  // -- data processing key generation --
public:
  static filter_key_type GenerateFilterKey();

public:

  // Options for the state of the data inside of the layer
  const static std::string CREATING_C;
  const static std::string PROCESSING_C;
  const static std::string AVAILABLE_C;
  const static std::string IN_USE_C;

};

} // end namespace Seg3D

#endif
