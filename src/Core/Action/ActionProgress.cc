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

// Application includes
#include <Core/Action/ActionProgress.h>
#include <Core/Action/ActionDispatcher.h>

namespace Core
{

ActionProgress::ActionProgress( const std::string& message,
    bool is_interuptable, bool has_progress_updates ) :
  message_( message ),
  is_interruptable_( is_interuptable ),
  has_progress_updates_( has_progress_updates ),  
  interrupt_( false ),
  progress_( 0.0 )
{
}

ActionProgress::~ActionProgress()
{
}

void ActionProgress::begin_progress_reporting()
{
  ActionDispatcher::Instance()->begin_progress_signal_( this->shared_from_this() );
}

void ActionProgress::end_progress_reporting()
{
  ActionDispatcher::Instance()->end_progress_signal_(  this->shared_from_this() );
}

void ActionProgress::set_interrupt( bool interrupt )
{
  lock_type lock( get_mutex() );
  interrupt_ = interrupt;
}

bool ActionProgress::get_interrupt()
{
  lock_type lock( get_mutex() );
  return interrupt_;
}

void ActionProgress::set_progress( double progress )
{
  lock_type lock( get_mutex() );
  progress_ = progress;
  ActionDispatcher::Instance()->report_progress_signal_( this->shared_from_this() ); 
}

double ActionProgress::get_progress()
{
  lock_type lock( get_mutex() );
  return progress_;
}

std::string ActionProgress::get_message() const
{
  return message_;
}

bool ActionProgress::is_interruptable() const
{
  return is_interruptable_;
}

bool ActionProgress::has_progress_updates() const
{
  return has_progress_updates_;
}

} // end namespace Core
