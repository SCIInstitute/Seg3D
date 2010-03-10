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
#include <QtGui>
#include <Application/Interface/Interface.h>
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/QtInterface/QtBridgeInternal.h>
//#include <Interface/ToolInterface/CustomWidgets/SliderSpinCombo.h>

namespace Seg3D
{

template<class QTPOINTER>
void QtSignal( QTPOINTER qpointer, boost::function<void ()> func )
{
  if ( !Interface::IsInterfaceThread() )
  {
    Interface::PostEvent( boost::bind( &QtSignal<QTPOINTER>, qpointer, func ) );
    return;
  }

  if ( qpointer.data() )
  {
    QtSlot::Block( qpointer );
    func();
    QtSlot::Unblock( qpointer );
  }
}

// -- Checkbox connector --
void QtCheckBoxSignal( QPointer< QCheckBox > qpointer, bool state, ActionSource source )
{
  if ( source != ActionSource::ACTION_SOURCE_INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QCheckBox::setChecked, 
      qpointer.data(), state ) ); 
  }
}

void QtComboBoxSignal( QComboBox* qcombobox, std::string state )
{
  QString qstring_state = QString::fromStdString( state );
  int index = qcombobox->findText( qstring_state, Qt::MatchFlags( Qt::CaseInsensitive ) );

  if ( index >= 0 )
  {
    qcombobox->setCurrentIndex( index );
  }
}

void QtComboBoxSignal( QPointer< QComboBox > qpointer, std::string state, ActionSource source )
{
  if ( source != ActionSource::ACTION_SOURCE_INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtComboBoxSignal, qpointer.data(), state ) );
  }
}

void QtSliderSpinComboRangedIntSignal( QPointer< SliderSpinComboInt > qpointer, int state,
    ActionSource source )
{
  if ( source != ActionSource::ACTION_SOURCE_INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &SliderSpinComboInt::setCurrentValue, qpointer.data(), state ) );
  }
}

void QtSliderSpinComboRangedDoubleSignal( QPointer< SliderSpinComboDouble > qpointer, double state,
    ActionSource source )
{
  if ( source != ActionSource::ACTION_SOURCE_INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &SliderSpinComboDouble::setCurrentValue, qpointer.data(), state ) );
  }
}

void QtActionGroupSignal( QActionGroup* qactiongroup, std::string option )
{
  QList< QAction* > actions = qactiongroup->actions();
  for ( QList< QAction* >::iterator it = actions.begin(); it != actions.end(); it++ )
  {
    if ( ( *it )->objectName().toStdString() == option )
    {
      ( *it )->trigger();
      break;
    }
  }
}

void QtActionGroupSignal( QPointer< QActionGroup > qpointer, std::string option,
    ActionSource source )
{
  if ( source != ActionSource::ACTION_SOURCE_INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtActionGroupSignal, qpointer.data(), option ) );
  }
}

bool QtBridge::connect( QCheckBox* qcheckbox, StateBoolHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtCheckBoxSlot( qcheckbox, state_handle );

  // Connect the state signal back into the Qt Variable
  state_handle->value_changed_signal_.connect( boost::bind( &QtCheckBoxSignal, qcheckbox, _1, _2 ) );

  return ( true );
}

bool QtBridge::connect( QComboBox* qcombobox, StateOptionHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtComboBoxSlot( qcombobox, state_handle );

  // Connect the state signal back to the Qt Variable
  state_handle->value_changed_signal_.connect( boost::bind( &QtComboBoxSignal, qcombobox, _1, _2 ) );

  return ( true );
}

bool QtBridge::connect( SliderSpinComboInt* sscombo, StateRangedIntHandle& state_handle )
{
  new QtSliderSpinComboRangedIntSlot( sscombo, state_handle );

  // Connect the state signal back to the Qt Variable
  state_handle->value_changed_signal_.connect( boost::bind( &QtSliderSpinComboRangedIntSignal,
      sscombo, _1, _2 ) );

  return ( true );
}

bool QtBridge::connect( SliderSpinComboDouble* sscombo, StateRangedDoubleHandle& state_handle )
{
  new QtSliderSpinComboRangedDoubleSlot( sscombo, state_handle );

  // Connect the state signal back to the Qt Variable
  state_handle->value_changed_signal_.connect( boost::bind( &QtSliderSpinComboRangedDoubleSignal,
      sscombo, _1, _2 ) );

  return ( true );
}

bool QtBridge::connect( QToolButton* qtoolbutton, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtToolButtonSlot( qtoolbutton, function );

  return ( true );
}

bool QtBridge::connect( QPushButton* qpushbutton, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtPushButtonSlot( qpushbutton, function );

  return ( true );
}

// Menu connectors
bool QtBridge::connect( QAction* qaction, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionSlot( qaction, function );

  return ( true );
}

bool QtBridge::connect( QAction* qaction, StateBoolHandle& state_handle )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionToggleSlot( qaction, state_handle );

  return ( true );
}

bool QtBridge::connect( QActionGroup* qactiongroup, StateOptionHandle& state_handle )
{
  new QtActionGroupSlot( qactiongroup, state_handle );

  state_handle->value_changed_signal_.connect( 
    boost::bind( &QtActionGroupSignal, qactiongroup, _1, _2 ) );

  return true;
}

} // end namespace Seg3D
