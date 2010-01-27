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

#ifndef APPLICATION_STATE_STATERANGEDVALUE_H
#define APPLICATION_STATE_STATERANGEDVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Application/State/StateBase.h>

namespace Seg3D {

// STATEVALUE:
// This class is a specification of State that is used to hold a single bound
// instance of a value.

// Forward declaration of the StateValue class
template<class T>
class StateRangedValue;

// Predefine the StateValue instantiation that are used in Seg3D

typedef StateRangedValue<double>                StateRangedDouble;
typedef boost::shared_ptr<StateRangedDouble>    StateRangedDoubleHandle;

typedef StateRangedValue<int>                   StateRangedInt;
typedef boost::shared_ptr<StateRangedInt>       StateRangedIntHandle;

// Definition of the templated StateValue class

template<class T>
class StateRangedValue : public StateBase {

// -- constructor/destructor --
  public:

    // CONSTRUCTOR
    StateRangedValue(const T& default_value, 
                     const T& min_value, 
                     const T& max_value,
                     const T& step_value) :
      value_(default_value),
      min_value_(min_value),
      max_value_(max_value),
      step_value_(step_value)
    {
      if (min_value_ > max_value_) std::swap(min_value_,max_value_);
    }

    // DESTRUCTOR
    virtual ~StateRangedValue() {}

// -- functions for accessing data --
  public:  
    // EXPORT_TO_STRING:
    // Convert the contents of the State into a string
    virtual std::string export_to_string() const
    {
      return (Utils::export_to_string(value_));
    }
    
    // IMPORT_FROM_STRING:
    // Set the State from a string
    virtual bool import_from_string(const std::string& str,
                                    bool from_interface = false)
    {
      T value;
      if (!(Utils::import_from_string(str,value))) return (false);
      if (value != value_)
      {
        if (value < min_value_) value = min_value_;
        if (value > max_value_) value = max_value_;
        value_ = value;
        value_changed_signal(value_,from_interface);
      }
      return (true);      
    }

  protected:    
    // EXPORT_TO_VARIANT
    // Export the state data to a variant parameter
    virtual void export_to_variant(ActionParameterVariant& variant) const
    {
      variant.set_value(value_);
    }
    
    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.
    virtual bool import_from_variant(ActionParameterVariant& variant,
                                     bool from_interface = false)
    {
      T value;
      if (!( variant.get_value(value) )) return (false);
      if (value != value_)
      {
        if (value < min_value_) value = min_value_;
        if (value > max_value_) value = max_value_;
        value_ = value;
        value_changed_signal(value_,from_interface);
      }
      return (true);
    }
          
    // VALIDATE_VARIANT:
    // Validate a variant parameter
    // This function returns false if the parameter is invalid or cannot be 
    // converted and in that case error will describe the error.
    virtual bool validate_variant(ActionParameterVariant& variant, 
                                  std::string& error)
    {
      if (!(variant.validate_type<T>()))
      {
        error = "Cannot convert the value '"+variant.export_to_string()+"'";
        return (false);
      }
      
      T value;
      variant.get_value(value);
      if (value < min_value_ || value > max_value_)
      {
        error = "Value "+Utils::to_string(value)+"is out of range ["+
          Utils::to_string(min_value_)+","+Utils::to_string(max_value_)+"]";
        return (false);
      }
      error = "";
      return (true);
    }

// -- Functions specific to this type of state --
  public:
    void set_range(const T& min_value, const T& max_value)
    {
      if (min_value < max_value) std::swap(min_value,max_value);
      min_value_ = min_value;
      max_value_ = max_value;
      
      if (value_ < min_value_) 
      {
        value_ = min_value_;
        value_changed_signal(value_,false);
      }
      else if (value_ > max_value_)
      {
        value_ = max_value_;
        value_changed_signal(value_,false);
      }
      
      range_changed_signal(min_value_,max_value_);
    }

// -- signals describing the state --

  public:
    // VALUE_CHANGED_SIGNAL:
    // Signal when the data in the state is changed, the second bool indicates
    // whether the signal was triggered from the interface, in which case it may
    // not need to update the interface.

    typedef boost::signals2::signal<void (T, bool)> value_changed_signal_type;
    value_changed_signal_type value_changed_signal;
    
    // RANGE_CHANGED_SIGNAL:
    // This signal is triggered when the range of the state is changed
    typedef boost::signals2::signal<void (T,T)> range_changed_signal_type;
    range_changed_signal_type range_changed_signal;
    
// -- internals of StateValue --
  private:

    // Storage for the actual value
    T value_;
    
    // Min and max values
    T min_value_;
    T max_value_;
    T step_value_;
  
};

} // end namespace Seg3D

#endif
