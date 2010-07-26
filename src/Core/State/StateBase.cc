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

#include <Core/State/StateEngine.h>
#include <Core/State/StateBase.h>

namespace Core {

StateBase::StateBase(const std::string& stateid) :
  stateid_( stateid ),
  signals_enabled_( true ),
  initializing_( false ),
  session_priority_( DEFAULT_LOAD_E )
{
}

StateBase::~StateBase()
{
}

void StateBase::enable_signals( bool signals_enabled )
{
  StateEngine::lock_type lock( StateEngine::Instance()->GetMutex() );
  this->signals_enabled_ = signals_enabled;
}

void StateBase::set_initializing( bool initializing )
{
  StateEngine::lock_type lock( StateEngine::Instance()->GetMutex() );
  this->initializing_ = initializing;
}

int StateBase::get_session_priority() const
{
  return  this->session_priority_;
}
  
void StateBase::set_session_priority( int priority )
{
  this->session_priority_ = priority;
}



std::string StateBase::get_stateid() const
{
  return this->stateid_;
}

std::string StateBase::get_baseid() const
{
  return ( this->stateid_.substr( 0, this->stateid_.find( ':' ) ) );
}

std::string StateBase::get_id() const
{
  return Core::SplitString(  this->stateid_, "::" )[ 1 ];
}

void StateBase::invalidate()
{
}

} // end namespace Core
