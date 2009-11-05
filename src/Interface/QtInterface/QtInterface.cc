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
#include <Core/Utils/Log.h>

// Application Layer includes
#include <Application/Application/Application.h>

// Interface includes
#include <Interface/QtInterface/QtInterface.h>
#include <Interface/QtInterface/QtInterfaceInternal.h>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>


namespace Seg3D {

QtInterface::QtInterface() :
  qt_application_(0)
{
}


bool
QtInterface::setup(int argc, char **argv)
{
  try
  {
    SCI_LOG_DEBUG("Creating QApplication");
    // Main application class
    qt_application_ = new QApplication(argc,argv);

    SCI_LOG_DEBUG("Creating QtInterface Context");
    // Interface class to the main class of the Application layer
    ApplicationContextHandle qt_interface_context = 
              ApplicationContextHandle(new QtInterfaceContext(qt_application_));

    SCI_LOG_DEBUG("Creating QtInterface EventFilter");
    // Filter needed to extract user events inserted in main Qt event loop
    QtInterfaceUserEventFilter* qt_interface_filter = 
                                    new QtInterfaceUserEventFilter(qt_application_);

    SCI_LOG_DEBUG("Install the QtInterface EventFilter");
    // Install the event filter that handles callbacks from other threads
    qt_application_->installEventFilter(qt_interface_filter);

    SCI_LOG_DEBUG("Install the QtInterface Context into the Application layer");
    // Insert the event handler into the application layer
    Application::instance()->install_context(qt_interface_context);    

  }
  catch(...)
  {
    SCI_LOG_ERROR("Qt Interface failed to initialize");
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
    qt_application_->exec();
  }
  catch(...)
  {
    SCI_LOG_ERROR("Main Qt event loop crashed by throwing an exception");
    return (false);
  }
  
  return (true);
}


QtInterface*
QtInterface::instance()
{
  // if no singleton was allocated, allocate it now
  if (!initialized_)   
  {
    //in case multiple threads try to allocate this one at once.
    {
      boost::unique_lock<boost::mutex> lock(instance_mutex_);
      // The first test was not locked and hence not thread safe
      // This one will do a thread-safe allocation of the interface
      // class
      if (instance_ == 0) instance_ = new QtInterface();
    }
    
    {
      // Enforce memory synchronization so the singleton is initialized
      // before we set initialized to true
      boost::unique_lock<boost::mutex> lock(instance_mutex_);
      initialized_ = true;
    }
  }
  return (instance_);
}

// Static variables that are located in Application and that need to be
// allocated here
boost::mutex QtInterface::instance_mutex_;
bool         QtInterface::initialized_ = false;
QtInterface* QtInterface::instance_ = 0;



} // end namespace Seg3D
