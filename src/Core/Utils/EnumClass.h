/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef CORE_UTILS_ENUMCLASS_H
#define CORE_UTILS_ENUMCLASS_H

namespace Core
{

#define CORE_ENUM_CLASS(CLASS_NAME, ...)\
class CLASS_NAME \
{\
public:\
  enum enum_type { __VA_ARGS__ };\
  \
  CLASS_NAME(enum_type type) : type_(type) {}\
  CLASS_NAME(const CLASS_NAME &copy) : type_(copy.type_) {}\
  \
  CLASS_NAME& operator=(enum_type type)\
  {\
    this->type_ = type;\
    return *this;\
  }\
  CLASS_NAME& operator=(const CLASS_NAME& copy)\
  {\
    this->type_ = copy.type_;\
    return *this;\
  }\
  \
  bool operator==(const CLASS_NAME& rhs) const\
  {\
    return this->type_ == rhs.type_;\
  }\
  bool operator!=(const CLASS_NAME& rhs) const\
  {\
    return this->type_ != rhs.type_;\
  }\
  bool operator==(enum_type type) const\
  {\
    return this->type_ == type;\
  }\
  bool operator!=(enum_type type) const\
  {\
    return this->type_ != type;\
  }\
  \
  operator int() const { return this->type_; }\
private:\
  enum_type type_;\
};

} // end namespace Core

#endif
