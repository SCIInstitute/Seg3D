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

#ifndef CORE_UTILS_EXCEPTION_H
#define CORE_UTILS_EXCEPTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

namespace Core {

// CLASS EXCEPTION:
// This exception model is modeled after the std::exception classes
// The base class is Exception and it has a variety of subclasses derived
// from it that mirror the std::exception classes.

// The difference with the std::exception classes is that macros are provided
// for these classes that allow for easy insertion of filename and line number
// of where the exception was generated.

// Hence it is recommended to use the macros for throwing the exceptions

class Exception {

  public:
    // Constructor/destructor
    Exception(std::string message, 
              unsigned int line, 
              const char* file);
              
    virtual ~Exception();
  
    // Description of the type of exception
    virtual std::string what()    const;

    // Description of what caused the exception
    std::string message() const;

  private:
    
    // Error message for debug purposes
    std::string   message_;
};


class LogicError : public Exception {

  public:
    // Constructor/destructor
    LogicError(std::string message, 
              unsigned int line, 
              const char* file);

    virtual ~LogicError();

    // Description of the type of exception
    virtual std::string what()    const;
};


class RunTimeError : public Exception {

  public:
    // Constructor/destructor
    RunTimeError(std::string message, 
              unsigned int line, 
              const char* file);

    virtual ~RunTimeError();
    
    // Description of the type of exception
    virtual std::string what()    const;
};

class InvalidArgument : public LogicError {

  public:
    // Constructor/destructor
    InvalidArgument(std::string message, 
              unsigned int line, 
              const char* file);

    virtual ~InvalidArgument();
    
    // Description of the type of exception
    virtual std::string what()    const;
};


class LengthError : public LogicError {

  public:
    // Constructor/destructor
    LengthError(std::string message, 
              unsigned int line, 
              const char* file);

    virtual ~LengthError();
    
    // Description of the type of exception
    virtual std::string what()    const;
};

class OutOfRange : public LogicError {

  public:
    // Constructor/destructor
    OutOfRange(std::string message, 
              unsigned int line, 
              const char* file);

    virtual ~OutOfRange();
    
    // Description of the type of exception
    virtual std::string what()    const;
};


// MACROS FOR AUTOMATICALLY INCLUDING LINE NUMBER AND FILE IN THE
// EXCEPTION THAT WAS THROWN

#define SCI_THROW_EXCEPTION(message)\
throw Core::Exception(message,__LINE__,__FILE__)

#define SCI_THROW_LOGICERROR(message)\
throw Core::LogicError(message,__LINE__,__FILE__)

#define SCI_THROW_RUNTIMEERROR(message)\
throw Core::RunTimeError(message,__LINE__,__FILE__)

#define SCI_THROW_INVALIDARGUMENT(message)\
throw Core::InvalidArgument(message,__LINE__,__FILE__)

#define SCI_THROW_LENGTHERROR(message)\
throw Core::LengthError(message,__LINE__,__FILE__)

#define SCI_THROW_OUTOFRANGE(message)\
throw Core::OutOfRange(message,__LINE__,__FILE__)

} // end namespace

#endif
