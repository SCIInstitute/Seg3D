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


///// ====  BEGIN QCheckBox FUNCTIONS ==== /////
// -- BEGIN SIGNAL CONNECTORS FOR THE QCheckBox's -- //
// signal for when the value of the QCheckBox has changed //
void QtCheckBoxSignal( QPointer< QCheckBox > qpointer, bool state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QCheckBox::setChecked, 
      qpointer.data(), state ) ); 
  }
}
// -- END SIGNAL CONNECTORS FOR THE QCheckBox's -- //


// -- BEGIN CONNECT FUNCTION FOR CONNECTING QCheckBox's TO StateBoolHandle's -- //
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
// -- END CONNECT FUNCTION -- //
///// ====  END QCheckBox FUNCTIONS ==== /////




///// ====  BEGIN QLineEdit FUNCTIONS ==== /////
// -- BEGIN SIGNAL CONNECTORS FOR THE QLineEdit's -- //
// signal for when the QLineEdit has changed //
void QtLineEditSignal( QPointer< QLineEdit > qpointer, std::string state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QLineEdit::setText, qpointer.data(), QString::fromStdString( state ) ) ); 
  }
}
// -- END SIGNAL CONNECTORS FOR THE QComboBox's -- //

// -- BEGIN CONNECT FUNCTION FOR CONNECTING QLineEdit's TO StateStringHandle's -- //
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
// -- END CONNECT FUNCTION -- //

// -- BEGIN CONNECT FUNCTION FOR CONNECTING QLineEdit's TO StateAliasHandle's -- //
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
// -- END CONNECT FUNCTION -- //
///// ====  END QLineEdit FUNCTIONS ==== /////


///// ====  BEGIN QDoubleSpinBox FUNCTIONS ==== /////
    // -- BEGIN SIGNAL CONNECTORS FOR THE QDoubleSpinBox's -- //
        // signal for when the QDoubleSpinBox has changed //
        void QtDoubleSpinBoxSignal( QPointer< QDoubleSpinBox > qpointer, double state, ActionSource source )
        {
          if ( source != ActionSource::INTERFACE_E )
          {
            QtSignal( qpointer, boost::bind( &QDoubleSpinBox::setValue, qpointer.data(), state ) ); 
          }
        }
    // -- END SIGNAL CONNECTORS FOR THE QDoubleSpinBox's -- //

    // -- BEGIN CONNECT FUNCTION FOR CONNECTING QDoubleSpinBox's TO StateDoubleHandle's -- //
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
    // -- END CONNECT FUNCTION -- //
///// ====  BEGIN QDoubleSpinBox FUNCTIONS ==== /////


///// ====  BEGIN QComboBox FUNCTIONS ==== /////
    // -- BEGIN SIGNAL CONNECTORS FOR THE QComboBox's -- //
        // signal for when the index of the QComboBox has changed //
        void QtComboBoxSignal( QComboBox* qcombobox, std::string state )
        {
          QString qstring_state = QString::fromStdString( state );
          int index = qcombobox->findText( qstring_state, Qt::MatchFlags( Qt::CaseInsensitive ) );

          if ( index >= 0 )
          {
            qcombobox->setCurrentIndex( index );
          }
        }
    // signal for when the value of the QComboBox has changed //
        void QtComboBoxSignal( QPointer< QComboBox > qpointer, std::string state, ActionSource source )
        {
          if ( source != ActionSource::INTERFACE_E )
          {
            QtSignal( qpointer, boost::bind( &QtComboBoxSignal, qpointer.data(), state ) );
          }
        }
    // -- END SIGNAL CONNECTORS FOR THE QComboBox's -- //


    // -- BEGIN CONNECT FUNCTION FOR CONNECTING QComboBox's TO StateOptionHandle's -- //
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
    // -- END CONNECT FUNCTION -- //
///// ====  END QComboBox FUNCTIONS ==== /////

