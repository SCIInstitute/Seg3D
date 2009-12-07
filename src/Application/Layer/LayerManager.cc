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
 *   LayerManager.cc
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/19/2009
 *
 *    
 *****************************************************************************
 */

// STL includes

// Boost includes 

#include <Application/Interface/Interface.h>

#include <Application/Layer/LayerManager.h>

#include <Application/Layer/Actions/ActionCloneLayer.h>
#include <Application/Layer/Actions/ActionLayerFromFile.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>
#include <Application/Layer/Actions/ActionRemoveLayer.h>

namespace Seg3D {

LayerManager::LayerManager() :
  StateHandler("LayerManager")
{
}
  
LayerManager::~LayerManager()
{
}

void 
LayerManager::dispatch_clonelayer(const std::string& from_layer_name) const
{
  // Build new action
  ActionCloneLayerHandle action(new ActionCloneLayer);
  // Set the action parameters
  action->set(from_layer_name);
  // Run the action on the application thread
  PostActionFromInterface(action);
} 

void 
LayerManager::dispatch_layerfromfile(const std::string& filename) const
{
  // Build a new action
  ActionLayerFromFileHandle action(new ActionLayerFromFile);
  // Set the action parameters
  action->set(filename);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

void 
LayerManager::dispatch_newmasklayer(const std::string& from_layer_name) const
{
  // Build a new action
  ActionNewMaskLayerHandle action(new ActionNewMaskLayer);
  // Set the action parameters
  action->set(from_layer_name);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

void 
LayerManager::dispatch_removelayer(const std::string& layer_name) const
{
  // Build a new action
  ActionRemoveLayerHandle action(new ActionRemoveLayer);
  // Set the action parameters
  action->set(layer_name);
  // Run the action on the application thread
  PostActionFromInterface(action);
}

const LayerHandle
LayerManager::get_layer(const std::string& name) const
{
  LayerHandle handle;
  return handle;
}

void
LayerManager::get_layers(int layer_kinds, std::vector<std::string>& layer_names) const
{}

void
LayerManager::get_layer_group(const std::vector< std::string >& layer_names, LayerGroup& group) const
{}

void 
LayerManager::get_layer_group(const LayerProperties& properties, LayerGroup& group) const
{}

std::vector<LayerGroup>
LayerManager::cluster_layers(const LayerProperties& properties) const
{
  std::vector<LayerGroup> group;
  return group;
}

DataVolumeHandle 
LayerManager::checkpoint_data(const std::string& name, const SliceRange& slices) const
{
  DataVolumeHandle handle;
  return handle;
}

LayerProperties 
LayerManager::checkpoint_properties(const std::string& name) const
{
  LayerProperties prop;
  return prop;
}

void 
LayerManager::load_data_checkpoint(const std::string& name, const SliceRange& slices, DataVolumeHandle& data)
{}

void 
LayerManager::load_properties_checkpoint(const std::string& name, const LayerProperties& properties)
{}

void 
LayerManager::clone_layer(const std::string& from_layer_name)
{}

void 
LayerManager::remove_layer(const std::string& name)
{}

void 
LayerManager::lock_layer(const std::string& name)
{}

void 
LayerManager::unlock_layer(const std::string& name)
{}

void 
LayerManager::add_layer(LayerHandle new_layer)
{}

std::string 
LayerManager::derive_new_name(const std::string& name) const
{
  return "";
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<LayerManager> LayerManager::instance_;

} // end namespace Seg3D

