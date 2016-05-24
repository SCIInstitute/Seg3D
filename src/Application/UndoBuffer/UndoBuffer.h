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

#ifndef APPLICATION_UNDOBUFFER_UNDOBUFFER_H
#define APPLICATION_UNDOBUFFER_UNDOBUFFER_H

// boost includes
#include <boost/signals2/signal.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Singleton.h>

// Application includes
#include <Application/UndoBuffer/UndoBufferItem.h>

namespace Seg3D
{

/// UndoBuffer
/// This singleton class keeps a list of the actions that can be undone or redone.

/// Internals for the UndoBuffer singleton
class UndoBufferPrivate;
typedef boost::shared_ptr< UndoBufferPrivate > UndoBufferPrivateHandle;

class UndoBuffer : private Core::ConnectionHandler
{
  CORE_SINGLETON( UndoBuffer );

  // -- Constructor/Destructor --
private:
  UndoBuffer();
  virtual ~UndoBuffer();

  // -- undo interface --
public:

  /// INSERT_UNDO_ITEM:
  /// Insert a new undo item in the queue
  /// NOTE: The action context is needed to verify whether it is inserted from the undo buffer
  /// itself or whether the undo item was created in a normal action.
  void insert_undo_item( Core::ActionContextHandle context, 
    UndoBufferItemHandle undo_item );

  /// UNDO:
  /// Undo the top item of the stack
  bool undo( Core::ActionContextHandle context  ); 
  
  /// REDO:
  /// redo the top item of the stack
  bool redo( Core::ActionContextHandle context );
  
  /// RESET_UNDO_BUFFER:
  /// Reset the buffer to its initial setting
  void reset_undo_buffer();
  
  /// GET_UNDO_TAG:
  /// Get the tag from the action stored on top of the undo stack.
  /// NOTE: if an empty string is returned the undo stack is empty.
  std::string get_undo_tag( size_t index = 0 ) const;

  /// GET_UNDO_BYTE_SIZE:
  /// Get the tag from the action stored on top of the undo stack.
  /// NOTE: if an empty string is returned the undo stack is empty.
  size_t get_undo_byte_size( size_t index = 0 ) const;

  /// GET_REDO_TAG:
  /// Get the tag from the action stored on top of the redo stack.
  /// NOTE: if an empty string is returned the redo stack is empty.
  std::string get_redo_tag( size_t index = 0 ) const;

  /// GET_REDO_BYTE_SIZE:
  /// Get the tag from the action stored on top of the undo stack.
  /// NOTE: if an empty string is returned the undo stack is empty.
  size_t get_redo_byte_size( size_t index = 0 ) const;

  /// HAS_UNDO:
  /// Check whether there is something to undo
  bool has_undo() const;
  
  /// HAS_REDO:
  /// Check whether there is something to redo
  bool has_redo() const;

  /// NUM_UNDO_ITEMS:
  /// Get the number of undo items on the stack
  size_t num_undo_items();

  /// NUM_REDO_ITEMS:
  /// Get the number of redo items on the stack
  size_t num_redo_items();


  // -- signals --
public:
  typedef boost::signals2::signal< void ( std::string ) > update_undo_tag_signal_type;
  typedef boost::signals2::signal< void ( std::string ) > update_redo_tag_signal_type;
  typedef boost::signals2::signal< void () > buffer_changed_signal_type;

  /// UPDATE_UNDO_TAG_SIGNAL:
  /// This signal is triggered when a new undo item is on top of the undo stack
  update_undo_tag_signal_type update_undo_tag_signal_;
  
  /// UPDATE_REDO_TAG_SIGNAL:
  /// This signal is triggered when a new redo item is on top of the redo stack
  update_redo_tag_signal_type update_redo_tag_signal_;
  
  /// BUFFER_CHANGED_SIGNAL:
  /// This signal is triggered whenever the buffer changes its contents
  buffer_changed_signal_type buffer_changed_signal_;
  
  // -- internals --
private:
  /// Handle to internals
  UndoBufferPrivateHandle private_;
};

} // end namespace Seg3D

#endif
