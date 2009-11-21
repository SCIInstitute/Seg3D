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

#include <Utils/Converter/StringParser.h>
#include <Utils/Converter/StringConverter.h>

#include <Application/Action/ActionFactory.h>

namespace Seg3D {

ActionFactory::ActionFactory()
{
}

bool
ActionFactory::create_action(const std::string& action_string,
                             ActionHandle& action,
                             std::string& error) const
{
  std::string command;
  std::string::size_type pos;
  
  // Scan for the command that needs to be instanted.
  if(!(Utils::scan_command(action_string,pos,command,error)))
  {
    error = std::string("SYNTAX ERROR: ") + error;
    return (false);
  }

  boost::to_lower(command);
  // NOTE: Factory is not locked as we assume that all actions are already
  // inserted.
  action_map_type::const_iterator it = action_builders_.find(command);

  // If we cannot find the maker report error.
  if (it == action_builders_.end())
  {
    error = std::string("SYNTAX ERROR: Unknown command '"+command+"'");
    return (false);
  }
  
  // Build the action of the right type
  action = (*it).second->build();
  
  if(!(action->import_action_from_string(action_string,error)))
  {
    // the import_action_from_string function reports the error and hence
    // we do not need to set it here.
    return (false);
  }
  return true;
}

// Singleton instance
Utils::Singleton<ActionFactory> ActionFactory::instance_;

} // end namespace seg3D
