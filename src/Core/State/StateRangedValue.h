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

#ifndef CORE_STATE_STATERANGEDVALUE_H
#define CORE_STATE_STATERANGEDVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Core/Math/MathFunctions.h>
#include <Core/State/StateBase.h>
#include <Core/State/StateEngine.h>

namespace Core
{

class StateRangedValueBase;
typedef boost::shared_ptr< StateRangedValueBase > StateRangedValueBaseHandle;
typedef boost::weak_ptr< StateRangedValueBase > StateRangedValueBaseWeakHandle;

// STATEVALUE:
/// This class is a specification of State that is used to hold a single bound
/// instance of a value.

// Forward declaration of the StateValue class
template< class T >
class StateRangedValue;

// Predefine the StateValue instantiation that are used in Seg3D

typedef StateRangedValue< double > StateRangedDouble;
typedef boost::shared_ptr< StateRangedDouble > StateRangedDoubleHandle;

typedef StateRangedValue< int > StateRangedInt;
typedef boost::shared_ptr< StateRangedInt > StateRangedIntHandle;

typedef StateRangedValue< unsigned int > StateRangedUInt;
typedef boost::shared_ptr< StateRangedUInt > StateRangedUIntHandle;
  
// Class StateValueBase:
/// This pure virtual class defines an extra interface that StateValue provides.
class StateRangedValueBase : public StateBase
{
public:
  StateRangedValueBase( const std::string& stateid ) :
    StateBase( stateid )
  {
  }

protected:
  friend class ActionOffset;
  friend class ActionSetRange;

  // IMPORT_OFFSET_FROM_VARIANT:
  /// Import the offset value from the variant and apply it to the current value.
  virtual bool import_offset_from_variant( Variant& variant, 
    ActionSource source = ActionSource::NONE_E ) = 0;

  // IMPORT_RANGE_FROM_VARIANT:
  /// Import range values from variants.
  virtual bool import_range_from_variant( Variant& variant_min, Variant& variant_max, 
    ActionSource source = ActionSource::NONE_E ) = 0;

  // VALIDATE_OFFSET_VARIANT:
  /// Returns true if the value stored in the variant is a valid offset value, otherwise false.
  virtual bool validate_value_type_variant( Variant& variant, 
    std::string& error ) = 0;
};

// Definition of the templated StateValue class

template< class T >
class StateRangedValue : public StateRangedValueBase
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
    StateRangedValueBase( stateid ),
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
  /// Convert the contents of the State into a string
  virtual std::string export_to_string() const
  {
    return Core::ExportToString( this->value_ );
  }

  // IMPORT_FROM_STRING:
  /// Set the State from a string
  virtual bool import_from_string( const std::string& str, Core::ActionSource source =
    Core::ActionSource::NONE_E )
  {
    T value;
    if ( !( Core::ImportFromString( str, value ) ) ) return false;
    return this->set( value, source );
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
  virtual bool import_from_variant( Variant& variant, 
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    // Get the value from the action parameter
    T value;
    if ( !( variant.get( value ) ) ) return false;

    // Set the parameter in this state variable
    return this->set( value, source );
  }

  // IMPORT_OFFSET_FROM_VARIANT:
  /// Import the offset value from the variant and apply it to the current value.
  virtual bool import_offset_from_variant( Variant& variant, 
    ActionSource source = ActionSource::NONE_E )
  {
    T offset_value;
    if ( !variant.get( offset_value ) )
    {
      return false;
    }
    return this->offset( offset_value, source );
  }

  // IMPORT_RANGE_FROM_VARIANT:
  /// Import range values from variants.
  virtual bool import_range_from_variant( Variant& variant_min, Variant& variant_max, 
    ActionSource source = ActionSource::NONE_E )
  {
    T min_val, max_val;
    if ( !variant_min.get( min_val ) ||
      !variant_max.get( max_val ) )
    {
      return false;
    }
    this->set_range( min_val, max_val, source );
    return true;
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

    T value;
    variant.get( value );
    if ( value < this->min_value_ || value > this->max_value_ )
    {
      error = "Value " + ExportToString( value ) + " is out of range [" + ExportToString(
          this->min_value_ ) + "," + ExportToString( this->max_value_ ) + "]";
      return false;
    }
    error = "";
    return true;
  }

  // VALIDATE_OFFSET_VARIANT:
  /// Returns true if the value stored in the variant is a valid offset value, otherwise false.
  virtual bool validate_value_type_variant( Variant& variant, std::string& error )
  {
    if ( !( variant.validate_type< T > () ) )
    {
      error = "Cannot convert the value '" + variant.export_to_string() + "'";
      return false;
    }

    error = "";
    return true;
  }

  // -- Functions specific to this type of state --
public:

  // SET_RANGE:
  /// Set the range of permissible values for this state variable. This
  /// variable normally is represented by a slider and this one records the
  /// min and max values so values can be validated correctly

  void set_range( T min_value, T max_value,
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
    
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );  

    if ( min_value > max_value ) std::swap( min_value, max_value );
  
    this->min_value_ = min_value;
    this->max_value_ = max_value;
    bool value_changed = false;
    T new_value = this->value_;

    if ( this->value_ < this->min_value_ )
    {
      this->value_ = this->min_value_;
      new_value = this->value_;
      value_changed = true;     
    }
    else if ( this->value_ > this->max_value_ )
    {
      this->value_ = this->max_value_;
      new_value = this->value_;
      value_changed = true;
    }

    lock.unlock();
    if ( this->signals_enabled() )
    {
      this->range_changed_signal_( min_value, max_value, source );
      if ( value_changed )
      {
        this->value_changed_signal_( new_value, ActionSource::NONE_E );
        this->state_changed_signal_();
      }
    }
  }

