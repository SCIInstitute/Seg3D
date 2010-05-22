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

// QtInterface includes
#include <QtInterface/Bridge/QtBridge.h>
#include <QtInterface/Bridge/detail/QtAbstractButtonConnector.h>
#include <QtInterface/Bridge/detail/QtActionConnector.h>
#include <QtInterface/Bridge/detail/QtActionGroupConnector.h>
#include <QtInterface/Bridge/detail/QtColorButtonConnector.h>
#include <QtInterface/Bridge/detail/QtColorBarWidgetConnector.h>
#include <QtInterface/Bridge/detail/QtComboBoxConnector.h>
#include <QtInterface/Bridge/detail/QtLineEditConnector.h>
#include <QtInterface/Bridge/detail/QtSpinBoxConnector.h>
#include <QtInterface/Bridge/detail/QtSliderIntComboConnector.h>
#include <QtInterface/Bridge/detail/QtSliderDoubleComboConnector.h>

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

bool QtBridge::Connect( QActionGroup* qactiongroup, Core::StateOptionHandle& state )
{
  new QtActionGroupConnector( qactiongroup, state );
  return true;
}

} // end namespace QtUtils
