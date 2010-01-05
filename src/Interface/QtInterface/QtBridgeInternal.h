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

#ifndef INTERFACE_QTINTERFACE_QTBRIDGEINTERNAL_H
#define INTERFACE_QTINTERFACE_QTBRIDGEINTERNAL_H

// Qt includes
#include <QtGui>

// boost includes
#include <boost/signals2/signal.hpp>

// Application includes
#include <Application/State/StateValue.h>
#include <Application/State/StateClampedValue.h>
#include <Application/State/StateOption.h>
#include <Application/State/StateManager.h>

namespace Seg3D {

// QObjects for linking QT signals to state variables
// QT does *not* support templated classes (the moc does not process it), hence
// for each signal type, we need to have a dedicated slot and we cannot template
// this unfortunately. Hence we have helper classes that are inserted in between. 

class QtCheckBoxSlot : public QObject {
  Q_OBJECT
  public:

    // Constructor
    QtCheckBoxSlot(QCheckBox* parent, StateValue<bool>::Handle& state_handle) :
      QObject(parent),
      state_handle_(state_handle) 
    {
      // Qt's connect function
      connect(parent,SIGNAL(stateChanged(int)),this,SLOT(slot(int)));
    }
    
    // Virtual destructor: needed by Qt
    virtual ~QtCheckBoxSlot() {}
    
  public Q_SLOTS:
    // Slot that Qt will call
    void slot(int state)
    {
      state_handle_->dispatch(static_cast<bool>(state));
    }
    
  private:
    // Function object
    StateValue<bool>::Handle state_handle_;
};


class QtActionSlot : public QObject {
  Q_OBJECT
  
  public:

    // Constructor
    QtActionSlot(QAction* parent, boost::function<void ()> function) :
      QObject(parent),
      function_(function)
    {
      // Qt's connect function
      connect(parent,SIGNAL(triggered()),this,SLOT(slot()));
    }

    // Virtual destructor: needed by Qt
    virtual ~QtActionSlot() {}
    
  public Q_SLOTS:
    // Slot that Qt will call
    void slot()
    {
      function_();
    }

  private:
    // Function object
    boost::function<void ()> function_;
};
  
class QtToolButtonSlot : public QObject {
  Q_OBJECT
  
public:
  
  // Constructor
  QtToolButtonSlot(QToolButton* parent, boost::function<void ()> function) :
  QObject(parent),
  function_(function)
  {
    // Qt's connect function
    connect(parent,SIGNAL(clicked()),this,SLOT(slot()));
  }
  
  // Virtual destructor: needed by Qt
  virtual ~QtToolButtonSlot() {}
  
  public Q_SLOTS:
  // Slot that Qt will call
  void slot()
  {
    function_();
  }
  
private:
  // Function object
  boost::function<void ()> function_;
};

} //end namespace Seg3D

#endif