  // SET_STEP:

  void set_step( T step_value,
    Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();

    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );  

    this->step_value_ = step_value;
    lock.unlock();
    if ( this->signals_enabled() )
    {
      this->step_changed_signal_( step_value, source );
    }
  }

  // -- access value --
public:
  // GET:
  /// Get the value of the state variable
  T get() const
  {
    return this->value_;
  }

  // SET:
  /// Set the value of the state variable
  /// NOTE: this function by passes the action mechanism and should only be used
  /// to enforce a constraint from another action. Normally use the action
  /// mechanism to ensure that the action is recorded correctly.
  bool set( T value, Core::ActionSource source = Core::ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
    
    // Lock the state engine so no other thread will be accessing it
    StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

    if ( value != this->value_ )
    {
      // NOTE: If the value is out of bound the variable from_interface is
      // removed to ensure that any updates make it to the widget as well
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

  // OFFSET:
  /// Offset the value of the state variable by the specified amount.
  bool offset( const T& offset_value, ActionSource source = ActionSource::NONE_E )
  {
    // NOTE: State variables can only be set from the application thread
    ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
    
    T new_value;
    bool value_changed = false;
    {
      // Lock the state engine so no other thread will be accessing it
      StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );
      
      // Clamp the new value to the current range
      // NOTE: The offset value will never come from a GUI widget, so it's not
      // necessary to alter the ActionSource if the value is out of range. On the 
      // other hand, it's desirable to preserve the source information because
      // certain signal handlers might need it.
      new_value = Clamp( this->value_ + offset_value, 
        this->min_value_, this->max_value_ );

      if ( this->value_ != new_value )
      {
        this->value_ = new_value;
        value_changed = true;
      }
    }

    if ( value_changed && this->signals_enabled() )
    {
      this->value_changed_signal_( new_value, source );
      this->state_changed_signal_();
    }

    return true;
  }

  // GET_RANGE:
  /// Get the range of the variable
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
  /// Signal when the data in the state is changed, the second parameter
  /// indicates the source of the change

  typedef boost::signals2::signal< void( T, Core::ActionSource ) > value_changed_signal_type;
  value_changed_signal_type value_changed_signal_;

  // RANGE_CHANGED_SIGNAL:
  /// This signal is triggered when the range of the state is changed
  typedef boost::signals2::signal< void( T, T, Core::ActionSource ) > range_changed_signal_type;
  range_changed_signal_type range_changed_signal_;

  // STEP_CHANGED_SIGNAL:
  /// This signal is triggered when the step of the state is changed
  typedef boost::signals2::signal< void( T, Core::ActionSource ) > step_changed_signal_type;
  step_changed_signal_type step_changed_signal_;

  // -- internals of StateValue --
private:

  /// Storage for the actual value
  T value_;

  /// Min and max values
  T min_value_;
  T max_value_;
  T step_value_;

};

} // end namespace Core

#endif
