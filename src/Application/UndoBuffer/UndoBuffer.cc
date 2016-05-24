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

// STL includes
#include <deque>

// Core includes
#include <Core/Action/ActionContextContainer.h>

// Application includes
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( UndoBuffer );

// Container class for an action context that forwards every call except the call to the source
// which is overloaded with a tag that the call is made from the undo buffer.

class UndoActionContext : public Core::ActionContextContainer
{
public:
  UndoActionContext( Core::ActionContextHandle context ) :
    Core::ActionContextContainer( context )
  {
  }
  
  virtual ~UndoActionContext() {}

  virtual Core::ActionSource source() const
  {
    return Core::ActionSource::UNDOBUFFER_E;
  }
};

class UndoBufferPrivate
{

public:
  typedef std::deque< UndoBufferItemHandle > undo_list_type;
  typedef std::deque< UndoBufferItemHandle > redo_list_type;
  
  // List of items on the undo stack
  undo_list_type undo_list_;
  
  // List of items on the redo stack
  redo_list_type redo_list_;
  
  UndoBuffer* buffer_;
  long long max_mem_;
  
  void handle_enable( bool enable );
};


void UndoBufferPrivate::handle_enable( bool enable )
{
  if ( enable == false )
  {
    this->buffer_->reset_undo_buffer();
  }
}

UndoBuffer::UndoBuffer() :
  private_( new UndoBufferPrivate )
{
  this->private_->buffer_ = this;
  this->private_->max_mem_ = Core::Application::Instance()->
    get_total_addressable_physical_memory();
  
  this->add_connection( PreferencesManager::Instance()->enable_undo_state_->
    value_changed_signal_.connect( boost::bind( 
    &UndoBufferPrivate::handle_enable, this->private_, _1 ) ) );
  this->add_connection( Core::Application::Instance()->reset_signal_.connect( boost::bind(
    &UndoBuffer::reset_undo_buffer, this ) ) );
}

UndoBuffer::~UndoBuffer()
{
  this->disconnect_all();
}

void UndoBuffer::insert_undo_item( Core::ActionContextHandle context, 
  UndoBufferItemHandle undo_item )
{
  if ( PreferencesManager::Instance()->enable_undo_state_->get() == false ) return;

  undo_item->compute_size();

  // Clear REDO buffer if a new item is added from anywhere else except the undo buffer itself
  if ( context->source() != Core::ActionSource::UNDOBUFFER_E )
  {
    this->private_->redo_list_.clear();
    this->update_redo_tag_signal_( this->get_redo_tag() );
  }

  size_t max_size = static_cast<size_t> ( this->private_->max_mem_ * 
    PreferencesManager::Instance()->percent_of_memory_state_->get() );

  // Get the size of the element
  size_t size = undo_item->get_byte_size();

  UndoBufferPrivate::undo_list_type::iterator it = this->private_->undo_list_.begin();
  UndoBufferPrivate::undo_list_type::iterator it_end = this->private_->undo_list_.end();

  size_t max_num_undos = 0;

  while ( it != it_end )
  {
    size += (*it)->get_byte_size();
    max_num_undos++;
    if ( size > max_size ) break;
    if ( max_num_undos >= 100 ) break;
    ++it;
  }

  this->private_->undo_list_.erase( it, it_end );
  this->private_->undo_list_.push_front( undo_item );
  
  this->update_undo_tag_signal_( undo_item->get_tag() );
  this->buffer_changed_signal_();
}

bool UndoBuffer::undo( Core::ActionContextHandle context )
{
  if ( this->private_->undo_list_.empty() )
  {
    context->report_error( "Undo list is empty" );
    return false;
  }  

  // This function removes the first item from the undo buffer
  // and reapplies it. 
  UndoBufferItemHandle undo_item;
  undo_item = this->private_->undo_list_.front();
  this->private_->undo_list_.pop_front();

  if ( ! ( undo_item->apply_and_clear_undo() ) )
  {
    context->report_error("Internal error encountered while applying undo "
      "check point" );
    return false;
  }
  
  // Move the action that was just undone on top of the redo stack, in case one wants to
  // redo the action
  this->private_->redo_list_.push_front( undo_item );

  // Update the entries in the menu
  this->update_undo_tag_signal_( this->get_undo_tag() );
  this->update_redo_tag_signal_( this->get_redo_tag() );
  this->buffer_changed_signal_();

  return true;
}

bool UndoBuffer::redo( Core::ActionContextHandle context )
{
  if ( this->private_->redo_list_.empty() )
  {
    context->report_error( "Redo list is empty" );
    return false;
  }  
  
  // This function removes the first item from the undo buffer
  // and reapplies it. 
  UndoBufferItemHandle redo_item;
  redo_item = this->private_->redo_list_.front();
  this->private_->redo_list_.pop_front();
  
  Core::ActionContextHandle undo_context( new UndoActionContext( context ) );
  redo_item->apply_redo( undo_context );

  // Update the entries in the menu
  this->update_redo_tag_signal_( this->get_redo_tag() );
  this->buffer_changed_signal_();
  
  // Applying the redo action should put a new undo check point onto the undo stack.
  return true;
}

void UndoBuffer::reset_undo_buffer()
{
  this->private_->redo_list_.clear();
  this->private_->undo_list_.clear();

  this->update_redo_tag_signal_( this->get_redo_tag() );
  this->update_undo_tag_signal_( this->get_undo_tag() );
  this->buffer_changed_signal_();
}

std::string UndoBuffer::get_undo_tag( size_t index ) const
{
  // Extract the first item from the undo list and get its tag
  if ( index < this->private_->undo_list_.size() )
  {
    return this->private_->undo_list_[ index ]->get_tag();
  }
  else
  {
    // An empty string indicates that there is no undo action on the list
    return "";
  }
}

std::string UndoBuffer::get_redo_tag( size_t index ) const
{
  // Extract the first item from the undo list and get its tag
  if ( index < this->private_->redo_list_.size() )
  {
    return this->private_->redo_list_[ index ]->get_tag();
  }
  else
  {
    // An empty string indicates that there is no undo action on the list
    return "";
  }
}

size_t UndoBuffer::get_undo_byte_size( size_t index ) const
{
  // Extract the first item from the undo list and get its tag
  if ( index < this->private_->undo_list_.size() )
  {
    return this->private_->undo_list_[ index ]->get_byte_size();
  }
  else
  {
    // An empty string indicates that there is no undo action on the list
    return 0;
  }
}

size_t UndoBuffer::get_redo_byte_size( size_t index ) const
{
  // Extract the first item from the redo list and get its tag
  if ( index < this->private_->redo_list_.size() )
  {
    return this->private_->redo_list_[ index ]->get_byte_size();
  }
  else
  {
    // An empty string indicates that there is no redo action on the list
    return 0;
  }
}

bool UndoBuffer::has_undo() const
{
  return ! ( this->private_->undo_list_.empty() );
}

bool UndoBuffer::has_redo() const
{
  return ! ( this->private_->redo_list_.empty() );
}

size_t UndoBuffer::num_undo_items()
{
  return this->private_->undo_list_.size();
}

size_t UndoBuffer::num_redo_items()
{
  return this->private_->redo_list_.size();
}

} // end namespace Seg3D

