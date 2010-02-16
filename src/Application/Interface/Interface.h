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

#ifndef APPLICATION_INTERFACE_INTERFACE_H
#define APPLICATION_INTERFACE_INTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL 
#include <functional>

// Core includes
#include <Utils/Core/Log.h>
#include <Utils/Singleton/Singleton.h>
#include <Utils/EventHandler/EventHandler.h>

// Action includes
#include <Application/Action/ActionContext.h>
#include <Application/Action/ActionDispatcher.h>

// Interface includes
#include <Application/Interface/InterfaceActionContext.h>

namespace Seg3D {
 
// CLASS INTERFACE: 
// Interface is the abstraction of the interface layer
// It manages the events and thread of the user interface.

// Forward declaration
class Interface; 
 
// Class defintion
class Interface : public Utils::EventHandler {

// -- Constructor/Destructor --
  private:
    friend class Utils::Singleton<Interface>;
    Interface();
    
  public:
    virtual ~Interface();

// -- Action context interface --
  public:
    // INTERFACE_ACTION_CONTEXT:
    // Get the current interface action context
    InterfaceActionContextHandle interface_action_context();

  private:
    // Handle to the interface context that the interface should be using
    InterfaceActionContextHandle context_;

// -- Interface thread --    
  public:  
    
    // ISINTERFACETHREAD:
    // Test whether the current thread is the interface thread
    static bool IsInterfaceThread()
      { return (Instance()->is_eventhandler_thread()); }

    // POSTEVENT:
    // Short cut to the event handler
    static void PostEvent(boost::function<void ()> function);

    // POSTANDWAITEVENT:
    // Short cut to the event handler
    static void PostAndWaitEvent(boost::function<void ()> function);

    // POSTACTION:
    // Function that runs an action with the interface context
    static void PostAction(ActionHandle action);

// -- Singleton interface --
  public:
    
    // INSTANCE:
    // Get the singleton interface
    static Interface* Instance() { return instance_.instance(); }

  private:  
    static Utils::Singleton<Interface> instance_;
    
};

} // end namespace Seg3D

#endif