///// ====  BEGIN SliderIntCombo FUNCTIONS ==== /////
    // -- BEGIN SIGNAL CONNECTORS FOR THE SliderIntCombo's -- //
        // signal for when the value of the SliderIntCombo has changed //
        void QtSliderIntComboValueChangedSignal( QPointer< SliderIntCombo > qpointer, int state,
            ActionSource source )
        {
          if ( source != ActionSource::INTERFACE_E )
          {
            QtSignal( qpointer, boost::bind( &SliderIntCombo::setCurrentValue, qpointer.data(), state ) );
          }
        }
        // signal for when the range of the SliderIntCombo has changed //
        void QtSliderIntComboRangeChangedSignal( QPointer< SliderIntCombo > qpointer, int min, int max,
            ActionSource source )
        {
          if ( source != ActionSource::INTERFACE_E )
          {
            QtSignal( qpointer, boost::bind( &SliderIntCombo::setRange, qpointer.data(), min, max ) );
          }
        }
    // -- END SIGNAL CONNECTORS FOR THE SliderIntCombo's -- //


    // -- BEGIN CONNECT FUNCTION FOR CONNECTING SliderIntCombo's TO StateRangedIntHandle's -- //
        bool QtBridge::Connect( SliderIntCombo* sscombo, StateRangedIntHandle& state_handle )
        {
          new QtSliderIntComboRangedSlot( sscombo, state_handle );
          
          QPointer< SliderIntCombo > qpointer( sscombo );

          // Connect the state signal back to the SliderIntCombo's value variable
          new QtDeleteSlot( sscombo, state_handle->value_changed_signal_.connect(
              boost::bind( &QtSliderIntComboValueChangedSignal, qpointer, _1, _2 ) ) );
          
          // Connect the state signal back to the SliderIntCombo's range variables  
          new QtDeleteSlot( sscombo, state_handle->range_changed_signal_.connect(
              boost::bind( &QtSliderIntComboRangeChangedSignal, qpointer, _1, _2, _3 ) ) );

          return true;
        }
    // -- END CONNECT FUNCTION -- //
///// ====  END SliderIntCombo FUNCTIONS ==== /////
  
  
///// ====  BEGIN ColorBarWidget FUNCTIONS ==== /////
    // -- BEGIN SIGNAL CONNECTORS FOR THE ColorBarWidget's -- //
  // signal for when the value of the ColorBarWidget has changed //
  void QtColorBarWidgetValueChangedSignal( QPointer< ColorBarWidget > qpointer, int state,
                      ActionSource source )
  {
    if ( source != ActionSource::INTERFACE_E )
    {
      QtSignal( qpointer, boost::bind( &ColorBarWidget::set_color_index, qpointer.data(), state ) );
    }
  }
    // -- END SIGNAL CONNECTORS FOR THE ColorBarWidget's -- //
  
    // -- BEGIN CONNECT FUNCTION FOR CONNECTING ColorBarWidget's TO StateIntHandle's -- //
  bool QtBridge::Connect( ColorBarWidget* cbwidget, StateIntHandle& state_handle )
  {
    new QtColorBarWidgetSlot( cbwidget, state_handle );
    
    QPointer< ColorBarWidget > qpointer( cbwidget );
    
    // Connect the state signal back to the ColorBarWidget's value variable
    new QtDeleteSlot( cbwidget, state_handle->value_changed_signal_.connect(
      boost::bind( &QtColorBarWidgetValueChangedSignal, qpointer, _1, _2 ) ) );
    
    return true;
  }
    // -- END CONNECT FUNCTION -- //
///// ====  END ColorBarWidget FUNCTIONS ==== /////


///// ====  BEGIN SliderDoubleCombo FUNCTIONS ==== /////
    // -- BEGIN SIGNAL CONNECTORS FOR THE SliderDoubleCombo's -- //
        // signal for when the value of the SliderDoubleCombo has changed //
        void QtSliderDoubleComboValueChangedSignal( QPointer< SliderDoubleCombo > qpointer, double state,
            ActionSource source )
        {
          if ( source != ActionSource::INTERFACE_E )
          {
            QtSignal( qpointer, boost::bind( &SliderDoubleCombo::setCurrentValue, qpointer.data(), state ) );
          }
        }
        // signal for when the range of the SliderDoubleCombo has changed //
        void QtSliderDoubleComboRangeChangedSignal( QPointer< SliderDoubleCombo > qpointer, double min, double max,
            ActionSource source )
        {
          if ( source != ActionSource::INTERFACE_E )
          {
            QtSignal( qpointer, boost::bind( &SliderDoubleCombo::setRange, qpointer.data(), min, max ) );
          }
        }
    // -- END SIGNAL CONNECTORS FOR THE SliderDoubleCombo's -- //



    // -- BEGIN CONNECT FUNCTION FOR CONNECTING SliderDoubleCombo's TO StateRangedDoubleHandle's -- //
        bool QtBridge::Connect( SliderDoubleCombo* sscombo, StateRangedDoubleHandle& state_handle )
        {
          new QtSliderDoubleComboRangedSlot( sscombo, state_handle );
          
          QPointer< SliderDoubleCombo > qpointer( sscombo );

          // Connect the state signal back to the SliderDoubleCombo's value variable
          new QtDeleteSlot( sscombo, state_handle->value_changed_signal_.connect(
              boost::bind( &QtSliderDoubleComboValueChangedSignal, qpointer, _1, _2 ) ) );
          
          // Connect the state signal back to the SliderDoubleCombo's range variables
            new QtDeleteSlot( sscombo, state_handle->range_changed_signal_.connect(
                boost::bind( &QtSliderDoubleComboRangeChangedSignal, qpointer, _1, _2, _3 ) ) );

          return true;
        }
    // -- END CONNECT FUNCTION -- //
