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

#ifndef APPLICATION_ACTION_ACTIONVARIANTPARAMETER_H
#define APPLICATION_ACTION_ACTIONVARIANTPARAMETER_H

// STL
#include <string>
#include <typeinfo>
#include <algorithm>

// Boost
#include <boost/shared_ptr.hpp>

// Utils
#include <Utils/Core/Log.h>
#include <Utils/Core/StringUtil.h>
#include <Application/Action/ActionParameter.h>

namespace Seg3D {

typedef boost::shared_ptr<ActionParameterBase> ActionParameterBaseHandle;

class ActionVariantParameter : public ActionParameterBase {

// -- define handle --
  public:
    typedef boost::shared_ptr<ActionParameterBase> Handle;

  public:
    virtual ~ActionVariantParameter();

    virtual std::string export_to_string() const;
    virtual bool import_from_string(const std::string& str);

    template<class T>
    void set_value(const T& value)
    {
      // Set the value typed
      typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>(value));
    }

    template<class T>
    bool get_value(T& value)
    {
      std::string error;
      bool success = get_value(value,error);
      if (success == false) SCI_LOG_ERROR(error);
      return (true);
    }
    
    
    template<class T>
    bool get_value(T& value, std::string& error)
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
          // Oops it was stored as a different type
          error = "The value stored in the parameter is not of the right type";
          return (false);
        }
        value = param_ptr->value();
      }
      else
      {
        // Generate a new typed version. So it is only converted once
        typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>);
        if(!(typed_value_->import_from_string(string_value_)))
        {
          error = "The value '"+string_value_+
                  "' cannot be converted into type "+ typeid(T).name();
          return (false);
        }
        ActionParameter<T>* param_ptr = 
                        dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        value = param_ptr->value();   
      }
        return (true);   
    }

// -- accessors --
    // Accessors to valid the internal state of the variant variable.
    // These two are used for validating the state variables whose type
    // cannot be accessed through the base class. Hence only the typed
    // state variable can assess the right type.

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

} // end namespace Seg3D

#endif
