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
#include <Interface/ToolInterface/CustomWidgets/SliderIntCombo.h>
#include <Interface/ToolInterface/CustomWidgets/SliderDoubleCombo.h>

// boost includes
#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

// Application includes
#include <Application/State/State.h>
#include <Application/State/Actions/ActionSet.h>
#include <Application/State/Actions/ActionSetRange.h>



namespace Seg3D
{


class QtDeleteSlot : public QObject
{
  Q_OBJECT
public:

  QtDeleteSlot( QObject* object, boost::signals2::connection connection ) :
      QObject( object ), connection_( connection )
  {
  }

    virtual ~QtDeleteSlot()
    {
       connection_.disconnect();
    }
 

private:
    
    boost::signals2::connection connection_; 
};


// QObjects for linking QT signals to state variables
// QT does *not* support templated classes (the moc does not process it), hence
// for each signal type, we need to have a dedicated slot and we cannot template
// this unfortunately. Hence we have helper classes that are inserted in between. 

// QtSlot class allows for blocking the signals going back into the application
// to be stopped. This should prevent a feedback of signals through the 
// interface. 

class QtSlot : public QObject
{
  Q_OBJECT
public:

  QtSlot( QObject* object, bool blocking = true ) :
      QObject( object ), blocked_( false ), blocking_( blocking )
  {
        setObjectName( QString( "AppSlot" ) );
  }

  virtual ~QtSlot()
  {
  }

  // BLOCK:
  // Block forwarding of signals to the application
  void block()
  {
    if ( blocking_ ) blocked_ = true;
  }

  // UNBLOCK:
  // Unblock forwarding of signals to the application
  void unblock()
  {
    if ( blocking_ ) blocked_ = false;
  }

protected:
  // Whether feedback to the UI is blocked
  bool blocked_;

  // Whether this slot is blocking
  bool blocking_;

public:

  template< class QPOINTER >
  static void Block( QPOINTER qpointer )
  {
    qpointer->findChild< QtSlot* > ( QString( "AppSlot" ) )->block();
  }

