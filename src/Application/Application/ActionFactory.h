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

#ifndef APPLICATION_APPLICATION_ACTIONFACTORY_H
#define APPLICATION_APPLICATION_ACTIONFACTORY_H

#include <Application/Application/Action.h>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// STL includes
#include <map>

namespace Seg3D {

// ---- Auxilary Classes -----
// ACTIONBUILDER:
// This class is the base functor for the class that builds the classes in
// the factory

class ActionBuilder {
  public:
    // ensure we can delete the builder correctly
    virtual ~ActionBuilder() {}
    // the functor call to build the object
    virtual ActionHandle build() = 0;
};

// ACTIONBUILDERT:
// The actual instantiation that builds the action of type ACTION. This class
// is loaded on top of the base functor and creates the action

template <class ACTION>
class ActionBuilderT: public ActionBuilder {

  public:
    // ensure we can delete the builder correctly 
    virtual ~ActionBuilderT<ACTION>() {}
    // The actual builder call
    virtual ActionHandle build() { return ActionHandle(new ACTION); }
};

// ------------------------------


// ACTIONFACTORY:
// The factory object that instantiates the actions for the scripting and 
// and playback systems in the application.

class ActionFactory : public boost::noncopyable  {

// -- Constructor/ Destructor --

  public:
    ActionFactory();
    virtual ~ActionFactory(); 

// -- Action registration --

  public:
    // REGISTER_ACTION:
    // Register an action so that it can be automatically build in the action
    // factory.
  
    template <class ACTION>
    void register_action()
    {
      // Lock the factory
      boost::unique_lock<boost::mutex> lock(action_factory_mutex_);
      // Register the action
      action_builders_[ACTION::get_classname()] = new ActionBuilderT<ACTION>;
    }

  private:
  
    // Mutex protecting the singleton interface  
    typedef std::map<std::string,ActionBuilder*> action_map_type;
    // List with builders that can be called to generate a new object
    action_map_type  action_builders_;

// -- Instantiate actions --

  public:
    
    // CREATE_ACTION:
    // Generate an action from an iostream object that contains the XML
    // specification of the action.
    
    // ActionHandle create_action(IOStream* iostream);

// -- Singleton interface --
  public:
  
    // INSTANCE:
    // Get the singleton pointer to the application
    
    static ActionFactory* instance();

  private:
  
    // Mutex protecting the singleton interface
    static boost::mutex     action_factory_mutex_;
    // Initialized or not?
    static bool initialized_;
    // Pointer that contains the singleton interface to this class
    static ActionFactory*   action_factory_;
};


} // end namespace seg3D

#endif
