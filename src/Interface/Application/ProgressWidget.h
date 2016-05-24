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

#ifndef INTERFACE_APPLICATION_LAYERIMPORTERWIDGET_H
#define INTERFACE_APPLICATION_LAYERIMPORTERWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#ifndef Q_MOC_RUN

// Qt includes
#include <QWidget>
#include <QPointer>

// boost includes
#include <boost/smart_ptr.hpp>

// Application Includes
#include <Core/Action/ActionProgress.h>

#endif

namespace Seg3D
{

class ProgressWidgetPrivate;
typedef boost::shared_ptr< ProgressWidgetPrivate > ProgressWidgetPrivateHandle;

class ProgressWidget : public QWidget
{
  // Needed to make it a Qt object
Q_OBJECT

  // -- Constructor / destructor --
public:
  typedef QPointer< ProgressWidget > qpointer_type;

  ProgressWidget( QWidget *parent = 0 );
  virtual ~ProgressWidget();

  // -- Update widget --
public:

  /// UPDATE_PROGRESS:
  /// Update the progress in this dialog
  void update_progress();
  
  /// SETUP_PROGRESS_WIDGET:
  /// this will setup the widget and make all the connections
  void setup_progress_widget( Core::ActionProgressHandle action_progress );
  
  /// CLEANUP_PROGRESS_WIDGET:
  /// this will cleanup the progress widget, disconnect the connections etc.
  void cleanup_progress_widget();

  // -- Slots --
public Q_SLOTS:

  /// INTERUPT:
  /// Interrupt the current process
  void interrupt();

  /// UPDATE_RUNNING:
  /// Update the running circle
  void update_running();
  
  // -- Internals --
private:
  ProgressWidgetPrivateHandle private_;
};

} //end namespace Seg3D

#endif
