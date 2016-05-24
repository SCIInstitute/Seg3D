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

#ifndef CORE_STATE_STATEBASE_H
#define CORE_STATE_STATEBASE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Action/ActionContext.h>
#include <Core/Action/ActionParameter.h>

namespace Core
{

// -- STATEBASE --

class StateBase;
typedef boost::shared_ptr< StateBase > StateBaseHandle;
typedef boost::weak_ptr< StateBase > StateBaseWeakHandle;

class StateBasePrivate;
typedef boost::shared_ptr< StateBasePrivate > StateBasePrivateHandle;

class StateBase : public boost::noncopyable
{

  // -- constructor / destructor --
public:
  StateBase(const std::string& stateid);

  virtual ~StateBase();

  // -- functions for accessing data --
public:
  // EXPORT_TO_STRING:
  /// Convert the contents of the State into a string
  virtual std::string export_to_string() const = 0;

  // IMPORT_FROM_STRING:
  /// Set the State from a string
  virtual bool import_from_string( const std::string& str, Core::ActionSource source =
    Core::ActionSource::NONE_E ) = 0;
  
  // SET_IS_PROJECT_DATA:
  /// Set whether the data is part of the project data
  void set_is_project_data( bool is_project_data );

protected:
  friend class ActionSet;
  friend class ActionGet;
  friend class ActionOffset;
  friend class StateHandler;

  // EXPORT_TO_VARIANT
  /// Export the state data to a variant parameter
  virtual void export_to_variant( Variant& variant ) const = 0;

  // IMPORT_FROM_VARIANT:
  /// Import the state data from a variant parameter.
  virtual bool import_from_variant( Variant& variant, 
    Core::ActionSource source = Core::ActionSource::NONE_E ) = 0;

  // VALIDATE_VARIANT:
  /// Validate a variant parameter
  /// This function returns false if the parameter is invalid or cannot be
  /// converted and in that case error will describe the error.
  virtual bool validate_variant( Variant& variant, std::string& error ) = 0;

  // ENABLE_SIGNALS:
  /// Allow signals to be triggered from this state variable
  /// NOTE: Signal enabling is used for loading sessions, switching off signalling can prevent
  /// a lot of signals being triggered when every state variable is reinitialized.
  void enable_signals( bool signals_enabled );
  
  // SIGNALS_ENABLED:
  /// Check whether signals are enabled
  bool signals_enabled();
    
  // SET_INITIALIZING:
  /// Indicate that the statehandler is still being created. Hence only one thread will have
  /// access to the state variable, and thread safety checking is not needed. 
  /// NOTE: This is mainly intended to be set in the constructor of the state handler of this
  /// variable, which may need to initialize the state variables to certain values. As that handlers
  /// can be singletons they can be created from any thread. Hence strict thread checking
  /// maybe an issue, however the creation of an instance normally is thread safe as no other
  /// thread has access to the state parameters and signals are still being blocked.
  void set_initializing( bool initializing );
  
  // GET_INITIALIZING:
  /// Query whether the state variable and its parent are still being initialized.
  bool get_initializing() const;
  
public: 
  // SET_LOCKED:
  /// Tell the program that this state variable cannot be changed by the action mechanism
  /// Only a direct set will work. This prevents any scripting system to override crucial
  /// variables.
  void set_locked( bool locked );
  
  // GET_LOCKED:
  /// Query whether the variable has been designated as locked
  bool get_locked() const; 
  
  // -- session handling -- 
public:
  enum 
  {
    DO_NOT_LOAD_E = -1,
    LOAD_LAST_E = 0,
    DEFAULT_LOAD_E = 100
  };
  
  // GET_SESSION_PRIORITY:
  /// Get the priority in which loading sets the states
  int get_session_priority() const;
  
  // SET_SESSION_PRIORITY:
  /// By default the priority is set to DEAFULT_LOAD_E.
  void set_session_priority( int priority );
  
  // -- stateid handling --
public:
  // GET_STATEID:
  /// Get the unique id assigned to the state variable
  std::string get_stateid() const;

  // IS_PROJECT_DATA:
  /// Whether the data is tagged as project data that is saved in a session
  bool is_project_data() const;

  // -- signal handling --
public:
  // STATE_CHANGED_SIGNAL:
  /// This signal is triggered when the state is changed
  typedef boost::signals2::signal< void() > state_changed_signal_type;
  state_changed_signal_type state_changed_signal_;

protected:
  // INVALIDATE:
  /// Invalidate the state variable. This function is called by the "invalidate" function of 
  /// StateHandler to release any resource occupied by the state.
  /// Default implementation does nothing.
  virtual void invalidate();

private:
  StateBasePrivateHandle private_;
};

} // end namespace Core

// Ensure that any set outside the initialization phase is done on the application thread
#define ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING() \
assert( Core::Application::IsApplicationThread() || get_initializing() )

#endif

