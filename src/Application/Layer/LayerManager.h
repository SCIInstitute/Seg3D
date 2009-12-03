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

/*
 *****************************************************************************
 *
 *   LayerManager.h
 *
 *   LayerManager provides the interface to access the various layers of data 
 *   that are managed in the system.
 *
 *   To perform an action to update a layer or layers, for instance add a layer,
 *   delete a layer, copy a layer, modify a layer, first use the dispatcher
 *   to create and dispatch an action.  This action will be given to the 
 *   handler to actually perform the appropriate action.
 *
 *   Also see: Layer
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      10/26/2009
 *
 *    
 *****************************************************************************
 */

#ifndef APPLICATION_LAYER_LAYERMANAGER_H
#define APPLICATION_LAYER_LAYERMANAGER_H 1

//#ifdef (_MSC_VER) && (_MSC_VER >= 1020)
//# pragma once
//#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

// Application includes
#include <Utils/Singleton/Singleton.h>
#include <Application/Layer/Layer.h>
#include <Application/State/StateHandler.h>


namespace Seg3D {

// Forward declarations

// typedefs

// class definitions

class LayerManager : public boost::noncopyable, public StateHandler {
public:
  LayerManager();
  ~LayerManager();
  
  static LayerManager* instance() { instance_.instance(); }

  // -- Signals for the User Interface --
  typedef boost::signals2::signal<void (LayerHandle)> layer_changed_signal_type;

  // CONNECT_LAYER_CHANGED:
  // Connect to the signal that indicates a layer has changed
  boost::signals2::connection
  connect_layer_changed(layer_changed_signal_type::slot_type slot)
  {
    return layer_changed_signal_.connect(slot);
  }

  // -- Dispatcher functions for User Interface --
  void dispatch_clonelayer(const std::string& from_layer_name) const;
  void dispatch_layerfromfile(const std::string& filename) const;
  void dispatch_newmasklayer(const std::string& from_layer_name) const;
  void dispatch_removelayer(const std::string& layer_name) const;

  const LayerHandle get_layer(const std::string& layer_name) const;
  void get_layers(int layer_kinds, std::vector<std::string>& layer_names) const;
  void get_layer_group(const std::vector<std::string>& layer_names, LayerGroup& group) const;
  void get_layer_group(const LayerProperties& properties, LayerGroup& group) const;
  std::vector<LayerGroup> cluster_layers(const LayerProperties& properties) const;

  DataVolumeHandle checkpoint_data(const std::string& layer_name, const SliceRange& slices) const;
  LayerProperties checkpoint_properties(const std::string& layer_name) const;
  void load_data_checkpoint(const std::string& layer_name, const SliceRange& slices, DataVolumeHandle& data);
  void load_properties_checkpoint(const std::string& layer_name, const LayerProperties& properties);

  // Create a unique layer name based on the provided name
  std::string derive_new_name(const std::string& name) const;

protected:
  friend class ActionLayerFromFile;
  friend class ActionCloneLayer;
  friend class ActionNewMaskLayer;
  friend class ActionRemoveLayer;
  friend class Action;

  void clone_layer(const std::string& from_layer_name);
  void remove_layer(const std::string& layer_name);
  void lock_layer(const std::string& layer_name);
  void unlock_layer(const std::string& layer_name);
  void user_lock_layer(const std::string& layer_name);
  void user_unlock_layer(const std::string& layer_name);
  void add_layer(LayerHandle new_layer);

  // LAYER_CHANGED_SIGNAL:
  // This signal is triggered after a layer has been modified
  layer_changed_signal_type layer_changed_signal_;

private:
  static Utils::Singleton<LayerManager> instance_;

  typedef boost::unordered_map<std::string, LayerHandle> layer_map_type; 
  layer_map_type layers_;
  boost::mutex layer_map_lock_;


}; // class LayerManager

} // end namespace Seg3D

#endif
