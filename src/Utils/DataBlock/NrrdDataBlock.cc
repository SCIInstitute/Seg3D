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


#include <teem/nrrd.h>
#include <Utils/DataBlock/NrrdDataBlock.h>

namespace Utils {

NrrdDataBlock::NrrdDataBlock(NrrdDataHandle nrrd_data) :
  nrrd_data_(nrrd_data)
{
  // Copy the information of the nrrd into the C++ class
  Nrrd* nrrd = nrrd_data->nrrd();

  unsigned int dim = nrrd->dim;
  if (dim > 0) set_nx(nrrd->axis[0].size); else set_nx(1);
  if (dim > 1) set_ny(nrrd->axis[1].size); else set_ny(1);
  if (dim > 2) set_nz(nrrd->axis[1].size); else set_nz(1);
  
  switch (nrrd->type)
  {
    case nrrdTypeChar: set_type(CHAR_E); break;
    case nrrdTypeUChar: set_type(UCHAR_E); break;
    case nrrdTypeShort: set_type(SHORT_E); break;
    case nrrdTypeUShort: set_type(USHORT_E); break;
    case nrrdTypeInt: set_type(INT_E); break;
    case nrrdTypeUInt: set_type(UINT_E); break;
    case nrrdTypeFloat: set_type(FLOAT_E); break;
    case nrrdTypeDouble: set_type(DOUBLE_E); break;
  }

  set_data(nrrd->data);
}   
    
NrrdDataBlock::~NrrdDataBlock()
{
}

} // end namespace Utils
