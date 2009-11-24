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

// Core includes
#include <Utils/Core/Log.h>

// Application Layer includes
#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>

// Interface includes
#include <Interface/QtInterface/QtInterface.h>
#include <Interface/QtInterface/QtInterfaceInternal.h>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>


namespace Seg3D {

QtInterface::QtInterface() :
  qapplication_(0)
{
}


bool
QtInterface::setup(int argc, char **argv)
{
  try
  {
    // Main application class
    SCI_LOG_DEBUG("Creating QApplication");
    qapplication_ = new QApplication(argc,argv);

    
    // Interface class to the main class of the event handler layer
    SCI_LOG_DEBUG("Creating QtEventHandlerContext");
    Utils::EventHandlerContextHandle qt_eventhandler_context = 
        Utils::EventHandlerContextHandle(new QtEventHandlerContext(qapplication_));


    // Insert the event handler into the application layer
    SCI_LOG_DEBUG("Install the QtEventHandlerContext into the Interface layer");
    Interface::instance()->install_eventhandler_context(qt_eventhandler_context);   
    Interface::instance()->start_eventhandler(); 
  }
  catch(...)
  {
    SCI_LOG_ERROR("QtInterface failed to initialize");
    return (false);
  }
  
  return (true);
}


bool
QtInterface::exec()
{
  try
  {
    SCI_LOG_DEBUG("Starting Qt main event loop");
    qapplication_->exec();
    
    delete qapplication_;
    qapplication_ = 0;
  }
  catch(...)
  {
    SCI_LOG_ERROR("Main Qt event loop crashed by throwing an exception");
    return (false);
  }
  
  return (true);
}


void QtCheckBoxSignal(QPointer<QCheckBox> qobject_ptr, bool state)
{
  if (!(Interface::instance()->is_interface_thread()))
  {
    PostInterface(boost::bind(&QtCheckBoxSignal,qobject_ptr,state));
    return;
  }

  if (qobject_ptr) qobject_ptr->setChecked(state); 
}

bool
QtInterface::connect(QCheckBox* qcheckbox, State<bool>::Handle& state_handle)
{
  // Connect the dispatch into the StateVariable (with auxillary object)
  QtCheckBoxSlot* slot = new QtCheckBoxSlot(qcheckbox,state_handle);
    
  // Connect the state signal back into the Qt Variable  
  state_handle->connect(boost::bind(&QtCheckBoxSignal,qcheckbox,_1));
  
  return (true);
}


// Singleton instantiation
Utils::Singleton<QtInterface> QtInterface::instance_;

} // end namespace Seg3D
