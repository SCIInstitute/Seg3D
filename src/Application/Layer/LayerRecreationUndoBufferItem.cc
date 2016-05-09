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

// Application includes
#include <Application/Layer/LayerRecreationUndoBufferItem.h>

namespace Seg3D
{

class LayerRecreationUndoBufferItemPrivate
{
public: 
  // The provenance ID to recreate
  ProvenanceIDList prov_ids_;

  // The sandbox used for the recreation
  SandboxID sandbox_;

  // The count of the id of a layer, this one has to be rolled back in case of undo
  LayerManager::id_count_type id_count_;
  
  // Size of the item
  size_t size_;
};

LayerRecreationUndoBufferItem::LayerRecreationUndoBufferItem( 
  const ProvenanceIDList& prov_ids, SandboxID sandbox ) :
  UndoBufferItem( "Recreate Layer" ),
  private_( new LayerRecreationUndoBufferItemPrivate )
{
  this->private_->size_ = 0;
  this->private_->id_count_ = LayerManager::GetLayerInvalidIdCount();
  this->private_->prov_ids_ = prov_ids;
  this->private_->sandbox_ = sandbox;
}

LayerRecreationUndoBufferItem::~LayerRecreationUndoBufferItem()
{
  ASSERT_IS_APPLICATION_THREAD();
}

bool LayerRecreationUndoBufferItem::apply_and_clear_undo()
{
  // Destroy the sandbox
  LayerManager::Instance()->delete_sandbox( this->private_->sandbox_ );
  
  for ( size_t i = 0; i < this->private_->prov_ids_.size(); ++i )
  {
    // Look for the layer with the provenance ID, delete it if exists
    LayerHandle layer = LayerManager::FindLayer( this->private_->prov_ids_[ i ] );
    if ( layer )
    {
      // Invalidate the layer so the layer ID can be reused
      layer->invalidate();
      LayerManager::Instance()->delete_layer( layer );
    }
  }
  
  // The counters need to be rolled back so when redo is done, it actually redos the same action
  // resulting in the same counters. This only needs to be done if layers got created and deleted
  // in the undo.
  
  LayerManager::SetLayerIdCount( this->private_->id_count_ );
  
  return true;
}

size_t LayerRecreationUndoBufferItem::get_byte_size() const
{
  return this->private_->size_;
}

void LayerRecreationUndoBufferItem::compute_size()
{
}

void LayerRecreationUndoBufferItem::add_id_count_to_restore( LayerManager::id_count_type id_count )
{
    this->private_->id_count_ = id_count;
}

} // end namespace Seg3D
