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

// File         : IRConnection.xx
// Author       : Joel Spaltenstein
// Created      : 2008/02/19
// Copyright    : (C) 2008 University of Utah
// Description  : 

#ifndef __IR_CONNECTION_HXX__
#define __IR_CONNECTION_HXX__

#include <vector>

#include <Core/ITKCommon/common.hxx>

class IRTransform;

class IRConnection
{
public:
  IRConnection(IRTransform* fristTransform, 
               IRTransform* secondTransform,
               bool verbose = false);
  ~IRConnection();
  
  vec2d_t tensionOnTransformation(IRTransform* transform);
  vec2d_t idealTransormationOffset();
  
  IRTransform* firstTransformation() {return _firstTransformation;}
  IRTransform* secondTransformation() {return _secondTransformation;}
  
  int findIdealOffset(const double maxOffset[], 
                      bool  maxOffsetIsPercent,
                      const double blackMaskPercent[],
                      bool  doClahe);
  
private:
  // TODO: logging?
  void printMessage( const char *str, bool verbose ) { if (verbose) printf("building an IRConnection between \n"); }
  
private:
  IRTransform* _firstTransformation;
  IRTransform* _secondTransformation;
  
  vec2d_t _idealTransormationOffset;
  double  _matchValue;
  bool    _verbose;
};

#endif


