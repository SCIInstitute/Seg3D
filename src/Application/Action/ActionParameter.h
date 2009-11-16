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

#ifndef APPLICATION_ACTION_ACTIONPARAMETER_H
#define APPLICATION_ACTION_ACTIONPARAMETER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Converter/StringConverter.h>

namespace Seg3D {

// CLASS ACTIONPARAMETER:
// Auxilary class for adding parameters into an action.

// ACTIONPARAMETERBASE:
// Base class needed for uniform access to import and export the value
// in a uniform way.

class ActionParameterBase {

  public:
    virtual ~ActionParameterBase();
    
    virtual std::string export_to_string() const = 0;
    virtual bool import_from_string(const std::string& str) = 0;

};

// ACTIONPARAMETER:
// Parameter for an action.

template<class T>
class ActionParameter : public ActionParameterBase {

  public:
    ActionParameter(const T& default_value) :
      value_(default_value)
    {}
  
    virtual ~ActionParameter()
    {}

    // General access to the parameter value
    T& value() { return value_; } 

    // export the value to a string
    virtual std::string export_to_string() const
    {
      return Utils::export_to_string<T>(value_);
    }

    // import the value from a string
    virtual bool import_from_string(const std::string& str)
    {
      return Utils::import_from_string<T>(str,value_);
    }
        
  private:
    // The actual value
    T value_;
};

} // namespace Seg3D


#endif
