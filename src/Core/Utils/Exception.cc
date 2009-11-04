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

#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>

namespace Core {

Exception::Exception(std::string message, 
              unsigned int line, 
              const char* file)
{
  message_ = std::string("[")+file+std::string(":")+to_string(line)+
             std::string("] ERROR: ")+message;
}

Exception::~Exception()
{
}

std::string
Exception::what() const
{
  return std::string("Exception");
}

std::string
Exception::message() const
{
  return message_;
}


LogicError::LogicError(std::string message, 
              unsigned int line, 
              const char* file) :
  Exception(message,line,file)
{
}

LogicError::~LogicError()
{
}

std::string
LogicError::what() const
{
  return std::string("LogicError");
}


RunTimeError::RunTimeError(std::string message, 
              unsigned int line, 
              const char* file) :
  Exception(message,line,file)
{
}

RunTimeError::~RunTimeError()
{
}

std::string
RunTimeError::what() const
{
  return std::string("RunTimeError");
}


InvalidArgument::InvalidArgument(std::string message, 
              unsigned int line, 
              const char* file) :
  LogicError(message,line,file)
{
}

InvalidArgument::~InvalidArgument()
{
}

std::string
InvalidArgument::what() const
{
  return std::string("InvalidArgument");
}


LengthError::LengthError(std::string message, 
              unsigned int line, 
              const char* file) :
  LogicError(message,line,file)
{
}

LengthError::~LengthError()
{
}

std::string
LengthError::what() const
{
  return std::string("LengthError");
}


OutOfRange::OutOfRange(std::string message, 
              unsigned int line, 
              const char* file) :
  LogicError(message,line,file)
{
}

OutOfRange::~OutOfRange()
{
}

std::string
OutOfRange::what() const
{
  return std::string("OutOfRange");
}

} // end namespace
