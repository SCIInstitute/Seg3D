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

#ifndef CORE_INTERFACE_INTERFACE_H
#define CORE_INTERFACE_INTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// boost includes 
#include <boost/function.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/EventHandler/EventHandler.h>
#include <Core/Interface/InterfaceActionContext.h>

namespace Core
{

// CLASS INTERFACE: 
/// Interface is the abstraction of the interface layer
/// It manages the events and thread of the user interface.

// Forward declaration
class Interface;
class InterfacePrivate;

// Private part of the class 
typedef boost::shared_ptr<InterfacePrivate> InterfacePrivateHandle;

// Class defintion
class Interface : public Core::EventHandler
{
  CORE_SINGLETON( Interface );

  // -- Constructor/Destructor --
private:
  Interface();
  virtual ~Interface();

  // -- Action context interface --
public:
  // GET_WIDGET_ACTION_CONTEXT:
  /// Get the current interface action context for widgets
  InterfaceActionContextHandle get_widget_action_context();

  // GET_MOUSE_ACTION_CONTEXT:
  /// Get the current interface action context for mouse commands
  InterfaceActionContextHandle get_mouse_action_context();

  // GET_KEYBOARD_ACTION_CONTEXT:
  /// Get the current interface action context for keyboard commands
  InterfaceActionContextHandle get_keyboard_action_context();

private:
  InterfacePrivateHandle private_;

  // -- Interface thread --
public:

  // ISINTERFACETHREAD:
  /// Test whether the current thread is the interface thread
  static bool IsInterfaceThread()
  {
    return ( Instance()->is_eventhandler_thread() );
  }

  // POSTEVENT:
  /// Short cut to the event handler
  static void PostEvent( boost::function< void() > function );

  // POSTANDWAITEVENT:
  /// Short cut to the event handler
  static void PostAndWaitEvent( boost::function< void() > function );

  // GETWIDGETACTIONCONTEXT:
  /// Get the action context that widgets use to post their actions
  static InterfaceActionContextHandle GetWidgetActionContext();

  // GETMOUSEACTIONCONTEXT:
  /// Get the action context that mouse actions use to post their actions
  static InterfaceActionContextHandle GetMouseActionContext();

  // GETKEYBOARDACTIONCONTEXT:
  /// Get the action context that keyboard actions use to post their actions
  static InterfaceActionContextHandle GetKeyboardActionContext();

};

#define ASSERT_IS_INTERFACE_THREAD() assert( Core::Interface::IsInterfaceThread() )

} // end namespace Core

#endif
