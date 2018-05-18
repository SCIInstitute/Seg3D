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
 
// Qt includes
#include <QWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>
#include <QLineEdit>
#include <QActionGroup>

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
#include <QtUtils/Bridge/detail/QtListWidgetConnector.h>
#include <QtUtils/Bridge/detail/QtShowConnector.h>
#include <QtUtils/Bridge/detail/QtLabelConnector.h>
#include <QtUtils/Bridge/detail/QtPlainTextEditConnector.h>
#include <QtUtils/Bridge/detail/QtTristateToolButtonConnector.h>
#include <QtUtils/Bridge/detail/QtTransferFunctionSceneConnector.h>
#include <QtUtils/Bridge/detail/QtTransferFunctionCurveConnector.h>
#include <QtUtils/Bridge/detail/QtGroupBoxConnector.h>
#include <QtUtils/Bridge/detail/QtTextMatrixConnector.h>


namespace QtUtils
{

Core::ConnectionHandle QtBridge::Connect( QAbstractButton* qbutton, Core::StateBoolHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtAbstractButtonConnector( qbutton, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QAbstractButton* qbutton, boost::function< void() > function )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtAbstractButtonConnector( qbutton, function ) ) );
}

Core::ConnectionHandle QtBridge::ConnectBoolean( QAbstractButton* qbutton, boost::function< void( bool ) > function )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtAbstractButtonConnector( qbutton, function ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QAbstractButton* qbutton,
                                          std::vector<Core::StateBoolHandle>& state,
                                          Core::StateIntSetHandle& index)
{
  return Core::ConnectionHandle( new QtConnection(
    new QtAbstractButtonVectorConnector( qbutton, state, index ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QGroupBox* qbox, Core::StateBoolHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtGroupBoxConnector( qbox, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QGroupBox* qbox, boost::function< void() > function )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtGroupBoxConnector( qbox, function ) ) );
}

Core::ConnectionHandle QtBridge::ConnectBoolean( QGroupBox* qbox, boost::function< void( bool ) > function )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtGroupBoxConnector( qbox, function ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QLineEdit* qlineedit, Core::StateStringHandle& state,
  bool immediate_update )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtLineEditConnector( qlineedit, state, immediate_update ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QLineEdit* qlineedit, Core::StateNameHandle& state,
  bool immediate_update )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtLineEditConnector( qlineedit, state, immediate_update ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QDoubleSpinBox* qdoublespinbox, Core::StateDoubleHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtSpinBoxConnector( qdoublespinbox, state ) ) );  
}

Core::ConnectionHandle QtBridge::Connect( QDoubleSpinBox* qdoublespinbox, Core::StateRangedDoubleHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtSpinBoxConnector( qdoublespinbox, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QSpinBox* qspinbox, Core::StateIntHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtSpinBoxConnector( qspinbox, state ) ) );  
}

Core::ConnectionHandle QtBridge::Connect( QSpinBox* qspinbox, Core::StateRangedIntHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtSpinBoxConnector( qspinbox, state ) ) );  
}

