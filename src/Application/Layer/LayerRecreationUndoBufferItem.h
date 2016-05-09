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

#ifndef APPLICATION_LAYER_LAYERRECREATIONUNDOBUFFERITEM_H
#define APPLICATION_LAYER_LAYERRECREATIONUNDOBUFFERITEM_H

// Application includes
#include <Application/UndoBuffer/UndoBufferItem.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

// Forward declarations
class LayerRecreationUndoBufferItem;
typedef boost::shared_ptr<LayerRecreationUndoBufferItem> LayerRecreationUndoBufferItemHandle;

class LayerRecreationUndoBufferItemPrivate;
typedef boost::shared_ptr<LayerRecreationUndoBufferItemPrivate> LayerRecreationUndoBufferItemPrivateHandle;


/// Class that describes all the steps that need to be undertaken to undo a layer action.
class LayerRecreationUndoBufferItem : public UndoBufferItem
{

  // -- constructor/destructor --
public:
  LayerRecreationUndoBufferItem( const ProvenanceIDList& prov_ids, SandboxID sandbox );
  virtual ~LayerRecreationUndoBufferItem();

  // -- creation of undo/redo action --
public:

  /// ADD_ID_COUNT_TO_RESTORE:
  /// Cache the count of layer and group ids, so they can be rolled back to the original ones
  /// NOTE: If they are set to -1, the function will query the current ones.
  void add_id_count_to_restore( LayerManager::id_count_type id_count );

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
  LayerRecreationUndoBufferItemPrivateHandle private_;
};

} // end namespace Seg3D

#endif
