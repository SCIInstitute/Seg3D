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

#ifndef APPLICATION_LAYER_LAYERGROUP_H
#define APPLICATION_LAYER_LAYERGROUP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <vector>
#include <list>

// Boost includes 
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>

// Core includes
#include <Core/Geometry/GridTransform.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/Action/Action.h>
#include <Core/State/State.h>
#include <Core/State/BooleanStateGroup.h>
#include <Core/Volume/Volume.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/Layer/LayerMetaData.h>
#include <Application/Provenance/Provenance.h>

namespace Seg3D
{

// Forward declarations
class LayerGroupPrivate;
typedef boost::shared_ptr< LayerGroupPrivate > LayerGroupPrivateHandle;

/// CLASS LayerGroup
/// This is the class that records the layers that are grouped together
class LayerGroup : public Core::StateHandler, public boost::enable_shared_from_this< LayerGroup >
{
  // -- constructor/destructor --
public:
  LayerGroup( Core::GridTransform grid_transform, ProvenanceID provenance_id, 
    const LayerMetaData& meta_data );
  LayerGroup( const std::string& state_id );
  virtual ~LayerGroup();

public:
  /// GRID_TRANSFORM
  /// Get the transform of the layer
  const Core::GridTransform& get_grid_transform() const;

  /// GET_GROUP_ID:
  /// Get the group ID.
  const std::string& get_group_id() const;

  /// GET_LAYERS:
  /// Get all layers in top to bottom order.
  void get_layers( std::vector< LayerHandle >& layers ) const;

  /// TOP_LAYER:
  /// Get the layer at the top.
  LayerHandle top_layer() const;

  /// BOTTOM_LAYER:
  /// Get the layer at the bottom.
  LayerHandle bottom_layer() const;

  /// NUMBER_OF_LAYERS:
  /// Get the number of layers in the group.
  size_t number_of_layers() const;

  /// IS_EMPTY:
  /// Check whether the group contains any layers.
  bool is_empty() const;

  /// HAS_A_VALID_LAYER:
  /// Check whether the group contains at least one valid layer.
  bool has_a_valid_layer() const;

  /// GET_LAYER_NAMES:
  /// Get a list of all the names of the layers in the group.
  void get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
    Core::VolumeType type, LayerHandle excluded_layer = LayerHandle() ) const;

  /// GET_LAYER_POSITION:
  /// Returns the position of the given layer within the group.
  size_t get_layer_position( LayerHandle layer ) const;
  
  // -- state variables --
public:

  /// = Isosurface state variables =
  Core::StateOptionHandle isosurface_quality_state_;
  Core::StateBoolHandle isosurface_capping_enabled_state_;

  /// The following three variables are for displaying only
  Core::StatePointHandle dimensions_state_;
  Core::StatePointHandle origin_state_;
  Core::StatePointHandle spacing_state_;

  /// Visibility of layers
  Core::StateOptionHandle layers_visible_state_;
  
  /// Visibility of mask layer isosurfaces
  Core::StateOptionHandle layers_iso_visible_state_;

  /// The provenance id of the layer that created this group
  Core::StateLongLongHandle provenance_id_state_;

  /// State of the MetaData associated with this group
  Core::StateStringHandle meta_data_state_;

  /// State variable that keeps track of what type of meta data was provided by the importer
  Core::StateStringHandle meta_data_info_state_;

  // -- GUI related states --
public:
  /// Whether to show the isosurface menu
  Core::StateBoolHandle show_iso_menu_state_;

  /// Whether to show the delete layers menu
  Core::StateBoolHandle show_delete_menu_state_;
  
  /// Whether to show the duplicate layers menu
  Core::StateBoolHandle show_duplicate_menu_state_;
  
  /// Whether the group is expanded or not
  Core::StateBoolHandle group_widget_expanded_state_;
  
  // -- Functions accessed by LayerManager --
private:
  friend class LayerManager;
  friend class LayerManagerPrivate;

  /// INSERT_LAYER:
  /// Inserts a layer in the correct position
  void insert_layer( LayerHandle new_layer );

  /// INSERT_LAYER:
  /// Insert a layer at the specified position
  void insert_layer( LayerHandle new_layer, size_t pos );

  /// MOVE_LAYER:
  /// Move the src_layer to the current position of dst_layer, and shift the dst_layer downward.
  /// If dst_layer is not given, then move src_layer to the bottom of this group.
  /// Returns true if the layer was moved, or false if nothing has changed.
  bool move_layer( LayerHandle src_layer, LayerHandle dst_layer );
    
  /// DELETE_LAYER:
  /// Delete a layer from the list
  void delete_layer( LayerHandle layer );

  /// CLEAR:
  /// Delete all the layers.
  void clear();

  /// GET_LAYER_LIST:
  /// Get a reference to the internal list of layers.
  LayerList& get_layer_list() ;
  
public: 
  /// GET_METADATA:
  /// NOTE: Group meta data is derived from the first layer that generates the group.
  /// Retrieve all the meta data that was part of this layer in one convenient structure
  LayerMetaData get_meta_data() const;

  /// SET_METADATA:
  /// Set all the metadata state variables
  void set_meta_data( const LayerMetaData& meta_data ); 
  
  // -- Functions defined in StateHandler --
protected:
  /// POST_SAVE_STATES:
  /// This function is called after the LayerGroup's states have been saved and then
  /// tells the layers to save their states as well.
  virtual bool post_save_states( Core::StateIO& state_io );
  
  /// POST_LOAD_STATES:
  /// this function creates the layers who's information was saved to file, and then tells them
  /// to populate their state variables from file
  virtual bool post_load_states( const Core::StateIO& state_io );
  
private:
  LayerGroupPrivateHandle private_;
};

} // end namespace Seg3D

#endif
