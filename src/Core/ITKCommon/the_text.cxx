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

// File         : the_text.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Aug 29 14:53:00 MDT 2004
// Copyright    : (C) 2004-2008 University of Utah
// Description  : ASCII text convenience class.

// local includes:
#include <Core/ITKCommon/the_text.hxx>

// system includes:
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <iostream>
#include <list>
#include <limits>


//----------------------------------------------------------------
// the_text_t::the_text_t
// 
the_text_t::the_text_t(const char * text):
  text_(NULL),
  size_(0)
{
  assign(text);
}

//----------------------------------------------------------------
// the_text_t::the_text_t
// 
the_text_t::the_text_t(const char * text, const size_t & size):
  text_(NULL),
  size_(0)
{
  assign(text, size);
}

//----------------------------------------------------------------
// the_text_t::the_text_t
// 
the_text_t::the_text_t(const the_text_t & text):
  text_(NULL),
  size_(0)
{
  assign(text.text_, text.size_);
}

//----------------------------------------------------------------
// the_text_t::the_text_t
// 
the_text_t::the_text_t(const std::list<char> & text):
  text_(NULL),
  size_(text.size())
{
  text_ = new char [size_ + 1];
  size_t j = 0;
  for (std::list<char>::const_iterator i = text.begin();
       i != text.end(); ++i, ++j)
  {
    text_[j] = *i;
  }
}

//----------------------------------------------------------------
// the_text_t::~the_text_t
// 
the_text_t::~the_text_t()
{
  clear();
}

//----------------------------------------------------------------
// the_text_t::assign
// 
void
the_text_t::assign(const char * text, const size_t & text_size)
{
  char * text_copy = new char [text_size + 1];
  for (size_t i = 0; i < text_size; i++)
  {
    text_copy[i] = text[i];
  }
  
  text_copy[text_size] = '\0';
  
  delete [] text_;
  text_ = text_copy;
  size_ = text_size;
}

//----------------------------------------------------------------
// the_text_t::append
// 
void
the_text_t::append(const char * text, const size_t & text_size)
{
  char * text_copy = new char [size_ + text_size + 1];
  
  for (size_t i = 0; i < size_; i++)
  {
    text_copy[i] = text_[i];
  }
  
  for (size_t i = 0; i < text_size; i++)
  {
    text_copy[i + size_] = text[i];
  }
  
  text_copy[size_ + text_size] = '\0';
  
  delete [] text_;
  text_ = text_copy;
  size_ += text_size;
}

//----------------------------------------------------------------
// the_text_t::fill
// 
void
the_text_t::fill(const char & c, const size_t size)
{
  char * text = new char [size + 1];
  for (size_t i = 0; i < size; i++)
  {
    text[i] = c;
  }
  text[size] = '\0';
  
  delete [] text_;
  text_ = text;
  size_ = size;
}
