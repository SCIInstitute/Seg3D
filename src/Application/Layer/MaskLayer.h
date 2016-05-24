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

#ifndef APPLICATION_LAYER_MASKLAYER_H
#define APPLICATION_LAYER_MASKLAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Core includes
#include <Core/Isosurface/Isosurface.h>
#include <Core/Volume/MaskVolume.h>

// Application includes
#include <Application/Layer/Layer.h>

namespace Seg3D
{

// Hide header includes, private interface and implementation
class MaskLayerPrivate;
typedef boost::shared_ptr< MaskLayerPrivate > MaskLayerPrivateHandle;

/// CLASS MaskLayer

// Class definition
class MaskLayer : public Layer
{

  // -- constructor/destructor --
public:

  MaskLayer( const std::string& name, const Core::MaskVolumeHandle& volume );
  MaskLayer( const std::string& state_id );

  // NOTE: This needs to be virtual so it can be deleted from the base class
  virtual ~MaskLayer();

public:
  /// GET_TYPE:
  /// This function returns the a Core::VolumeType indicating that this is a MaskLayer.
  virtual Core::VolumeType get_type() const { return Core::VolumeType::MASK_E; }

  /// GET_BYTE_SIZE:
  /// Get the size of the data contained in the layer in bytes
  virtual size_t get_byte_size() const;

  /// GET_GRID_TRANSFORM:
  /// This function returns the grid transform of the mask volume.
  /// Locks: StateEngine
  virtual Core::GridTransform get_grid_transform() const;

  /// SET_GRID_TRANSFORM:
  /// Set the grid transform of the mask volume.
  virtual void set_grid_transform( const Core::GridTransform& grid_transform, 
    bool preserve_centering );

  /// GET_DATA_TYPE:
  /// Get the data type of the underlying data
  virtual Core::DataType get_data_type() const;

  /// HAS_VALID_DATA:
  /// Check whether the layer has valid data
  virtual bool has_valid_data() const;
  
  /// DUPLICATE_LAYER:
  /// Duplicate layer
  virtual LayerHandle duplicate() const;  
  
  /// GET_VOLUME:
  /// Get the underlying volume.
  virtual Core::VolumeHandle get_volume() const;

  /// GET_MASK_VOLUME:
  /// This function returns the mask volume.
  Core::MaskVolumeHandle get_mask_volume() const;
  
  /// SET_MASK_VOLUME:
  /// This function set the mask volume to a new mask.
  bool set_mask_volume( Core::MaskVolumeHandle volume );
  

  // -- isosurface handling --
public:
  /// GET_ISOSURFACE:
  /// Get the isosurface associated with this layer
  Core::IsosurfaceHandle get_isosurface();

  /// COMPUTE_ISOSURFACE
  /// Compute the isosurface for this layer using the given quality factor.
  /// Quality factor must be one of: 1.0, 0.5, 0.25, 0.125
  void compute_isosurface( double quality_factor, bool capping_enabled );
  
  /// CALCULATE_VOLUME:
  /// function that is called by the calculate volume action that calculate the volume of the mask
  void calculate_volume();

  /// DELETE_ISOSURFACE:
  /// Delete the isosurface associated with this layer
  void delete_isosurface();
  
  // -- Signals --
public:
  boost::signals2::signal< void () > isosurface_updated_signal_;
  
  
  // -- state variables --
public:

  /// Which color to use for displaying the mask
  Core::StateIntHandle color_state_;

  /// State that describes whether to use a fat border for the mask
  Core::StateOptionHandle border_state_;

  /// State that describes whether to fill the mask solid
  Core::StateOptionHandle fill_state_;

  /// State that describes whether to show the  isosurface state
  Core::StateBoolHandle show_isosurface_state_;
  
  /// State that describes whether the isosurface has been generated
  Core::StateBoolHandle iso_generated_state_;

  /// The area of the generated isosurface
  Core::StateDoubleHandle isosurface_area_state_;
  
  /// State that describes the calculated volume of the mask, as string because its connected to a 
  /// label that will show a non numeric value, when the volume has not been calculated.
  Core::StateStringHandle calculated_volume_state_;
  
  Core::StateStringHandle counted_pixels_state_;

  Core::StateDoubleHandle min_value_state_;
  Core::StateDoubleHandle max_value_state_;
  
protected:
  /// PRE_SAVE_STATES:
  /// this function synchronize the generation number for the session saving
  virtual bool pre_save_states( Core::StateIO& state_io );

  /// POST_LOAD_STATES:
  /// this function takes care of connecting the MaskVolume to the mask layer after it's settings
  /// have been loaded
  virtual bool post_load_states( const Core::StateIO& state_io );

  /// CLEAN_UP:
  /// this function cleans up the mask volume for when you are deleting the mask and reloading 
  virtual void clean_up();

  // -- color functions --
public:
  
  /// GETCOLORCOUNT:
  /// Get the current count from which the next color is derived
  static size_t GetColorCount();
  
  /// SETCOLORCOUNT:
  /// Set the color count to a specific number
  static void SetColorCount( size_t count );
  
private:
  MaskLayerPrivateHandle private_;
};

} // end namespace Seg3D

#endif
