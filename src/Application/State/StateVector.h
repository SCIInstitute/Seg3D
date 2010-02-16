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

#ifndef APPLICATION_STATE_STATEVECTOR_H
#define APPLICATION_STATE_STATEVECTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// boost includes
#include <boost/smart_ptr.hpp>

// Utils includes
#include <Utils/Converter/StringConverter.h>

// Application includes
#include <Application/State/StateBase.h>
#include <Application/State/StateEngine.h>

namespace Seg3D {

// STATEVECTOR:
// This class is a specification of State that is used to hold a single unbound
// instance of a value.

// Forward declaration of the StateVector class
template<class T>
class StateVector;
  
// Predefine the StateVector instantiation that are used in Seg3D 

typedef StateVector<Utils::Point>               StatePointVector;
typedef boost::shared_ptr<StatePointVector>     StatePointVectorHandle;

typedef StateVector<bool>                       StateBoolVector;
typedef boost::shared_ptr<StateBoolVector>      StateBoolVectorHandle;

typedef StateVector<double>                     StateDoubleVector;
typedef boost::shared_ptr<StateDoubleVector>    StateDoubleVectorHandle;
  

template<class T>
class StateVector : public StateBase {
  
// -- constructor/destructor --
  public:
    // CONSTRUCTOR
    StateVector(const std::vector<T>& default_value) :
      values_vector_(default_value)
    { }
    
    // DESTRUCTOR
    virtual ~StateVector()
    { }
  
// -- functions for accessing data --
  public:  
    // EXPORT_TO_STRING:
    // Convert the contents of the State into a string
    virtual std::string export_to_string() const
    {
      return (Utils::export_to_string(values_vector_));
    }
    
    // IMPORT_FROM_STRING:
    // Set the State from a string
    virtual bool import_from_string(const std::string& str,
                                    ActionSource source = ACTION_SOURCE_NONE_E)
    {
      // Lock the state engine so no other thread will be accessing it
      StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());

      std::vector<T> value;
      if (!(Utils::import_from_string(str,value))) return (false);
      if (value != values_vector_)
      {
        values_vector_ = value;
        value_changed_signal_(values_vector_,source);
        state_changed_signal_();
      }
      return (true);      
    }
    
  protected:    
    // EXPORT_TO_VARIANT
    // Export the state data to a variant parameter
    virtual void export_to_variant(ActionParameterVariant& variant) const
    {
      variant.set_value(values_vector_);
    }
    
    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.
    virtual bool import_from_variant(ActionParameterVariant& variant,
                                     bool from_interface = false)
    {
      // Lock the state engine so no other thread will be accessing it
      StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());

      std::vector<T> value;
      if (!( variant.get_value(value) )) return (false);
      if (value != values_vector_)
      {
        values_vector_ = value;
        value_changed_signal_(values_vector_,from_interface);
        state_changed_signal_();
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
      error = "";
      return (true);
    }

// -- access value --
  public:
    // GET:
    // Get the value of the state variable
    const std::vector<T>& get() const  
    { 
      return values_vector_; 
    }

    // SET:
    // Set the value of the state variable
    // NOTE: this function by passes the action mechanism and should only be used
    // to enforce a constraint from another action.
    bool set(const std::vector<T>& value, 
             ActionSource source = ACTION_SOURCE_NONE_E)
    {
      // Lock the state engine so no other thread will be accessing it
      StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());
      
      if (value != values_vector_)
      {
        values_vector_ = value;
        value_changed_signal_(values_vector_,source);
        state_changed_signal_();
      }
      return (true);    
    }  


// -- signals describing the state --
  public:
    // VALUE_CHANGED_SIGNAL:
    // Signal when the data in the state is changed.
    
    typedef boost::signals2::signal<void (ActionSource)> 
                                                      value_changed_signal_type;
    value_changed_signal_type value_changed_signal;

// -- internals of StateValue --
  private:
    // Storage for the actual vector
    std::vector<T> values_vector_;
};
  
} // end namespace Seg3D

#endif
