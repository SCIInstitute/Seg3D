/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef QTUTILS_UTILS_QTAPPLICATION_H
#define QTUTILS_UTILS_QTAPPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Qt includes
#include <QApplication>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/State/StateHandler.h>

// QtUtils includes
#include <QtUtils/Utils/QtRenderResources.h>


namespace QtUtils
{

// -- QtUtils class (singleton) --
// This class is a wrapper around the QApplication class

// Forward declaration
class QtApplication;
class QtApplicationPrivate;

// Class definition
class QtApplication : public boost::noncopyable
{
  CORE_SINGLETON( QtApplication );
  
  // -- constructor --
private:
  QtApplication();

  // -- entry point --

public:
  /// SETUP:
  /// Setup the interface context
  bool setup( int& argc, char **argv );

  /// EXEC:
  /// Start the interface execution
  /// This effectively will start the program.
  bool exec();

  void setExternalInstance(QApplication* app);

  // -- accessors --
public:
  /// QT_APPLICATION:
  /// Get the pointer to the main QT Application
  QApplication* qt_application();

  /// QT_RENDERRESOURCES_CONTEXT:
  /// Get the handle to the renderresources
  QtRenderResourcesContextHandle qt_renderresources_context();

  // -- Signals --
public:
  /// ENTER_SIZE_MOVE_SIGNAL:
  /// Triggered when the user starts resizing the application window.
  boost::signals2::signal< void () > enter_size_move_signal_;

  /// EXIT_SIZE_MOVE_SIGNAL:
  /// Triggered when the user stops resizing the application window.
  boost::signals2::signal< void () > exit_size_move_signal_;

  /// OSX_FILE_OPEN_EVENT_SIGNAL:
  /// Triggered via Mac OS X file associations
  boost::signals2::signal< void (std::string) > osx_file_open_event_signal_;

private:
  friend class QtApplicationPrivate;
  QtApplicationPrivate* private_;
};

} // end namespace QtUtils

#endif
