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

class ActionSocketContext;

typedef boost::shared_ptr<ActionSocketContext> ActionSocketContextHandle;

class ActionSocketContext : public ActionContext {

// -- Constructor/destructor --
  public:
    // Wrap a context around an action
    ActionSocketContext() {}
 
    // Virtual destructor for memory management
    virtual ~ActionSocketContext() {}

// -- Reporting functions --

    virtual void report_error(const std::string& error)
    {
      message_ += std::string("ERROR: ")+error+std::string("\r\n");
    }

    virtual void report_warning(const std::string& warning)
    {
      message_ += std::string("WARNING: ")+warning+std::string("\r\n");
    }

    virtual void report_message(const std::string& message)
    {
      message_ += std::string("MESSAGE: ")+message+std::string("\r\n");
    }

    virtual void report_result(const ActionResultHandle& result)
    {
      message_ += std::string("RESULT: ")+result->export_to_string()+std::string("\r\n");
    }

// -- Source information --

    // The action is run from a script: the interface needs to be updated.
    virtual bool from_script() const { return true; }

// -- ActionSocket specific function --

    void reset() { message_.clear(); }
    std::string message() const { return message_; }

  private:
    std::string message_;
};

ActionSocket::ActionSocket() :
  action_socket_thread_(0)
{  
}

void
ActionSocket::start(int portnum)
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
    boost::asio::ip::tcp::socket socket(io_service);
    acceptor.accept(socket);
    
    boost::asio::write(socket,boost::asio::buffer(std::string("Welcome to Seg3D\r\n")));
    
    std::vector<char> data(512);
    boost::system::error_code read_ec;
    std::string action;
    
    ActionSocketContextHandle context(new ActionSocketContext);
    
    while (!read_ec)
    {
      boost::asio::streambuf buffer;
    
      boost::asio::read_until(socket,buffer,std::string("\r\n"),read_ec);
      std::istream is(&buffer);
      
      std::string action_string;
      std::getline(is,action_string);
      
      if (!read_ec)
      {
        ActionHandle action;
        std::string  error;
        std::string  usage;
        if(!(ActionFactory::Instance()->create_action(action_string,action,
            error,usage)))
        {
          error += "\r\n";
          boost::asio::write(socket,boost::asio::buffer(error));
        }
        else
        {
          context->reset();
          PostAndWaitAction(action,context);
          std::string message = context->message();
          boost::asio::write(socket,boost::asio::buffer(message));
        }
      }
    }
  }
}

// Singleton interface needs to be defined somewhere
Utils::Singleton<ActionSocket> ActionSocket::instance_;

} // end namespace Seg3D

