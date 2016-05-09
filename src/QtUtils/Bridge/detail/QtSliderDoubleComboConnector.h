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

#ifndef QTUTILS_BRIDGE_DETAIL_QTSLIDERDOUBLECOMBOCONNECTOR_H
#define QTUTILS_BRIDGE_DETAIL_QTSLIDERDOUBLECOMBOCONNECTOR_H

#ifndef Q_MOC_RUN

#include <QPointer>

#include <Core/State/StateRangedValue.h>

#include <QtUtils/Widgets/QtSliderDoubleCombo.h>
#include <QtUtils/Bridge/detail/QtConnectorBase.h>

#endif

namespace QtUtils
{

class QtSliderDoubleComboConnector : public QtConnectorBase
{
  Q_OBJECT

public:
  QtSliderDoubleComboConnector( QtSliderDoubleCombo* parent, 
    Core::StateRangedDoubleHandle& state, bool blocking = true );

  virtual ~QtSliderDoubleComboConnector();

  // -- slot functions for boost signals --
private:
  static void SetValue( QPointer< QtSliderDoubleComboConnector > qpointer,
    double val, Core::ActionSource source );

  static void SetRange( QPointer< QtSliderDoubleComboConnector > qpointer,
    double min_val, double max_val, Core::ActionSource source );
  
  static void SetStep( QPointer< QtSliderDoubleComboConnector > qpointer,
    double step, Core::ActionSource source );

  // -- slot functions for Qt signals --
private Q_SLOTS:
  void set_state_value( double val );
  void set_state_range( double min_val, double max_val );

private:
  QtSliderDoubleCombo* parent_;
  Core::StateRangedDoubleHandle state_;
};

}

#endif
