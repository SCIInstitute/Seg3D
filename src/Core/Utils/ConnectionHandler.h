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

#ifndef CORE_UTILS_CONNECTIONHANDLER_H
#define CORE_UTILS_CONNECTIONHANDLER_H

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/signals2/connection.hpp>

// STL includes
#include <list>

namespace Core
{

class ConnectionHandler;
class ConnectionHandlerPrivate;
class ConnectionHandlerConnection;
typedef boost::shared_ptr<ConnectionHandlerPrivate> ConnectionHandlerPrivateHandle;
typedef boost::shared_ptr<ConnectionHandlerConnection> ConnectionHandlerConnectionHandle;
typedef ConnectionHandlerConnectionHandle ConnectionHandle;

class ConnectionHandlerConnection : public boost::noncopyable 
{
public:
  virtual ~ConnectionHandlerConnection();

  // DISCONNECT:
  /// This function is overloaded to disconnect the connection
  virtual void disconnect() = 0;
};

// CLASS CONNECTIONHANDLER:
/// A simple class for managing connections

/// NOTE: To use this class the derived class needs to call disconnect_all()
/// in the distructor before any of the structures are deleted.

class ConnectionHandler : public boost::noncopyable
{

  // -- constructor / destructor --
public:
  ConnectionHandler();
  virtual ~ConnectionHandler();

  // -- connection management --
public:

  // ADD_CONNECTION:
  /// Add a connection into the list so it can be deleted when disconnect is
  /// called
  void add_connection( const boost::signals2::connection& connection );

  // ADD_CONNECTION:
  /// Add a connection into the list so it can be deleted when disconnect is
  /// called
  void add_connection( const ConnectionHandlerConnectionHandle& connection );


  // DISCONNECT_ALL:
  /// Disconnect alal the connections that are stored in this class.
  /// NOTE: this function needs to be called in the most derived class
  void disconnect_all();

  // -- internals --
private:
  ConnectionHandlerPrivateHandle private_;
};

} // end namespace Core

#endif
