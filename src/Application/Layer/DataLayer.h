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

#ifndef APPLICATION_LAYER_DATALAYER_H
#define APPLICATION_LAYER_DATALAYER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Application includes
#include <Application/Layer/Layer.h>
#include <Core/Volume/DataVolume.h>

namespace Seg3D
{

class DataLayerPrivate;
typedef boost::shared_ptr< DataLayerPrivate > DataLayerPrivateHandle;

// CLASS DataLayer

// Class definition
class DataLayer : public Layer
{
  friend class DataLayerPrivate;

  // -- constructor/destructor --
public:

  DataLayer( const std::string& name, const Core::DataVolumeHandle& volume );
  DataLayer( const std::string& state_id );
  virtual ~DataLayer();

  /// GET_TYPE:
  /// this function returns the a Core::VolumeType indicating that this is a DataLayer 
  virtual Core::VolumeType get_type() const { return Core::VolumeType::DATA_E; }

  /// GET_BYTE_SIZE:
  /// Get the size of the data contained in the layer in bytes
  virtual size_t get_byte_size() const;

  /// GET_GRID_TRANSFORM:
  /// this function returns the grid transform of the data volume
  /// Locks: StateEngine
  virtual Core::GridTransform get_grid_transform() const;
  
  /// SET_GRID_TRANSFORM:
  /// Set the grid transform of the data volume
  virtual void set_grid_transform( const Core::GridTransform& grid_transform, 
    bool preserve_centering );

  /// HAS_VALID_DATA:
  /// Check whether the layer has valid data.
  virtual bool has_valid_data() const;  
    
  /// GET_DATA_TYPE:
  /// Get the data type of the data volume
  virtual Core::DataType get_data_type() const;
  
  /// GET_VOLUME:
  /// Get the underlying volume.
  virtual Core::VolumeHandle get_volume() const;

  /// DUPLICATE_LAYER:
  /// Duplicate layer
  virtual LayerHandle duplicate() const;

  /// GET_DATA_VOLUME:
  /// this function returns the data volume
  Core::DataVolumeHandle get_data_volume() const;

  /// SET_DATA_VOLUME:
  /// this function sets the data_volume
  bool set_data_volume( Core::DataVolumeHandle data_volume );
  
  // -- state variables --
public:

  /// Which color to use for displaying the mask
  Core::StateIntHandle color_state_;

  /// State describing contrast
  Core::StateRangedDoubleHandle contrast_state_;

  /// State describing brightness
  Core::StateRangedDoubleHandle brightness_state_;

  Core::StateRangedDoubleHandle display_min_value_state_;

  Core::StateRangedDoubleHandle display_max_value_state_;

  Core::StateBoolHandle adjust_display_min_max_state_;

  Core::StateOptionHandle colormap_state_;

  /// State describing whether volume is volume rendered
  Core::StateBoolHandle volume_rendered_state_;

  Core::StateStringHandle data_type_state_;
  Core::StateDoubleHandle min_value_state_;
  Core::StateDoubleHandle max_value_state_;

protected:
  /// PRE_SAVE_STATES:
  /// this function synchronize the generation number for the session saving
  virtual bool pre_save_states( Core::StateIO& state_io );

  /// POST_LOAD_STATES:
  /// this function takes care of connecting the DataVolume to the data layer after it's settings
  virtual bool post_load_states( const Core::StateIO& state_io );
  
  /// CLEAN_UP:
  /// this function cleans up the data volume for when you are deleting the data layer and reloading
  virtual void clean_up();

  // -- color functions --
public:

	/// GETCOLORCOUNT:
	/// Get the current count from which the next color is derived
	static size_t GetColorCount();

	/// SETCOLORCOUNT:
	/// Set the color count to a specific number
	static void SetColorCount(size_t count);

private:
  void initialize_states();

private:
  Core::DataVolumeHandle data_volume_;
  DataLayerPrivateHandle private_;
};

} // end namespace Seg3D

#endif
