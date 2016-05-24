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

#ifndef APPLICATION_CLIPBOARD_CLIPBOARDITEM_H
#define APPLICATION_CLIPBOARD_CLIPBOARDITEM_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/DataBlock/DataType.h>

#include <Application/Provenance/Provenance.h>

namespace Seg3D
{

// Forward declarations
class ClipboardItem;
typedef boost::shared_ptr< ClipboardItem > ClipboardItemHandle;
typedef boost::shared_ptr< const ClipboardItem > ClipboardItemConstHandle;

class ClipboardItemPrivate;
typedef boost::shared_ptr< ClipboardItemPrivate > ClipboardItemPrivateHandle;

class ClipboardItem : public boost::noncopyable
{ 
private:
  friend class Clipboard;

  ClipboardItem( size_t width, size_t height, Core::DataType data_type );

public:
  ~ClipboardItem();

public:

  /// CLONE:
  /// Make a copy of the item.
  ClipboardItemHandle clone() const;

  /// GET_WIDTH:
  /// Returns the width of the buffer.
  size_t get_width() const;

  /// GET_HEIGHT:
  /// Returns the height of the buffer.
  size_t get_height() const;

  /// GET_DATA_TYPE:
  /// Returns the actual data type of the buffer.
  Core::DataType get_data_type() const;

  /// BUFFER_SIZE:
  /// Returns the number of bytes in the buffer.
  size_t buffer_size() const;

  /// GET_BUFFER:
  /// Returns the pointer to the buffer of the clipboard item.
  const void* get_buffer() const;

  /// GET_BUFFER:
  /// Returns the pointer to the buffer of the clipboard item.
  void* get_buffer();

  /// SET_PROVENANCE_ID:
  /// Set the provenance ID of the clipboard item.
  void set_provenance_id( const ProvenanceID& pid );

  /// GET_PROVENANCE_ID:
  /// Get the provenance ID of the clipboard item.
  ProvenanceID get_provenance_id() const;

private:
  /// RESIZE:
  /// Resize the buffer to match the new width, height, and data type.
  void resize(  size_t width, size_t height, Core::DataType data_type );

private:
  ClipboardItemPrivateHandle private_;
};

} // end namespace Seg3D

#endif
