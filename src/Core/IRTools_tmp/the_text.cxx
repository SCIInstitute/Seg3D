/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
#include <Core/IRTools_tmp/the_text.hxx>

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
// the_text_t::replace
//        replaces all occurances of one character with another.
void 
the_text_t::replace(const char find, const char replace)
{
  char * cursor = text_;
  while (*cursor != '\0')
  {
    if ( *cursor == find )
      *cursor = replace;
    cursor++;
  }
}

//----------------------------------------------------------------
// the_text_t::toShort
// 
short int
the_text_t::toShort(bool * ok, int base) const
{
  long int num = toULong(ok, base);
  if (ok != NULL) *ok &= ((num >= SHRT_MIN) && (num <= SHRT_MAX));
  return (short int)(num);
}

//----------------------------------------------------------------
// the_text_t::toUShort
// 
unsigned short int
the_text_t::toUShort(bool * ok, int base) const
{
  unsigned long int num = toULong(ok, base);
  if (ok != NULL) *ok &= (num <= USHRT_MAX);
  return (unsigned short int)(num);
}

//----------------------------------------------------------------
// the_text_t::toInt
// 
int
the_text_t::toInt(bool * ok, int base) const
{
  long int num = toULong(ok, base);
  if (ok != NULL) *ok &= ((num >= INT_MIN) && (num <= INT_MAX));
  return int(num);
}

//----------------------------------------------------------------
// the_text_t::toUInt
// 
unsigned int
the_text_t::toUInt(bool * ok, int base) const
{
  unsigned long int num = toULong(ok, base);
  if (ok != NULL) *ok &= (num <= UINT_MAX);
  return (unsigned int)(num);
}

//----------------------------------------------------------------
// the_text_t::toLong
// 
long int
the_text_t::toLong(bool * ok, int base) const
{
  char * endptr = NULL;
  long int num = strtol(text_, &endptr, base);
  if (ok != NULL) *ok = !(text_ == endptr || errno == ERANGE);
  return num;
}

//----------------------------------------------------------------
// the_text_t::toULong
// 
unsigned long int
the_text_t::toULong(bool * ok, int base) const
{
  char * endptr = NULL;
  unsigned long int num = strtoul(text_, &endptr, base);
  if (ok != NULL) *ok = !(text_ == endptr || errno == ERANGE);
  return num;
}

//----------------------------------------------------------------
// the_text_t::toFloat
// 
float
the_text_t::toFloat(bool * ok) const
{
  double num = toDouble(ok);
  return float(num);
}

//----------------------------------------------------------------
// the_text_t::toDouble
// 
double
the_text_t::toDouble(bool * ok) const
{
  char * endptr = NULL;
  double num = strtod(text_, &endptr);
  if (ok != NULL) *ok = !(text_ == endptr || errno == ERANGE);
  return num;
}

//----------------------------------------------------------------
// the_text_t::to_ascii
// 
void
the_text_t::to_ascii()
{
  for (size_t i = 0; i < size_; i++)
  {
    text_[i] = ((size_t)(text_[i]) < 128) ? text_[i] : '?';
  }
}

//----------------------------------------------------------------
// the_text_t::to_lower
// 
void
the_text_t::to_lower()
{
  for (size_t i = 0; i < size_; i++)
  {
    text_[i] = tolower(text_[i]);
  }
}

