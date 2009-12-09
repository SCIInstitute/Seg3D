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

#ifndef APPLICATION_STATE_STATECLAMPEDVALUE_H
#define APPLICATION_STATE_STATECLAMPEDVALUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Application/State/State.h>

namespace Seg3D {

// STATECLAMPEDVALUE:
// This class is a specification of State that is use to hold a single bound
// instance of a value.

template<class T>
class StateClampedValue;

template<class T>
class StateClampedValue : public State<T> {
  public:
    // One cannot define a templated typedef of StateHandle<>,
    // Hence we settle for State<T>::Handle
    typedef boost::shared_ptr<StateClampedValue<T> > Handle;

// -- constructor/destructor --
  public:

    // It is always good to initialize a state Value before
    // using it.
    StateClampedValue(const T& min, const T& max, const T& step, const T& default_value) :
      State<T>(default_value),
      min_(min),
      max_(max),
      step_(step)
    {
      if (this->value_ > max) this->value_ = max;
      if (this->value_ < min) this->value_ = min;
    }

    virtual ~StateClampedValue()
    {
    }  
    

// -- min/max/set value --
  public:
  
    T min() const
    {
      return min_;
    }

    T max() const
    {
      return max_;
    }

    T step() const
    {
      return step_;
    }
    
    void set_range(const T& min, const T& max)
    {
      min_ = min;
      max_ = max;
      if (this->value_ < min_) 
      {
        this->value_ = min_;
        value_changed_signal_(this->value_);
      }

      if (this->value_ > max_) 
      {
        this->value_ = max_;
        value_changed_signal_(this->value_);
      }
    }

    // SET:
    // Set the value in this State variable
    virtual void set(const T& val)
    {
      T new_value = val;
      if (new_value > max_) new_value = max_;
      if (new_value < min_) new_value = min_;
      if (this->value_ == new_value) return; 

      this->value_ = new_value;      
      value_changed_signal_(this->value_);
    }
    
// -- actual state of this object --
  protected:
    
    // The internal parameters of the state variable
    T min_;
    T max_;
    T step_;
    
// -- action handling --
  public:  
  
    // VALIDATE_AND_COMPARE_VARIANT:
    // Validate that the data contained in the variant parameter can actually
    // be used and check whether it changed from the current value.
    virtual bool validate_and_compare_variant(ActionVariantParameter& variant, 
                                              bool& changed,
                                              std::string& error) const
    {
      T new_value;
      if (!(variant.get_value(new_value,error))) return (false);
      if (new_value < min_ || new_value > max_) 
      {
        error = "The new value is out of range";
        return (false);
      }

      changed = (this->value_ != new_value);
      return (true);   
    }
    
    
    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.  
    virtual bool import_from_variant(ActionVariantParameter& variant, 
                                     bool trigger_signal = true)
    {
      T val;
      if(!(variant.get_value(val))) return (false);
      if (val < min_) val = min_;
      if (val > max_) val = max_;
      
      if (val != this->value_)
      {
        this->value_ = val;
        if (trigger_signal) value_changed_signal_(this->value_);
      }
      return (true);
    }
  
};

} // end namespace Seg3D

#endif
