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

#include <Application/PythonModule/ActionModule.h>

#include <string>

#include <Core/Action/ActionFactory.h>
#include <Core/Action/ActionDispatcher.h>

#include <Application/PythonModule/PythonInterpreter.h>

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

PyObject* PyInit_seg3d()
{
  return PyModule_Create( &seg3d_module );
}