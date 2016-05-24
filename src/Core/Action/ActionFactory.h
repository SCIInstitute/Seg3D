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

#ifndef CORE_ACTION_ACTIONFACTORY_H
#define CORE_ACTION_ACTIONFACTORY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>
#include <Core/Action/Action.h>

namespace Core
{

// ---- Auxiliary Classes -----
// ACTIONBUILDER:
/// This class is the base functor for the class that builds the classes in
/// the factory

class ActionBuilder;
typedef boost::shared_ptr<ActionBuilder> ActionBuilderHandle;
template <class ACTION> class ActionBuilderT;

class ActionBuilder
{
public:
  // ensure we can delete the builder correctly
  virtual ~ActionBuilder()
  {
  }
  
  // BUILD:
  /// the function call to build the object
  virtual ActionHandle build() = 0;
};

// ACTIONBUILDERT:
/// The actual instantiation that builds the action of type ACTION. This class
/// is loaded on top of the base functor and creates the action

template <class ACTION>
class ActionBuilderT: public ActionBuilder
{
public:
  // ensure we can delete the builder correctly
  virtual ~ActionBuilderT<ACTION>()
  {
  }
  
  // BUILD:
  /// the function call to build the object
  virtual ActionHandle build()
  { 
    return ActionHandle( new ACTION );
  }
};

// ------------------------------

// ACTIONFACTORY:
/// The factory object that instantiates the actions for the scripting and 
/// and playback systems in the application.

// Forward declaration of the private pieces of this class
class ActionFactoryPrivate;
typedef boost::shared_ptr< ActionFactoryPrivate > ActionFactoryPrivateHandle;


class ActionFactory : public Core::Lockable
{
  CORE_SINGLETON( ActionFactory );
  // -- Constructor / Destructor --
private:
  ActionFactory();
  virtual ~ActionFactory();
  

  // -- Instantiate actions --
public:
  // ACTION_LIST:
  /// Retrieve the full list of registered actions
  bool action_list( std::vector<std::string>& action_list );

  // CREATE_ACTION:
  // Generate an action from an iostream object that contains the XML
  /// specification of the action.
  bool create_action(const std::string& actionstring, ActionHandle& action,
    std::string& error, std::string& usage );

  // -- Action registration --
public:
  // REGISTER_ACTION:
  /// Register an action so that it can be automatically build in the action
  /// factory.
  void register_action( ActionBuilderHandle builder, ActionInfoHandle info );

  // -- implementation details --
private:
  ActionFactoryPrivateHandle private_;

  // -- Shortcuts for creating actions --
public:
  // CREATEACTION:
  /// Create an action from a string using the ActionFactory object
  static bool CreateAction( const std::string& actionstring,  ActionHandle& action,
    std::string& error);

  // CREATEACTION:
  /// Create an action from a string using the ActionFactory object
  /// This version also returns the usage of the action if the action could
  /// not be created
  static bool CreateAction( const std::string& actionstring, ActionHandle& action,
    std::string& error, std::string& usage );
};

/// Macro for adding function for registering action
// Note these functions will be called in the init
// call of the program.

#define CORE_REGISTER_ACTION(namesp, name)\
namespace Core\
{\
  using namespace namesp;\
  void register_Action##name()\
  {\
    ActionFactory::Instance()->register_action(\
      ActionBuilderHandle( new ActionBuilderT<Action##name> ),\
      Action##name::GetActionInfo() );\
  }\
}

} // end namespace Core

#endif
