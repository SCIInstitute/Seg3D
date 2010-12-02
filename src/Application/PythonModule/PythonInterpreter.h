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

#ifndef APPLICATION_PYTHONMODULE_PYTHONINTERPRETER_H
#define APPLICATION_PYTHONMODULE_PYTHONINTERPRETER_H

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/EventHandler/EventHandler.h>

// Application includes
#include <Application/PythonModule/PythonActionContext.h>

namespace Seg3D
{

// CLASS PYTHONINTERPRETER
// A wrapper class of the python interpreter.
// It calls the python interpreter on a separate thread.

// Forward declaration
class PythonInterpreter;
class PythonInterpreterPrivate;
typedef boost::shared_ptr< PythonInterpreterPrivate > PythonInterpreterPrivateHandle;

// Class definition
class PythonInterpreter : private Core::EventHandler
{
  CORE_SINGLETON( PythonInterpreter );
  
  // -- constructor/destructor --
private:
  PythonInterpreter();
  virtual ~PythonInterpreter();

  // -- overloaded event handler --
private:
  // INITIALIZE_EVENTHANDLER:
  // This function initializes the event handler associated with the singleton
  // class. It initializes the python interpreter.
  virtual void initialize_eventhandler();

public:
  void initialize( wchar_t* program_name );
  PythonActionContextHandle get_action_context();

  void run_string( std::string command );
  void run_file( std::string file_name );
  void start_terminal();

private:
  PythonInterpreterPrivateHandle private_;

public:
  static PythonActionContextHandle GetActionContext();
};

} // end namespace Core

#endif
