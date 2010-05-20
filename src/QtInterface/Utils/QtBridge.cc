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
#include <QtInterface/Utils/QtBridge.h>
#include <QtInterface/Utils/QtBridgeInternal.h>

namespace Core
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


///// ====  BEGIN QCheckBox FUNCTIONS ==== /////

// signal for when the value of the QCheckBox has changed //
void QtCheckBoxSignal( QPointer< QCheckBox > qpointer, bool state, Core::ActionSource source )
{
  if ( source != Core::ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QCheckBox::setChecked, 
      qpointer.data(), state ) ); 
  }
}

bool QtBridge::Connect( QCheckBox* qcheckbox, Core::StateBoolHandle& state_handle )
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
///// ====  END QCheckBox FUNCTIONS ==== /////


///// ====  BEGIN QLineEdit FUNCTIONS ==== /////

void QtLineEditSignal( QPointer< QLineEdit > qpointer, std::string state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QLineEdit::setText, qpointer.data(), QString::fromStdString( state ) ) ); 
  }
}

// Signal for connecting to StateName's value_changed_signal_
// The actual value set in StateName can be different from interface input, so this signal
// should always be passed to the interface no matter what the source is
void QtNameEditSignal( QPointer< QLineEdit > qpointer, std::string state )
{
  QtSignal( qpointer, boost::bind( &QLineEdit::setText, qpointer.data(), 
    QString::fromStdString( state ) ) );
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

bool QtBridge::Connect( QLineEdit* qlineedit, StateNameHandle& state_handle )
{
  new QtNameEditSlot( qlineedit, state_handle );

  QPointer< QLineEdit > qpointer( qlineedit );

  new QtDeleteSlot( qlineedit, state_handle->value_changed_signal_.connect(
    boost::bind( &QtNameEditSignal, qpointer, _2 ) ) );

  return true;
}

///// ====  END QLineEdit FUNCTIONS ==== /////


///// ====  BEGIN QDoubleSpinBox FUNCTIONS ==== /////

void QtDoubleSpinBoxSignal( QPointer< QDoubleSpinBox > qpointer, double state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QDoubleSpinBox::setValue, qpointer.data(), state ) ); 
  }
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

///// ====  END QDoubleSpinBox FUNCTIONS ==== /////

///// ====  BEGIN QSpinBox FUNCTIONS ==== /////

void QtSpinBoxSignal( QPointer< QSpinBox > qpointer, int state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QSpinBox::setValue, qpointer.data(), state ) ); 
  }
}

bool QtBridge::Connect( QSpinBox* qspinbox, StateIntHandle& state_handle )
{
  // Connect the dispatch into the StateVariable (with auxiliary object)
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtSpinBoxSlot( qspinbox, state_handle );
  QPointer< QSpinBox > qpointer( qspinbox );
  
  // Connect the state signal back into the Qt Variable
  new QtDeleteSlot( qspinbox, state_handle->value_changed_signal_.connect( 
    boost::bind( &QtSpinBoxSignal, qpointer, _1, _2 ) ) );
  
  return true;
}

///// ====  END QSpinBox FUNCTIONS ==== /////
  
  

///// ====  BEGIN QComboBox FUNCTIONS ==== /////

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
    
    
bool QtBridge::Connect( QComboBox* qcombobox, StateStringHandle& state_handle )
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

///// ====  END QComboBox FUNCTIONS ==== /////

///// ====  BEGIN SliderIntCombo FUNCTIONS ==== /////

void QtSliderIntComboValueChangedSignal( QPointer< QtSliderIntCombo > qpointer, int state,
  ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtSliderIntCombo::setCurrentValue, qpointer.data(), state ) );
  }
}

void QtSliderIntComboRangeChangedSignal( QPointer< QtSliderIntCombo > qpointer, int min, int max,
  ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtSliderIntCombo::setRange, qpointer.data(), min, max ) );
  }
}

bool QtBridge::Connect( QtSliderIntCombo* sscombo, StateRangedIntHandle& state_handle )
{
  new QtSliderIntComboRangedSlot( sscombo, state_handle );
  
  QPointer< QtSliderIntCombo > qpointer( sscombo );

  // Connect the state signal back to the SliderIntCombo's value variable
  new QtDeleteSlot( sscombo, state_handle->value_changed_signal_.connect(
    boost::bind( &QtSliderIntComboValueChangedSignal, qpointer, _1, _2 ) ) );
  
  // Connect the state signal back to the SliderIntCombo's range variables  
  new QtDeleteSlot( sscombo, state_handle->range_changed_signal_.connect(
    boost::bind( &QtSliderIntComboRangeChangedSignal, qpointer, _1, _2, _3 ) ) );

  return true;
}
///// ====  END SliderIntCombo FUNCTIONS ==== /////
  
  
///// ====  BEGIN ColorBarWidget FUNCTIONS ==== /////
void QtColorBarWidgetValueChangedSignal( QPointer< QtColorBarWidget > qpointer, int state,
  ActionSource source )
{
  if ( source != Core::ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtColorBarWidget::set_color_index, qpointer.data(), state ) );
  }
}


