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

#ifndef CORE_STATE_STATEVALUE_H
#define CORE_STATE_STATEVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Core includes
#include <Core/State/StateBase.h>
#include <Core/State/StateEngine.h>
#include <Core/Geometry/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Path.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/GridTransform.h>

namespace Core
{

// STATEVALUE:
/// This class is a specification of State that is used to hold a single unbound
/// instance of a value.

// Forward declaration of the StateValue class
template< class T >
class StateValue;

// Predefine the StateValue instantiation that are used in Seg3D

typedef StateValue< double > StateDouble;
typedef boost::shared_ptr< StateDouble > StateDoubleHandle;
typedef boost::weak_ptr< StateDouble > StateDoubleWeakHandle;

typedef StateValue< Point > StatePoint;
typedef boost::shared_ptr< StatePoint > StatePointHandle;
typedef boost::weak_ptr< StatePoint > StatePointWeakHandle;

typedef StateValue< Color > StateColor;
typedef boost::shared_ptr< StateColor > StateColorHandle;
typedef boost::weak_ptr< StateColor > StateColorWeakHandle;

typedef StateValue< bool > StateBool;
typedef boost::shared_ptr< StateBool > StateBoolHandle;
typedef boost::weak_ptr< StateBool > StateBoolWeakHandle;

typedef StateValue< int > StateInt;
typedef boost::shared_ptr< StateInt > StateIntHandle;
typedef boost::weak_ptr< StateInt > StateIntWeakHandle;

typedef StateValue< unsigned int > StateUInt;
typedef boost::shared_ptr< StateInt > StateUIntHandle;
typedef boost::weak_ptr< StateInt > StateUIntWeakHandle;
  
typedef StateValue< long long > StateLongLong;
typedef boost::shared_ptr< StateLongLong > StateLongLongHandle;
typedef boost::weak_ptr< StateLongLong > StateLongLongWeakHandle;

typedef StateValue< std::string > StateString;
typedef boost::shared_ptr< StateString > StateStringHandle;
typedef boost::weak_ptr< StateString > StateStringWeakHandle;

typedef StateValue< Path > StateSpeedlinePath;
typedef boost::shared_ptr< StateSpeedlinePath > StateSpeedlinePathHandle;

typedef StateValue< BBox > StateBBox;
typedef boost::shared_ptr< StateBBox > StateBBoxHandle;

typedef StateValue< GridTransform > StateGridTransform;
typedef boost::shared_ptr< StateGridTransform > StateGridTransformHandle;

// Definition of the templated StateValue class

template< class T >
class StateValue : public StateBase
{
  // -- type definitions --
public:
  typedef T value_type;
  typedef boost::shared_ptr< StateValue< T > > handle_type;

  // -- constructor/destructor --
public:

  // CONSTRUCTOR
  StateValue( const std::string& stateid, const T& default_value ) :
    StateBase( stateid ),
    value_( default_value )
  {
  }

  // DESTRUCTRO
  virtual ~StateValue()
  {
  }

  // -- functions for accessing data --

public:
  // EXPORT_TO_STRING:
  /// Convert the contents of the State into a string
  virtual std::string export_to_string() const
  {
    return Core::ExportToString( value_ );
  }

  // IMPORT_FROM_STRING:
  /// Set the State from a string
  virtual bool import_from_string( const std::string& str, ActionSource source =
    ActionSource::NONE_E )
  {
    T value;
    if ( !( Core::ImportFromString( str, value ) ) ) return false;

    return set( value, source );
  }

protected:
  // EXPORT_TO_VARIANT
  /// Export the state data to a variant parameter
  virtual void export_to_variant( Variant& variant ) const
  {
    variant.set( this->value_ );
  }

  // IMPORT_FROM_VARIANT:
  /// Import the state data from a variant parameter.
  virtual bool import_from_variant( Variant& variant, ActionSource source = ActionSource::NONE_E )
  {
    T value;
    if ( !( variant.get( value ) ) ) return false;
    return set( value, source );
  }

  // VALIDATE_VARIANT:
  /// Validate a variant parameter
  /// This function returns false if the parameter is invalid or cannot be
  /// converted and in that case error will describe the error.
  virtual bool validate_variant( Variant& variant, std::string& error )
  {
    if ( !( variant.validate_type< T > () ) )
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
  /// Get the value of the state variable
  const T& get() const
  {
    return this->value_;
  }

  // SET:
  /// Set the value of the state variable
  /// NOTE: Please use action to set the state of a variable
  bool set( const T& value, ActionSource source = ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
    
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

    if ( value != this->value_ )
    {
      this->value_ = value;
      
      // NOTE: We need to unlock the state engine before triggering the signals
      lock.unlock();
      
      if ( this->signals_enabled() )
      {
        this->value_changed_signal_( value, source );
        this->state_changed_signal_();
      }
    }
    return true;
  }

  // -- signals describing the state --
public:
  // VALUE_CHANGED_SIGNAL:
  /// Signal when the data in the state is changed, the second parameter
  /// indicates the source of the change

  typedef boost::signals2::signal< void( T, Core::ActionSource ) > value_changed_signal_type;
  value_changed_signal_type value_changed_signal_;

  // -- internals of StateValue --
private:

  // Storage for the actual value
  T value_;
};

} // end namespace Core

#endif
