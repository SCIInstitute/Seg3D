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

#ifndef APPLICATION_LAYER_LAYERUNDOBUFFERITEM_H
#define APPLICATION_LAYER_LAYERUNDOBUFFERITEM_H

// Core includes
#include <Core/Action/Action.h>

// Application includes
#include <Application/UndoBuffer/UndoBufferItem.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerCheckPoint.h>
#include <Application/Provenance/ProvenanceStep.h>

namespace Seg3D
{

// Forward declarations
class LayerUndoBufferItem;
class LayerUndoBufferItemPrivate;
typedef boost::shared_ptr<LayerUndoBufferItem> LayerUndoBufferItemHandle;
typedef boost::weak_ptr< LayerUndoBufferItem > LayerUndoBufferItemWeakHandle;
typedef boost::shared_ptr<LayerUndoBufferItemPrivate> LayerUndoBufferItemPrivateHandle;


/// Class that describes all the steps that need to be undertaken to undo a layer action.
class LayerUndoBufferItem : public UndoBufferItem
{

  // -- constructor/destructor --
public:
  LayerUndoBufferItem( const std::string& tag );
  virtual ~LayerUndoBufferItem();

  // -- creation of undo/redo action --
public:
  
  /// ADD_FILTER_TO_ABORT:
  /// This adds a base filter weak handle to the process that will compute the
  /// filter output.
  void add_filter_to_abort( LayerAbstractFilterHandle filter );
  
  /// ADD_LAYER_TO_DELETE:
  /// Add layer to delete list
  /// If a new layer is created by the action it should be deleted in the undo
  /// This functions adds a handle to the layer that needs to be deleted in the undo
  void add_layer_to_delete( LayerHandle layer );

  /// ADD_LAYER_TO_ADD:
  /// Add layer to delete list
  void add_layer_to_add( LayerHandle layer );

  /// ADD_LAYER_TO_RESTORE:
  /// Add layer to restore list, layers are restored using check points.
  void add_layer_to_restore( LayerHandle layer, LayerCheckPointHandle checkpoint);

  /// ADD_ID_COUNT_TO_RESTORE:
  /// Cache the count of layer and group ids, so they can be rolled back to the original ones
  /// NOTE: If they are set to -1, the function will query the current ones.
  void add_id_count_to_restore( LayerManager::id_count_type id_count );

  /// SET_PROVENANCE_STEP_ID:
  /// Set the provenance record ID associated with the action.
  void set_provenance_step_id( ProvenanceStepID step_id );

  /// SET_PROVENANCE_STEP_IDS:
  /// Set the provenance record ID associated with the action.
  void set_provenance_step_ids( const std::vector< ProvenanceStepID >& step_ids );

  /// ROLLBACK_LAYER_CHANGES:
  /// Abort corresponding filters (if any) and rollback all the layer changes.
  /// NOTE: This function should only be called by LayerFilter when aborted by the user.
  void rollback_layer_changes();

  // -- apply undo/redo action --
public:

  /// APPLY_AND_CLEAR_UNDO:
  /// Apply the undo information
  virtual bool apply_and_clear_undo();

  // -- size information --
public:
  /// GET_BYTE_SIZE:
  /// The size of the item in memory ( approximately )
  virtual size_t get_byte_size() const;

  /// COMPUTE_SIZE:
  /// Compute the size of the item
  virtual void compute_size();

  // -- internals --
private:
  LayerUndoBufferItemPrivateHandle private_;
};

} // end namespace Seg3D

#endif
