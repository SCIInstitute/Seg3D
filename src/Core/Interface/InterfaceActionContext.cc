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

#include <Core/Interface/InterfaceActionContext.h>

namespace Core
{

InterfaceActionContext::InterfaceActionContext( ActionSource source ) :
  source_( source )
{
}

InterfaceActionContext::~InterfaceActionContext()
{
}

void InterfaceActionContext::report_error( const std::string& error )
{
  CORE_LOG_ERROR( error );
}

void InterfaceActionContext::report_warning( const std::string& warning )
{
  CORE_LOG_WARNING( warning );
}

void InterfaceActionContext::report_message( const std::string& message )
{
  CORE_LOG_MESSAGE( message );
}

void InterfaceActionContext::report_need_resource( NotifierHandle notifier )
{
  std::string error = std::string( "'" ) + notifier->get_name() + "' is currently not available.";
  CORE_LOG_ERROR( error );
}

void InterfaceActionContext::report_done()
{
  action_done_signal_( status() );
}

} // end namespace Core
