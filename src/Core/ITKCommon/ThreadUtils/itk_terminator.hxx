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

// File         : itk_terminator.hxx
// Author       : Pavel A. Koshevoy
// Created      : Sun Sep 24 18:16:00 MDT 2006
// Copyright    : (C) 2004-2008 University of Utah
// Description  : a ITK filter terminator convenience class

#ifndef ITK_TERMINATOR_HXX_
#define ITK_TERMINATOR_HXX_

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_terminator.hxx>

#ifdef USE_ITK_TERMINATORS
#define itk_terminator_t the_terminator_t


//----------------------------------------------------------------
// terminator_t
//
template <typename process_t>
class terminator_t : public the_terminator_t
{
public:
  terminator_t(process_t * proc):
    itk_terminator_t(proc->GetNameOfClass()),
    process_(proc)
  {}
  
  // virtual:
  void terminate()
  {
    process_->AbortGenerateDataOn();
    itk_terminator_t::terminate();
  }
  
private:
  typename process_t::Pointer process_;
};


#endif // USE_ITK_TERMINATORS
#endif // ITK_TERMINATOR_HXX_
