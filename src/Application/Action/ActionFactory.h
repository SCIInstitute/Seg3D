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

#ifndef APPLICATION_ACTION_ACTIONFACTORY_H
#define APPLICATION_ACTION_ACTIONFACTORY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/StringUtil.h>
#include <Utils/Singleton/Singleton.h>

// Application includes
#include <Application/Action/Action.h>

namespace Seg3D {

// ---- Auxilary Classes -----
// ACTIONBUILDER:
// This class is the base functor for the class that builds the classes in
// the factory

class ActionBuilderBase {
  public:
    // ensure we can delete the builder correctly
    virtual ~ActionBuilderBase() {}
    // the functor call to build the object
    virtual ActionHandle build() = 0;
};

// ACTIONBUILDERT:
// The actual instantiation that builds the action of type ACTION. This class
// is loaded on top of the base functor and creates the action

template <class ACTION>
class ActionBuilder: public ActionBuilderBase {

  public:
    // ensure we can delete the builder correctly 
    virtual ~ActionBuilder<ACTION>() {}
    // The actual builder call
    virtual ActionHandle build() { return ActionHandle(new ACTION); }
};


// ------------------------------

// ACTIONFACTORY:
// The factory object that instantiates the actions for the scripting and 
// and playback systems in the application.

class ActionFactory : public boost::noncopyable  {

// -- Constructor --

  public:
    ActionFactory();

// -- Action registration --

  public:
    // REGISTER_ACTION:
    // Register an action so that it can be automatically build in the action
    // factory.
  
    template <class ACTION>
    void register_action(std::string action_name)
    {
      // ensure name is unique
      boost::to_lower(action_name);
      // Lock the factory
      boost::unique_lock<boost::mutex> lock(action_builders_mutex_);

      // Test is action was registered before.
      if (action_builders_.find(action_name) != action_builders_.end())
      {
        // Actions that are registered twice, will cause problems
        // Hence the program will throw an exception.
        // As registration is done on startup, this will cause a
        // faulty program to fail always on startup.
        SCI_THROW_LOGICERROR(std::string("Action '")+action_name+"' was registered twice");
      }

      // Register the action
      action_builders_[action_name] = new ActionBuilder<ACTION>;
      SCI_LOG_DEBUG(std::string("Registering action : ") + action_name);
    }

  private:
  
    // Mutex protecting the singleton interface  
    typedef boost::unordered_map<std::string,ActionBuilderBase*> action_map_type;
    // List with builders that can be called to generate a new object
    action_map_type  action_builders_;

    // Mutex for protecting registration
    boost::mutex     action_builders_mutex_;
    
// -- Instantiate actions --
  public:
    
    // CREATE_ACTION:
    // Generate an action from an iostream object that contains the XML
    // specification of the action.
    bool create_action(const std::string& actionstring,
                       ActionHandle& action,
                       std::string& error) const;

// -- Singleton interface --
  public:
    
    static ActionFactory* instance() { instance_.instance(); }

  private:
    static Utils::Singleton<ActionFactory> instance_;
};


// Macro for adding function for registering action
// Note these functions will be called in the init
// call of the program.
 
#define SCI_REGISTER_ACTION(name)\
void register_Action##name()\
{\
  ActionFactory::instance()->register_action<Action##name>(#name);\
} 

} // end namespace seg3D

#endif
