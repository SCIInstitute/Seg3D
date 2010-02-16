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

namespace Seg3D {

// -- Checkbox connector --

void 
QtCheckBoxSignal(QPointer<QCheckBox> qpointer, bool state, 
                 ActionSource source)
{
  if(source != ACTION_SOURCE_INTERFACE_E)
  {
    if (!(Interface::Instance()->IsInterfaceThread()))
    {
      Interface::PostEvent(boost::bind( &QtCheckBoxSignal, qpointer, state,
         ACTION_SOURCE_NONE_E));
      return;
    }
    
    if (qpointer.data()) 
    {
      // Blocking the slot will prevent the signal to loop back to the application
      QtSlot::Block(qpointer);
      qpointer->setChecked(state);
      QtSlot::Unblock(qpointer);
    }
  }
}


void 
QtComboBoxSignal(QPointer<QComboBox> qpointer, std::string state,
                 ActionSource source)
{
  if(source != ACTION_SOURCE_INTERFACE_E)
  {
    if (!(Interface::Instance()->IsInterfaceThread()))
    {
      Interface::PostEvent(boost::bind( &QtComboBoxSignal, qpointer, state,
         ACTION_SOURCE_NONE_E));
      return;
    }
    if (qpointer.data()) 
    {
      //convert from our std::string back to a QString so that we can check to see if it exists
      //in the combo box.
      QString qstring_state = QString::fromStdString(state);

      int index = qpointer->findText(qstring_state, Qt::MatchFlags(Qt::CaseInsensitive));

      if(index >= 0)
      {
        QtSlot::Block(qpointer);
        qpointer->setCurrentIndex(index);
        QtSlot::Unblock(qpointer);
      }
    }
  }
}


void 
QtSliderSpinComboRangedIntSignal(QPointer<SliderSpinComboInt> qpointer, 
                                 int state, ActionSource source)
{
  if(source != ACTION_SOURCE_INTERFACE_E)
  { 
     if (!(Interface::Instance()->IsInterfaceThread()))
    {
      Interface::PostEvent(boost::bind(&QtSliderSpinComboRangedIntSignal,
                    qpointer, state, ACTION_SOURCE_NONE_E));
      return;
    }

    if (qpointer.data()) 
    {
      QtSlot::Block(qpointer);
      qpointer->setCurrentValue(state);
      QtSlot::Unblock(qpointer);
    }
  }
}

void
QtSliderSpinComboRangedDoubleSignal(QPointer<SliderSpinComboDouble> qpointer, 
                                    double state, ActionSource source)
{
  if(source != ACTION_SOURCE_INTERFACE_E)
  { 
     if (!(Interface::Instance()->IsInterfaceThread()))
    {
      Interface::PostEvent(boost::bind(&QtSliderSpinComboRangedDoubleSignal,
                    qpointer, state, ACTION_SOURCE_NONE_E));
      return;
    }

    if (qpointer.data()) 
    {
      QtSlot::Block(qpointer);
      qpointer->setCurrentValue(state);
      QtSlot::Unblock(qpointer);
    }
  }
}


bool
QtBridge::connect(QCheckBox* qcheckbox, 
                  StateBoolHandle& state_handle)
{
  // Connect the dispatch into the StateVariable (with auxillary object)
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtCheckBoxSlot(qcheckbox,state_handle);
    
  // Connect the state signal back into the Qt Variable  
  state_handle->value_changed_signal_.connect(
    boost::bind( &QtCheckBoxSignal, qcheckbox, _1, _2 ));
  
  return (true);
}


bool
QtBridge::connect(QComboBox* qcombobox,
                  StateOptionHandle& state_handle)
{
  // Connect the dispatch into the StateVariable (with auxillary object)
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtComboBoxSlot(qcombobox,state_handle);

  // Connect the state signal back to the Qt Variable
  state_handle->value_changed_signal_.connect(
    boost::bind( &QtComboBoxSignal, qcombobox, _1, _2 ));

  return (true);
}


bool
QtBridge::connect(SliderSpinComboInt* sscombo,
                  StateRangedIntHandle& state_handle)
{
  new QtSliderSpinComboRangedIntSlot(sscombo,state_handle);

  // Connect the state signal back to the Qt Variable
  state_handle->value_changed_signal_.connect(
    boost::bind( &QtSliderSpinComboRangedIntSignal, sscombo, _1, _2 ));

  return (true);
}


bool
QtBridge::connect(SliderSpinComboDouble* sscombo,
                    StateRangedDoubleHandle& state_handle)
{
  new QtSliderSpinComboRangedDoubleSlot(sscombo,state_handle);

  // Connect the state signal back to the Qt Variable
  state_handle->value_changed_signal_.connect(
    boost::bind( &QtSliderSpinComboRangedDoubleSignal, sscombo, _1, _2 ));

  return (true);
}

  
bool
QtBridge::connect(QToolButton* qtoolbutton, 
                  boost::function<void ()> function)
{
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtToolButtonSlot(qtoolbutton,function);
  
  return (true);
}


bool
QtBridge::connect(QPushButton* qpushbutton, 
                  boost::function<void ()> function)
{
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtPushButtonSlot(qpushbutton,function);
  
  return (true);
}


// Menu connectors
bool
QtBridge::connect(QAction* qaction, 
                  boost::function<void ()> function)
{
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionSlot( qaction, function );

  return (true);
}


bool
QtBridge::connect(QAction* qaction, 
                  StateBoolHandle& state_handle)
{
  // Link tbe slot to the parent widget, so Qt's memory manager will
  // manage this one.
  new QtActionToggleSlot( qaction, state_handle );

  return (true);
}


} // end namespace Seg3D