bool QtBridge::Connect( QtColorBarWidget* colorbar_widget, StateIntHandle& state_handle,
  std::vector<StateColorHandle>& colors )
{
  new QtColorBarWidgetSlot( colorbar_widget, state_handle );
  
  for ( size_t j = 0; j < colors.size(); j++ )
  {
    QtColorButton* color_button =  new QtColorButton( colorbar_widget, 
      j, colors[ j ]->get(), 16, 16 );
    QtBridge::Connect( color_button, colors[ j ] );
    colorbar_widget->add_color_button( color_button, j );
  }
  
  // Connect the state signal back to the ColorBarWidget's value variable
  QPointer< QtColorBarWidget > qpointer( colorbar_widget );
  new QtDeleteSlot( colorbar_widget, state_handle->value_changed_signal_.connect(
    boost::bind( &QtColorBarWidgetValueChangedSignal, qpointer, _1, _2 ) ) );
  
  return true;
}

///// ====  END ColorBarWidget FUNCTIONS ==== /////


///// ====  BEGIN SliderDoubleCombo FUNCTIONS ==== /////

void QtSliderDoubleComboValueChangedSignal( QPointer< QtSliderDoubleCombo > qpointer, double state,
  ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtSliderDoubleCombo::setCurrentValue, 
      qpointer.data(), state ) );
  }
}

void QtSliderDoubleComboRangeChangedSignal( QPointer< QtSliderDoubleCombo > qpointer, 
  double min, double max, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtSliderDoubleCombo::setRange, qpointer.data(), min, max ) );
  }
}

bool QtBridge::Connect( QtSliderDoubleCombo* sscombo, StateRangedDoubleHandle& state_handle )
{
  new QtSliderDoubleComboRangedSlot( sscombo, state_handle );
  
  QPointer< QtSliderDoubleCombo > qpointer( sscombo );

  // Connect the state signal back to the SliderDoubleCombo's value variable
  new QtDeleteSlot( sscombo, state_handle->value_changed_signal_.connect(
    boost::bind( &QtSliderDoubleComboValueChangedSignal, qpointer, _1, _2 ) ) );
  
  // Connect the state signal back to the SliderDoubleCombo's range variables
  new QtDeleteSlot( sscombo, state_handle->range_changed_signal_.connect(
    boost::bind( &QtSliderDoubleComboRangeChangedSignal, qpointer, _1, _2, _3 ) ) );

  return true;
}
///// ====  END SliderDoubleCombo FUNCTIONS ==== /////

///// ====  BEGIN QToolButton FUNCTIONS ==== /////

void QtToolButtonToggleSignal( QPointer< QToolButton > qpointer, bool state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
     QtSignal( qpointer, boost::bind( &QToolButton::setChecked, qpointer.data(), state ) ); 
  }
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


///// ====  END QToolButton FUNCTIONS ==== /////
  
///// ====  BEGIN ColorButton FUNCTIONS ==== /////
void ColorButtonSignal( QPointer< QtColorButton > qpointer, Color state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QtColorButton::set_color, qpointer.data(), state ) ); 
  }
}

bool QtBridge::Connect( QtColorButton* colorbutton, StateColorHandle& state_handle )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new ColorButtonSlot( colorbutton, state_handle );
  QPointer< QtColorButton > qpointer( colorbutton );
  
  new QtDeleteSlot( colorbutton, state_handle->value_changed_signal_.connect( 
      boost::bind( &ColorButtonSignal, qpointer, _1, _2 ) ) );
  
  return true;
}
///// ====  END ColorButton FUNCTIONS ==== /////
  
///// ====  BEGIN QAction FUNCTIONS ==== /////

bool QtBridge::Connect( QAction* qaction, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionSlot( qaction, function );

  return true;
}

void QtActionToggledSignal( QPointer< QAction > qpointer, bool state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QAction::setChecked, qpointer.data(), state ) );
  }
}

bool QtBridge::Connect( QAction* qaction, StateBoolHandle& state_handle )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionToggleSlot( qaction, state_handle );

  QPointer< QAction > qpointer( qaction );
  new QtDeleteSlot( qaction, state_handle->value_changed_signal_.connect( 
    boost::bind( &QtActionToggledSignal, qpointer, _1, _2 ) ) );

  return true;
}

///// ====  END QAction FUNCTIONS ==== /////


///// ====  BEGIN QActionGroup FUNCTIONS ==== /////
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

bool QtBridge::Connect( QActionGroup* qactiongroup, StateOptionHandle& state_handle )
{
  new QtActionGroupSlot( qactiongroup, state_handle );
  QPointer< QActionGroup > qpointer( qactiongroup );

  new QtDeleteSlot( qactiongroup, state_handle->value_changed_signal_.connect( 
    boost::bind( &QtActionGroupSignal, qpointer, _1, _2 ) ) );

  return true;
}
///// ====  END QActionGroup FUNCTIONS ==== /////

} // end namespace Core