Core::ConnectionHandle QtBridge::Connect( QComboBox* qcombobox, Core::StateOptionHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtComboBoxConnector( qcombobox, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QComboBox* qcombobox, 
  Core::StateLabeledOptionHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtComboBoxConnector( qcombobox, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtSliderIntCombo* sscombo, Core::StateRangedIntHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtSliderIntComboConnector( sscombo, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtLogSliderIntCombo* sscombo, Core::StateRangedIntHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
  new QtLogSliderIntComboConnector( sscombo, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtColorBarWidget* colorbar_widget, 
  Core::StateIntHandle& state, std::vector< Core::StateColorHandle >& colors )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtColorBarWidgetConnector( colorbar_widget, state, colors ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtSliderDoubleCombo* sscombo, Core::StateRangedDoubleHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtSliderDoubleComboConnector( sscombo, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtColorButton* colorbutton, Core::StateColorHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtColorButtonConnector( colorbutton, state ) ) ); 
}

Core::ConnectionHandle QtBridge::Connect( QAction* qaction, boost::function< void() > function )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtActionConnector( qaction, function ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QAction* qaction, Core::StateBoolHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtActionConnector( qaction, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QAction* qaction, std::vector<Core::StateBoolHandle>& state,
  Core::StateIntHandle& index)
{
  return Core::ConnectionHandle( new QtConnection(
    new QtActionVectorConnector( qaction, state, index ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QActionGroup* qactiongroup, Core::StateOptionHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtActionGroupConnector( qactiongroup, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QButtonGroup* qbuttongroup, Core::StateOptionHandle& state_handle )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtButtonGroupConnector( qbuttongroup, state_handle ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QButtonGroup* qbuttongroup, Core::StateLabeledOptionHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtButtonGroupConnector( qbuttongroup, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QListWidget* qlistwidget, Core::StateLabeledMultiOptionHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtListWidgetConnector( qlistwidget, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect(QPlainTextEdit* plain_text,
	Core::StateDoubleVectorHandle& state_vector, int dim1, int dim2)
{
	return Core::ConnectionHandle(new QtConnection(
		new QtTextMatrixConnector(plain_text,state_vector,dim1,dim2)));
}

Core::ConnectionHandle QtBridge::Connect( QListWidget* qwidget, Core::StateStringVectorHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtListWidgetConnector( qwidget, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QLabel* qlabel, Core::StateBaseHandle state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtLabelConnector( qlabel, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QPlainTextEdit* qwidget, Core::StateStringHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtPlainTextEditConnector( qwidget, state ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtTristateToolButton* tristate_button, Core::StateOptionHandle& state )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtTristateToolButtonConnector( tristate_button, state ) ) );
}
  
Core::ConnectionHandle QtBridge::Connect( QtTransferFunctionScene* tf_scene, Core::TransferFunctionHandle& tf )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtTransferFunctionSceneConnector( tf_scene, tf ) ) );
}

Core::ConnectionHandle QtBridge::Connect( QtTransferFunctionCurve* tf_curve, Core::TransferFunctionFeatureHandle& feature )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtTransferFunctionCurveConnector( tf_curve, feature ) ) );
}

Core::ConnectionHandle QtBridge::Enable( QAction* qaction, Core::StateBoolHandle& state, bool opposite_logic )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtEnableConnector( qaction, state, opposite_logic ) ) );
} 

Core::ConnectionHandle QtBridge::Enable( QWidget* qwidget, Core::StateBoolHandle& state, bool opposite_logic )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtEnableConnector( qwidget, state, opposite_logic ) ) );
}

Core::ConnectionHandle QtBridge::Enable( QtHistogramWidget* histogram, Core::StateBoolHandle& state, bool opposite_logic /*= false */ )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtEnableConnector( histogram, state, opposite_logic ) ) );
}

Core::ConnectionHandle QtBridge::Enable( QWidget* qwidget, std::vector< Core::StateBaseHandle >& states,
            boost::function< bool () > condition )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtEnableConnector( qwidget, states, condition ) ) );
}

Core::ConnectionHandle QtBridge::Enable( QWidget* qwidget, Core::StateBaseHandle state, 
            boost::function< bool () > condition )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtEnableConnector( qwidget, state, condition ) ) );
}

Core::ConnectionHandle QtBridge::Show( QWidget* qwidget, Core::StateBoolHandle& state, bool opposite_logic )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtShowConnector( qwidget, state, opposite_logic ) ) );
}

Core::ConnectionHandle QtBridge::Show( QtCustomDockWidget* dockwidget, Core::StateBoolHandle& state, bool opposite_logic /*= false */ )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtShowConnector( dockwidget, state, opposite_logic ) ) );
}

Core::ConnectionHandle QtBridge::Show( QtCustomDialog* dialog, Core::StateBoolHandle& state, bool opposite_logic /*= false */ )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtShowConnector( dialog, state, opposite_logic ) ) );
}


Core::ConnectionHandle QtBridge::Show( QWidget* qwidget, std::vector< Core::StateBaseHandle >& states, 
          boost::function< bool () > condition )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtShowConnector( qwidget, states, condition ) ) );
}

Core::ConnectionHandle QtBridge::Show( QWidget* qwidget, Core::StateBaseHandle state, 
          boost::function< bool () > condition )
{
  return Core::ConnectionHandle( new QtConnection(
    new QtShowConnector( qwidget, state, condition ) ) );
}

} // end namespace QtUtils
