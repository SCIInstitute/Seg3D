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

// File         : the_mutex_interface.cxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Feb 21 08:22:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : An abstract mutex class interface.

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_mutex_interface.hxx>

// system includes:
#include <stddef.h>


//----------------------------------------------------------------
// the_mutex_interface_t::creator_
// 
the_mutex_interface_t::creator_t
the_mutex_interface_t::creator_ = NULL;

//----------------------------------------------------------------
// the_mutex_interface_t::~the_mutex_interface_t
// 
the_mutex_interface_t::~the_mutex_interface_t()
{}

//----------------------------------------------------------------
// the_mutex_interface_t::set_creator
// 
void
the_mutex_interface_t::set_creator(the_mutex_interface_t::creator_t creator)
{
  creator_ = creator;
}

//----------------------------------------------------------------
// the_mutex_interface_t::create
// 
the_mutex_interface_t *
the_mutex_interface_t::create()
{
  if (creator_ == NULL) return NULL;
  return creator_();
}
