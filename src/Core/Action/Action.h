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

#ifndef CORE_ACTION_ACTION_H
#define CORE_ACTION_ACTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <vector>
#include <string>
#include <map>

// Boost includes
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/Utils/EnumClass.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/IntrusiveBase.h>

// Application includes
#include <Core/Action/ActionContext.h>
#include <Core/Action/ActionParameter.h>
#include <Core/Action/ActionCachedHandle.h>
#include <Core/Action/ActionResult.h>

namespace Core
{

class Action;
typedef boost::intrusive_ptr< Action > ActionHandle;
typedef std::vector< ActionHandle > ActionHandleList;

// CLASS ACTION:
// Main class that defines an action in the program
// An action is not copyable as that would invalidate 
// the ActionParameter pointers used internally.

class Action : public Core::IntrusiveBase
{
  // -- Constructor/Destructor --
public:
  // Construct an action of a certain type and with certain properties
  Action();

  // Virtual destructor for memory management of derived classes
  virtual ~Action(); // << NEEDS TO BE REIMPLEMENTED

  // These functions define the properties of the action and are implemented
  // by the SCI_ACTION_TYPE macro
  virtual std::string type() const = 0;
  virtual std::string usage() const = 0;

  // -- Run/Validate interface --
public:

  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  // NOTE: If this function is *not* overloaded the function will return true.
  // NOTE: This function is *not* const and may alter the values of the parameters
  //       and correct faulty input. Run on the other hand is not allowed to
  //       change anything in the action, as it is posted to any observers
  //       after the action is validated.

  virtual bool validate( ActionContextHandle& context );

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  // NOTE: In case of an asynchronous action, the return value is ignored and the
  // program relies on report_done() from the context to be triggered when
  // the asynchronous part has finished. In any other case the ActionDispatcher
  // will issue the report_done() when run returns.

  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) = 0;
  // -- Action parameters --

public:

  // ADD_ARGUMENT:
  // A argument to the action needs to be registered with the base
  // class so we can import and export the arguments to a string.
  // This function links the arguments of the action to an internal
  // record of all the arguments
  template< class ARGUMENT >
  void add_argument( ARGUMENT& argument )
  {
    add_argument_ptr( &argument );
  }

  // ADD_ARGUMENT (WITH DEFAULT VALUE):
  // A argument to the action needs to be registered with the base
  // class so we can import and export the arguments to a string.
  // This function links the arguments of the action to an internal
  // record of all the arguments
  template< class ARGUMENT, class T >
  void add_argument( ARGUMENT& argument, const T& default_value )
  {
    argument.value() = default_value;
    add_argument_ptr( &argument );
  }

  // ADD_PARAMETER:
  // A parameter needs to be registered with the base class
  // so we can import and export the parameters to a string.
  // This function links the parameters of the action to an internal
  // key value pair system to records all the parameters
  template< class PARAMETER >
  void add_parameter( const std::string& key, PARAMETER& param )
  {
    add_parameter_ptr( key, &param );
  }

  // ADD_PARAMETER (WITH DEFAULT VALUE):
  // A parameter needs to be registered with the base class
  // so we can import and export the parameters to a string.
  // This function links the parameters of the action to an internal
  // key value pair system to records all the parameters
  template< class PARAMETER, class T >
  void add_parameter( const std::string& key, PARAMETER& param, const T& default_value )
  {
    param.value() = default_value;
    add_parameter_ptr( key, &param );
  }

  // ADD_CACHEDHANDLE:
  // A cached handle needs to be registered so they can be deleted
  // once the action has been completed.
  template< class CACHEDHANDLE >
  void add_cachedhandle( CACHEDHANDLE& handle )
  {
    add_cached_handle_ptr( &handle );
  }

  // EXPORT_TO_STRING:
  // Export the action command into a string, so it can stored
  // The action factory can recreate the action from this string
  std::string export_to_string() const;

  // IMPORT_ACTION_FROM_STRING:
  // Import an action command from astring. This function is used by the
  // ActionFactory.
  bool import_from_string( const std::string& action, std::string& error );

  // IMPORT_ACTION_FROM_STRING:
  // Same as function above, but without the error report
  bool import_from_string( const std::string& action );

  // CLEAR_CACHE:
  // Clear any objects that were given as a short cut to improve performance.
  void clear_cache();

private:

  // IMPLEMENTATION OF ADD_PARAMETER, ADD_ARGUMENT AND ADD_CACHEDHANDLE:
  // These take pointers to the base class, the ones defined above work
  // with references of the parameters for more convenience.
  void add_argument_ptr( ActionParameterBase* param );
  void add_parameter_ptr( const std::string& key, ActionParameterBase* param );
  void add_cached_handle_ptr( ActionCachedHandleBase* handle );

  // Typedefs
  typedef std::vector< ActionParameterBase* > argument_vector_type;
  typedef std::map< std::string, ActionParameterBase* > parameter_map_type;
  typedef std::vector< ActionCachedHandleBase* > cached_handle_vector_type;

  // Vector that stores the required arguments of the action.
  argument_vector_type arguments_;

  // Map that stores the location of the parameters in the action.
  parameter_map_type parameters_;
  
  // Vector that stores shared pointers to temporary objects
  cached_handle_vector_type cached_handles_;

};

// CORE_ACTION:
// Action type should be defined at the top of each action. It renders code that
// allows both the class as well as the Action object to determine what its 
// properties are. By defining class specific static functions the class 
// properties can be queried without instantiating the action. On the other
// hand you want to query these properties from the object as well, even when
// we only have a pointer to the base object. Hence we need virtual functions
// as well. 

// Note: one would expect to use virtual static functions, but those are not
// allowed in C++

#define CORE_ACTION(type_string,usage_string) \
  public: \
    static std::string Type()  { return type_string; }\
    static std::string Usage() { return usage_string; }\
    \
    virtual std::string type() const  { return Type(); } \
    virtual std::string usage() const { return Usage(); }
} // end namespace Core

#endif
