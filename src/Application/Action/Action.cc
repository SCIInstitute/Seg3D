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

#include <iostream>

#include <boost/algorithm/string.hpp>

// For the conversion code
#include <Utils/Core/StringUtil.h>
#include <Utils/Converter/StringParser.h>

#include <Application/Action/ActionParameter.h>
#include <Application/Action/Action.h>

namespace Seg3D {

Action::Action() :
  result_(0)
{
}

Action::~Action()
{
}

bool
Action::validate(ActionHandle& self, ActionContextHandle& context)
{
  return (true);
}

void
Action::add_argument_ptr(ActionParameterBase* param)
{
  arguments_.push_back(param);
}

void
Action::add_parameter_ptr(const std::string& key,ActionParameterBase* param)
{
  parameters_[boost::to_lower_copy(key)] = param;
}

void
Action::add_result_ptr(ActionParameterBase* param)
{
  result_ = param;
}

std::string 
Action::export_action_to_string() const
{
  // Add action name to string
  std::string command = std::string(type()) + " ";

  // Loop through all the arguments and add them
  for (size_t j=0; j<arguments_.size(); j++)
  {
    command += arguments_[j]->export_to_string()+" ";
  }
  
  // Loop through all the parameters and add them
  parameter_map_type::const_iterator it = parameters_.begin();
  parameter_map_type::const_iterator it_end = parameters_.end();
  while (it != it_end)
  {
    command += (*it).first+"="+(*it).second->export_to_string()+" ";
    ++it;
  }
  
  // Return the command
  return command;
}

std::string 
Action::export_result_to_string() const
{
  if (result_ ) return ( result_->export_to_string());
  else return std::string("");
}

bool
Action::import_action_from_string(const std::string& action, std::string& error)
{
  std::string::size_type pos = 0;
  std::string command;
  std::string value;

  // First part of the string is the command
  if (!(Utils::scan_command(action,pos,command,error)))
  {
    error = std::string("SYNTAX ERROR: ") + error;
    return (false);
  }

  if (command.empty())
  {
    error = std::string("ERROR: missing command.");
    return (false);
  }
  
  for (size_t j=0; j<arguments_.size(); j++)
  {
    if (!(Utils::scan_value(action,pos,value,error)))
    {
      error = std::string("SYNTAX ERROR: ")+error;
    }
    
    if (value.empty())
    {
      error = std::string("ERROR: not enough arguments, argument ")+
            Utils::to_string(j+1)+" is missing.";
      return false;
    }
    
    if (!(arguments_[j]->import_from_string(value)))
    {
      error = std::string("SYNTAX ERROR: Could not interpret '"+value+"'");
      return (false);
    }
  }
  
  // Get all the key value pairs and stream them into the action
  std::string key;
  
  while (1)
  {
    if (!(Utils::scan_keyvaluepair(action,pos,key,value,error)))
    {
      error = std::string("SYNTAX ERROR: ") + error;
      return (false);
    }

    if (key.empty()) break;

    boost::to_lower(key);
     
    parameter_map_type::iterator it = parameters_.find(key);
    if (it != parameters_.end()) 
    {
      if(!((*it).second->import_from_string(value)))
      {
        error = std::string("SYNTAX ERROR: Could not interpret '"+value+"'");
        return (false);
      }
    }
  }

  return (true);
}

} // end namespace Seg3D
