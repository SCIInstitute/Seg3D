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

namespace Utils {


bool
scan_command(const std::string& str, 
             std::string::size_type& start, 
             std::string& command,
             std::string& error)
{
  // Scan for where the command starts
  while ((start < str.size()) && 
    ((str[start] == ' ')||(str[start] == '\t')||
     (str[start] == '\n')||(str[start] == '\r'))) start++;

  if (start >= str.size())
  {
    // An empty value signals the end
    command = "";
    return (true);
  }    

  
  std::string::size_type command_start = start;
  std::string::size_type command_size = 0;

  // Scan for the end of the command
  while ((start < str.size()) &&
    ((str[start] != ' ')&&(str[start] != '\t')&&
     (str[start] != '\n')&&(str[start] != '\r'))) 
  {
    // Check whether command name is alpha numeric
    if ((str[start] >= 'a' && str[start] <= 'z')||
        (str[start] >= 'A' && str[start] <= 'Z')||
        (str[start] >= '0' && str[start] <= '9')||
        (str[start] == '_'))
    {
      start++; 
      command_size++; 
    }
    else
    {
      // If not post an error
      error = std::string("Detected illegal symbol '")+ str[start] +"' in command name '"+
        str.substr(command_start,command_size)+"'";
      return (false);
    }
  }

  command = str.substr(command_start,command_size);
  return (true);
}


bool
scan_value(const std::string& str, 
                  std::string::size_type& start,
                  std::string& value,
                  std::string& error)
{
  // Scan for where the key starts
  while ((start < str.size()) && 
    ((str[start] == ' ')||(str[start] == '\t')||
     (str[start] == '\n')||(str[start] == '\r'))) start++;
  
  if (start >= str.size())
  {
    // An empty value signals the end
    value = "";
    return (true);
  }    
              
  std::string::size_type value_start = start;
  std::string::size_type value_size = 0;

  // Scan the value
  // This can be of any format like
  // ( ..... ) with white spaces in between
  // [ ..... ] with white spaces in between
  // " ..... " with white spaces in between Note the quotes are removed
  // any other string with no white spaces
  if (str[start] == '[')
  {
    int paren_count = 1;
    start++;
    while(start < str.size())
    {
      if (str[start] == '[') paren_count++; 
      else if (str[start] == ']') paren_count--;
      else if (str[start] == '"')
      {
        start++; value_size++;
        while(start < str.size())
        {
          if (str[start] == '"') break;
          start++; value_size++;
        }
        // MISSING " at the end of an argument
        if (str[start] != '"')
        {
          error = "Missing '\"' at the end of the key value pair '"+
            str.substr(value_start,value_size+1)+"'";
          return (false);
        }
      }
      value_size++; start++;
      if (paren_count == 0) break;
    }
    
    if (paren_count != 0)
    {
      // MISSING ] at the end of an argument
      error = "Missing ']' at the end of the key value pair '"+
            str.substr(value_start,value_size)+"'";
      return (false);
    }
    value = str.substr(value_start,value_size);
    return (true);
  }
  else if (str[start] == '(')
  {
    int paren_count = 1;
    start++;
    while(start < str.size())
    {
      if (str[start] == '(') paren_count++; 
      else if (str[start] == ')') paren_count--;
      else if (str[start] == '"')
      {
        start++; value_size++;
        while(start < str.size())
        {
          if (str[start] == '"') break;
          start++; value_size++;
        }
        // MISSING " at the end of an argument
        if (str[start] != '"')
        {
          error = "Missing '\"' at the end of the key value pair '"+
            str.substr(value_start,value_size+1)+"'";
          return (false);
        }
      }
      value_size++; start++;
      if (paren_count == 0) break;
    }
    
    if (paren_count != 0)
    {
      // MISSING ] at the end of an argument
      error = "Missing ']' at the end of the key value pair '"+
            str.substr(value_start,value_size)+"'";
      return (false);
    }
    value = str.substr(value_start,value_size);
    return (true);
  }
  else if (str[start] == '"')
  {
    start++; value_size++;
    value_start++;
    while(start < str.size())
    {
      if (str[start] == '"') break;
      start++; value_size++;
    }
    // MISSING " at the end of an argument
    if (str[start] != '"') 
    {
      error = "Missing '\"' at the end of the key value pair '"+
            str.substr(value_start,value_size)+"'";
      return (false);  
    }
    // Strip away the quotes
    value = str.substr(value_start,value_size);
    return (true);
  }
  else
  {
    while ((start < str.size()) &&
      ((str[start] != ' ')&&(str[start] != '\t')&&
      (str[start] != '\n')&&(str[start] != '\r'))) { start++; value_size++; }
    value = str.substr(value_start,value_size);
    return (true);    
  }
}


bool
scan_keyvaluepair(const std::string& str, 
                  std::string::size_type& start,
                  std::string& key,
                  std::string& value,
                  std::string& error)
{
  // Scan for where the key starts
  while ((start < str.size()) && 
    ((str[start] == ' ')||(str[start] == '\t')||
     (str[start] == '\n')||(str[start] == '\r'))) start++;

  if (start >= str.size())
  {
    // An empty value signals the end
    key = "";
    value = "";
    return (true);
  } 
  
  std::string::size_type key_start = start;
  std::string::size_type key_size = 0;

  // Scan the key
  while ((start < str.size()) &&
    ((str[start] != '=')&&(str[start] != ' ')&&(str[start] != '\t')&&
     (str[start] != '\n')&&(str[start] != '\r'))) 
  { 
    // Check whether command name is alpha numeric
    if ((str[start] >= 'a' && str[start] <= 'z')||
        (str[start] >= 'A' && str[start] <= 'Z')||
        (str[start] >= '0' && str[start] <= '9')||
        (str[start] == '_'))
    {
      start++; 
      key_size++; 
    }
    else
    {
      // If not post an error
      error = std::string("Detected illegal symbol '") + 
        str[start] +"' in key name '"+
        str.substr(key_start,key_size)+"'";
      return (false);    
    }
  }

  // Check whether the key is followed by an equal sign
  if ((start >= str.size()) || (str[start] != '=')) return (false);
  
  key = str.substr(key_start,key_size);
  start++;
  
  // If there is an empty string behind the key
  if (start == str.size()) { value = ""; return (true); }

  std::string::size_type value_start = start;
  std::string::size_type value_size = 0;

  // Scan the value
  // This can be of any format like
  // ( ..... ) with white spaces in between
  // [ ..... ] with white spaces in between
  // " ..... " with white spaces in between Note the quotes are removed
  // any other string with no white spaces
  if (str[start] == '[')
  {
    int paren_count = 1;
    start++;
    while(start < str.size())
    {
      if (str[start] == '[') paren_count++; 
      else if (str[start] == ']') paren_count--;
      else if (str[start] == '"')
      {
        start++; value_size++;
        while(start < str.size())
        {
          if (str[start] == '"') break;
          start++; value_size++;
        }
        // MISSING " at the end of an argument
        if (str[start] != '"')
        {
          error = "Missing '\"' at the end of the key value pair '"+
            str.substr(value_start,value_size+1)+"'";
          return (false);
        }
      }
      value_size++; start++;
      if (paren_count == 0) break;
    }
    
    if (paren_count != 0)
    {
      // MISSING ] at the end of an argument
      error = "Missing ']' at the end of the key value pair '"+
            str.substr(value_start,value_size)+"'";
      return (false);
    }
    value = str.substr(value_start,value_size);
    return (true);
  }
  else if (str[start] == '(')
  {
    int paren_count = 1;
    start++;
    while(start < str.size())
    {
      if (str[start] == '(') paren_count++; 
      else if (str[start] == ')') paren_count--;
      else if (str[start] == '"')
      {
        start++; value_size++;
        while(start < str.size())
        {
          if (str[start] == '"') break;
          start++; value_size++;
        }
        // MISSING " at the end of an argument
        if (str[start] != '"')
        {
          error = "Missing '\"' at the end of the key value pair '"+
            str.substr(value_start,value_size+1)+"'";
          return (false);
        }
      }
      value_size++; start++;
      if (paren_count == 0) break;
    }
    
    if (paren_count != 0)
    {
      // MISSING ] at the end of an argument
      error = "Missing ']' at the end of the key value pair '"+
            str.substr(value_start,value_size)+"'";
      return (false);
    }
    value = str.substr(value_start,value_size);
    return (true);
  }
  else if (str[start] == '"')
  {
    start++; value_size++;
    value_start++;
    while(start < str.size())
    {
      if (str[start] == '"') break;
      start++; value_size++;
    }
    // MISSING " at the end of an argument
    if (str[start] != '"') 
    {
      error = "Missing '\"' at the end of the key value pair '"+
            str.substr(value_start,value_size)+"'";
      return (false);  
    }
    // Strip away the quotes
    value = str.substr(value_start,value_size);
    return (true);
  }
  else
  {
    while ((start < str.size()) &&
      ((str[start] != ' ')&&(str[start] != '\t')&&
      (str[start] != '\n')&&(str[start] != '\r'))) { start++; value_size++; }
    value = str.substr(value_start,value_size);
    return (true);    
  }
}


} // end namesapce Utils
