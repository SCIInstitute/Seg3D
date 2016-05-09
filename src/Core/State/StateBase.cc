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

#include <Core/State/StateEngine.h>
#include <Core/State/StateBase.h>

namespace Core {

class StateBasePrivate
{
public:
  // The name of this state, so it can be saved in human readable form
  std::string stateid_;

  // Whether the signals are enabled
  bool signals_enabled_;

  // Whether the state variable is still being initialized
  bool initializing_;

  // The session saving/loading priority
  int session_priority_;

  // Whether the data is part of the session data
  bool is_project_data_;
  
  // Whether data is locked and cannot be changed by the action mechanism
  bool locked_;
};

StateBase::StateBase(const std::string& stateid) :
  private_( new StateBasePrivate )
{
  this->private_->stateid_ = stateid;
  this->private_->signals_enabled_ = true;
  this->private_->initializing_ = false;
  this->private_->session_priority_ = DEFAULT_LOAD_E;
  this->private_->is_project_data_ = false;
  this->private_->locked_ = false;
}

StateBase::~StateBase()
{
}

std::string StateBase::get_stateid() const
{
  return this->private_->stateid_;
}

void StateBase::enable_signals( bool signals_enabled )
{
  StateEngine::lock_type lock( StateEngine::Instance()->GetMutex() );
  this->private_->signals_enabled_ = signals_enabled;
}

bool StateBase::signals_enabled()
{
  return this->private_->signals_enabled_;
}

void StateBase::set_initializing( bool initializing )
{
  StateEngine::lock_type lock( StateEngine::Instance()->GetMutex() );
  this->private_->initializing_ = initializing;
}

bool StateBase::get_initializing() const
{
  return this->private_->initializing_;
}

void StateBase::set_locked( bool locked )
{
  StateEngine::lock_type lock( StateEngine::Instance()->GetMutex() );
  this->private_->locked_ = locked;
}

bool StateBase::get_locked() const
{
  return this->private_->locked_;
}

int StateBase::get_session_priority() const
{
  return  this->private_->session_priority_;
}
  
void StateBase::set_session_priority( int priority )
{
  this->private_->session_priority_ = priority;
}

void StateBase::invalidate()
{
}

bool StateBase::is_project_data() const
{
  return this->private_->is_project_data_;
}

void StateBase::set_is_project_data( bool is_project_data )
{
  this->private_->is_project_data_ = is_project_data;
}



} // end namespace Core
