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

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Application/Application.h>

// Application includes
#include <Application/Clipboard/Clipboard.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Implementation of class ClipboardPrivate
//////////////////////////////////////////////////////////////////////////

typedef std::map< long long , ClipboardItemHandle > SandboxMap;

class ClipboardPrivate
{
public:
  // RESET:
  // Rest the clipboard.
  void reset();

  ClipboardItemHandle item_;
  SandboxMap sandboxes_;
};

void ClipboardPrivate::reset()
{
  this->item_.reset();
  this->sandboxes_.clear();
}


//////////////////////////////////////////////////////////////////////////
// Implementation of class Clipboard
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( Clipboard );

Clipboard::Clipboard() :
  private_( new ClipboardPrivate )
{
  // Reset the clipboard on application reset signal.
  Core::Application::Instance()->reset_signal_.connect( boost::bind( 
    &ClipboardPrivate::reset, this->private_ ) );
}

Clipboard::~Clipboard()
{
}

ClipboardItemConstHandle Clipboard::get_item( long long sandbox )
{
  ASSERT_IS_APPLICATION_THREAD();

  if ( sandbox == -1 )
  {
    return this->private_->item_;
  }
  
  SandboxMap::iterator it = this->private_->sandboxes_.find( sandbox );
  if ( it != this->private_->sandboxes_.end() )
  {
    return it->second;
  }
  
  CORE_THROW_LOGICERROR( "Sandbox not found!" );
}

ClipboardItemHandle Clipboard::get_item( size_t width, size_t height, 
                    Core::DataType data_type, long long sandbox )
{
  ASSERT_IS_APPLICATION_THREAD();

  ClipboardItemHandle item;
  if ( sandbox == -1 )
  {
    item = this->private_->item_;
  }
  else
  {
    SandboxMap::iterator it = this->private_->sandboxes_.find( sandbox );
    if ( it == this->private_->sandboxes_.end() )
    {
      CORE_THROW_LOGICERROR( "Sandbox not found!" );
    }
    item = it->second;    
  }

  if ( !item )
  {
    item.reset( new ClipboardItem( width, height, data_type ) );
    if ( sandbox == -1 )
    {
      this->private_->item_ = item;
    }
    else
    {
      this->private_->sandboxes_[ sandbox ] = item;
    }
  }
  else
  {
    item->resize( width, height, data_type );
  }

  return item;
}

void Clipboard::set_item( ClipboardItemHandle item )
{
  ASSERT_IS_APPLICATION_THREAD();
  this->private_->item_ = item;
}

void Clipboard::create_sandbox( long long sandbox_id )
{
  this->private_->sandboxes_[ sandbox_id ] = ClipboardItemHandle();
}

bool Clipboard::delete_sandbox( long long sandbox_id )
{
  return this->private_->sandboxes_.erase( sandbox_id ) == 1;
}

} // end namespace Seg3D
