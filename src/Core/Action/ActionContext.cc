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

#include <Core/Utils/Log.h>

#include <Core/Action/ActionContext.h>

namespace Core
{

ActionContext::ActionContext() :
  status_( ActionStatus::SUCCESS_E )
{
}

ActionContext::~ActionContext()
{
}

void ActionContext::report_error( const std::string& error )
{
  CORE_LOG_ERROR(error);
}

void ActionContext::report_warning( const std::string& warning )
{
  CORE_LOG_WARNING(warning);
}

void ActionContext::report_message( const std::string& message )
{
  CORE_LOG_MESSAGE(message);
}

void ActionContext::report_result( const ActionResultHandle& result )
{
  this->result_ = result;
}

void ActionContext::report_status( ActionStatus status )
{
  status_ = status;
}

void ActionContext::report_need_resource( NotifierHandle notifier )
{
  this->notifier_ = notifier;
  this->error_msg_ = std::string( "'" ) + notifier->get_name() +
    std::string("' is currently unavailable" );
}

ActionStatus ActionContext::status() const
{
  return status_;
}

ActionSource ActionContext::source() const
{
  return ActionSource::COMMANDLINE_E;
}

void ActionContext::report_done()
{
  // currently no post action for base ActionContext
  if (! is_success() ) CORE_LOG_DEBUG("ActionContext done: " + this->error_msg_);
}

Core::NotifierHandle ActionContext::get_resource_notifier()
{
  return this->notifier_;
}

Core::ActionResultHandle ActionContext::get_result()
{
  return this->result_;
}

void ActionContext::reset_context()
{
  this->notifier_.reset();
  this->result_.reset();
  this->error_msg_.clear();
}

std::string ActionContext::get_error_message()
{
  return this->error_msg_;
}


} // end namespace Core
