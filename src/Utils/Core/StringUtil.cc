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
  std::string dstr = str + "\0";
  strip_spaces(dstr);
  
  // if empty just return
  if (dstr.size() == 0) return (false);
  
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if (dstr.size() > 2 && 
      (dstr[0] == 'n' || dstr[0] == 'N') &&
      (dstr[1] == 'A' || dstr[1] == 'A') &&
      (dstr[2] == 'n' || dstr[2] == 'N'))      
  {
    value = std::numeric_limits<double>::quiet_NaN(); 
    return (true);
  }
  // handle inf
  else if (dstr.size() > 2 && 
      (dstr[0] == 'i' || dstr[0] == 'I') &&
      (dstr[1] == 'n' || dstr[1] == 'N') &&
      (dstr[2] == 'f' || dstr[2] == 'F'))   
  {
    value = std::numeric_limits<double>::infinity(); 
    return (true);
  }
  // handle +inf and -inf
  else if (dstr.size() > 3 && 
      (dstr[0] == '-' || dstr[0] == '+') &&
      (dstr[1] == 'i' || dstr[1] == 'I') &&
      (dstr[2] == 'n' || dstr[2] == 'N') &&
      (dstr[3] == 'f' || dstr[3] == 'F'))
  {
    if (dstr[0] == '-')
    {
      value = -std::numeric_limits<double>::infinity(); 
    }
    else
    {
      value = std::numeric_limits<double>::infinity();     
    }

    return (true);
  }
  
  // Default conversion
  char* eptr = 0;
  value = strtod(&(dstr[0]),&eptr);
  if (eptr == &(dstr[0])) return (false);

  return (true);
}

bool from_string(const std::string &str, float &value)
{
  std::string dstr = str + "\0";
  strip_spaces(dstr);
  
  // if empty just return
  if (dstr.size() == 0) return (false);
  
  // Handle special cases: nan, inf, and -inf
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if (dstr.size() > 2 && 
      (dstr[0] == 'n' || dstr[0] == 'N') &&
      (dstr[1] == 'A' || dstr[1] == 'A') &&
      (dstr[2] == 'n' || dstr[2] == 'N'))      
  {
    value = std::numeric_limits<float>::quiet_NaN(); 
    return (true);
  }
  // handle inf
  else if (dstr.size() > 2 && 
      (dstr[0] == 'i' || dstr[0] == 'I') &&
      (dstr[1] == 'n' || dstr[1] == 'N') &&
      (dstr[2] == 'f' || dstr[2] == 'F'))   
  {
    value = std::numeric_limits<float>::infinity(); 
    return (true);
  }
  // handle +inf and -inf
  else if (dstr.size() > 3 && 
      (dstr[0] == '-' || dstr[0] == '+') &&
      (dstr[1] == 'i' || dstr[1] == 'I') &&
      (dstr[2] == 'n' || dstr[2] == 'N') &&
      (dstr[3] == 'f' || dstr[3] == 'F'))
  {
    if (dstr[0] == '-')
    {
      value = -std::numeric_limits<float>::infinity(); 
    }
    else
    {
      value = std::numeric_limits<float>::infinity();     
    }

    return (true);
  }
  
  // Handle normal numbers
  char *eptr = 0;
  double temp_value = strtod(&(dstr[0]),&eptr);

  value = static_cast<float>(temp_value);
  if (eptr == &(dstr[0])) return (false);
  
  return (true);
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

