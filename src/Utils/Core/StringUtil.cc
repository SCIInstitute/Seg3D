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

#include <Utils/Core/StringUtil.h>

#include <stdlib.h>
#include <limits>

namespace Utils {

std::string
string_to_upper(std::string str) 
{
  std::string::iterator iter = str.begin();
  std::string::iterator iend = str.end();
  for (; iter != iend; ++iter)
    *iter = toupper(*iter);
  return str;
}

std::string
string_to_lower(std::string str) 
{
  std::string::iterator iter = str.begin();
  std::string::iterator iend = str.end();
  for (; iter != iend; ++iter)
    *iter = tolower(*iter);
  return str;
}

bool from_string(const std::string &str, double &value)
{
  // Clear out any markup of the numbers that make it easier to read and
  // replace it all with spaces.
  std::string data = str;
  for (size_t j=0; j<data.size(); j++) 
    if ((data[j] == '\t')||(data[j] == '\r')||(data[j] == '\n')||(data[j]=='"')
        ||(data[j]==',')||(data[j]=='[')||(data[j]==']')||(data[j]=='(')
        ||(data[j]==')')) data[j] = ' ';
  
  // if empty just return
  if (data.size() == 0) return (false);
  
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if (data.size() > 2 && 
      (data[0] == 'n' || data[0] == 'N') &&
      (data[1] == 'A' || data[1] == 'A') &&
      (data[2] == 'n' || data[2] == 'N'))      
  {
    value = std::numeric_limits<double>::quiet_NaN(); 
    return (true);
  }
  // handle inf
  else if (data.size() > 2 && 
      (data[0] == 'i' || data[0] == 'I') &&
      (data[1] == 'n' || data[1] == 'N') &&
      (data[2] == 'f' || data[2] == 'F'))   
  {
    value = std::numeric_limits<double>::infinity(); 
    return (true);
  }
  // handle +inf and -inf
  else if (data.size() > 3 && 
      (data[0] == '-' || data[0] == '+') &&
      (data[1] == 'i' || data[1] == 'I') &&
      (data[2] == 'n' || data[2] == 'N') &&
      (data[3] == 'f' || data[3] == 'F'))
  {
    if (data[0] == '-')
    {
      value = -std::numeric_limits<double>::infinity(); 
    }
    else
    {
      value = std::numeric_limits<double>::infinity();     
    }

    return (true);
  }
  
  std::istringstream iss(data);
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }  
}

// interal version skips the cleanup phase
bool from_string_internal(const std::string &data, double &value)
{  
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if (data.size() > 2 && 
      (data[0] == 'n' || data[0] == 'N') &&
      (data[1] == 'A' || data[1] == 'A') &&
      (data[2] == 'n' || data[2] == 'N'))      
  {
    value = std::numeric_limits<double>::quiet_NaN(); 
    return (true);
  }
  // handle inf
  else if (data.size() > 2 && 
      (data[0] == 'i' || data[0] == 'I') &&
      (data[1] == 'n' || data[1] == 'N') &&
      (data[2] == 'f' || data[2] == 'F'))   
  {
    value = std::numeric_limits<double>::infinity(); 
    return (true);
  }
  // handle +inf and -inf
  else if (data.size() > 3 && 
      (data[0] == '-' || data[0] == '+') &&
      (data[1] == 'i' || data[1] == 'I') &&
      (data[2] == 'n' || data[2] == 'N') &&
      (data[3] == 'f' || data[3] == 'F'))
  {
    if (data[0] == '-')
    {
      value = -std::numeric_limits<double>::infinity(); 
    }
    else
    {
      value = std::numeric_limits<double>::infinity();     
    }

    return (true);
  }
  
  std::istringstream iss(data);
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }  
}

