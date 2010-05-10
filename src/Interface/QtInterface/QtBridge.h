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

#include <Interface/ToolInterface/CustomWidgets/SliderIntCombo.h>
#include <Interface/ToolInterface/CustomWidgets/SliderDoubleCombo.h>
#include <Interface/AppInterface/ColorBarWidget.h>
#include <Interface/AppPreferences/ColorButton.h>

// Application includes
#include <Core/State/StateValue.h>
#include <Core/State/StateOption.h>
#include <Core/State/StateVector.h>
#include <Core/State/StateRangedValue.h>
#include <Core/State/StateAlias.h>

namespace Seg3D
{

// CLASS QTBRIDGE:
// This class provides bridges between widgets and state variables

class QtBridge : public boost::noncopyable
{

  // -- widget/menu connectors --
public:

  // Connect a QCheckBox to StateValue<bool>
  static bool Connect( QCheckBox* qcheckbox, Core::StateBoolHandle& state_handle );
  
  // Coonnect a SliderIntCombo to a StateRangedIntValue
  static bool Connect( ColorBarWidget* cbwidget, Core::StateIntHandle& state_handle );
  
  // Coonnect a SliderIntCombo to a StateRangedIntValue
  static bool Connect( SliderIntCombo* sscombo, Core::StateRangedIntHandle& state_handle );
  
  // Coonnect a SliderDoubleCombo to a StateRangedIntValue
  static bool Connect( SliderDoubleCombo* sscombo, Core::StateRangedDoubleHandle& state_handle );
  
  // Connect a QDoubleSpinBox to a StateDoubleValue
  static bool Connect( QDoubleSpinBox* qdoublespinbox, Core::StateDoubleHandle& state_handle );
  
  // Connect a QSpinBox to a StateIntValue
  static bool Connect( QSpinBox* qspinbox, Core::StateIntHandle& state_handle );
  
  // Connect a QComboBox to StateOptionHandle
  static bool Connect( QComboBox* qcombobox, Core::StateOptionHandle& state_handle );
  
  static bool Connect( QComboBox* qcombobox, Core::StateStringHandle& state_handle );

  // Connect QToolButton & QPushButtons
  static bool Connect( QToolButton* qtoolbutton, Core::StateBoolHandle& state_handle );
  
  static bool Connect( ColorButton* colorbutton, Core::StateColorHandle& state_handle );
  
  static bool Connect( QToolButton* qtoolbutton, boost::function< void() > function );

  static bool Connect( QPushButton* qpushbutton, boost::function< void() > function );
  
  // Coonect QLineEdits
  static bool Connect( QLineEdit* qlineedit, Core::StateStringHandle& state_handle );
  
  static bool Connect( QLineEdit* qlineedit, Core::StateAliasHandle& state_handle );

  // Connect QActionGroup and StateOption
  // NOTE: This requires that each QAction in the QActionGroup has its objectName
  // the same as its corresponding option string
  static bool Connect( QActionGroup* qactiongroup, Core::StateOptionHandle& state_handle );

  // Connect menu action to dispatcher
  static bool Connect( QAction* qaction, boost::function< void() > function );

  // Connect menu toggle action to dispatcher
  static bool Connect( QAction* qaction, Core::StateBoolHandle& state_handle );

};

} // end namespace Seg3D

#endif
