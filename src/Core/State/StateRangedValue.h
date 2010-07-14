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

#ifndef CORE_STATE_STATERANGEDVALUE_H
#define CORE_STATE_STATERANGEDVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Core/State/StateBase.h>
#include <Core/State/StateEngine.h>

namespace Core
{

// STATEVALUE:
// This class is a specification of State that is used to hold a single bound
// instance of a value.

// Forward declaration of the StateValue class
template< class T >
class StateRangedValue;

// Predefine the StateValue instantiation that are used in Seg3D

typedef StateRangedValue< double > StateRangedDouble;
typedef boost::shared_ptr< StateRangedDouble > StateRangedDoubleHandle;

typedef StateRangedValue< int > StateRangedInt;
typedef boost::shared_ptr< StateRangedInt > StateRangedIntHandle;

// Definition of the templated StateValue class

template< class T >
class StateRangedValue : public StateBase
{
  // -- type definitions --
public:
  typedef T value_type;
  typedef boost::shared_ptr< StateRangedValue< T > > handle_type;

  // -- constructor/destructor --
public:

  // CONSTRUCTOR
  StateRangedValue( const std::string& stateid, const T& default_value, const T& min_value, 
    const T& max_value, const T& step_value ) :
    StateBase( stateid ),
    value_( default_value ), 
    min_value_( min_value ), 
    max_value_( max_value ), 
    step_value_( step_value )
  {
    if ( this->min_value_ > this->max_value_ ) std::swap( this->min_value_, this->max_value_ );
  }

  // DESTRUCTOR
  virtual ~StateRangedValue()
  {
  }

  // -- functions for accessing data --
public:
  // EXPORT_TO_STRING:
  // Convert the contents of the State into a string
  virtual std::string export_to_string() const
  {
    return Core::ExportToString( this->value_ );
  }

  // IMPORT_FROM_STRING:
  // Set the State from a string
  virtual bool import_from_string( const std::string& str, Core::ActionSource source =
    Core::ActionSource::NONE_E )
  {
    T value;
    if ( !( Core::ImportFromString( str, value ) ) ) return false;
    return this->set( value, source );
  }

protected:
  // EXPORT_TO_VARIANT
  // Export the state data to a variant parameter
  virtual void export_to_variant( Core::ActionParameterVariant& variant ) const
  {
    variant.set_value( this->value_ );
  }

  // IMPORT_FROM_VARIANT:
  // Import the state data from a variant parameter.
  virtual bool import_from_variant( Core::ActionParameterVariant& variant, 
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    // Get the value from the action parameter
    T value;
    if ( !( variant.get_value( value ) ) ) return false;

    // Set the parameter in this state variable
    return this->set( value, source );
  }

  // VALIDATE_VARIANT:
  // Validate a variant parameter
  // This function returns false if the parameter is invalid or cannot be
  // converted and in that case error will describe the error.
  virtual bool validate_variant( Core::ActionParameterVariant& variant, std::string& error )
  {
    if ( !( variant.validate_type< T > () ) )
    {
      error = "Cannot convert the value '" + variant.export_to_string() + "'";
      return false;
    }

    T value;
    variant.get_value( value );
    if ( value < this->min_value_ || value > this->max_value_ )
    {
      error = "Value " + ExportToString( value ) + "is out of range [" + ExportToString(
          this->min_value_ ) + "," + ExportToString( this->max_value_ ) + "]";
      return false;
    }
    error = "";
    return true;
  }

  // -- Functions specific to this type of state --
public:

  // SET_RANGE:
  // Set the range of permissible values for this state variable. This
  // variable normally is represented by a slider and this one records the
  // min and max values so values can be validated correctly

  void set_range( T min_value, T max_value,
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );  

    if ( min_value > max_value ) std::swap( min_value, max_value );
  
    this->min_value_ = min_value;
    this->max_value_ = max_value;

    if ( this->value_ < this->min_value_ )
    {
      this->value_ = this->min_value_;
      T value = this->value_;
      lock.unlock();
      
      if ( this->signals_enabled() )
      {
        this->value_changed_signal_( value, ActionSource::NONE_E );
        this->state_changed_signal_();
        this->range_changed_signal_( min_value, max_value, source );
      }
    }
    else if ( this->value_ > this->max_value_ )
    {
      this->value_ = this->max_value_;
      T value = this->value_;
      lock.unlock();
    
      if ( this->signals_enabled() )
      {
        this->value_changed_signal_( value, ActionSource::NONE_E );
        this->state_changed_signal_();
      }
    }
    else
    {
      lock.unlock();

      if ( this->signals_enabled() )
      {
        this->range_changed_signal_( min_value, max_value, source );
      }
    }
  }

  // -- access value --
public:
  // GET:
  // Get the value of the state variable
  T get() const
  {
    return this->value_;
  }

  // SET:
  // Set the value of the state variable
  // NOTE: this function by passes the action mechanism and should only be used
  // to enforce a constraint from another action. Normally use the action
  // mechanism to ensure that the action is recorded correctly.
  bool set( T value, Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

    if ( value != this->value_ )
    {
      // NOTE: If the value is out of bound the variable from_interface is
      // rewmoved to ensure that any updates make it to the widget as well
      // Normally from_interface will ensure that the widget is not updated
      // by the user and the application at the same time. This prevents
      // loop backs of signals between the application layer and the
      // interface layer.
      if ( value < this->min_value_ )
      {
        value = this->min_value_;
        source = ActionSource::NONE_E;
      }
      if ( value > this->max_value_ )
      {
        value = this->max_value_;
        source = ActionSource::NONE_E;
      }
      this->value_ = value;
      
      // NOTE: Unlock before triggering signals
      lock.unlock();
      
      if ( this->signals_enabled() )
      {     
        this->value_changed_signal_( value, source );
        this->state_changed_signal_();
      }
    }
    return true;
  }

  // GET_RANGE:
  // Get the range of the variable
  void get_range( T& min_value, T& max_value )
  {
    min_value = this->min_value_;
    max_value = this->max_value_;
  }
    
    void get_step( T& step )
    {
        step = this->step_value_;
    }

  // -- signals describing the state --

public:

  // VALUE_CHANGED_SIGNAL:
  // Signal when the data in the state is changed, the second parameter
  // indicates the source of the change

  typedef boost::signals2::signal< void( T, Core::ActionSource ) > value_changed_signal_type;
  value_changed_signal_type value_changed_signal_;

  // RANGE_CHANGED_SIGNAL:
  // This signal is triggered when the range of the state is changed
  typedef boost::signals2::signal< void( T, T, Core::ActionSource ) > range_changed_signal_type;
  range_changed_signal_type range_changed_signal_;

  // -- internals of StateValue --
private:

  // Storage for the actual value
  T value_;

  // Min and max values
  T min_value_;
  T max_value_;
  T step_value_;

};

} // end namespace Core

#endif
