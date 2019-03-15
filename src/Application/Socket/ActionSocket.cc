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

#ifdef _MSC_VER
#pragma warning( disable: 4244 4267 )
#endif

// STL includes
#include <fstream>
#include <sstream>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Log.h>
#include <Core/Python/PythonInterpreter.h>

// Boost includes
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/ref.hpp>
#include <boost/system/system_error.hpp>

// Application includes
#include <Application/Socket/ActionSocket.h>

namespace Seg3D
{

using namespace boost::system;

CORE_SINGLETON_IMPLEMENTATION( ActionSocket );

ActionSocket::ActionSocket() :
  action_socket_thread_( 0 )
{
}

void ActionSocket::start( int portnum )
{
  // Create new thread for running socket code
  this->action_socket_thread_ = new boost::thread(
    boost::bind( &ActionSocket::run_action_socket, portnum ) );
}

ActionSocket::~ActionSocket()
{
}

static void WriteOutputToSocket( boost::asio::ip::tcp::socket& socket, std::string output )
{
  error_code ec;
  boost::asio::write( socket, boost::asio::buffer( output ), ec );

  // ignore errors in release builds
#ifndef NDEBUG
  if ( ec.value() == errc::broken_pipe )
  {
    CORE_LOG_DEBUG( "write to socket failed: broken pipe" );
  }
  else if ( ec )
  {
    std::ostringstream oss;
    oss << "write to socket failed: " << ec.category().name() << " (" << ec.value() << "): " << ec.message();
    CORE_LOG_ERROR( oss.str() );
  }
#endif
}

static void WriteErrorToSocket( boost::asio::ip::tcp::socket& socket, std::string output)
{
  // TODO: trying to differentiate from sending output, since error output
  //       from python interpreter is probably better logged by Seg3D
  // TODO: revisit and come up with better message to client
  error_code ec;
  boost::asio::write( socket, boost::asio::buffer( "error\r\n" ), ec );

  // ignore errors in release builds
#ifndef NDEBUG
  {
    std::ostringstream oss;
    oss << "Error output from Python interpreter: [" << output << "]";
    CORE_LOG_DEBUG( oss.str() );
  }

  if ( ec.value() == errc::broken_pipe )
  {
    CORE_LOG_DEBUG( "write to socket failed: broken pipe" );
  }
  else if ( ec )
  {
    std::ostringstream oss;
    oss << "write to socket failed: " << ec.category().name() << " (" << ec.value() << "): " << ec.message();
    CORE_LOG_ERROR( oss.str() );
  }
#endif
}

static void WritePromptToSocket( boost::asio::ip::tcp::socket& socket, std::string output )
{
  output = "\r\n" + output;
  WriteOutputToSocket( socket, output );
}


void ActionSocket::run_action_socket( int portnum )
{
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portnum));
  portnum = acceptor.local_endpoint().port();

  // Write the port number out to file
  try
  {
    if ( boost::filesystem::exists( "port" ) )
    {
      boost::filesystem::remove( "port" );
    }
  }
  catch ( system_error& error )
  {
    std::ostringstream oss;
    oss << "Couldn't remove port file: " << error.what();
    CORE_LOG_ERROR( oss.str() );
  }

  std::ofstream ofile( "port_tmp" );
  if ( ofile )
  {
    ofile << portnum;
    ofile.close();
    rename( "port_tmp", "port" );
  }

  Core::ConnectionHandler connection_handler;
  CORE_LOG_MESSAGE( "Started listening on port " + Core::ExportToString( portnum ) );

  for (;;)
  {
    boost::asio::ip::tcp::socket socket(io_service);
    error_code ec;
    acceptor.accept(socket, ec);
    if (ec)
    {
      std::ostringstream oss;
      oss << "Could not connect to client: "  << ec.category().name() << " (" << ec.value() << "): " << ec.message();
      CORE_LOG_ERROR( oss.str() );
      return;
    }

    // Connect to PythonInterpreter signals
    connection_handler.add_connection( Core::PythonInterpreter::Instance()->prompt_signal_.connect(
      boost::bind( &WritePromptToSocket, boost::ref( socket ), _1 ) ) );
    connection_handler.add_connection( Core::PythonInterpreter::Instance()->error_signal_.connect(
      boost::bind( &WriteErrorToSocket, boost::ref( socket ), _1 ) ) );
    connection_handler.add_connection( Core::PythonInterpreter::Instance()->output_signal_.connect(
      boost::bind( &WriteOutputToSocket, boost::ref( socket ), _1 ) ) );

    CORE_LOG_MESSAGE( "Socket connected." );

    error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer( std::string( "Welcome to Seg3D\r\n" ) ), ignored_error);

    // read until exit or error
    error_code read_ec;
    while ( ! read_ec )
    {
      boost::asio::streambuf buffer;
      boost::asio::read_until(socket, buffer, "\r\n", read_ec);

      if ( ! read_ec )
      {
        std::istream is(&buffer);
        std::string action_string;
        std::getline(is, action_string);

        if ( action_string == "exit\r" )
        {
          error_code exit_ignored_error;
          boost::asio::write( socket, boost::asio::buffer( "Goodbye!\r\n" ), exit_ignored_error );
          socket.close();
          break;
        }
        else
        {
          Core::PythonInterpreter::Instance()->run_string( action_string );
        }
      }
    }

    CORE_LOG_MESSAGE( "Socket disconnected." );
    connection_handler.disconnect_all();
  }

  CORE_LOG_MESSAGE( "Stopped listening on port " + Core::ExportToString( portnum ) );
}


} // end namespace Core

