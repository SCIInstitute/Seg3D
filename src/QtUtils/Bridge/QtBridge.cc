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
 
// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QActionGroup>

// Core includes
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Bridge/detail/QtAbstractButtonConnector.h>
#include <QtUtils/Bridge/detail/QtAbstractButtonVectorConnector.h>
#include <QtUtils/Bridge/detail/QtActionConnector.h>
#include <QtUtils/Bridge/detail/QtActionVectorConnector.h>
#include <QtUtils/Bridge/detail/QtActionGroupConnector.h>
#include <QtUtils/Bridge/detail/QtButtonGroupConnector.h>
#include <QtUtils/Bridge/detail/QtColorButtonConnector.h>
#include <QtUtils/Bridge/detail/QtColorBarWidgetConnector.h>
#include <QtUtils/Bridge/detail/QtComboBoxConnector.h>
#include <QtUtils/Bridge/detail/QtLineEditConnector.h>
#include <QtUtils/Bridge/detail/QtSpinBoxConnector.h>
#include <QtUtils/Bridge/detail/QtSliderIntComboConnector.h>
#include <QtUtils/Bridge/detail/QtLogSliderIntComboConnector.h>
#include <QtUtils/Bridge/detail/QtSliderDoubleComboConnector.h>
#include <QtUtils/Bridge/detail/QtEnableConnector.h>

namespace QtUtils
{

bool QtBridge::Connect( QAbstractButton* qbutton, Core::StateBoolHandle& state )
{
  new QtAbstractButtonConnector( qbutton, state );
  return true;
}

bool QtBridge::Connect( QAbstractButton* qbutton, boost::function< void() > function )
{
  new QtAbstractButtonConnector( qbutton, function );
  return true;
}

bool QtBridge::Connect( QAbstractButton* qbutton, std::vector<Core::StateBoolHandle>& state,
  Core::StateIntHandle& index)
{
  new QtAbstractButtonVectorConnector( qbutton, state, index );
  return true;
}

bool QtBridge::Connect( QLineEdit* qlineedit, Core::StateStringHandle& state )
{
  new QtLineEditConnector( qlineedit, state );
  return true;
}

bool QtBridge::Connect( QLineEdit* qlineedit, Core::StateNameHandle& state )
{
  new QtLineEditConnector( qlineedit, state );
  return true;
}

bool QtBridge::Connect( QDoubleSpinBox* qdoublespinbox, Core::StateDoubleHandle& state )
{
  new QtSpinBoxConnector( qdoublespinbox, state );  
  return true;
}

bool QtBridge::Connect( QSpinBox* qspinbox, Core::StateIntHandle& state )
{
  new QtSpinBoxConnector( qspinbox, state );  
  return true;
}
  
bool QtBridge::Connect( QComboBox* qcombobox, Core::StateOptionHandle& state )
{
  new QtComboBoxConnector( qcombobox, state );
  return true;
}

bool QtBridge::Connect( QComboBox* qcombobox, 
  Core::StateLabeledOptionHandle& state )
{
  new QtComboBoxConnector( qcombobox, state );
  return true;
}
    
bool QtBridge::Connect( QComboBox* qcombobox, Core::StateStringHandle& state_handle )
{
  return true;
}

bool QtBridge::Connect( QtSliderIntCombo* sscombo, Core::StateRangedIntHandle& state )
{
  new QtSliderIntComboConnector( sscombo, state );
  return true;
}

bool QtBridge::Connect( QtLogSliderIntCombo* sscombo, Core::StateRangedIntHandle& state )
{
  new QtLogSliderIntComboConnector( sscombo, state );
  return true;
}

bool QtBridge::Connect( QtColorBarWidget* colorbar_widget, 
  Core::StateIntHandle& state, std::vector< Core::StateColorHandle >& colors )
{
  new QtColorBarWidgetConnector( colorbar_widget, state, colors );
  return true;
}

bool QtBridge::Connect( QtSliderDoubleCombo* sscombo, Core::StateRangedDoubleHandle& state )
{
  new QtSliderDoubleComboConnector( sscombo, state );
  return true;
}

bool QtBridge::Connect( QtColorButton* colorbutton, Core::StateColorHandle& state )
{
  new QtColorButtonConnector( colorbutton, state ); 
  return true;
}

bool QtBridge::Connect( QAction* qaction, boost::function< void() > function )
{
  new QtActionConnector( qaction, function );
  return true;
}

bool QtBridge::Connect( QAction* qaction, Core::StateBoolHandle& state )
{
  new QtActionConnector( qaction, state );
  return true;
}

bool QtBridge::Connect( QAction* qaction, std::vector<Core::StateBoolHandle>& state,
  Core::StateIntHandle& index)
{
  new QtActionVectorConnector( qaction, state, index );
  return true;
}

bool QtBridge::Connect( QActionGroup* qactiongroup, Core::StateOptionHandle& state )
{
  new QtActionGroupConnector( qactiongroup, state );
  return true;
}

bool QtBridge::Connect( QButtonGroup* qbuttongroup, Core::StateOptionHandle& state_handle )
{
  new QtButtonGroupConnector( qbuttongroup, state_handle );
  return true;
}

bool QtBridge::Enable( QWidget* qwidget, Core::StateBoolHandle& state, bool opposite_logic )
{
  new QtEnableConnector( qwidget, state, opposite_logic );
  return true;
}

} // end namespace QtUtils
