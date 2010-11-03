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

#ifndef APPLICATION_LAYERMANAGER_LAYERUNDOBUFFERITEM_H
#define APPLICATION_LAYERMANAGER_LAYERUNDOBUFFERITEM_H

// Core includes
#include <Core/Action/Action.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerCheckPoint.h>

namespace Seg3D
{

// Forward declarations
class LayerUndoBufferItem;
class LayerUndoBufferItemPrivate;
typedef boost::shared_ptr<LayerUndoBufferItem> LayerUndoBufferItemHandle;
typedef boost::shared_ptr<LayerUndoBufferItemPrivate> LayerUndoBufferItemPrivateHandle;


// Class that describes all the steps that need to be undertaken to undo a layer action.
class LayerUndoBufferItem 
{

  // -- constructor/destructor --
public:
  LayerUndoBufferItem( const std::string& tag );
  virtual ~LayerUndoBufferItem();

  // -- creation of undo/redo action --
public:
  // SET_REDO_ACTION:
  // Set a redo action for undoing the undo
  // NOTE: This is general the action that inserts the undo step onto the queue
  void set_redo_action( Core::ActionHandle action );
  
  // ADD_FILTER_TO_ABORT:
  // This adds a base filter weak handle to the process that will compute the
  // filter output.
  void add_filter_to_abort( LayerAbstractFilterHandle filter );
  
  // ADD_LAYER_TO_DELETE:
  // Add layer to delete list
  // If a new layer is created by the action it should be deleted in the undo
  // This functions adds a handle to the layer that needs to be deleted in the undo
  void add_layer_to_delete( LayerHandle layer );

  // ADD_LAYER_TO_ADD:
  // Add layer to delete list
  void add_layer_to_add( LayerHandle layer );

  // ADD_LAYER_TO_RESTORE:
  // Add layer to restore list, layers are restored using check points.
  void add_layer_to_restore( LayerHandle layer, LayerCheckPointHandle checkpoint);

  // -- apply undo/redo action --
public:
  // APPLY_REDO:
  // Apply the redo information
  bool apply_redo( Core::ActionContextHandle& context );

  // APPLY_AND_CLEAR_UNDO:
  // Apply the undo information
  bool apply_and_clear_undo();

  // -- size information --
public:
  // GET_BYTE_SIZE:
  // The size of the item in memory ( approximately )
  size_t get_byte_size() const;

  // GET_TAG:
  // Tag that appears in the menu for this item
  std::string get_tag() const;

  // -- internals --
private:
  LayerUndoBufferItemPrivateHandle private_;
  
};

} // end namespace Seg3D

#endif
