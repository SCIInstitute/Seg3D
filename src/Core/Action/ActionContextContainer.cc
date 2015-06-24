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

#include <Core/Action/ActionContextContainer.h>

namespace Core
{

ActionContextContainer::ActionContextContainer( ActionContextHandle context ) :
  context_( context )
{
}

ActionContextContainer::~ActionContextContainer()
{
}

void ActionContextContainer::report_error( const std::string& error )
{
  if ( this->context_ ) this->context_->report_error( error ); 
}

void ActionContextContainer::report_warning( const std::string& warning )
{
  if ( this->context_ ) this->context_->report_warning( warning ); 
}

void ActionContextContainer::report_message( const std::string& message )
{
  if ( this->context_ ) this->context_->report_message( message ); 
}

void ActionContextContainer::report_result( const ActionResultHandle& result )
{
  if ( this->context_ ) this->context_->report_result( result ); 
}

void ActionContextContainer::report_status( ActionStatus status )
{
  if ( this->context_ )
  {
    this->context_->report_status( status );
    status_ = status;
  }
}

void ActionContextContainer::report_need_resource( NotifierHandle notifier )
{
  if ( this->context_ ) this->context_->report_need_resource( notifier );
}

ActionSource ActionContextContainer::source() const
{
  if ( this->context_ ) return this->context_->source();
  else return ActionSource::COMMANDLINE_E;
}

void ActionContextContainer::report_done()
{
  if ( this->context_ ) this->context_->report_done();
}

Core::NotifierHandle ActionContextContainer::get_resource_notifier()
{
  if ( this->context_ ) return this->context_->get_resource_notifier();
  else return Core::NotifierHandle();
}

Core::ActionResultHandle ActionContextContainer::get_result()
{
  if ( this->context_ ) return this->context_->get_result();
  else return Core::ActionResultHandle();
}

void ActionContextContainer::reset_context()
{
  if ( this->context_ ) this->context_->reset_context();
}

} // end namespace Core
