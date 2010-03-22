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

#ifndef INTERFACE_APPINTERFACE_LAYERIMPORTERWIDGET_H
#define INTERFACE_APPINTERFACE_LAYERIMPORTERWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Qt includes
#include <QWidget>
#include <QDialog>
#include <QPointer>

// boost includes
#include <boost/smart_ptr.hpp>

// Application Includes
#include <Application/Action/ActionProgress.h>

namespace Seg3D
{

class ProgressWidgetPrivate;
typedef boost::shared_ptr< ProgressWidgetPrivate > ProgressWidgetPrivateHandle;

class ProgressWidget : public QDialog
{
  // Needed to make it a Qt object
Q_OBJECT

  // -- Constructor / destructor --
public:
  typedef QPointer< ProgressWidget > qpointer_type;

  ProgressWidget( ActionProgressHandle action_progress, QWidget *parent = 0 );
  virtual ~ProgressWidget();

  // -- Update widget --
public:

  // UPDATE_PROGRESS:
  // Update the progress in this dialog
  void update_progress();

  // -- Slots --
public Q_SLOTS:

  // INTERUPT:
  // Interrupt the current process
  void interrupt();

  // UPDATE_RUNNING:
  // Update the running circle
  void update_running();
  
  // -- Internals --
private:
  ActionProgressHandle action_progress_;
  
  ProgressWidgetPrivateHandle private_;
};

} //endnamespace Seg3d

#endif
