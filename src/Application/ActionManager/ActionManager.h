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

#ifndef APPLICATION_ACTIONMANAGER_ACTIONMANAGER_H
#define APPLICATION_ACTIONMANAGER_ACTIONMANAGER_H

// Boost includes for singleton
#include <boost/thread/mutex.hpp>
#include <boost/utility.hpp>

#include <Application/Component/Component.h>

namespace Seg3D {

class ActionManager;


// -- STATE --

class ActionManagerState : public StateBase {

// -- Constructor / Destructor --
  public:
    ActionManagerState(const std::string key) :
      StateBase(key) {}

};


// -- DISPATCHER --

class ActionManagerDispatcher : public DispatcherBase {

// -- Constructor / Destructor --
  public:
    ActionManagerDispatcher(ActionManager* component) : component_(component) {}
      
  private:
    ActionManager* component_;
};




// -- HANDLER --

class ActionManagerHandler : public HandlerBase {

// -- Constructor / Destructor --
  public:
    ActionManagerHandler(ActionManager* component) : component_(component) {}

  private:
    ActionManager* component_;


// -- Undo/Redo functions --

    void undo_action();
    
    void redo_action();

};




class ActionManager : public Component<DispatcherBase,StateBase,HandlerBase>
{

// -- Constructor / Destructor --
  public:
    ActionManager() : 
      Component<DispatcherBase,StateBase,HandlerBase("ActionManager") {}


// -- Singleton interface --
  public:
    
    static ActionManager* instance() { instance_.instance(); }
    static Singleton<ActionManager> instance_;
};

} // namespace Seg3D

#endif
