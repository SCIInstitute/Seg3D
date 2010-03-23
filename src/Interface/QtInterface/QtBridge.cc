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
  if ( source != ActionSource::INTERFACE_E )
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
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtComboBoxSignal, qpointer.data(), state ) );
  }
}

void QtToolButtonToggleSignal( QPointer< QToolButton > qpointer, bool state, ActionSource source )
{
    if ( source != ActionSource::INTERFACE_E )
    {
       QtSignal( qpointer, boost::bind( &QToolButton::setChecked, 
      qpointer.data(), state ) ); 
    }
}
void QtSliderIntComboRangedIntSignal( QPointer< SliderIntCombo > qpointer, int state,
    ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &SliderIntCombo::setCurrentValue, qpointer.data(), state ) );
  }
}

void QtSliderDoubleComboValueChangedSignal( QPointer< SliderDoubleCombo > qpointer, double state,
    ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &SliderDoubleCombo::setCurrentValue, qpointer.data(), state ) );
  }
}


void QtSliderDoubleComboRangeChangedSignal( QPointer< SliderDoubleCombo > qpointer, double min, double max,
    ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &SliderDoubleCombo::setRange, qpointer.data(), min, max ) );
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
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtActionGroupSignal, qpointer.data(), option ) );
  }
}


void QtLineEditSignal( QPointer< QLineEdit > qpointer, std::string state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QLineEdit::setText, qpointer.data(), QString::fromStdString( state ) ) ); 
  }
}


void QtDoubleSpinBoxSignal( QPointer< QDoubleSpinBox > qpointer, double state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QDoubleSpinBox::setValue, qpointer.data(), state ) ); 
  }
}



bool QtBridge::Connect( QCheckBox* qcheckbox, StateBoolHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtCheckBoxSlot( qcheckbox, state_handle );

  // Connect the state signal back into the Qt Variable
  QPointer< QCheckBox > qpointer( qcheckbox );
  
  new QtDeleteSlot( qcheckbox, state_handle->value_changed_signal_.connect( 
      boost::bind( &QtCheckBoxSignal, qpointer, _1, _2 ) ) );

  return true;
}

bool QtBridge::Connect( QLineEdit* qlineedit, StateStringHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtLineEditSlot( qlineedit, state_handle );
  QPointer< QLineEdit > qpointer( qlineedit );

  // Connect the state signal back into the Qt Variable
  new QtDeleteSlot( qlineedit, state_handle->value_changed_signal_.connect( 
      boost::bind( &QtLineEditSignal, qpointer, _1, _2 ) ) );

  return true;
}

bool QtBridge::Connect( QLineEdit* qlineedit, StateAliasHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtLineEditAliasSlot( qlineedit, state_handle );
  QPointer< QLineEdit > qpointer( qlineedit );

  // Connect the state signal back into the Qt Variable
  new QtDeleteSlot( qlineedit, state_handle->value_changed_signal_.connect( 
      boost::bind( &QtLineEditSignal, qpointer, _1, _2 ) ) );

  return true;
}

bool QtBridge::Connect( QDoubleSpinBox* qdoublespinbox, StateDoubleHandle& state_handle )
{
    // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
    new QtDoubleSpinBoxSlot( qdoublespinbox, state_handle );
    QPointer< QDoubleSpinBox > qpointer( qdoublespinbox );
    
    // Connect the state signal back into the Qt Variable
    new QtDeleteSlot( qdoublespinbox, state_handle->value_changed_signal_.connect( 
      boost::bind( &QtDoubleSpinBoxSignal, qpointer, _1, _2 ) ) );
      
  return true;
}


bool QtBridge::Connect( QComboBox* qcombobox, StateOptionHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtComboBoxSlot( qcombobox, state_handle );
  QPointer< QComboBox > qpointer( qcombobox );

  // Connect the state signal back to the Qt Variable
  new QtDeleteSlot( qcombobox, state_handle->value_changed_signal_.connect( 
      boost::bind( &QtComboBoxSignal, qpointer, _1, _2 ) ) );

  return true;
}

bool QtBridge::Connect( SliderIntCombo* sscombo, StateRangedIntHandle& state_handle )
{
  new QtSliderIntComboRangedIntSlot( sscombo, state_handle );
  
  QPointer< SliderIntCombo > qpointer( sscombo );

  // Connect the state signal back to the Qt Variable
  new QtDeleteSlot( sscombo, state_handle->value_changed_signal_.connect(
      boost::bind( &QtSliderIntComboRangedIntSignal, qpointer, _1, _2 ) ) );

  return true;
}

bool QtBridge::Connect( SliderDoubleCombo* sscombo, StateRangedDoubleHandle& state_handle )
{
  new QtSliderDoubleComboRangedDoubleSlot( sscombo, state_handle );
  
  QPointer< SliderDoubleCombo > qpointer( sscombo );

  // Connect the state signal back to the Qt Variable
  new QtDeleteSlot( sscombo, state_handle->value_changed_signal_.connect(
      boost::bind( &QtSliderDoubleComboValueChangedSignal, qpointer, _1, _2 ) ) );
  
    new QtDeleteSlot( sscombo, state_handle->range_changed_signal_.connect(
        boost::bind( &QtSliderDoubleComboRangeChangedSignal, qpointer, _1, _2, _3 ) ) );

  return true;
}


bool QtBridge::Connect( QToolButton* qtoolbutton, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtToolButtonSlot( qtoolbutton, function );

  return true;
}

bool QtBridge::Connect( QToolButton* qtoolbutton, StateBoolHandle& state_handle )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtToolButtonToggleSlot( qtoolbutton, state_handle );
  QPointer< QToolButton > qpointer( qtoolbutton );
  
  new QtDeleteSlot( qtoolbutton, state_handle->value_changed_signal_.connect( 
      boost::bind( &QtToolButtonToggleSignal, qpointer, _1, _2 ) ) );
  
  return true;
}

  

bool QtBridge::Connect( QPushButton* qpushbutton, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtPushButtonSlot( qpushbutton, function );

  return true;
}

// Menu connectors
bool QtBridge::Connect( QAction* qaction, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionSlot( qaction, function );

  return true;
}

bool QtBridge::Connect( QAction* qaction, StateBoolHandle& state_handle )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionToggleSlot( qaction, state_handle );

    // TODO: Check whether there needs to be signal

  return true;
}

bool QtBridge::Connect( QActionGroup* qactiongroup, StateOptionHandle& state_handle )
{
  new QtActionGroupSlot( qactiongroup, state_handle );
  QPointer< QActionGroup > qpointer( qactiongroup );

  new QtDeleteSlot( qactiongroup, state_handle->value_changed_signal_.connect( 
    boost::bind( &QtActionGroupSignal, qpointer, _1, _2 ) ) );

  return true;
}

} // end namespace Seg3D
