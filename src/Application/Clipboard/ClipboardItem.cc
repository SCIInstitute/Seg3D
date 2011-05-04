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

#include <vector>

#include <Application/Clipboard/ClipboardItem.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Implementation of class ClipboardItemPrivate
//////////////////////////////////////////////////////////////////////////

class ClipboardItemPrivate
{
public:
  ClipboardItemPrivate() : data_type_( Core::DataType::UNKNOWN_E ) {}

  size_t width_;
  size_t height_;
  Core::DataType data_type_;
  std::vector< unsigned char > buffer_;

  // Source information
  ProvenanceID provenance_id_;
  int slice_type_;
  size_t slice_number_;
};

//////////////////////////////////////////////////////////////////////////
// Implementation of class ClipboardItem
//////////////////////////////////////////////////////////////////////////

ClipboardItem::ClipboardItem( size_t width, size_t height, Core::DataType data_type ) :
  private_( new ClipboardItemPrivate )
{
  this->resize( width, height, data_type );
}

ClipboardItem::~ClipboardItem()
{
}

ClipboardItemHandle ClipboardItem::clone() const
{
  ClipboardItem* cpy = new ClipboardItem( this->private_->width_, 
    this->private_->height_, this->private_->data_type_ );
  cpy->private_->buffer_ = this->private_->buffer_;
  return ClipboardItemHandle( cpy );
}

size_t ClipboardItem::get_width() const
{
  return this->private_->width_;
}

size_t ClipboardItem::get_height() const
{
  return this->private_->height_;
}

Core::DataType ClipboardItem::get_data_type() const
{
  return this->private_->data_type_;
}

size_t ClipboardItem::buffer_size() const
{
  return this->private_->buffer_.size();
}

const void* ClipboardItem::get_buffer() const
{
  return &this->private_->buffer_[ 0 ];
}

void* ClipboardItem::get_buffer()
{
  return &this->private_->buffer_[ 0 ];
}

void ClipboardItem::resize( size_t width, size_t height, Core::DataType data_type )
{
  this->private_->width_ = width;
  this->private_->height_ = height;
  this->private_->data_type_ = data_type;

  size_t buffer_size = 0;
  switch ( data_type )
  {
  case Core::DataType::CHAR_E:
  case Core::DataType::UCHAR_E:
    buffer_size = sizeof( char );
    break;
  case Core::DataType::SHORT_E:
  case Core::DataType::USHORT_E:
    buffer_size = sizeof( short );
    break;
  case Core::DataType::INT_E:
  case Core::DataType::UINT_E:
    buffer_size = sizeof( int );
    break;
  case Core::DataType::FLOAT_E:
    buffer_size = sizeof( float );
    break;
  case Core::DataType::DOUBLE_E:
    buffer_size = sizeof( double );
    break;
  default:
    assert( false );
  }

  buffer_size *= ( width * height );
  this->private_->buffer_.resize( buffer_size );
}

void ClipboardItem::set_source( const ProvenanceID& pid, const int slice_type, const size_t slice_number )
{
  this->private_->provenance_id_ = pid;
  this->private_->slice_type_ = slice_type;
  this->private_->slice_number_ = slice_number;
}

void ClipboardItem::get_source( ProvenanceID& pid, int& slice_type, size_t& slice_number ) const
{
  pid = this->private_->provenance_id_;
  slice_type = this->private_->slice_type_;
  slice_number = this->private_->slice_number_;
}

} // end namespace Seg3D