bool from_string(const std::string &str, float &value)
{
  // Clear out any markup of the numbers that make it easier to read and
  // replace it all with spaces.
  std::string data = str;
  for (size_t j=0; j<data.size(); j++) 
    if ((data[j] == '\t')||(data[j] == '\r')||(data[j] == '\n')||(data[j]=='"')
        ||(data[j]==',')||(data[j]=='[')||(data[j]==']')||(data[j]=='(')
        ||(data[j]==')')) data[j] = ' ';
  
  // if empty just return
  if (data.size() == 0) return (false);
  
  // Handle special cases: nan, inf, and -inf
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if (data.size() > 2 && 
      (data[0] == 'n' || data[0] == 'N') &&
      (data[1] == 'A' || data[1] == 'A') &&
      (data[2] == 'n' || data[2] == 'N'))      
  {
    value = std::numeric_limits<float>::quiet_NaN(); 
    return (true);
  }
  // handle inf
  else if (data.size() > 2 && 
      (data[0] == 'i' || data[0] == 'I') &&
      (data[1] == 'n' || data[1] == 'N') &&
      (data[2] == 'f' || data[2] == 'F'))   
  {
    value = std::numeric_limits<float>::infinity(); 
    return (true);
  }
  // handle +inf and -inf
  else if (data.size() > 3 && 
      (data[0] == '-' || data[0] == '+') &&
      (data[1] == 'i' || data[1] == 'I') &&
      (data[2] == 'n' || data[2] == 'N') &&
      (data[3] == 'f' || data[3] == 'F'))
  {
    if (data[0] == '-')
    {
      value = -std::numeric_limits<float>::infinity(); 
    }
    else
    {
      value = std::numeric_limits<float>::infinity();     
    }

    return (true);
  }
  
  std::istringstream iss(data);
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }  
}

// internal version skips the cleanup
bool from_string_internal(const std::string &data, float &value)
{
  // Handle special cases: nan, inf, and -inf
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if (data.size() > 2 && 
      (data[0] == 'n' || data[0] == 'N') &&
      (data[1] == 'A' || data[1] == 'A') &&
      (data[2] == 'n' || data[2] == 'N'))      
  {
    value = std::numeric_limits<float>::quiet_NaN(); 
    return (true);
  }
  // handle inf
  else if (data.size() > 2 && 
      (data[0] == 'i' || data[0] == 'I') &&
      (data[1] == 'n' || data[1] == 'N') &&
      (data[2] == 'f' || data[2] == 'F'))   
  {
    value = std::numeric_limits<float>::infinity(); 
    return (true);
  }
  // handle +inf and -inf
  else if (data.size() > 3 && 
      (data[0] == '-' || data[0] == '+') &&
      (data[1] == 'i' || data[1] == 'I') &&
      (data[2] == 'n' || data[2] == 'N') &&
      (data[3] == 'f' || data[3] == 'F'))
  {
    if (data[0] == '-')
    {
      value = -std::numeric_limits<float>::infinity(); 
    }
    else
    {
      value = std::numeric_limits<float>::infinity();     
    }

    return (true);
  }
  
  std::istringstream iss(data);
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }  
}

  
bool from_string(const std::string &str, int &value)
{
  std::string dstr = str+ "\0";
  char *eptr = 0;
  value = static_cast<int>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, unsigned int &value)
{
  std::string dstr = str+ "\0";
  char *eptr = 0;
  value = static_cast<unsigned int>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, long &value)
{
  std::string dstr = str+ "\0";
  char *eptr = 0;
  value = static_cast<long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, unsigned long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<unsigned long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, long long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;  
  value = static_cast<long long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, unsigned long long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<unsigned long long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}


// Strip out space at the start and at the end of the string
void
strip_spaces(std::string& str)
{
  size_t esize = str.size();
  size_t idx = 0;
  
  // Strip out spaces at the start of the str
  while((idx <esize)&&((str[idx] == ' ') ||(str[idx] == '\t')||
        (str[idx] == '\n') || (str[idx] == '\r')||
        (str[idx] == '\f') || (str[idx] == '\v'))) idx++;
    
  // Get the substring without spaces at the start or at the end
  str = str.substr(idx,(str.size()-idx));
}


// Strip out space at the start and at the end of the string
void
strip_surrounding_spaces(std::string& str)
{
  size_t esize = str.size();
  size_t idx = 0;
  
  // Strip out spaces at the start of the str
  while((idx <esize)&&((str[idx] == ' ') ||(str[idx] == '\t')||
        (str[idx] == '\n') || (str[idx] == '\r')||
        (str[idx] == '\f') || (str[idx] == '\v'))) idx++;
  
  size_t ridx = 0;
  if (str.size()) ridx = str.size()-1;
  
  // Strip out spaces at the end of the str
  while((ridx > 0)&&((str[ridx] == ' ') ||(str[ridx] == '\t')||
        (str[ridx] == '\n') || (str[ridx] == '\r')||
        (str[ridx] == '\f') || (str[ridx] == '\v'))) ridx--;
  
  // Get the substring without spaces at the start or at the end
  str = str.substr(idx,(ridx-idx+1));
}

} // End namespace Utils

