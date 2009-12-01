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

#ifndef INTERFACE_QTINTERFACE_QTINTERFACE_H
#define INTERFACE_QTINTERFACE_QTINTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QApplication>
#include <QtGui>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Singleton/Singleton.h>

// Application includes
#include <Application/State/StateHandler.h>


namespace Seg3D {

// -- QtInterface class (singleton) --

// This class is a wrapper around the QApplication class

class QtInterface : public boost::noncopyable {

// -- constuctor --
  public:
  
    QtInterface();

// -- entry point --

  public:
    // SETUP:
    // Setup the interface context
    bool setup(int argc, char **argv);

    // EXEC:
    // Start the interface execution
    // This effectively will start the program.
    bool exec();

// -- accessors --
  public:
    // GET_QAPPLICATION:
    // Get the pointer to the main qt application
    // NOTE: This function should only be called from the interface thread
    QApplication* get_qapplication() { return qapplication_; }

  private:  
    // main QT application class
    QApplication* qapplication_;


// -- widget connectors --
  public:
    
    // CONNECT:
    // A series of helper functions to directly connect a widget with its underlying
    // State variable.
    
    static bool connect(QCheckBox* qcheckbox, StateValue<bool>::Handle& state_handle);



// -- Singleton interface --
  public:
    
    // INSTANCE:
    static QtInterface* instance() { instance_.instance(); }
    
  private:  
    static Utils::Singleton<QtInterface> instance_;
};

} // end namespace Seg3D

#endif
