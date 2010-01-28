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

#ifndef INTERFACE_QTINTERFACE_QTAPPLICATION_H
#define INTERFACE_QTINTERFACE_QTAPPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Singleton/Singleton.h>

// Application includes
#include <Application/State/StateHandler.h>

// Interface includes
#include <Interface/QtInterface/QtRenderResources.h>

// Qt includes
#include <QApplication>
#include <QtGui>

namespace Seg3D {

// -- QtInterface class (singleton) --

// This class is a wrapper around the QApplication class

class QtApplication : public boost::noncopyable {

// -- constuctor --
  public:
  
    QtApplication();

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
    // QT_APPLICATION:
    // Get the pointer to the main QT Application
    QApplication* qt_application();

    // QT_RENDERRESOURCES_CONTEXT:
    // Get the handle to the renderresources
    QtRenderResourcesContextHandle qt_renderresources_context();
    
  private:  
    // Main QT application class
    QApplication* qt_application_;

    // Class for managing the opengl rendering resources
    QtRenderResourcesContextHandle qt_renderresources_context_;

// -- Singleton interface --
  public:
    
    // INSTANCE:
    static QtApplication* Instance() { return instance_.instance(); }
    
  private:  
    static Utils::Singleton<QtApplication> instance_;
    
};

} // end namespace Seg3D

#endif
