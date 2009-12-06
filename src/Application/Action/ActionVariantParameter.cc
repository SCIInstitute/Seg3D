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

#include <Application/Action/ActionVariantParameter.h>

namespace Seg3D {

ActionVariantParameter::~ActionVariantParameter()
{
}

std::string
ActionVariantParameter::export_to_string() const
{
  // Export a value that is still typed or has been convereted to a string
  // if typed_value exist, we need to convert it
  if (typed_value_.get())
  {
    return (typed_value_->export_to_string());
  }
  else
  {
    // in case typed_value does not exist it must be recorded as a string
    return (string_value_);
  }
}

bool
ActionVariantParameter::import_from_string(const std::string& str)
{
  // As we do not know the implied type. It can only be recorded as a string
  typed_value_.reset();
  string_value_ = str;
  
  return (true);
}

bool 
ActionVariantParameter::validate_and_compare_option_value(
               const std::vector<std::string>& option_list,
               const std::string& old_val,
               bool& changed,
               std::string& error)
{
  // If a typed version exists
  if (typed_value_.get())
  {
    // if the typed version exists, use that one
    // use a dynamic cast to ensure that the type is correct
    ActionParameter<std::string>* param_ptr = 
        dynamic_cast<ActionParameter<std::string>*>(typed_value_.get());
    if (param_ptr == 0) 
    {
      error = "The value stored in the parameter is not of the right type";
      return (false);
    }
    std::string value = Utils::string_to_lower(param_ptr->value());
    
    if (option_list.end() == std::find(option_list.begin(),option_list.end(),
        value)) 
    {
      error = "The option value is not one of the predefined options";
      return (false);
    }
    
    changed = (old_val != value);
    return (true);
  }
  else
  {
    // Generate a new typed version. So it is only converted once
    typed_value_ = ActionParameterBaseHandle(new ActionParameter<std::string>);
    if(!(typed_value_->import_from_string(string_value_))) 
    {
      error = "The value '"+string_value_+"' cannot be converted into type string";
      return (false);
    }
    ActionParameter<std::string>* param_ptr = 
            static_cast<ActionParameter<std::string>*>(typed_value_.get());
    std::string value = Utils::string_to_lower(param_ptr->value());
    
    if (option_list.end() == std::find(option_list.begin(),option_list.end(),
          value)) 
    {
      error = "The option value is not one of the predefined options";
      return (false);
    }
    return (true);           
  }
}

} // end namespace Seg3D
