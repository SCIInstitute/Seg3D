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

#ifndef CORE_ACTION_ACTIONPARAMETER_H
#define CORE_ACTION_ACTIONPARAMETER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL
#include <string>

// Core
#include <Core/Utils/StringUtil.h>

namespace Core
{

// ACTIONPARAMETERBASE:
// Base class needed for uniform access to import and export the value
// in a uniform way.

class ActionParameterBase
{
  // -- destructor --
public:
  virtual ~ActionParameterBase();

  // -- functions for accessing data --
public:
  // IMPORT_FROM_STRING
  // Import a parameter from a string. The function returns true
  // if the import succeeded
  virtual bool import_from_string( const std::string& str ) = 0;

  // EXPORT_TO_STRING
  // Export the contents of the parameter to string
  virtual std::string export_to_string() const = 0;

  // HAS_EXTENSION
  // Has exented information in the derived class
  virtual bool has_extension() const = 0;
};

// ACTIONPARAMETER:
// Parameter for an action.

// Class definition:
template< class T >
class ActionParameter : public ActionParameterBase
{
  // -- constructor/destructor --
public:
  ActionParameter( T& parameter ) :
    parameter_( parameter )
  {
  }

  // -- access to value --
public:

  // IMPORT_FROM_STRING
  // import a parameter from a string. The function returns true
  // if the import succeeded
  virtual bool import_from_string( const std::string& str )
  {
    return ImportFromString( str, this->parameter_ );
  }
  
  // EXPORT_TO_STRING
  // export the contents of the parameter to string
  virtual std::string export_to_string() const
  {
    return ExportToString( this->parameter_ );
  }

  // HAS_EXTENSION
  // Has exented information in the derived class
  virtual bool has_extension() const
  {
    return false;
  }

protected:
  // The actual parameter (as a reference)
  T& parameter_;
};

} // namespace Core

#endif
