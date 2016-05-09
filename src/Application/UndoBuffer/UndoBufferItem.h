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

#ifndef APPLICATION_UNDOBUFFER_UNDOBUFFERITEM_H
#define APPLICATION_UNDOBUFFER_UNDOBUFFERITEM_H

// Core includes
#include <Core/Action/Action.h>

namespace Seg3D
{

// Forward declarations
class UndoBufferItem;
class UndoBufferItemPrivate;
typedef boost::shared_ptr< UndoBufferItem > UndoBufferItemHandle;
typedef boost::shared_ptr< UndoBufferItemPrivate > UndoBufferItemPrivateHandle;


// Class that describes all the steps that need to be undertaken to undo an action.
class UndoBufferItem 
{

  // -- constructor/destructor --
public:
  UndoBufferItem( const std::string& tag );
  virtual ~UndoBufferItem();

  // -- creation of undo/redo action --
public:
  /// SET_REDO_ACTION:
  /// Set a redo action for undoing the undo
  /// NOTE: This is generally the action that inserts the undo step onto the queue
  void set_redo_action( Core::ActionHandle action );

  // -- apply undo/redo action --
public:
  /// APPLY_REDO:
  /// Apply the redo information
  bool apply_redo( Core::ActionContextHandle& context );

  /// APPLY_AND_CLEAR_UNDO:
  /// Apply the undo information
  virtual bool apply_and_clear_undo() = 0;

  // -- size information --
public:
  /// GET_BYTE_SIZE:
  /// The size of the item in memory ( approximately )
  virtual size_t get_byte_size() const = 0;

  /// COMPUTE_SIZE:
  /// Compute the size of the item
  virtual void compute_size() = 0;

  /// GET_TAG:
  /// Tag that appears in the menu for this item
  std::string get_tag() const;

  // -- internals --
private:
  UndoBufferItemPrivateHandle private_;
};

} // end namespace Seg3D

#endif