//----------------------------------------------------------------
// the_text_t::to_upper
// 
void
the_text_t::to_upper()
{
  for (size_t i = 0; i < size_; i++)
  {
    text_[i] = toupper(text_[i]);
  }
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

//----------------------------------------------------------------
// the_text_t::match_head
// 
bool
the_text_t::match_head(const the_text_t & t, bool ignore_case) const
{
  if (t.size_ > size_) return false;
  return match_text(t, 0, ignore_case);
}

//----------------------------------------------------------------
// the_text_t::match_tail
// 
bool
the_text_t::match_tail(const the_text_t & t, bool ignore_case) const
{
  if (t.size_ > size_) return false;
  return match_text(t, size_ - t.size_, ignore_case);
}

//----------------------------------------------------------------
// the_text_t::match_text
// 
bool
the_text_t::match_text(const the_text_t & t,
		       const size_t & start,
		       bool ignore_case) const
{
  size_t end = start + t.size_;
  if (end > size_) return false;
  
  for (size_t i = start; i < end; i++)
  {
    char a = text_[i];
    char b = t.text_[i - start];
    
    if (ignore_case)
    {
      a = tolower(a);
      b = tolower(b);
    }
    
    if (a != b) return false;
  }
  
  return true;
}

//----------------------------------------------------------------
// the_text_t::simplify_ws
// 
the_text_t
the_text_t::simplify_ws() const
{
  // find the first non-whitespace character:
  int start = 0;
  for (; start < int(size_) && isspace(text_[start]); start++)
  {
    // skipping whitespace...
  }
  
  // NOTE: an all-whitespace string will simplify to an empty string:
  if (start == int(size_))
  {
    return the_text_t("");
  }
  
  // find the last non-whitespace character:
  int finish = int(size_) - 1;
  for (; finish >= start && isspace(text_[finish]); finish--);
  
  // intermediate storage:
  the_text_t tmp;
  tmp.fill('\0', size_t((finish + 1) - start));
  
  size_t j = 0;
  bool prev_ws = false;
  for (int i = start; i <= finish; i++)
  {
    char c = isspace(text_[i]) ? ' ' : text_[i];
    if (c == ' ' && prev_ws) continue;
    
    prev_ws = (c == ' ');
    tmp.text_[j] = c;
    j++;
  }
  
  the_text_t out(tmp.text(), j);
  return out;
}

//----------------------------------------------------------------
// the_text_t::split
// 
size_t
the_text_t::split(std::vector<the_text_t> & tokens,
		  const char & separator,
		  const bool & empty_ok) const
{
  if (size_ == 0)
  {
    tokens.resize(0);
    return 0;
  }
  
  // find the separators:
  typedef std::list<size_t> list_t;
  list_t separators;
  for (size_t i = 0; i < size_; i++)
  {
    if (text_[i] != separator) continue;
    separators.push_back(i);
  }
  separators.push_back(size_);
  
  std::list<the_text_t> tmp;
  
  typedef std::list<size_t>::iterator iter_t;
  size_t a = 0;
  for (iter_t i = separators.begin(); i != separators.end(); ++i)
  {
    size_t b = *i;
    
    if (b - a == 0 && empty_ok)
    {
      tmp.push_back(the_text_t(""));
    }
    else if (b - a > 0)
    {
      tmp.push_back(the_text_t(&text_[a], b - a));
    }
    
    a = b + 1;
  }
  
  tokens.resize(tmp.size());
  a = 0;
  for (std::list<the_text_t>::iterator i = tmp.begin(); i != tmp.end(); ++i)
  {
    tokens[a] = *i;
    a++;
  }
  
  return tmp.size();
}

//----------------------------------------------------------------
// the_text_t::splitAt
//        Splits the string around the n'th occurance of split_char
//        if n > size_, it is split around the last occurance.
std::vector<the_text_t> the_text_t::splitAt(const char split_char, 
                                            unsigned int n) const
{
  char * cursor = text_;

  // Find the desired character.
  unsigned int position = 0;
  unsigned int found_pos = std::numeric_limits<unsigned int>::max(); 
  while (*cursor != '\0')
  {
    if ( *cursor == split_char )
    {
      n--;
      found_pos = position;
      if ( n == 0 )
        break;
    }
    cursor++;
    position++;
  }

  std::vector<the_text_t> str_parts;
  str_parts.resize(2);
  if ( found_pos != std::numeric_limits<unsigned int>::max() )
  {
    // Split point found, do split.
    the_text_t left(&text_[0], found_pos);
    the_text_t right(&text_[found_pos + 1], size_ - found_pos - 1);

    str_parts[0] = left;
    str_parts[1] = right;
  }
  else
  {
    // No split point found.
    str_parts[0] = "";
    str_parts[1] = the_text_t( text_ );
  }
  
  return str_parts;
}

//----------------------------------------------------------------
// the_text_t::contains
// 
size_t
the_text_t::contains(const char & symbol) const
{
  size_t count = 0;
  for (size_t i = 0; i < size_; i++)
  {
    if (text_[i] != symbol) continue;
    count++;
  }
  
  return count;
}


//----------------------------------------------------------------
// operator <<
// 
std::ostream &
operator << (std::ostream & out, const the_text_t & text)
{
  std::string tmp(text.text(), text.size());
  out << tmp;
  return out;
}

//----------------------------------------------------------------
// operator >>
// 
std::istream &
operator >> (std::istream & in, the_text_t & text)
{
  std::string tmp;
  in >> tmp;
  text.assign(tmp.data(), tmp.size());
  return in;
}

//----------------------------------------------------------------
// getline
// 
std::istream &
getline(std::istream & in, the_text_t & text)
{
  std::string tmp;
  getline(in, tmp);
  if (!tmp.empty())
  {
    std::size_t len = tmp.size();
    if (tmp[len - 1] == '\r')
    {
      // truncate the \r character
      tmp.resize(len - 1);
    }
  }
  
  text.assign(tmp.data(), tmp.size());
  return in;
}

//----------------------------------------------------------------
// to_binary
// 
the_text_t
to_binary(const unsigned char & byte, bool lsb_first)
{
  the_text_t str;
  str.fill('0', 8);
  
  unsigned char mask = 1;
  if (lsb_first)
  {
    for (int i = 0; i < 8; i++, mask *= 2)
    {
      str[i] = (byte & mask) ? '1' : '0';
    }
  }
  else
  {
    for (int i = 7; i > -1; i--, mask *= 2)
    {
      str[i] = (byte & mask) ? '1' : '0';
    }
  }
  
  return str;
}
