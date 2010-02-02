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

#ifndef APPLICATION_STATE_STATEVALUE_H
#define APPLICATION_STATE_STATEVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Application/State/StateBase.h>
#include <Application/State/StateEngine.h>

namespace Seg3D {

// STATEVALUE:
// This class is a specification of State that is used to hold a single unbound
// instance of a value.

// Forward declaration of the StateValue class
template<class T>
class StateValue;

// Predefine the StateValue instantiation that are used in Seg3D

typedef StateValue<double>                      StateDouble;
typedef boost::shared_ptr<StateDouble>          StateDoubleHandle;

typedef StateValue<Utils::Point>                StatePoint;
typedef boost::shared_ptr<StatePoint>           StatePointHandle;

typedef StateValue<bool>                        StateBool;
typedef boost::shared_ptr<StateBool>            StateBoolHandle;

typedef StateValue<int>                         StateInt;
typedef boost::shared_ptr<StateInt>             StateIntHandle;

typedef StateValue<std::string>                 StateString;
typedef boost::shared_ptr<StateString>          StateStringHandle;

// Definition of the templated StateValue class

template<class T>
class StateValue : public StateBase {

// -- constructor/destructor --
  public:

    // CONSTRUCTOR
    StateValue(const T& default_value) :
      value_(default_value)
    {}

    // DESTRUCTOR
    virtual ~StateValue() {}

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
      // Lock the state engine so no other thread will be accessing it
      StateEngine* engine = StateEngine::Instance();
      StateEngine::lock_type lock(engine->get_mutex() );
      
      T value;
      if (!(Utils::import_from_string(str,value))) return (false);
      if (value != value_)
      {
        value_ = value;
        value_changed_signal(value_,from_interface);
        state_changed_signal();
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
      // Lock the state engine so no other thread will be accessing it
      StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());
      
      T value;
      if (!( variant.get_value(value) )) return (false);
      if (value != value_)
      {
        value_ = value;
        value_changed_signal(value_,from_interface);
        state_changed_signal();
      }
      return (true);
    }
          
    // VALIDATE_VARIANT:
    // Validate a variant parameter
    // This function returns false if the parameter is invalid or cannot be 
    // converted and in that case error will describe the error.
    virtual bool validate_variant(ActionParameterVariant& variant, std::string& error)
    {
      if (!(variant.validate_type<T>()))
      {
        error = "Cannot convert the value '"+variant.export_to_string()+"'";
        return (false);
      }
      error = "";
      return (true);
    }

// -- access value --
  public:
   // GET:
   // Get the value of the state variable
   T get() { return value_; }

// -- signals describing the state --
  public:
    // VALUE_CHANGED_SIGNAL:
    // Signal when the data in the state is changed, the second bool indicates
    // whether the signal was triggered from the interface, in which case it may
    // not need to update the interface.

    typedef boost::signals2::signal<void (T, bool)> value_changed_signal_type;
    value_changed_signal_type value_changed_signal;
    
// -- internals of StateValue --
  private:

    // Storage for the actual value
    T value_;
};

} // end namespace Seg3D

#endif
