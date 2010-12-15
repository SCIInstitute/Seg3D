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

#include <Python.h>

#include <boost/thread/mutex.hpp>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Lockable.h>

#include <Application/PythonModule/ActionModule.h>
#include <Application/PythonModule/PythonInterpreter.h>

// Py_IgnoreEnvironmentFlag
// defined in pythonrun.c
extern int Py_IgnoreEnvironmentFlag;

namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( PythonInterpreter );

class PythonInterpreterPrivate : public Core::Lockable
{
public:
  // The name of the executable
  wchar_t* program_name_;

  bool initialized_;
  bool terminal_running_;
  PythonActionContextHandle action_context_;

  // Condition variable to make sure the PythonInterpreter thread has 
  // completed initialization before continuing the main thread.
  boost::condition_variable thread_condition_variable_;
};
  
PythonInterpreter::PythonInterpreter() :
  Core::EventHandler(),
  private_( new PythonInterpreterPrivate )
{
  this->private_->program_name_ = L"";
  this->private_->initialized_ = false;
  this->private_->terminal_running_ = false;
  this->private_->action_context_.reset( new PythonActionContext );
}

PythonInterpreter::~PythonInterpreter()
{
  Py_Finalize();
}

void PythonInterpreter::initialize_eventhandler()
{
  PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
  PyImport_AppendInittab( "seg3d", PyInit_seg3d );
  Py_SetProgramName( this->private_->program_name_ );
  Py_IgnoreEnvironmentFlag = 1;
  Py_Initialize();
  PyRun_SimpleString( "import seg3d\n"
    "from seg3d import *\n" );
  this->private_->thread_condition_variable_.notify_one();
}

void PythonInterpreter::initialize( wchar_t* program_name )
{
  this->private_->program_name_ = program_name;

  PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
  this->start_eventhandler();
  this->private_->thread_condition_variable_.wait( lock );
  this->private_->initialized_ = true;
}

void PythonInterpreter::run_string( std::string command )
{
  {
    PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
    if ( !this->private_->initialized_ )
    {
      CORE_THROW_LOGICERROR( "The python interpreter hasn't been initialized!" );
    }
  }
  
  if ( !this->is_eventhandler_thread() )
  {
    this->post_event( boost::bind( &PythonInterpreter::run_string, this, command ) );
    return;
  }
  
  command += "\n";
  PyRun_SimpleString( command.c_str() );
}

void PythonInterpreter::run_file( std::string file_name )
{
  {
    PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
    if ( !this->private_->initialized_ )
    {
      CORE_THROW_LOGICERROR( "The python interpreter hasn't been initialized!" );
    }
  }

  if ( !this->is_eventhandler_thread() )
  {
    this->post_event( boost::bind( &PythonInterpreter::run_file, this, file_name ) );
    return;
  }

  FILE* fp = fopen( file_name.c_str(), "r" );
  if ( fp != 0 )
  {
    PyRun_SimpleFileEx( fp, file_name.c_str(), 1 );
  }
}

void PythonInterpreter::start_terminal()
{
  {
    PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
    if ( !this->private_->initialized_ )
    {
      CORE_THROW_LOGICERROR( "The python interpreter hasn't been initialized!" );
    }

    if ( this->private_->terminal_running_ )
    {
      return;
    }
  }

  if ( !this->is_eventhandler_thread() )
  {
    this->post_event( boost::bind( &PythonInterpreter::start_terminal, this ) );
    return;
  }
  
  PythonInterpreterPrivate::lock_type lock( this->private_->get_mutex() );
  this->private_->terminal_running_ = true;
  lock.unlock();

  //PyRun_SimpleString("import sys\n");
  //PyRun_SimpleString("import io\n");
  //PyRun_SimpleString("sys.stdout=io.open('CONOUT$', 'wt')\n");
  //PyObject* sys = PyImport_ImportModule("sys");
  //PyObject* io = PyImport_ImportModule("io");
  //PyObject* pystdout = PyObject_CallMethod(io, "open", "ss", "CONOUT$", "wt");
  //if (-1 == PyObject_SetAttrString(sys, "stdout", pystdout)) {
  //  /* Announce your error to the world */
  //}
  //Py_DECREF(sys);
  //Py_DECREF(io);
  //Py_DECREF(pystdout);
  //wchar_t** argv = new wchar_t*[ 2 ];
  //argv[ 0 ] = this->private_->program_name_;
  //argv[ 1 ] = L"";
  //Py_Main( 1, argv );
  //delete[] argv;
  
  lock.lock();
  this->private_->terminal_running_ = false;
}

PythonActionContextHandle PythonInterpreter::get_action_context()
{
  return this->private_->action_context_;
}

PythonActionContextHandle PythonInterpreter::GetActionContext()
{
  return Instance()->get_action_context();
}

} // end namespace Core

