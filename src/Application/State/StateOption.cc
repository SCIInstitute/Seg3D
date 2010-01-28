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

StateOption::StateOption(const std::string& default_value,
                         const std::string& option_list ) :
  value_(default_value)
{
  // Unwrap the option lists
  std::string option_list_string = Utils::string_to_lower(option_list);
  while(1)
  {
    size_t loc = option_list_string.find('|');
    if (loc >= option_list_string.size())
    {
      option_list_.push_back(option_list_string);
      break;
    }
    option_list_.push_back(option_list_string.substr(0,loc));
    option_list_string = option_list_string.substr(loc+1);
  }
}

StateOption::StateOption(const std::string& default_value,
                         const std::vector<std::string>& option_list ) :
  value_(default_value)
{
  option_list_.resize(option_list.size());
  for (size_t j=0; j<option_list.size(); j++)
  {
    option_list_[j] = Utils::string_to_lower(option_list[j]);
  }
}

StateOption::~StateOption()
{
}  

std::string 
StateOption::export_to_string() const
{
  return (value_);
}

bool 
StateOption::import_from_string(const std::string& str,
                                bool from_interface)
{
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());

  // Ensure that value is only changed when the string can
  // successfully converted.
  std::string value;
  if(!(Utils::import_from_string(str,value))) return (false);
  
  value = Utils::string_to_lower(value);
  if (value != value_) 
  {
    if (option_list_.end() == std::find(option_list_.begin(),
            option_list_.end(),value)) return (false);
    value_ = value;

    value_changed_signal(value_,from_interface);
  }
  return (true);
}

void 
StateOption::export_to_variant(ActionParameterVariant& variant) const
{
  variant.set_value(value_);
}

bool 
StateOption::import_from_variant(ActionParameterVariant& variant,
                                 bool from_interface)
{
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());

  std::string value;
  if (!( variant.get_value(value) )) return (false);

  value = Utils::string_to_lower(value);
  if (value != value_)
  {
    if (option_list_.end() == std::find(option_list_.begin(),
            option_list_.end(),value)) return (false);
    value_ = value;
    value_changed_signal(value_,from_interface);
  }
  return (true);
}

bool 
StateOption::validate_variant(ActionParameterVariant& variant, 
                              std::string& error)
{
  std::string value;
  if (!(variant.get_value(value)))
  {
    error = "Cannot convert the value '"+variant.export_to_string()+"'";
    return (false);
  }
  
  value = Utils::string_to_lower(value);
  if (option_list_.end() == std::find(option_list_.begin(),
      option_list_.end(),value))
  {
    error = "Option '"+value+"' is not a valid option";
    return (false);
  }
  
  error = "";
  return (true);
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
  if (option_list_.end() == std::find(option_list_.begin(),option_list_.end(),value_))
  {
    if (option_list.size()) value_ = option_list[0]; else value_ = "";
    value_changed_signal(value_,false);
  }
  
  optionlist_changed_signal();
}



void 
StateOption::set_option_list(const std::string& option_list)
{
  // Unwrap the option lists
  std::string option_list_string = Utils::string_to_lower(option_list);
  
  option_list_.clear();
  while(1)
  {
    size_t loc = option_list_string.find('|');
    if (loc >= option_list_string.size())
    {
      option_list_.push_back(option_list_string);
      break;
    }
    option_list_.push_back(option_list_string.substr(0,loc));
    option_list_string = option_list_string.substr(loc+1);
  }
  
  if (option_list_.end() == std::find(option_list_.begin(),option_list_.end(),value_))
  {
    if (option_list.size()) value_ = option_list[0]; else value_ = "";
    value_changed_signal(value_,false);
  }
  
  optionlist_changed_signal();
}

void 
StateOption::set_option_list(const std::string& option_list,
                             const std::string& option)
{
  // Unwrap the option lists
  std::string option_list_string = Utils::string_to_lower(option_list);
  
  option_list_.clear();
  while(1)
  {
    size_t loc = option_list_string.find('|');
    if (loc >= option_list_string.size())
    {
      option_list_.push_back(option_list_string);
      break;
    }
    option_list_.push_back(option_list_string.substr(0,loc));
    option_list_string = option_list_string.substr(loc+1);
  }

  std::string lower_option = Utils::string_to_lower(option);
  
  if (option_list_.end() == 
      std::find(option_list_.begin(),option_list_.end(),lower_option))
  {
    if (option_list.size()) value_ = option_list[0]; else value_ = "";
    value_changed_signal(value_,false);
  }
  else
  {
    if (value_ != lower_option)
    { 
      value_ = lower_option;
      value_changed_signal(value_,false);
    }
  }
  
  optionlist_changed_signal();
}


} // end namespace Seg3D
