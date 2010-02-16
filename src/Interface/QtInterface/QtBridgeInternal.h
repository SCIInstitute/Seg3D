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
#include <Interface/ToolInterface/CustomWidgets/SliderSpinComboInt.h>
#include <Interface/ToolInterface/CustomWidgets/SliderSpinComboDouble.h>

// boost includes
#include <boost/signals2/signal.hpp>

// Application includes
#include <Application/State/State.h>
#include <Application/State/Actions/ActionSet.h>


namespace Seg3D {

// QObjects for linking QT signals to state variables
// QT does *not* support templated classes (the moc does not process it), hence
// for each signal type, we need to have a dedicated slot and we cannot template
// this unfortunately. Hence we have helper classes that are inserted in between. 

// QtSlot class allows for blocking the signals going back into the application
// to be stopped. This should prevent a feedback of signals through the 
// interface. 

class QtSlot : public QObject {
  Q_OBJECT
  public:
  
    QtSlot( QObject* object, bool blocking = true) :
      QObject(object),
      blocked_(false),
      blocking_(blocking)
    {
      setObjectName(QString("AppSlot"));
    }
    
    virtual ~QtSlot() {}

    // BLOCK:
    // Block forwarding of signals to the application
    void block() { if (blocking_) blocked_ = true; }

    // UNBLOCK:
    // Unblock forwarding of signals to the application
    void unblock() { if (blocking_)  blocked_ = false; }    

  protected:
    // Whether feedback to the UI is blocked
    bool blocked_;
    
    // Whether this slot is blocking
    bool blocking_;
    
  public:

    template<class QPOINTER>
    static void Block(QPOINTER qpointer)
    {
      qpointer->findChild<QtSlot*>( QString("AppSlot"))->block();
    }

    template<class QPOINTER>
    static void Unblock(QPOINTER qpointer)
    {
      qpointer->findChild<QtSlot*>( QString("AppSlot"))->unblock();
    }
};


class QtCheckBoxSlot : public QtSlot {
  Q_OBJECT
  public:

    // Constructor
    QtCheckBoxSlot(QCheckBox* parent, 
      StateBoolHandle& state_handle,
      bool blocking = true) :
        QtSlot(parent,blocking),
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
      if (!blocked_) ActionSet::Dispatch(state_handle_,static_cast<bool>(state));
    }
    
  private:
    // Function object
    StateBoolHandle state_handle_;
};


class QtSliderSpinComboRangedIntSlot : public QtSlot {
  Q_OBJECT
  public:
    
     // Constructor
    QtSliderSpinComboRangedIntSlot(SliderSpinComboInt* parent, 
      StateRangedIntHandle& state_handle,
      bool blocking = true) :
        QtSlot(parent,blocking),
        state_handle_(state_handle) 
    {
      // Qt's connect function
      connect(parent,SIGNAL(valueAdjusted(int)),this,SLOT(slot(int)));
    }

    // Virtual destructor: needed by Qt
    virtual ~QtSliderSpinComboRangedIntSlot() {}

  public Q_SLOTS:
    // Slot that Qt will call
    void slot(int state)
    {
      if (!blocked_) ActionSet::Dispatch(state_handle_,static_cast<int>(state));
    }
    
  private:
    // Function object
    StateRangedIntHandle state_handle_;
};

class QtSliderSpinComboRangedDoubleSlot : public QtSlot {
  Q_OBJECT
  public:
    
     // Constructor
    QtSliderSpinComboRangedDoubleSlot(SliderSpinComboDouble* parent, 
      StateRangedDoubleHandle& state_handle,
      bool blocking = true) :
        QtSlot(parent,blocking),
        state_handle_(state_handle) 
    {
      // Qt's connect function
      connect(parent,SIGNAL(valueAdjusted(double)),this,SLOT(slot(double)));
    }

    // Virtual destructor: needed by Qt
    virtual ~QtSliderSpinComboRangedDoubleSlot() {}
    
  public Q_SLOTS:
    // Slot that Qt will call
    void slot(double state)
    {
      if (!blocked_) ActionSet::Dispatch(state_handle_,static_cast<double>(state));
    }
    
  private:
    // Function object
    StateRangedDoubleHandle state_handle_;

};


class QtComboBoxSlot : public QtSlot {
  Q_OBJECT
  public:

    // Constructor
    QtComboBoxSlot(QComboBox* parent, 
      StateOptionHandle& state_handle,
      bool blocking = true) :
        QtSlot(parent,blocking),
        state_handle_(state_handle) 
    {
      // Qt's connect function
      connect(parent,SIGNAL(currentIndexChanged(QString)),this,SLOT(slot(QString)));
    }
    
    // Virtual destructor: needed by Qt
    virtual ~QtComboBoxSlot() {}
    
  public Q_SLOTS:
    // Slot that Qt will call
    void slot(QString state)
    {
      if (!blocked_) ActionSet::Dispatch(state_handle_,state.toStdString());
    }
    
  private:
    // Function object
    StateOptionHandle state_handle_;
};


class QtActionSlot : public QObject {
  Q_OBJECT
  
  public:

    // Constructor
    QtActionSlot(QAction* parent, 
      boost::function<void ()> function) :
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
  

class QtActionToggleSlot : public QObject {
  Q_OBJECT
  
  public:

    // Constructor
    QtActionToggleSlot(QAction* parent, 
      StateBoolHandle& state_handle) :
        QObject(parent),
        state_handle_(state_handle)
    {
      // Qt's connect function
      connect(parent,SIGNAL(toggled(bool)),this,SLOT(slot(bool)));
    }

    // Virtual destructor: needed by Qt
    virtual ~QtActionToggleSlot() {}
    
  public Q_SLOTS:
    // Slot that Qt will call
    void slot(bool state)
    {
      ActionSet::Dispatch(state_handle_,state);
    }

  private:
    // Function object
    StateBoolHandle state_handle_;
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
  
class QtPushButtonSlot : public QObject {
  Q_OBJECT
  
public:
  
  // Constructor
  QtPushButtonSlot(QPushButton* parent, boost::function<void ()> function) :
  QObject(parent),
  function_(function)
  {
    // Qt's connect function
    connect(parent,SIGNAL(clicked()),this,SLOT(slot()));
  }
  
  // Virtual destructor: needed by Qt
  virtual ~QtPushButtonSlot() {}
  
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
