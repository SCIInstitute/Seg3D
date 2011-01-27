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
#include <QAbstractButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QActionGroup>
#include <QListWidget>
#include <QPlainTextEdit>

// QtUtils includes
#include <QtUtils/Widgets/QtHistogramWidget.h>
#include <QtUtils/Widgets/QtSliderIntCombo.h>
#include <QtUtils/Widgets/QtLogSliderIntCombo.h>
#include <QtUtils/Widgets/QtSliderDoubleCombo.h>
#include <QtUtils/Widgets/QtColorButton.h>
#include <QtUtils/Widgets/QtColorBarWidget.h>
#include <QtUtils/Widgets/QtTristateToolButton.h>
#include <QtUtils/Widgets/QtCustomDockWidget.h>
#include <QtUtils/Widgets/QtCustomDialog.h>
#include <QtUtils/Widgets/QtHistogramWidget.h>

// Core includes
#include <Core/State/StateLabeledOption.h>
#include <Core/State/StateLabeledMultiOption.h>
#include <Core/State/StateName.h>
#include <Core/State/StateValue.h>
#include <Core/State/StateOption.h>
#include <Core/State/StateVector.h>
#include <Core/State/StateRangedValue.h>
#include <Core/State/StateSet.h>

namespace QtUtils
{

// CLASS QTBRIDGE:
// This class provides bridges between widgets and state variables

class QtBridge : public boost::noncopyable
{

  // -- widget/menu connectors --
public:

  // Connect a QAbstractButton or its subclasses to StateValue<bool>
  static void Connect( QAbstractButton* qbutton, Core::StateBoolHandle& state );
  
  static void Connect( QAbstractButton* qbutton, boost::function< void() > function );
  
  static void ConnectBoolean( QAbstractButton* qbutton, boost::function< void( bool ) > function );
    
  // Connect vector of bools to one button controlled by an index
  static void Connect(  QAbstractButton* qbutton, 
    std::vector<Core::StateBoolHandle>& state_handles, 
    Core::StateIntSetHandle& index_handle );
  
  // Connect a QtTristateToolButton to a tri-option state.
  static void Connect( QtTristateToolButton* tristate_button, Core::StateOptionHandle& state );

  static void Connect( QButtonGroup* qbuttongroup, Core::StateOptionHandle& state_handle );

  static void Connect( QButtonGroup* qbuttongroup, Core::StateLabeledOptionHandle& state );

  // Connect a SliderIntCombo to a StateRangedIntValue
  static void Connect( QtColorBarWidget* cbwidget, Core::StateIntHandle& state_handle,
    std::vector< Core::StateColorHandle >& colors );
  
  // Connect a SliderIntCombo to a StateRangedIntValue
  static void Connect( QtSliderIntCombo* sscombo, Core::StateRangedIntHandle& state_handle );

  // Connect a LogSliderIntCombo to a StateRangedIntValue
  static void Connect( QtLogSliderIntCombo* sscombo, Core::StateRangedIntHandle& state_handle );
  
  // Connect a SliderDoubleCombo to a StateRangedIntValue
  static void Connect( QtSliderDoubleCombo* sscombo, Core::StateRangedDoubleHandle& state_handle );
  
  // Connect a QDoubleSpinBox to a StateDoubleValue
  static void Connect( QDoubleSpinBox* qdoublespinbox, Core::StateDoubleHandle& state_handle );
  
  // Connect a QDoubleSpinBox to a StateRangedDouble
  static void Connect( QDoubleSpinBox* qdoublespinbox, Core::StateRangedDoubleHandle& state );

  // Connect a QSpinBox to a StateInt
  static void Connect( QSpinBox* qspinbox, Core::StateIntHandle& state );

  // Connect a QSpinBox to a StateRangedInt
  static void Connect( QSpinBox* qspinbox, Core::StateRangedIntHandle& state );
  
  // Connect a QComboBox to StateOptionHandle
  static void Connect( QComboBox* qcombobox, Core::StateOptionHandle& state_handle );

  static void Connect( QComboBox* qcombobox, Core::StateLabeledOptionHandle& state );
  
  static void Connect( QtColorButton* colorbutton, Core::StateColorHandle& state_handle );
  
  // Connect QLineEdits
  static void Connect( QLineEdit* qlineedit, Core::StateStringHandle& state_handle );
  
  static void Connect( QLineEdit* qlineedit, Core::StateNameHandle& state_handle );

  static void Connect( QPlainTextEdit* qwidget, Core::StateStringHandle& state );

  // Connect QLabel to display the string representation of a state variable
  static void Connect( QLabel* qlabel, Core::StateBaseHandle state );
  
  // Connect QActionGroup and StateOption
  // NOTE: This requires that each QAction in the QActionGroup has its objectName
  // the same as its corresponding option string
  static void Connect( QActionGroup* qactiongroup, Core::StateOptionHandle& state_handle );

  // Connect menu action to dispatcher
  static void Connect( QAction* qaction, boost::function< void() > function );

  // Connect menu toggle action to state variable
  static void Connect( QAction* qaction, Core::StateBoolHandle& state_handle );
  
  // Connect vector of bools to one button controlled by an index
  static void Connect( QAction* qaction, std::vector<Core::StateBoolHandle>& state_handles,
    Core::StateIntHandle& index_handle );

  // Connect QListWidget and StateLabeledMultiOption
  static void Connect( QListWidget* qlistwidget, Core::StateLabeledMultiOptionHandle& state );

  static void Connect( QListWidget* qwidget, Core::StateStringVectorHandle& state );

  // Enable / Disable a button from state engine
  static void Enable( QAction* qaction, Core::StateBoolHandle& state, bool opposite_logic = false );
  static void Enable( QWidget* qwidget, Core::StateBoolHandle& state, bool opposite_logic = false );
  static void Enable( QtHistogramWidget* histogram, Core::StateBoolHandle& state, bool opposite_logic = false );

  static void Enable( QWidget* qwidget, Core::StateBaseHandle state,
    boost::function< bool () > condition );

  static void Enable( QWidget* qwidget, std::vector< Core::StateBaseHandle >& states,
    boost::function< bool () > condition );

  // Connect the visibility of the QWidget to a StateBool
  static void Show( QWidget* qwidget, Core::StateBoolHandle& state, bool opposite_logic = false );
  
  // Connect the visibility of the QtCustomDockWidget to a StateBool
  static void Show( QtCustomDockWidget* dockwidget, Core::StateBoolHandle& state, bool opposite_logic = false );
  
  // Connect the visibility of the QtCustomDialog to a StateBool
  static void Show( QtCustomDialog* dialog, Core::StateBoolHandle& state, bool opposite_logic = false );

  // Connect the visibility of the QWidget to a Statebase
  static void Show( QWidget* qwidget, Core::StateBaseHandle state,
    boost::function< bool () > condition );

  // Connect the visibility of the QWidget to a StateBool
  static void Show( QWidget* qwidget, std::vector< Core::StateBaseHandle >& states,
    boost::function< bool () > condition );
};

} // end namespace QtUtils

#endif
