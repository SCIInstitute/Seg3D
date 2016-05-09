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

#ifndef APPLICATION_CLIPBOARD_CLIPBOARD_H
#define APPLICATION_CLIPBOARD_CLIPBOARD_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <Core/Utils/Singleton.h>

#include <Application/Clipboard/ClipboardItem.h>

namespace Seg3D
{

// Forward declarations
class ClipboardPrivate;
typedef boost::shared_ptr< ClipboardPrivate > ClipboardPrivateHandle;

class Clipboard : public boost::noncopyable
{
  CORE_SINGLETON( Clipboard );
  
private:
  Clipboard();
  ~Clipboard();

public:
  /// GET_ITEM:
  /// Get the current item stored at slot index.
  ClipboardItemConstHandle get_item( long long sandbox = -1 );

  /// GET_ITEM:
  /// Create a new item with the specified width, height, and data type at the slot
  /// index, and return a handle to it.
  ClipboardItemHandle get_item( size_t width, size_t height, 
    Core::DataType data_type, long long sandbox = -1 );

  /// CREATE_SANDBOX:
  /// Create a sandbox with specified ID.
  void create_sandbox( long long sandbox_id );

  /// DELETE_SANDBOX:
  /// Delete a sandbox.
  bool delete_sandbox( long long sandbox_id );

private:
  friend class ClipboardUndoBufferItem;

  /// SET_ITEM:
  /// Set the item stored at the specified slot.
  void set_item( ClipboardItemHandle item );

private:
  ClipboardPrivateHandle private_;

};

} // end namespace Seg3D

#endif
