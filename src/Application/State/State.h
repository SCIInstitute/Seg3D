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

#ifndef APPLICATION_STATE_STATE_H
#define APPLICATION_STATE_STATE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes
#include <boost/signals2/signal.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Core/Log.h>
#include <Utils/Converter/StringConverter.h>

#include <Application/Interface/Interface.h>
#include <Application/State/StateBase.h>
#include <Application/State/Actions/ActionSet.h>

namespace Seg3D {

// STATE:
// Class that records the state of a variable

template<class T>
class State;

template<class T>
class State : public StateBase {
// -- typedefs --
  public:
    typedef T value_type;

// -- constructor/destructor --
  public:
    // Constructor with default value
    State(const T& default_value) :
      value_(default_value)
    {
    }

    // The destructor needs to virtual, to ensure a proper 
    // destruction. Each derived class should have this even
    // if it is not strickly needed.
    virtual ~State()
    {
      value_changed_signal_.disconnect_all_slots();
    }  
    
// -- set/get value --
  public:
  
    // DISPATCH:
    // Set the value of this state from the interface
    virtual void dispatch(const T& value, bool trigger_update = true)
    {
      ActionSetHandle action(new ActionSet);
      action->set(stateid_,value);
      PostActionFromInterface(action,trigger_update);
    }
    
    // GET:
    // Get the value from the State
    virtual T get() const
    {
      return value_;
    }

    // SET:
    // Set the value in this State variable
    virtual void set(const T& val)
    {
      value_ = val;
      value_changed_signal_(value_);
    }

    // EXPORT_TO_STRING:
    // Convert the contents of the State into a string
    
    virtual std::string export_to_string() const
    {
      return (Utils::export_to_string(value_));
    }

    // IMPORT_FROM_STRING:
    // Set the State from a string
    // NOTE: this does not trigger the value_changed_signal.
    // This function returns whether the conversion was successful
    // As the user may have altered the input the return value of
    // this function needs to be checked.
    virtual bool import_from_string(const std::string& str)
    {
      // Ensure that value is only changed when the string can
      // successfully converted.
      T value;
      bool success = Utils::import_from_string(str,value);

      if (success) value_ = value; 
      return (success);
    }
    
// -- signal /slot part --
  public:
    typedef boost::signals2::signal<void (T)> value_changed_signal_type;
  
    // CONNECT: 
    // Connect a functor to the internal signal
    template <class FUNCTOR>
    boost::signals2::connection connect(FUNCTOR functor)
    {
      return value_changed_signal_.connect(functor);
    }
    
    // SIGNAL_VALUE_CHANGED:
    // Trigger a value_changed signal
    void signal_value_changed()
    {
      value_changed_signal_(value_);
    }
    
// -- actual state of this object --
  protected:
    
    // The value stored in this state Value
    T value_;
    
    // Value changed, this signal is triggered when the value changed
    value_changed_signal_type  value_changed_signal_;
    
// -- action handling --
  public:
  
    // VALIDATE_AND_COMPARE_VARIANT:
    // Validate that the data contained in the variant parameter can actually
    // be used and check whether it changed from the current value.
    
    virtual bool validate_and_compare_variant(
          ActionVariantParameter& variant, 
          bool& changed,
          std::string& error) const
    {
      T new_value;
      if (!(variant.get_value(new_value,error))) return (false);
      changed = (value_ != new_value);
      return (true);           
    }

    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.  
    virtual bool import_from_variant(ActionVariantParameter& variant, 
                                     bool trigger_signal = true)
    {
      T val;
      if(!(variant.get_value(val))) return (false);
      if (val != value_)
      {
        value_ = val;
        if (trigger_signal) value_changed_signal_(value_);
      }
      return (true);
    }
  
    // EXPORT_TO_VARIANT
    // Export the state data to a variant parameter
    virtual void export_to_variant(ActionVariantParameter& variant)
    {
      variant.set_value(value_);
    }
        
};

} // end namespace Seg3D

#endif
