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

    // VALIDATE_AND_COMPARE_VALUE:
    // Validate the input in a variant parameter. As this can come from a script
    // it needs to be validated properly. The function will return both whether
    // the value changes as well as if the value can be converted. If it cannot
    // be converted the function will return false.

    template<class T>
    bool validate_and_compare_value(const T& old_val,
                                    bool& changed,
                                    std::string& error)
    {
      // If a typed version exists
      if (typed_value_.get())
      {
        // if the typed version exists, use that one
        // use a dynamic cast to ensure that the type is correct
        ActionParameter<T>* param_ptr = dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        if (param_ptr == 0) 
        {
          error = "The value stored in the parameter is not of the right type";
          return (false);
        }
        changed = (old_val != param_ptr->value());
        return (true);
      }
      else
      {
        // Generate a new typed version. So it is only converted once
        typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>);
        if(!(typed_value_->import_from_string(string_value_))) 
        {
          error = "The value '"+string_value_+"' cannot be converted into type "+ typeid(T).name();
          return (false);
        }
        ActionParameter<T>* param_ptr = static_cast<ActionParameter<T>*>(typed_value_.get());
        changed = (old_val != param_ptr->value());        
        return (true);           
      }
    }

    // VALIDATE_AND_COMPARE_CLAMPED_VALUE:
    // Validate the input in a variant parameter. As this can come from a script
    // it needs to be validated properly. The function will return both whether
    // the value changes as well as if the value can be converted. If it cannot
    // be converted the function will return false.
    // This version of the function checks as well the range. If the value is
    // out of range the value cannot be converted.
    
    template<class T>
    bool validate_and_compare_clamped_value(const T& min, 
                                            const T& max, 
                                            const T& old_value,
                                            bool& changed,
                                            std::string& error)
    {
      // If a typed version exists
      if (typed_value_.get())
      {
        // if the typed version exists, use that one
        // use a dynamic cast to ensure that the type is correct
        ActionParameter<T>* param_ptr = dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        if (param_ptr == 0)
        {
          error = "The value stored in the parameter is not of the right type";
          return (false);
        }

        
        T new_value = param_ptr->value();
        if (new_value < min || new_value > max) 
        {
          error = "The new value is out of range";
          return (false);
        }
        
        changed = (old_value != new_value);
        return (true);
      }
      else
      {
        // Generate a new typed version. So it is only converted once
        typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>);
        if(!(typed_value_->import_from_string(string_value_))) 
        {
          error = "The value '"+string_value_+"' cannot be converted into type "+ typeid(T).name();
          return (false);
        }
                
        ActionParameter<T>* param_ptr = static_cast<ActionParameter<T>*>(typed_value_.get());

        T new_value = param_ptr->value();
        if (new_value < min || new_value > max) 
        {
          error = "The new value is out of range";
          return (false);
        }
                
        changed = (old_value != new_value);
        return (true);
      }
    }


    // VALIDATE_AND_COMPARE_OPTION_VALUE:
    // Validate the input in a variant parameter. As this can come from a script
    // it needs to be validated properly. The function will return both whether
    // the value changes as well as if the value can be converted. If it cannot
    // be converted the function will return false.

    bool validate_and_compare_option_value(const std::vector<std::string>& option_list,
                                           const std::string& old_val,
                                           bool& changed,
                                           std::string& error);

    template<class T>
    bool get_value(T& value)
    {
      // If a typed version exists
      if (typed_value_.get())
      {
        // if the typed version exists, use that one
        // use a dynamic cast to ensure that the type is correct
        ActionParameter<T>* param_ptr = dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        if (param_ptr == 0)
        {
          // Oops it was stored as a different type
          SCI_LOG_ERROR("Trying to access a value of a different type");
          return (false);
        }
        value = param_ptr->value();
      }
      else
      {
        // Generate a new typed version. So it is only converted once
        typed_value_ = ActionParameterBaseHandle(new ActionParameter<T>);
        if(!(typed_value_->import_from_string(string_value_))) return (false);
        
        ActionParameter<T>* param_ptr = dynamic_cast<ActionParameter<T>*>(typed_value_.get());
        value = param_ptr->value();   
      }
        return (true);   
    }

  private:
  
    // Typed version
    ActionParameterBaseHandle  typed_value_;

    // String version
    std::string                string_value_; 

};

} // end namespace Seg3D

#endif
