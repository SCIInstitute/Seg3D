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

#include <Core/State/BooleanStateGroup.h>
#include <Core/Utils/ConnectionHandler.h>

namespace Core
{

class BooleanStateGroupPrivate : public ConnectionHandler
{
public:
  void handle_state_changed( size_t index, bool state_value );

  std::vector< StateBoolHandle > states_;
};

void BooleanStateGroupPrivate::handle_state_changed( size_t index, bool state_value )
{
  if ( !state_value )
  {
    return;
  }
  
  for ( size_t i = 0; i < index; ++i )
  {
    this->states_[ i ]->set( false );
  }
  
  for ( size_t i = index + 1; i < this->states_.size(); ++i )
  {
    this->states_[ i ]->set( false );
  }
}


BooleanStateGroup::BooleanStateGroup() :
  private_( new BooleanStateGroupPrivate )
{
}

BooleanStateGroup::~BooleanStateGroup()
{
  this->private_->disconnect_all();
  this->private_->states_.clear();
}

void BooleanStateGroup::add_boolean_state( StateBoolHandle state )
{
  StateEngine::lock_type lock( StateEngine::GetMutex() );

  this->private_->states_.push_back( state );
  this->private_->add_connection( state->value_changed_signal_.connect(
    boost::bind( &BooleanStateGroupPrivate::handle_state_changed, 
    this->private_, this->private_->states_.size() - 1, _1 ) ) );
  this->private_->handle_state_changed( this->private_->states_.size() - 1, state->get() );
}

void BooleanStateGroup::clear_selection()
{
  ASSERT_IS_APPLICATION_THREAD();

  for ( size_t i = 0; i < this->private_->states_.size(); ++i )
  {
    this->private_->states_[ i ]->set( false );
  }
}
  
bool BooleanStateGroup::get_enabled() const
{
  for ( size_t i = 0; i < this->private_->states_.size(); ++i )
  {
    if( this->private_->states_[ i ]->get() ) return true;
  }
  return false;
}

} // end namespace Core
