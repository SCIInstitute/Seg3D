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

#ifndef CORE_STATE_STATEVECTOR_H
#define CORE_STATE_STATEVECTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <algorithm>
#include <vector>

// boost includes
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/State/StateBase.h>
#include <Core/State/StateEngine.h>
#include <Core/Geometry/Color.h>
#include <Core/Geometry/Point.h>

namespace Core
{

class StateVectorBase;
typedef boost::shared_ptr< StateVectorBase > StateVectorBaseHandle;
typedef boost::weak_ptr< StateVectorBase > StateVectorBaseWeakHandle;

// STATEVECTOR:
// This class is a specification of State that is used to hold a single unbound
// instance of a value.

// Forward declaration of the StateVector class
template< class T >
class StateVector;

// Predefine the StateVector instantiation that are used in Seg3D 

//class Point;
typedef StateVector< Point > StatePointVector;
typedef boost::shared_ptr< StatePointVector > StatePointVectorHandle;
  
//class Color;
typedef StateVector< Color > StateColorVector;
typedef boost::shared_ptr< StateColorVector > StateColorVectorHandle;

typedef StateVector< bool > StateBoolVector;
typedef boost::shared_ptr< StateBoolVector > StateBoolVectorHandle;

typedef StateVector< double > StateDoubleVector;
typedef boost::shared_ptr< StateDoubleVector > StateDoubleVectorHandle;
  
typedef StateVector< std::string > StateStringVector;
typedef boost::shared_ptr< StateStringVector > StateStringVectorHandle;

class StateVectorBase : public StateBase
{
public:
  StateVectorBase( const std::string& stateid ) :
    StateBase( stateid )
  {
  }

  virtual ~StateVectorBase() {}

protected:
  friend class ActionAdd;
  friend class ActionRemove;
  friend class ActionClear;

  virtual bool add( ActionParameterVariant& variant, 
    ActionSource source = ActionSource::NONE_E ) = 0;

  virtual bool remove( ActionParameterVariant& variant, 
    ActionSource source = ActionSource::NONE_E ) = 0;

  virtual void clear( ActionSource source = ActionSource::NONE_E ) = 0;

  virtual bool validate_element_variant( ActionParameterVariant& variant, 
    std::string& error ) = 0;
};


template< class T >
class StateVector : public StateVectorBase
{
  // -- type definitions --
public:
  typedef std::vector<T> value_type;
  typedef boost::shared_ptr< StateVector< T > > handle_type;
  
  // -- constructor/destructor --
public:
  // CONSTRUCTOR
  StateVector( const std::string& stateid, const std::vector< T >& default_value ) :
    StateVectorBase( stateid ),
    values_vector_( default_value )
  {
  }

  StateVector( const std::string& stateid ) :
    StateVectorBase( stateid )
  {
  }

  // DESTRUCTOR
  virtual ~StateVector()
  {
  }

  // -- functions for accessing data --
public:
  // EXPORT_TO_STRING:
  // Convert the contents of the State into a string
  virtual std::string export_to_string() const
  {
    return Core::ExportToString( values_vector_ );
  }

  // IMPORT_FROM_STRING:
  // Set the State from a string
  virtual bool import_from_string( const std::string& str, ActionSource source =
    ActionSource::NONE_E )
  {
    std::vector<T> value;
    if ( !( Core::ImportFromString( str, value ) ) ) return false;

    return this->set( value, source );
  }

protected:
  // EXPORT_TO_VARIANT
  // Export the state data to a variant parameter
  virtual void export_to_variant( ActionParameterVariant& variant ) const
  {
    variant.set_value( this->values_vector_ );
  }

  // IMPORT_FROM_VARIANT:
  // Import the state data from a variant parameter.
  virtual bool import_from_variant( ActionParameterVariant& variant, 
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    std::vector<T> value;
    if ( !( variant.get_value( value ) ) ) return false;
    return this->set( value, source );  
  }

  virtual bool add( ActionParameterVariant& variant, 
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    T element_value;
    if ( !variant.get_value( element_value ) )
    {
      return false;
    }
    return this->add( element_value, source );
  }

  virtual bool remove( ActionParameterVariant& variant, 
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    T element_value;
    if ( !variant.get_value( element_value ) )
    {
      return false;
    }
    return this->remove( element_value, source );
  }

  virtual bool validate_element_variant( ActionParameterVariant& variant, 
    std::string& error )
  {
    if ( !variant.validate_type< T >() )
    {
      error = "Cannot convert the value '" + variant.export_to_string() + "'";
      return false;
    }
    error = "";
    return true;
  }

  // VALIDATE_VARIANT:
  // Validate a variant parameter
  // This function returns false if the parameter is invalid or cannot be
  // converted and in that case error will describe the error.
  virtual bool validate_variant( ActionParameterVariant& variant, std::string& error )
  {
    if ( !( variant.validate_type< std::vector< T > > () ) )
    {
      error = "Cannot convert the value '" + variant.export_to_string() + "'";
      return false;
    }
    error = "";
    return true;
  }

  // -- access value --
public:
  // GET:
  // Get the value of the state variable
  const std::vector< T >& get() const
  {
    return this->values_vector_;
  }

  // SET:
  // Set the value of the state variable
  // NOTE: this function by passes the action mechanism and should only be used
  // to enforce a constraint from another action.
  bool set( const std::vector< T >& value, ActionSource source = ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

    if ( value != this->values_vector_ )
    {
      this->values_vector_ = value;
      lock.unlock();
      
      if ( this->signals_enabled() )
      {
        this->value_changed_signal_( value, source );
        this->state_changed_signal_();
      }
    }
    return true;
  }

  bool add( const T& value, ActionSource source = ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
  
    {
      StateEngine::lock_type lock( StateEngine::GetMutex() );
      this->values_vector_.push_back( value );
    }

    if ( this->signals_enabled() )
    {
      this->value_changed_signal_( this->values_vector_, source );
      this->state_changed_signal_();
    }

    return true;
  }

  bool remove( const T& value, ActionSource source = ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

    bool removed = false;
    {
      StateEngine::lock_type lock( StateEngine::GetMutex() );
      typename value_type::iterator it = std::find( this->values_vector_.begin(), 
        this->values_vector_.end(), value );
      if ( it != this->values_vector_.end() )
      {
        this->values_vector_.erase( it );
        removed = true;
      } 
    }

    if ( removed && this->signals_enabled() )
    {
      this->value_changed_signal_( this->values_vector_, source );
      this->state_changed_signal_();
    }
    return removed;
  }

  virtual void clear( ActionSource source = ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

    {
      StateEngine::lock_type lock( StateEngine::GetMutex() );
      this->values_vector_.clear();
    }

    if ( this->signals_enabled() )
    {
      this->value_changed_signal_( this->values_vector_, source );
      this->state_changed_signal_();
    }
  }

  // -- signals describing the state --
public:
  // VALUE_CHANGED_SIGNAL:
  // Signal when the data in the state is changed.
  typedef boost::signals2::signal< void( std::vector< T >, 
    Core::ActionSource ) > values_changed_signal_type;
  values_changed_signal_type value_changed_signal_;

  // -- internals of StateValue --
private:
  // Storage for the actual vector
  std::vector< T > values_vector_;
};

} // end namespace Seg3D

#endif
