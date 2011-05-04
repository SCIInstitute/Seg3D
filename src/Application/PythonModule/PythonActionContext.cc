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

#include <Application/PythonModule/PythonActionContext.h>

namespace Seg3D
{

PythonActionContext::PythonActionContext()
{
}

PythonActionContext::~PythonActionContext()
{
}

void PythonActionContext::report_error( const std::string& error )
{
  this->action_message_signal_( Core::LogMessageType::ERROR_E, error );
}

void PythonActionContext::report_warning( const std::string& warning )
{
  this->action_message_signal_( Core::LogMessageType::WARNING_E, warning );
}

void PythonActionContext::report_message( const std::string& message )
{
  this->action_message_signal_( Core::LogMessageType::MESSAGE_E, message );
}

void PythonActionContext::report_need_resource( const Core::NotifierHandle& notifier )
{
  std::string message = std::string( "'" ) + notifier->get_name() + std::string(
      "' is currently unavailable" );
  this->action_message_signal_( Core::LogMessageType::ERROR_E, message );
}

void PythonActionContext::report_done()
{
  this->action_done_signal_( this->status() );
}

Core::ActionSource PythonActionContext::source() const
{
  return Core::ActionSource::COMMANDLINE_E;
}

} //end namespace Seg3D
