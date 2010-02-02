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

#include <Utils/Core/ConnectionHandler.h>
#include <Utils/Core/Log.h>


namespace Utils {

ConnectionHandler::ConnectionHandler()
{
}

ConnectionHandler::~ConnectionHandler()
{
  if (!connections_.empty())
  {
    SCI_LOG_ERROR("disconnect_all() needs to be called in the destructor of the derived class");
    // Correct the problem, although this is not fully thread safe
    disconnect_all();
  }
}

void 
ConnectionHandler::add_connection(const boost::signals2::connection& connection)
{
  connections_.push_back(connection);
}

void
ConnectionHandler::disconnect_all()
{
  connections_type::iterator it = connections_.begin();
  connections_type::iterator it_end = connections_.end();

  // disconnect all the connections
  while(it != it_end)
  {
    (*it).disconnect();
    ++it;
  }

  // clear the connections
  connections_.clear();
}

} // end namespace Utils
