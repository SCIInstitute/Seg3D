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

// rename text formatter

// File         : the_text.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Aug 29 14:53:00 MDT 2004
// Copyright    : (C) 2004-2008 University of Utah
// Description  : text convenience class.

#ifndef THE_TEXT_HXX_
#define THE_TEXT_HXX_

// system includes:
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <list>

#ifdef _WIN32
#ifndef snprintf
#define snprintf _snprintf_s
#endif
#endif


//----------------------------------------------------------------
// the_text_t
// 
class the_text_t
{
public:
  the_text_t(const char * text = "");
  the_text_t(const char * text, const size_t & size);
  the_text_t(const the_text_t & text);
  the_text_t(const std::list<char> & text);
  ~the_text_t();
  
  // assignment operator:
  inline the_text_t & operator = (const the_text_t & text)
  {
    if (this != &text)
    {
      assign(text.text_, text.size_);
    }
    
    return *this;
  }
  
  // clear the string:
  inline void clear()
  {
    delete [] text_;
    text_ = NULL;
    size_ = 0;
  }
  
  // shorthand:
  inline bool is_empty() const
  { return size_ == 0; }
  
  // assign a new string to this text:
  inline void assign(const char * text)
  { assign(text, strlen(text)); }
  
  void assign(const char * text, const size_t & text_size);
  
  // append a new string to this text:
  inline void append(const char * text)
  { append(text, strlen(text)); }
  
  void append(const char * text, const size_t & text_size);
  
  inline bool operator < (const the_text_t & text) const
  { return (strcmp(text_, text.text_) < 0); }
  
  // arithmetic:
  inline the_text_t & operator += (const the_text_t & text)
  {
    append(text.text_, text.size_);
    return *this;
  }
  
  inline the_text_t operator + (const the_text_t & text) const
  {
    the_text_t text_sum(*this);
    text_sum += text;
    return text_sum;
  }

  // accessors:
  inline const char * text() const
  { return text_; }
  
  inline const size_t & size() const
  { return size_; }
  
  // conversion operator:
  inline operator const char * () const
  { return text_; }
  

  inline static const char* pad(const char * str,
                                const size_t width = 0,
                                const char pad_char = ' ',
                                const bool pad_left = true)
  {
    the_text_t txt(str);
    
    if (width > txt.size())
    {
      the_text_t padding;
      padding.fill(pad_char, width - txt.size());
      txt = pad_left ? padding + txt : txt + padding;
    }
    
    return txt;
  }
  
  
  // helpers:
  template <class number_t>
  static const char* number(const number_t & number,
                           const size_t width = 0,
                           const char pad_char = ' ',
                           const bool pad_left = true)
  {
    std::ostringstream os;
    os << number;
    
    std::string str = os.str();
    return pad(str.c_str(), width, pad_char, pad_left);
  }
  
  // needed because C++ stream operators don't convert
  // unsigned char
  inline static const char* number(const size_t & number,
                                  const size_t width = 0,
                                  const char pad_char = ' ',
                                  const bool pad_left = true)
  {
//#ifdef _WIN32
//#ifndef snprintf
//#define snprintf _snprintf_s
//#endif
//#endif
    
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%llu", (long long unsigned int)(number));
    return pad(buffer, width, pad_char, pad_left);
  }

  void fill(const char & c, const size_t size);
  
  void fill(const char & c)
  { fill(c, size_); }
  
private:
  // the text itself:
  char * text_;
  
  // the length of the text:
  size_t size_;
};

#endif // THE_TEXT_HXX_
