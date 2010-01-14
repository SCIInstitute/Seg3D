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


#include <Utils/Core/Environment.h>


namespace Seg3D {

//This map stores the parameters that were set when Seg3D was started.  
static std::map <std::string, std::string> parameters;

//This is a function to check parameters.  
//This avoids accidentally putting data into the map that we dont want
const char *
checkParameter( const std::string &key )
{
  if (parameters.find(key) == parameters.end()) {
    return 0;
  }
  else {
    return parameters[key].c_str();
  }
}
  
//This function sets parameters in the parameters map.
void
setParameter( const std::string &key, const std::string &val )
{
  parameters[key] = val;
}
  
  
}  // end namespace