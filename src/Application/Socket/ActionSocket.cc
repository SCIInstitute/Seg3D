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

// Boost includes
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/ref.hpp>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Log.h>
#include <Core/Python/PythonInterpreter.h>

// Application includes
#include <Application/Socket/ActionSocket.h>

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( ActionSocket );

ActionSocket::ActionSocket() :
  action_socket_thread_( 0 )
{
}

void ActionSocket::start( int portnum )
{
  // Create new thread for running socket code
  action_socket_thread_ = new boost::thread( boost::bind( &ActionSocket::run_action_socket,
      portnum ) );
}

ActionSocket::~ActionSocket()
{
}

static void WriteOutputToSocket( boost::asio::ip::tcp::socket& socket, std::string output )
{
  try
  {
    boost::asio::write( socket, boost::asio::buffer( output ) );
  }
  catch ( ... ) {}
}

static void WritePromptToSocket( boost::asio::ip::tcp::socket& socket, std::string output )
{
  output = "\r\n" + output;
  WriteOutputToSocket( socket, output );
}

void ActionSocket::run_action_socket( int portnum )
{
  boost::asio::io_service io_service;

  boost::asio::ip::tcp::acceptor acceptor( io_service );
  try
  {
    boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), portnum );
    acceptor.open( endpoint.protocol() );
    acceptor.set_option( boost::asio::socket_base::reuse_address( true ) );
    acceptor.bind( endpoint );
    acceptor.listen();
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "Failed to open socket on port " + 
      Core::ExportToString( portnum ) + "." );
    return;
  }

  portnum = acceptor.local_endpoint().port();

  // Write the port number out to file
  try
  {
    if ( boost::filesystem::exists( "port" ) )
    {
      boost::filesystem::remove( "port" );
    }
  }
  catch ( ... ) 
  {
    CORE_LOG_ERROR( "Couldn't remove port file." );
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

  while ( 1 )
  {
    boost::asio::ip::tcp::socket socket( io_service );
    try
    {
      acceptor.accept( socket );
    }
    catch ( ... )
    {
      break;
    }

    // Connect to PythonInterpreter signals
    connection_handler.add_connection( Core::PythonInterpreter::Instance()->prompt_signal_.connect( 
      boost::bind( &WritePromptToSocket, boost::ref( socket ), _1 ) ) );
    connection_handler.add_connection( Core::PythonInterpreter::Instance()->error_signal_.connect( 
      boost::bind( &WriteOutputToSocket, boost::ref( socket ), _1 ) ) );
    connection_handler.add_connection( Core::PythonInterpreter::Instance()->output_signal_.connect( 
      boost::bind( &WriteOutputToSocket, boost::ref( socket ), _1 ) ) );

    CORE_LOG_MESSAGE( "Socket connected." );
    try
    {
      boost::asio::write( socket, boost::asio::buffer( std::string( "Welcome to Seg3D\r\n" ) ) );
    }
    catch ( ... ) {}

    boost::system::error_code read_ec;
    while ( !read_ec )
    {
      boost::asio::streambuf buffer;

      boost::asio::read_until( socket, buffer, std::string( "\r\n" ), read_ec );
      std::istream is( &buffer );

      std::string action_string;
      std::getline( is, action_string );

      if ( !read_ec )
      {
        if ( action_string == "exit\r" )
        {
          try
          {
            boost::asio::write( socket, boost::asio::buffer( "Goodbye!\r\n" ) );
            socket.close();
          }
          catch ( ... ) {}
          break;
        }

        Core::PythonInterpreter::Instance()->run_string( action_string );
      }
    }

    CORE_LOG_MESSAGE( "Socket disconnected." );
    connection_handler.disconnect_all();
  }

  CORE_LOG_MESSAGE( "Stopped listening on port " + Core::ExportToString( portnum ) );
}

} // end namespace Core

