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

#ifndef QTUTILS_BRIDGE_QTBRIDGE_H
#define QTUTILS_BRIDGE_QTBRIDGE_H

// QT includes
#include <QtGui/QAbstractButton>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QActionGroup>

// QtUtils includes
#include <QtUtils/Widgets/QtHistogramWidget.h>
#include <QtUtils/Widgets/QtSliderIntCombo.h>
#include <QtUtils/Widgets/QtLogSliderIntCombo.h>
#include <QtUtils/Widgets/QtSliderDoubleCombo.h>
#include <QtUtils/Widgets/QtColorButton.h>
#include <QtUtils/Widgets/QtColorBarWidget.h>

// Core includes
#include <Core/State/StateLabeledOption.h>
#include <Core/State/StateName.h>
#include <Core/State/StateValue.h>
#include <Core/State/StateOption.h>
#include <Core/State/StateVector.h>
#include <Core/State/StateRangedValue.h>

namespace QtUtils
{

// CLASS QTBRIDGE:
// This class provides bridges between widgets and state variables

class QtBridge : public boost::noncopyable
{

  // -- widget/menu connectors --
public:

  // Connect a QAbstractButton or its subclasses to StateValue<bool>
  static bool Connect( QAbstractButton* qbutton, Core::StateBoolHandle& state );
  
  static bool Connect( QAbstractButton* qbutton, boost::function< void() > function );
    
  // Connect vector of bools to one button controlled by an index
  static bool Connect(  QAbstractButton* qbutton, 
    std::vector<Core::StateBoolHandle>& state_handles, Core::StateIntHandle& index_handle );

  static bool Connect( QButtonGroup* qbuttongroup, Core::StateOptionHandle& state_handle );
    
  // Connect a SliderIntCombo to a StateRangedIntValue
  static bool Connect( QtColorBarWidget* cbwidget, Core::StateIntHandle& state_handle,
    std::vector< Core::StateColorHandle >& colors );
  
  // Connect a SliderIntCombo to a StateRangedIntValue
  static bool Connect( QtSliderIntCombo* sscombo, Core::StateRangedIntHandle& state_handle );

  // Connect a LogSliderIntCombo to a StateRangedIntValue
  static bool Connect( QtLogSliderIntCombo* sscombo, Core::StateRangedIntHandle& state_handle );
  
  // Connect a SliderDoubleCombo to a StateRangedIntValue
  static bool Connect( QtSliderDoubleCombo* sscombo, Core::StateRangedDoubleHandle& state_handle );
  
  // Connect a QDoubleSpinBox to a StateDoubleValue
  static bool Connect( QDoubleSpinBox* qdoublespinbox, Core::StateDoubleHandle& state_handle );
  
  // Connect a QSpinBox to a StateIntValue
  static bool Connect( QSpinBox* qspinbox, Core::StateIntHandle& state_handle );
  
  // Connect a QComboBox to StateOptionHandle
  static bool Connect( QComboBox* qcombobox, Core::StateOptionHandle& state_handle );

  static bool Connect( QComboBox* qcombobox, Core::StateLabeledOptionHandle& state );
  
  static bool Connect( QComboBox* qcombobox, Core::StateStringHandle& state_handle );

  static bool Connect( QtColorButton* colorbutton, Core::StateColorHandle& state_handle );
  
  // Coonect QLineEdits
  static bool Connect( QLineEdit* qlineedit, Core::StateStringHandle& state_handle );
  
  static bool Connect( QLineEdit* qlineedit, Core::StateNameHandle& state_handle );
  
  // Connect QActionGroup and StateOption
  // NOTE: This requires that each QAction in the QActionGroup has its objectName
  // the same as its corresponding option string
  static bool Connect( QActionGroup* qactiongroup, Core::StateOptionHandle& state_handle );

  // Connect menu action to dispatcher
  static bool Connect( QAction* qaction, boost::function< void() > function );

  // Connect menu toggle action to state variable
  static bool Connect( QAction* qaction, Core::StateBoolHandle& state_handle );
  
  // Connect vector of bools to one button controlled by an index
  static bool Connect( QAction* qaction, std::vector<Core::StateBoolHandle>& state_handles,
    Core::StateIntHandle& index_handle );

  // Enable / Disable a button from state engine
  static bool Enable( QWidget* qwidget, Core::StateBoolHandle& state, bool opposite_logic = false );
};

} // end namespace QtUtils

#endif
