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

#ifndef QTUTILS_BRIDGE_DETAIL_QTLISTWIDGETCONNECTOR_H
#define QTUTILS_BRIDGE_DETAIL_QTLISTWIDGETCONNECTOR_H

#ifndef Q_MOC_RUN

#include <QListWidget>
#include <QPointer>

#include <Core/State/StateLabeledMultiOption.h>
#include <Core/State/StateVector.h>

#include <QtUtils/Bridge/detail/QtConnectorBase.h>

#endif

namespace QtUtils
{

class QtListWidgetConnector : public QtConnectorBase
{
  Q_OBJECT

public:
  QtListWidgetConnector( QListWidget* parent, Core::StateLabeledMultiOptionHandle& state,
    bool blocking = true );
  QtListWidgetConnector( QListWidget* parent, Core::StateStringVectorHandle& state ); 

  virtual ~QtListWidgetConnector();

  // -- Slot functions for boost signals --
  // NOTE: These functions need to be static because they might be called outside the 
  // main Qt thread, when the Qt object pointed to by qpointer might no longer exist.
private:  
  static void SetListWidgetSelectedItems( QPointer< QtListWidgetConnector > qpointer, 
    std::vector< std::string > selections, Core::ActionSource source );

  static void UpdateListWidgetOptionItems( QPointer< QtListWidgetConnector > qpointer );
  static void UpdateListWidgetStringItems( QPointer< QtListWidgetConnector > qpointer );

  // -- Slot functions for Qt signals --
private Q_SLOTS:
  void set_state();

private:
  QListWidget* parent_;
  Core::StateLabeledMultiOptionHandle option_state_;
  Core::StateStringVectorHandle string_vector_state_;
};

} // end namespace QtUtils

#endif
