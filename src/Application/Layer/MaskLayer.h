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

#ifndef APPLICATION_LAYER_MASKLAYER_H
#define APPLICATION_LAYER_MASKLAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Core includes
#include <Core/Isosurface/Isosurface.h>
#include <Core/Volume/MaskVolume.h>
#include <Core/Utils/AtomicCounter.h>

// Application includes
#include <Application/Layer/Layer.h>

namespace Seg3D
{

// CLASS MaskLayer

// Forward declarations
class MaskLayer;
typedef boost::shared_ptr< MaskLayer > MaskLayerHandle;

// Class definition
class MaskLayer : public Layer
{

  // -- constructor/destructor --
public:

  MaskLayer( const std::string& name, const Core::MaskVolumeHandle& volume );
  MaskLayer( const std::string& name, const Core::GridTransform& grid_transform );
  MaskLayer( const std::string& state_id );
  virtual ~MaskLayer();

  // TYPE:
  // this function returns the a Core::VolumeType indicating that this is a MaskLayer
  virtual Core::VolumeType type() const { return Core::VolumeType::MASK_E; }

  // GET_GRID_TRANSFORM:
  // this function returns the grid transform of the mask volume
  virtual const Core::GridTransform& get_grid_transform() const 
  { 
    return mask_volume_->get_grid_transform(); 
  }

  // GET_MASK_VOLUME:
  // this function returns the mask volume
  Core::MaskVolumeHandle get_mask_volume()
  {
    return this->mask_volume_;
  }
  
  // -- state variables --
public:

  // Which color to use for displaying the mask
  Core::StateIntHandle color_state_;

  // State that describes whether to use a fat border for the mask
  Core::StateOptionHandle border_state_;

  // State that describes whether to fill the mask solid
  Core::StateOptionHandle fill_state_;

  // State that describes whether to show the  isosurface state
  Core::StateBoolHandle show_isosurface_state_;

protected:
  // PRE_SAVE_STATES:
  // this function synchronize the generation number for the session saving
  virtual bool pre_save_states( Core::StateIO& state_io );

  // POST_LOAD_STATES:
  // this function takes care of connecting the MaskVolume to the mask layer after it's settings
  // have been loaded
  virtual bool post_load_states( const Core::StateIO& state_io );

  // CLEAN_UP:
  // this function cleans up the mask volume for when you are deleting the mask and reloading 
  virtual void clean_up();

private:
  void initialize_states();
  void handle_mask_data_changed();
  
private:
  
  Core::StateIntHandle bit_state_;
  const static size_t VERSION_NUMBER_C;
  Core::MaskVolumeHandle mask_volume_;
  Core::IsosurfaceHandle isosurface_;
  
  // counter for generating new colors for each new mask
  static Core::AtomicCounter color_count_;
};

} // end namespace Seg3D

#endif
