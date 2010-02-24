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

#ifndef UTILS_DATABLOCK_NRRDDATABLOCK_H
#define UTILS_DATABLOCK_NRRDDATABLOCK_H

// Utils includes
#include <Utils/DataBlock/DataBlock.h>
#include <Utils/DataBlock/NrrdData.h>

namespace Utils {

// Forward Declaration
class NrrdDataBlock;
typedef boost::shared_ptr<NrrdDataBlock> NrrdDataBlockHandle;

// Class definition
class NrrdDataBlock : public DataBlock {

// -- Constructor/destructor --
  public:
    NrrdDataBlock(NrrdDataHandle nrrd_data);    
    virtual ~NrrdDataBlock();

// -- Information for retrieving nrrd --
  private:
    // Location where the original nrrd is stored
    NrrdDataHandle nrrd_data_;
};

} // end namespace Utils

#endif