  template< class QPOINTER >
  static void Unblock( QPOINTER qpointer )
  {
    qpointer->findChild< QtSlot* > ( QString( "AppSlot" ) )->unblock();
  }
};

class QtCheckBoxSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtCheckBoxSlot( QCheckBox* parent, StateBoolHandle& state_handle, bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL( stateChanged( int ) ), this, SLOT( slot( int ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtCheckBoxSlot()
  {
  }

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

class QtLineEditSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtLineEditSlot( QLineEdit* parent, StateStringHandle& state_handle, bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL( textChanged( QString ) ), this, SLOT( slot( QString ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtLineEditSlot()
  {
  }

public Q_SLOTS:
  // Slot that Qt will call
  void slot(QString state)
  {
      std::string std_state = state.toStdString();
    if (!blocked_) ActionSet::Dispatch( state_handle_, std_state );
  }

private:
  // Function object
  StateStringHandle state_handle_;
};


class QtDoubleSpinBoxSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtDoubleSpinBoxSlot( QDoubleSpinBox* parent, StateDoubleHandle& state_handle, bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL( valueChanged( double ) ), this, SLOT( slot( double ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtDoubleSpinBoxSlot()
  {
  }

public Q_SLOTS:
  // Slot that Qt will call
  void slot(double state)
  {
    if (!blocked_) ActionSet::Dispatch( state_handle_, state );
  }

private:
  // Function object
  StateDoubleHandle state_handle_;
};




class QtLineEditAliasSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtLineEditAliasSlot( QLineEdit* parent, StateAliasHandle& state_handle, bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL( textChanged( QString ) ), this, SLOT( slot( QString ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtLineEditAliasSlot()
  {
  }

public Q_SLOTS:
  // Slot that Qt will call
  void slot(QString state)
  {
      std::string std_state = state.toStdString();
    if (!blocked_) ActionSet::Dispatch( state_handle_, std_state );
  }

private:
  // Function object
  StateAliasHandle state_handle_;
};

// SLOT FOR CONNECTING THE SliderIntCombo TO THE STATE ENGINE
class QtSliderIntComboRangedSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtSliderIntComboRangedSlot( SliderIntCombo* parent, StateRangedIntHandle& state_handle,
    bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL( valueAdjusted( int ) ), this, SLOT( value_slot( int ) ) );
    connect( parent, SIGNAL( rangeChanged( int, int ) ), this, SLOT( range_slot( int, int ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtSliderIntComboRangedSlot()
  {
  }

public Q_SLOTS:
  // Slot that Qt will call
  void value_slot(int state)
  {
    if (!blocked_) ActionSet::Dispatch( state_handle_, state );
  }
    void range_slot(int min, int max)
  {
      
    if (!blocked_) ActionSetRange::Dispatch( state_handle_, min, max );
  }

private:
  // Function object
  StateRangedIntHandle state_handle_;
};

// SLOT FOR CONNECTING THE SliderDoubleCombo TO THE STATE ENGINE
class QtSliderDoubleComboRangedSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtSliderDoubleComboRangedSlot( SliderDoubleCombo* parent, StateRangedDoubleHandle& state_handle,
    bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL( valueAdjusted( double ) ), this, SLOT( value_slot( double ) ) );
    connect( parent, SIGNAL( rangeChanged( double, double ) ), this, SLOT( range_slot( double, double ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtSliderDoubleComboRangedSlot()
  {
  }

public Q_SLOTS:
  // Slot that Qt will call
  void value_slot(double state)
  {
    if (!blocked_) ActionSet::Dispatch( state_handle_, state );
  }
    void range_slot(double min, double max)
  {
      
    if (!blocked_) ActionSetRange::Dispatch( state_handle_, min, max );
  }

private:
  // Function object
  StateRangedDoubleHandle state_handle_;
};



class QtComboBoxSlot : public QtSlot
{
  Q_OBJECT
public:

  // Constructor
  QtComboBoxSlot( QComboBox* parent, StateOptionHandle& state_handle, bool blocking = true) :
    QtSlot(parent,blocking),
    state_handle_(state_handle)
  {
    // Qt's connect function
    connect( parent, SIGNAL( currentIndexChanged( QString )), this,SLOT( option_slot( QString ) ));
    
  }
  
  QtComboBoxSlot( QComboBox* parent, StateStringHandle& state_handle, bool blocking = true) :
  QtSlot(parent,blocking),
  state_string_handle_(state_handle)
  {
    // Qt's connect function
    connect( parent, SIGNAL( currentIndexChanged( QString )), this,SLOT( string_slot( QString ) ));
    
  }

  // Virtual destructor: needed by Qt
  virtual ~QtComboBoxSlot()
  {}

public Q_SLOTS:
  // Slot that Qt will call
  void option_slot(QString state)
  {
    if (!blocked_) ActionSet::Dispatch(state_handle_,state.toStdString());
  }
  void string_slot(QString state)
  {
    if (!blocked_) ActionSet::Dispatch(state_string_handle_,state.toStdString());
  }

private:
  // Function object
  StateOptionHandle state_handle_;
  StateStringHandle state_string_handle_;
};

class QtActionSlot : public QObject
{
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
  virtual ~QtActionSlot()
  {}

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

class QtActionToggleSlot : public QtSlot
{
  Q_OBJECT

public:

  // Constructor
  QtActionToggleSlot( QAction* parent, StateBoolHandle& state_handle ) :
    QtSlot( parent ),
    state_handle_( state_handle )
  {
    // Qt's connect function
    connect( parent, SIGNAL(  toggled( bool ) ), this, SLOT( slot( bool ) ) );
  }

  // Virtual destructor: needed by Qt
  virtual ~QtActionToggleSlot()
  {}

public Q_SLOTS:
  // Slot that Qt will call
  void slot(bool state)
  {
    if ( !this->blocked_ )
    {
      ActionSet::Dispatch(state_handle_,state);
    }
  }

private:
  // Function object
  StateBoolHandle state_handle_;
};
  
class QtToolButtonToggleSlot : public QtSlot
{
  Q_OBJECT
  
public:
  
  // Constructor
  QtToolButtonToggleSlot(QToolButton* parent, StateBoolHandle& state_handle, bool blocking = true ) :
  QtSlot( parent, blocking ), state_handle_( state_handle )
  {
    // Qt's connect function
    connect(parent,SIGNAL(toggled(bool)),this,SLOT(slot(bool)));
  }
  
  // Virtual destructor: needed by Qt
  virtual ~QtToolButtonToggleSlot()
  {}
  
  public Q_SLOTS:
  // Slot that Qt will call
  void slot(bool state)
  {
    if (!blocked_) ActionSet::Dispatch(state_handle_,state);
  }
  
private:
  // Function object
  StateBoolHandle state_handle_;
};



class QtToolButtonSlot : public QObject
{
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
  virtual ~QtToolButtonSlot()
  {}

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

class QtPushButtonSlot : public QObject
{
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
  virtual ~QtPushButtonSlot()
  {}

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

class QtActionGroupSlot : public QtSlot
{
  Q_OBJECT

public:
  QtActionGroupSlot(QActionGroup* parent, StateOptionHandle& state_handle) :
    QtSlot(parent), option_state_(state_handle)
  {
    this->connect(parent, SIGNAL(triggered(QAction*)), this, SLOT(slot(QAction*)));
  }

  virtual ~QtActionGroupSlot()
  {}

public Q_SLOTS:

  void slot(QAction* action)
  {
    if (!this->blocked_)
    {
      ActionSet::Dispatch(this->option_state_, action->objectName().toStdString());
    }
  }

private:
  StateOptionHandle option_state_;
};

} //end namespace Seg3D

#endif
