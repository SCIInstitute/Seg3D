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

#include <Application/Clipboard/Clipboard.h>
#include <Application/Clipboard/ClipboardUndoBufferItem.h>
#include <Application/ProjectManager/ProjectManager.h>

namespace Seg3D
{

class ClipboardUndoBufferItemPrivate
{
public: 
  // Clipboard item checkpoint
  ClipboardItemHandle clipboard_item_;

  // Size of the item
  size_t size_;

  // ID of the provenance record to delete when this undo item is applied.
  ProvenanceStepID prov_step_id_;
};

ClipboardUndoBufferItem::ClipboardUndoBufferItem( const std::string& tag,
  ClipboardItemHandle clipboard_item ) :
  UndoBufferItem( tag ),
  private_( new ClipboardUndoBufferItemPrivate )
{
  this->private_->size_ = 0;
  this->private_->clipboard_item_ = clipboard_item;
}

ClipboardUndoBufferItem::~ClipboardUndoBufferItem()
{
}

bool ClipboardUndoBufferItem::apply_and_clear_undo()
{
  Clipboard::Instance()->set_item( this->private_->clipboard_item_ );
  ProjectManager::Instance()->get_current_project()->
    delete_provenance_record( this->private_->prov_step_id_ );
  // Clear the checkpoint
  this->private_->clipboard_item_.reset();
  return true;
}

size_t ClipboardUndoBufferItem::get_byte_size() const
{
  return this->private_->size_;
}

void ClipboardUndoBufferItem::compute_size()
{
  if ( this->private_->clipboard_item_ )
  {
    this->private_->size_ = this->private_->clipboard_item_->buffer_size();
  }
  else
  {
    this->private_->size_ = 0;
  }
}

void ClipboardUndoBufferItem::set_provenance_step_id( ProvenanceStepID step_id )
{
  this->private_->prov_step_id_ = step_id;
}

} // end namespace Seg3D
