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

#ifdef _MSC_VER
#pragma warning( disable: 4244 4267 )
#endif

#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>

#include <string>

#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>

#include <Application/PythonModule/ActionModule.h>
#include <Application/PythonModule/PythonInterpreter.h>

//static boost::python::object post_and_wait_action( const std::string& action_name, 
//                            const boost::python::dict args )
//{
//  std::string action_str( action_name );
//  boost::python::list arg_items = args.items();
//  boost::python::ssize_t num_of_args = boost::python::len( arg_items );
//  for ( boost::python::ssize_t i = 0; i < num_of_args; ++i )
//  {
//    action_str += ( " " + boost::python::extract< std::string >( 
//      boost::python::str( arg_items[ i ][ 0 ] ) + "=" + 
//      boost::python::str( arg_items[ i ][ 1 ] ) )() );
//  }
//
//  return boost::python::object();
//}

static boost::python::object post_and_wait_action( boost::python::tuple args,
                          boost::python::dict kw_args )
{
  std::string action_str;
  boost::python::ssize_t num_of_args = boost::python::len( args );
  for ( boost::python::ssize_t i = 0; i < num_of_args; ++i )
  {
    action_str += boost::python::extract< std::string >( 
      boost::python::str( args[ i ] ) + " " )();
  }

  boost::python::list kw_pairs = kw_args.items();
  num_of_args = boost::python::len( kw_pairs );
  for ( boost::python::ssize_t i = 0; i < num_of_args; ++i )
  {
    action_str += boost::python::extract< std::string >( 
      boost::python::str( kw_pairs[ i ][ 0 ] ) + "=" + 
      boost::python::str( kw_pairs[ i ][ 1 ] ) + " " )();
  }

  std::string action_error;
  std::string action_usage;

  Core::ActionHandle action;

  if ( !Core::ActionFactory::CreateAction( action_str, action, action_error, action_usage ) )
  {
    //action_error += "\n";
    //Seg3D::PythonInterpreter::Instance()->error_signal_( action_error );
    PyErr_SetString( PyExc_Exception, action_error.c_str() );
  }
  else
  {
    Seg3D::PythonActionContextHandle action_context = Seg3D::PythonInterpreter::GetActionContext();

    bool repeat = true;
    do 
    {
      Core::ActionDispatcher::PostAndWaitAction( action, Core::ActionContextHandle( action_context ) );
      Core::ActionStatus action_status = action_context->status();
      Core::ActionResultHandle action_result = action_context->get_result();
      Core::NotifierHandle resource_notifier = action_context->get_resource_notifier();
      Core::ActionSource action_source = action_context->source();
      std::string err_msg = action_context->get_error_message();
      action_context->reset_context();

      if ( action_status == Core::ActionStatus::SUCCESS_E )
      {
        if ( action_result )
        {
          return boost::python::object( *action_result );
        }
        repeat = false;
      }
      else
      {
        if ( resource_notifier && ( action_source == Core::ActionSource::SCRIPT_E ||
          action_source == Core::ActionSource::PROVENANCE_E ) )
        {
          resource_notifier->wait();
          continue;
        }
        PyErr_SetString( PyExc_Exception, err_msg.c_str() );
        repeat = false;
      }
    } while ( repeat );
  }

  return boost::python::object();
}

static PyObject* seg3d_action( PyObject *self, PyObject *args )
{
  const char *params;

  if ( !PyArg_ParseTuple( args, "s", &params ) )
    return NULL;

  std::string action_string( params );
  std::string action_error;
  std::string action_usage;

  Core::ActionHandle action;
  
  if ( !Core::ActionFactory::CreateAction( action_string, action, action_error, action_usage ) )
  {
    PyErr_SetString( PyExc_Exception, action_error.c_str() );
    return NULL;
  }

  Core::ActionContextHandle action_context = Seg3D::PythonInterpreter::GetActionContext();
  Core::ActionDispatcher::PostAction( action, action_context );
  Py_RETURN_NONE;
}

static PyMethodDef seg3d_methods[] = {
  {"action",  seg3d_action, METH_VARARGS,
  "Issue an action."},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef seg3d_module = {
   PyModuleDef_HEAD_INIT,
   "seg3d",   /* name of module */
   NULL,    /* module documentation, may be NULL */
   -1,      /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   seg3d_methods
};

//PyObject* PyInit_seg3d()
//{
//  return PyModule_Create( &seg3d_module );
//}

void register_python_action_module()
{
  boost::python::def( "action", boost::python::raw_function( post_and_wait_action, 1 ) );
  //boost::python::def( "action", seg3d_action );
}
