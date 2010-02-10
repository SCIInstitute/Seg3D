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

#ifndef INTERFACE_QTINTERFACE_QTBRIDGE_H
#define INTERFACE_QTINTERFACE_QTBRIDGE_H

// QT includes/custom widget
#include <QtGui>
#include <Interface/ToolInterface/CustomWidgets/SliderSpinComboInt.h>
#include <Interface/ToolInterface/CustomWidgets/SliderSpinComboDouble.h>

// Application includes
#include <Application/State/StateValue.h>
#include <Application/State/StateOption.h>
#include <Application/State/StateVector.h>
#include <Application/State/StateRangedValue.h>


namespace Seg3D {

// CLASS QTBRIDGE:
// This class provides bridges between widgets and state variables

class QtBridge : public boost::noncopyable {

// -- widget connectors --
  public:
    
    // Connect a QCheckBox to StateValue<bool>
    static bool connect(QCheckBox* qcheckbox, StateBoolHandle& state_handle);
    
    // Connect a SliderSpinCombo to StateRangedIntValue
    static bool connect(SliderSpinComboInt* sscombo, StateRangedIntHandle& state_handle);

    // Connect a SliderSpinCombo to StateRangedDoubleValue
    static bool connect(SliderSpinComboDouble* sscombo, StateRangedDoubleHandle& state_handle);
    
    // Connect a QComboBox to StateValue<int>
    static bool connect(QComboBox* qcombobox, StateOptionHandle& state_handle);

    // Connect QToolButton & QPushButtons
    static bool connect(QToolButton* qtoolbutton, boost::function<void ()> function);
    static bool connect(QPushButton* qpushbutton, boost::function<void ()> function);


// -- menu connectors --
  public:
  
    // Connect menu action to dispatcher

    static bool connect(QAction* qaction, boost::function<void ()> function);
    
};


} // end namespace Seg3D

#endif
