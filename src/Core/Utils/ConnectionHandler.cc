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

#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Log.h>

namespace Core
{

ConnectionHandlerConnection::~ConnectionHandlerConnection()
{
}

class ConnectionHandlerPrivate : public boost::noncopyable
{
public:
  // List of boost connections that need to be disconnected
  typedef std::list< boost::signals2::connection > boost_connections_type;
  boost_connections_type boost_connections_;
  
  // List of other connection that need to be disconnected
  typedef std::list<ConnectionHandlerConnectionHandle> other_connections_type;
  other_connections_type other_connections_;
};

ConnectionHandler::ConnectionHandler() :
  private_( new ConnectionHandlerPrivate )
{
}

ConnectionHandler::~ConnectionHandler()
{
  if ( ! this->private_->boost_connections_.empty() ||
     ! this->private_->other_connections_.empty() )
  {
    CORE_LOG_ERROR( "disconnect_all() needs to be called in the destructor of the derived class" );
    // Correct the problem, although this is not fully thread safe
    this->disconnect_all();
  }
}

void ConnectionHandler::add_connection( const boost::signals2::connection& connection )
{
  this->private_->boost_connections_.push_back( connection );
}

void ConnectionHandler::add_connection( const ConnectionHandlerConnectionHandle& connection )
{
  this->private_->other_connections_.push_back( connection );
}

void ConnectionHandler::disconnect_all()
{
  {
    ConnectionHandlerPrivate::boost_connections_type::iterator it = 
      this->private_->boost_connections_.begin();
    ConnectionHandlerPrivate::boost_connections_type::iterator it_end = 
      this->private_->boost_connections_.end();

    // disconnect all the connections
    while ( it != it_end )
    {
      ( *it ).disconnect();
      ++it;
    }
  }
  
  {
    ConnectionHandlerPrivate::other_connections_type::iterator it = 
      this->private_->other_connections_.begin();
    ConnectionHandlerPrivate::other_connections_type::iterator it_end = 
      this->private_->other_connections_.end();

    // disconnect all the connections
    while ( it != it_end )
    {
      ( *it )->disconnect();
      ++it;
    }
  }
  
  // clear the connections
  this->private_->boost_connections_.clear();
  this->private_->other_connections_.clear();
}

} // end namespace Core
