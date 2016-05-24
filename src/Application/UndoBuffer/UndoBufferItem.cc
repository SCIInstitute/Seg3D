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

#include <Application/UndoBuffer/UndoBufferItem.h>

namespace Seg3D
{

class UndoBufferItemPrivate
{
public:
  // -- tag for the menu --
  std::string tag_;

  // An action can always be redone by executing the action again 
  Core::ActionHandle redo_action_;
};

UndoBufferItem::UndoBufferItem( const std::string& tag ) :
  private_( new UndoBufferItemPrivate )
{
  this->private_->tag_ = tag;
}

UndoBufferItem::~UndoBufferItem()
{
}

void UndoBufferItem::set_redo_action( Core::ActionHandle action )
{
  this->private_->redo_action_ = action;
}

bool UndoBufferItem::apply_redo( Core::ActionContextHandle& context )
{

  // Validate the action. It should validate, but if it doesn't it should fail
  // gracefully. Hence we check anyway.
  if ( !( this->private_->redo_action_->validate( context ) ) )
  {
    return false;
  }
  
  // Run the action. Make the changes to the state engine.
  // NOTE: We do not use the result yet
  // TODO: Need to implement result handling 
  // --JGS
  
  Core::ActionResultHandle result;
  if ( !( this->private_->redo_action_->run( context, result ) ) )
  {
    return false;
  }
  
  this->private_->redo_action_->clear_cache();
  
  return true;
}

std::string UndoBufferItem::get_tag() const
{
  return this->private_->tag_;
}

} // end namespace Seg3D
