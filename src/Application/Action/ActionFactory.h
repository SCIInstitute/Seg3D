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
#include <Utils/Core/Singleton.h>
#include <Utils/Core/Lockable.h>

// Application includes
#include <Application/Action/Action.h>

namespace Seg3D
{

// ---- Auxilary Classes -----
// ACTIONBUILDER:
// This class is the base functor for the class that builds the classes in
// the factory

class ActionBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~ActionBuilderBase()
  {
  }
  
  // the function call to build the object
  virtual ActionHandle build() = 0;
};

// ACTIONBUILDER:
// The actual instantiation that builds the action of type ACTION. This class
// is loaded on top of the base functor and creates the action

template <class ACTION>
class ActionBuilder: public ActionBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~ActionBuilder<ACTION>()
  {}
  
  // The actual builder call
  virtual ActionHandle build()
  { 
    return ActionHandle( new ACTION );
  }
};

// ------------------------------

// ACTIONFACTORY:
// The factory object that instantiates the actions for the scripting and 
// and playback systems in the application.

// Forward declaration of the private pieces of this class
class ActionFactoryPrivate;
typedef boost::shared_ptr< ActionFactoryPrivate > ActionFactoryPrivateHandle;


class ActionFactory : public Utils::Lockable
{
  CORE_SINGLETON( ActionFactory );
  // -- Constructor / Destructor --
private:
  ActionFactory();
  virtual ~ActionFactory();
  
public:
  // ACTION_LIST:
  // Retrieve the full list of registered actions
  typedef std::vector<std::string> action_list_type;
  bool action_list(action_list_type& action_list);

  // -- implementation details --
private:
  ActionFactoryPrivateHandle private_;

  // -- Instantiate actions --
public:

  // CREATE_ACTION:
  // Generate an action from an iostream object that contains the XML
  // specification of the action.
  bool create_action(const std::string& actionstring,
    ActionHandle& action,
    std::string& error,
    std::string& usage);

  // -- Action registration --
public:
  // REGISTER_ACTION:
  // Register an action so that it can be automatically build in the action
  // factory.
  void register_action( ActionBuilderBase* builder, std::string type );

  // -- Shortcuts for creating actions --
public:
  // CREATEACTION:
  // Create an action from a string using the ActionFactory object
  static bool CreateAction(const std::string& actionstring,
  ActionHandle& action,
  std::string& error);

  // CREATEACTION:
  // Create an action from a string using the ActionFactory object
  // This version also returns the usage of the action if the action could
  // not be created
  static bool CreateAction(const std::string& actionstring,
  ActionHandle& action,
  std::string& error,
  std::string& usage);

};

// Macro for adding function for registering action
// Note these functions will be called in the init
// call of the program.

#define CORE_REGISTER_ACTION(name)\
void register_Action##name()\
{\
  ActionFactory::Instance()->register_action( new ActionBuilder<Action##name>, Action##name::Type() );\
} 

} // end namespace seg3D

#endif
