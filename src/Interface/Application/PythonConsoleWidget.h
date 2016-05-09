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

#ifndef INTERFACE_APPLICATION_PYTHONCONSOLEWIDGET_H
#define INTERFACE_APPLICATION_PYTHONCONSOLEWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

// Boost includes
#include <boost/shared_ptr.hpp>

// QT includes
#include <QWidget>

// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Log.h>
#include <Core/Action/Actions.h>

#include <QtUtils/Widgets/QtCustomDialog.h>

#endif

namespace Seg3D
{

class PythonConsoleWidgetPrivate;
typedef boost::shared_ptr< PythonConsoleWidgetPrivate > PythonConsoleWidgetPrivateHandle;

// Forward declaration
class PythonConsoleWidget;

// Class definition
class PythonConsoleWidget : public QtUtils::QtCustomDialog, private Core::ConnectionHandler
{
  Q_OBJECT

  // -- constructor/destructor --
public:
  PythonConsoleWidget( QWidget* parent = 0 );
  virtual ~PythonConsoleWidget();

private:
  PythonConsoleWidgetPrivateHandle private_;
};

} // end namespace Seg3D

#endif