///// ====  END SliderDoubleCombo FUNCTIONS ==== /////

///// ====  BEGIN QToolButton FUNCTIONS ==== /////
    // -- BEGIN SIGNAL CONNECTORS FOR THE QToolButton's -- //
        // signal for when the value of the QToolButton has changed //
            void QtToolButtonToggleSignal( QPointer< QToolButton > qpointer, bool state, ActionSource source )
            {
                if ( source != ActionSource::INTERFACE_E )
                {
                   QtSignal( qpointer, boost::bind( &QToolButton::setChecked, 
                  qpointer.data(), state ) ); 
                }
            }
        // -- END SIGNAL CONNECTORS FOR THE QToolButton's -- //

    // -- BEGIN CONNECT FUNCTION FOR CONNECTING QToolButton's TO void() function's -- //
        bool QtBridge::Connect( QToolButton* qtoolbutton, boost::function< void() > function )
        {
          // Link the slot to the parent widget, so Qt's memory manager will
          // manage this one.
          new QtToolButtonSlot( qtoolbutton, function );

          return true;
        }
    // -- END CONNECT FUNCTION -- //

    // -- BEGIN CONNECT FUNCTION FOR CONNECTING QToolButton's TO StateBoolHandle's -- //
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
    // -- END CONNECT FUNCTION -- //
///// ====  END QToolButton FUNCTIONS ==== /////
  

// -- BEGIN CONNECT FUNCTION FOR CONNECTING QPushButton's TO void() function's -- //
bool QtBridge::Connect( QPushButton* qpushbutton, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtPushButtonSlot( qpushbutton, function );

  return true;
}
// -- END CONNECT FUNCTION -- //


// Menu connectors
///// ====  BEGIN QAction FUNCTIONS ==== /////
// -- BEGIN CONNECT FUNCTION FOR CONNECTING QAction's TO void() function's -- //
bool QtBridge::Connect( QAction* qaction, boost::function< void() > function )
{
  // Link the slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionSlot( qaction, function );

  return true;
}
// -- END CONNECT FUNCTION -- //

void QtActionToggledSignal( QPointer< QAction > qpointer, bool state, ActionSource source )
{
  if ( source != ActionSource::INTERFACE_E )
  {
    QtSignal( qpointer, boost::bind( &QAction::setChecked, qpointer.data(), state ) );
  }
}

// -- BEGIN CONNECT FUNCTION FOR CONNECTING QAction's TO StateBoolHandle's -- //
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
// -- END CONNECT FUNCTION -- //
///// ====  END QAction FUNCTIONS ==== /////


///// ====  BEGIN QActionGroup FUNCTIONS ==== /////
// -- BEGIN SIGNAL CONNECTORS FOR THE QActionGroup's -- //
// signal for when the value of the QActionGroup has changed //
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
// -- END SIGNAL CONNECTORS FOR THE QActionGroup's -- //


// -- BEGIN CONNECT FUNCTION FOR CONNECTING QActionGroup's TO StateOptionHandle's -- //
bool QtBridge::Connect( QActionGroup* qactiongroup, StateOptionHandle& state_handle )
{
  new QtActionGroupSlot( qactiongroup, state_handle );
  QPointer< QActionGroup > qpointer( qactiongroup );

  new QtDeleteSlot( qactiongroup, state_handle->value_changed_signal_.connect( 
    boost::bind( &QtActionGroupSignal, qpointer, _1, _2 ) ) );

  return true;
}
// -- END CONNECT FUNCTION -- //
///// ====  END QActionGroup FUNCTIONS ==== /////


} // end namespace Seg3D
