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

#ifndef APPLICATION_PYTHONMODULE_ABSTRACTPYTHONTERMINALINTERFACE_H
#define APPLICATION_PYTHONMODULE_ABSTRACTPYTHONTERMINALINTERFACE_H

// Boost includes
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D
{

// Forward declaration
class AbstractPythonTerminalInterface;
typedef boost::shared_ptr< AbstractPythonTerminalInterface > AbstractPythonTerminalInterfaceHandle;

// Class AbstractPythonTerminalInterface:
// Defines interfaces that a subclass must define in order to function as a python terminal interface.
class AbstractPythonTerminalInterface : public boost::noncopyable
{
  // -- I/O functions --
public:
  // READ:
  // Read and return up to n bytes, or if n is negative, read until EOF is reached.
  virtual std::string read( int n );

  // WRITE:
  // Write data out to the stream, and return the number of bytes written.
  virtual int write( std::string data );
};

} // end namespace Seg3D

#endif
