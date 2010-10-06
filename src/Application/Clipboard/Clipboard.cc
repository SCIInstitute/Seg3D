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

#include <Core/Application/Application.h>

#include <Application/Clipboard/Clipboard.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Implementation of class ClipboardPrivate
//////////////////////////////////////////////////////////////////////////

class ClipboardPrivate
{
public:
  std::vector< ClipboardItemHandle > slots_;
};

//////////////////////////////////////////////////////////////////////////
// Implementation of class Clipboard
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( Clipboard );

Clipboard::Clipboard() :
  private_( new ClipboardPrivate )
{
  this->private_->slots_.resize( 3 );
}

Clipboard::~Clipboard()
{
}

ClipboardItemConstHandle Clipboard::get_item( size_t index )
{
  ASSERT_IS_APPLICATION_THREAD();
  assert( index < this->private_->slots_.size() );
  
  return this->private_->slots_[ index ];
}

ClipboardItemHandle Clipboard::get_item( size_t width, size_t height, 
                    Core::DataType data_type, size_t index )
{
  ASSERT_IS_APPLICATION_THREAD();
  assert( index < this->private_->slots_.size() );

  ClipboardItemHandle item = this->private_->slots_[ index ];
  if ( !item )
  {
    item.reset( new ClipboardItem( width, height, data_type ) );
    this->private_->slots_[ index ] = item;
  }
  else
  {
    item->resize( width, height, data_type );
  }

  return item;
}

size_t Clipboard::number_of_slots()
{
  return this->private_->slots_.size();
}

} // end namespace Seg3D