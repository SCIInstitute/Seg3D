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

#ifndef MRC2000IO_MRCUTIL_H
#define MRC2000IO_MRCUTIL_H 1

#include <string>

#include "mrcheader.h"

namespace MRC2000IO {

class MRCUtil {
public:
  MRCUtil();
  ~MRCUtil();

  bool read_header(const std::string& filename, MRCHeader& header);
  std::string get_error() { return error_; }
  bool use_new_origin() { return use_new_origin_; }
  bool swap_endian() { return swap_endian_; }
  std::string export_header(const MRCHeader& header);
  bool import_header(const std::string& header_string, MRCHeader& header);
  

private:
  bool process_header(void* header, int buffer_len);
  std::string error_;

  bool host_is_big_endian_;
  bool swap_endian_;
  bool use_new_origin_;

  const int MASK_UPPER_;
  const int MASK_LOWER_;
  const std::string DELIM;
  const std::string DELIM2;
};

}

#endif
