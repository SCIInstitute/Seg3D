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

// STL includes
#include <algorithm>

#include <Application/State/StateOption.h>

namespace Seg3D {

StateOption::StateOption(const std::vector<std::string>& option_list, 
            const std::string& default_option) :
  State<std::string>(default_option),
  option_list_(option_list),
  default_option_(default_option)
{
  // Ensure that options are lower case
  for (size_t j=0; j<option_list_.size();j++)
  {
    option_list_[j] = Utils::string_to_lower(option_list_[j]);
  }
}

StateOption::~StateOption()
{
}  
    
void 
StateOption::set(const std::string& option)
{
  std::string lower_option = Utils::string_to_lower(option);
  
  // Ensure that options are lower case
  for (size_t j=0; j<option_list_.size();j++)
  {
    option_list_[j] = Utils::string_to_lower(option_list_[j]);
  }
  
  // Ensure it is a valid option
  if (option_list_.end() == std::find(option_list_.begin(),option_list_.end(),
    lower_option)) return;

  value_ = lower_option;
  value_changed_signal_(value_);
}

bool 
StateOption::is_option(const std::string& option)
{
  if (option_list_.end() == std::find(option_list_.begin(),option_list_.end(),
    Utils::string_to_lower(option))) return (false);
  return (true);
}

void 
StateOption::set_option_list(const std::vector<std::string>& option_list)
{
  option_list_ = option_list;
  if (option_list_.end() == std::find(option_list_.begin(),option_list_.end(),
    value_))
  {
    value_ = default_option_;
    value_changed_signal_(value_);
  }
}

bool 
StateOption::import_from_string(const std::string& str)
{
  // Ensure that value is only changed when the string can
  // successfully converted.
  std::string value;
  bool success = Utils::import_from_string(str,value);

  if (success) 
  {
    value = Utils::string_to_lower(value);
    if (option_list_.end() == std::find(option_list_.begin(),
            option_list_.end(),value)) success = false;
    // only initialize with a valid value
    if (success) value_ = value;
  }
  return (success);
}
    
bool 
StateOption::validate_and_compare_variant(ActionVariantParameter& variant, 
                                          bool& changed,
                                          std::string& error) const
{
  std::string new_value;
  if (!(variant.get_value(new_value,error))) return (false);

  new_value = Utils::string_to_lower(new_value);    
  if (option_list_.end() == std::find(option_list_.begin(),option_list_.end(),
      new_value)) 
  {
    error = "The option value is not one of the predefined options";
    return (false);
  }
    
  changed = (value_ != new_value);
  return (true);   
}
  
  
bool 
StateOption::import_from_variant(ActionVariantParameter& variant, 
                                     bool trigger_signal)
{
  std::string val;
  if(!(variant.get_value(val))) return (false);
  
  val = Utils::string_to_lower(val);
  // Check if it is valid option
  if (option_list_.end() == std::find(option_list_.begin(),
                                option_list_.end(),val)) return (false);

  if (val != value_)
  {
    value_ = val;
    if (trigger_signal) value_changed_signal_(value_);
  }
  return (true);
}        

} // end namespace Seg3D
