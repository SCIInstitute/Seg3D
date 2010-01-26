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

#ifndef APPLICATION_ACTION_ACTIONPARAMETER_H
#define APPLICATION_ACTION_ACTIONPARAMETER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL
#include <string>
#include <algorithm>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils
#include <Utils/Core/Log.h>
#include <Utils/Core/StringUtil.h>
#include <Utils/Converter/StringConverter.h>

namespace Seg3D {

// PARAMETER CLASSES
// ACTIONPARAMETER<TYPE> and ACTIONPARAMETERVARIANT
// These two classes are both used to store parameters the first one explicitly 
// states the type and the second one does type conversion. The difference is
// that for the first class type the compiler checks the type integrity and 
// hence is the preferred model. However types are not always know up front
// or the action is so general that the parameter is not of a specied class
// for the latter case do we use the variant version.


// ACTIONPARAMETERBASE:
// Base class needed for uniform access to import and export the value
// in a uniform way.

class ActionParameterBase;
typedef boost::shared_ptr<ActionParameterBase> ActionParameterBaseHandle;

class ActionParameterBase {
// -- define handle --
  public:
    typedef boost::shared_ptr<ActionParameterBase> Handle;
  
// -- destructor --
  public:
    virtual ~ActionParameterBase();
    
// -- functions for accessing data --
    // EXPORT_TO_STRING
    // export the contents of the parameter to string
    virtual std::string export_to_string() const = 0;
 
    // IMPORT_FROM_STRING
    // import a parameter from a string. The function returns true
    // if the import succeeded
    virtual bool import_from_string(const std::string& str) = 0;

};


// ACTIONPARAMETER:
// Parameter for an action.

// Forward declaration:
template<class T> class ActionParameter;

// Class definition:
template<class T>
class ActionParameter : public ActionParameterBase {

// -- define handle --
  public:
    typedef boost::shared_ptr<ActionParameter<T> > Handle;

// -- constructor/destructor --
  public:
    ActionParameter()
    {}
    
    ActionParameter(const T& default_value) :
      value_(default_value)
    {}
  
    virtual ~ActionParameter()
    {}

// -- access to value --
  public:
    // General access to the parameter value
    T& value() { return value_; } 

    // For run when running with constness
    const T& value() const { return value_; } 

    // Get access similar to the variant version
    bool get_value(T& value) { value = value_; return (true); }
    
    // Set access similar to the variant version
    void set_value(const T& value) { value_ = value; }

    // EXPORT_TO_STRING
    // export the contents of the parameter to string
    virtual std::string export_to_string() const
    {
      return Utils::export_to_string(value_);
    }

    // IMPORT_FROM_STRING
    // import a parameter from a string. The function returns true
    // if the import succeeded
    virtual bool import_from_string(const std::string& str)
    {
      return Utils::import_from_string(str,value_);
    }
        
  private:
    // The actual value
    T value_;
};

// ACTIONPARAMETERVARIANT:
// Parameter for an action.

// Forward declaration:
class ActionParameterVariant;

// Class definition:
class ActionParameterVariant : public ActionParameterBase {

// -- define handle --
  public:
    typedef boost::shared_ptr<ActionParameterVariant> Handle;

// -- constructor/destructor --
  public:
    // CONSTRUCTOR of untyped parameter
    ActionParameterVariant();

    // CONSTRUCTOR of typed version
    template<class T>
    ActionParameterVariant(const T& default_value)
    {
      typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>(default_value));
    }
    
    // DESTRUCTOR
    virtual ~ActionParameterVariant();

// -- functions for accessing data --
  public:
  
    // EXPORT_TO_STRING
    // export the contents of the parameter to string
    virtual std::string export_to_string() const;
    
    // IMPORT_FROM_STRING
    // import a parameter from a string. The function returns true
    // if the import succeeded
    virtual bool import_from_string(const std::string& str);

    // SET_VALUE
    // Set the value using a typed version of the parameter
    template<class T>
    void set_value(const T& value)
    {
      // Set the value typed
      typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>(value));
      // Clear string version
      string_value_.clear();
    }

    // GET_VALUE
    // Get the value from string or typed value. If a typed one is available
    // use that one.
    template<class T>
    bool get_value(T& value)
    {
      // If a typed version exists
      if (typed_value_.get())
      {
        // if the typed version exists, use that one
        // use a dynamic cast to ensure that the type is correct
        ActionParameter<T>* param_ptr = 
                        dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        if (param_ptr == 0)
        {
          if(!(Utils::import_from_string(typed_value_->export_to_string(),value)))
          {
            return (false);
          }

          return (true);
        }
        
        value = param_ptr->value();
        return (true);  
      }
      else
      {
        // Generate a new typed version. So it is only converted once
        ActionParameter<T>* param_ptr = new ActionParameter<T>;
        typed_value_ = ActionParameterBaseHandle(param_ptr);
        if(!(typed_value_->import_from_string(string_value_)))
        {
          return (false);
        }

        value = param_ptr->value(); 
        return (true);    
      }
    }

    // VALIDATE_TYPE
    // Check and convert to a certain type, but do not return the value
    // This function is intended for validating the action by forcing the
    // contained value to be converted to a certain type
    template<class T>
    bool validate_type()
    {
      // If a typed version exists
      if (typed_value_.get())
      {
        // if the typed version exists, use that one
        // use a dynamic cast to ensure that the type is correct
        ActionParameter<T>* param_ptr = 
                        dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        if (param_ptr == 0)
        {
          T value;
          if(!(Utils::import_from_string(typed_value_->export_to_string(),value)))
          {
            return (false);
          }
        }
        return (true);  
      }
      else
      {
        // Generate a new typed version. So it is only converted once
        ActionParameter<T>* param_ptr = new ActionParameter<T>;
        typed_value_ = ActionParameterBaseHandle(param_ptr);
        if(!(typed_value_->import_from_string(string_value_)))
        {
          return (false);
        }
        return (true);    
      }
    }




// -- accessors --

    // TYPED_VALUE:
    // Access to the parameter variable that stores the typed variable.
    ActionParameterBaseHandle& typed_value()  { return typed_value_;  }
 
    // STRING_VALUE:
    // The value stored as a string. This one is only valid if there is
    // no typed value.
    std::string&               string_value() { return string_value_; }

  private:
  
    // Typed version
    ActionParameterBaseHandle  typed_value_;

    // String version
    std::string                string_value_; 

};


} // namespace Seg3D


#endif
