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

// Boost includes
#include <boost/asio.hpp>

// Application includes
#include <Application/Action/Actions.h>
#include <Application/Action/ActionSocket.h>

namespace Seg3D {

ActionSocket::ActionSocket(int portnum) :
  action_socket_thread_(0)
{  
  // Create new thread for running socket code
  action_socket_thread_ = 
    new boost::thread(boost::bind(&ActionSocket::run_action_socket,portnum));
}

ActionSocket::~ActionSocket()
{
}

void
ActionSocket::run_action_socket(int portnum)
{
  boost::asio::io_service io_service;

  boost::asio::ip::tcp::acceptor acceptor(io_service, 
      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portnum));
  
  while (1)
  {
    boost::asio::ip::tcp::socket sock(io_service);
    acceptor.accept(sock);
    
    std::vector<char> data(512);
    boost::system::error_code read_ec;
    std::string action;
    
    while (!read_ec)
    {
      boost::asio::streambuf buffer;
    
      boost::asio::read_until(sock,buffer,std::string("\r\n"),read_ec);
      std::istream is(&buffer);
      std::string action;
      is >> action;
      
      if (!read_ec)
      {
        ActionHandle action_handle;
        std::string  error;
        std::string  usage;
        if(!(ActionFactory::Instance()->create_action(action,action_handle,
            error,usage)))
        {
          error += "\r\n";
          boost::asio::write(sock,boost::asio::buffer(error));
        }
        else
        {
        
        }
      }
    }
  }
}

} // end namespace Seg3D

