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

namespace Seg3D {

// CLASS Layer
// This is the main class for collecting state information on a layer

// Forward declarations
class Layer;
typedef boost::shared_ptr<Layer> LayerHandle;

// Class definition
class Layer : public StateHandler {
    
// -- constructor/destructor --    
  public:

    Layer(const std::string& name, Utils::VolumeType type, const Utils::VolumeHandle& volume);
    virtual ~Layer();

// -- Layer properties --
  public:

    // TYPE
    // Get the type of the layer
    inline Utils::VolumeType type() const { return type_; }

    // GRID_TRANSFORM
    // Get the transform of the layer
    inline Utils::GridTransform grid_transform() const { return grid_transform_; }

    // VOLUME
    // Return the underlying volume
    inline Utils::VolumeHandle volume() const { return volume_; }

    // SET_VOLUME
    // Add a volume to this layer
    void set_volume(Utils::VolumeHandle volume);

  protected:
    // The type of the layer
    Utils::VolumeType type_;
  
    // The underlying structure that contains data plus transform, but no
    // state information
    Utils::VolumeHandle volume_;

    // Cached version of the grid transform of the underlying volume object
    Utils::GridTransform grid_transform_;

// -- State variables --
  public:
    // The name of the layer 
    StateAliasHandle                name_state_;

    // Per viewer state of whether this layer is visible
    std::vector<StateBoolHandle>    visible_state_;
    
    // State indicating whether the layer is locked
    StateBoolHandle                 lock_state_;

    // State that describes the opacity with which the layer is displayed
    StateRangedDoubleHandle         opacity_state_;
    
    // State of the checkbox that records which layer needs to be processed in
    // the group
    StateBoolHandle                 selected_state_;

    // State that describes which menu is currently shown
    StateOptionHandle               edit_mode_state_;

// -- Locking system --
  public:
    // This code just aligns the layer locking with the StateEngine locking
    typedef StateEngine::mutex_type mutex_type;
    typedef StateEngine::lock_type  lock_type;
  
    // NOTE: The locking of making changes to the layer layout should be inline
    // with the StateEngine. Since this is a recursive lock, this will force
    // things to locked while the layer layout is redone.
    
    // LOCK
    // Lock the layer state engine
    static void Lock() { StateEngine::Lock(); }

    // UNLOCK
    // Unlock the layer state engine
    static void Unlock() { StateEngine::Unlock(); }

    // GETMUTEX
    // Get the mutex of the state engine
    static mutex_type& GetMutex() { return StateEngine::GetMutex(); }
};

} // end namespace Seg3D

#endif
