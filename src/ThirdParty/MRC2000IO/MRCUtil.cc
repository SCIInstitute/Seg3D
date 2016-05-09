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

/*
 * MRC file format reader: http://www2.mrc-lmb.cam.ac.uk/image2000.html
 * Implementation follows EMAN2 and Chimera.
 */


#include "MRCUtil.h"

#include <algorithm>
#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <cstring>

namespace MRC2000IO {

bool isBigEndian()
{
  unsigned short i = 0x4321;
  if ( (*(reinterpret_cast<unsigned char *>(&i))) != 0x21 )
  {
    return true;
  } 
  else
  {
    return false;
  }
}

MRCUtil::MRCUtil()
  : host_is_big_endian_(isBigEndian()),
    swap_endian_(false),
    use_new_origin_(true),
    MASK_UPPER_(0xFF000000),
    MASK_LOWER_(0x000000FF),
    DELIM(" "),
    DELIM2("-**-")
{
}

MRCUtil::~MRCUtil() {}


bool MRCUtil::read_header(const std::string& filename, MRCHeader& header)
{
  try
  {
    std::ifstream::pos_type size = 0;
    
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if (! in )
    {
      error_ = std::string("Failed to open file ") + filename;
      return false;
    }
    
    in.seekg(0, std::ios::end);
    size = in.tellg();
    in.seekg(0, std::ios::beg);
    std::ifstream::pos_type data_block_size = size - static_cast<std::ifstream::pos_type>(MRC_HEADER_LENGTH);
    
    char* buffer = new char[MRC_HEADER_LENGTH];
    in.read(buffer, MRC_HEADER_LENGTH);    
    memcpy(&header, buffer, MRC_HEADER_LENGTH);
    delete [] buffer;
    
    process_header(&header, MRC_HEADER_LENGTH);
    
    if (header.mode == MRC_SHORT_COMPLEX || header.mode == MRC_FLOAT_COMPLEX)
    {
      error_ = "MRC complex mode is not supported.";
      return false;
    }
    
    if ((0 == header.xorigin || 0 == header.yorigin || 0 == header.zorigin) &&
        (0 != header.nxstart || 0 != header.nystart || 0 != header.nzstart))
    {
      // use n[x|y|z]start
      this->use_new_origin_ = false;
    }
    in.close();
  }
  // TODO: ios specific exceptions
  catch (...)
  {
    error_ = std::string("Failed to read header for file ") + filename;
    return false;
  }
  
  return true;
}

bool MRCUtil::process_header(void* header, int buffer_len)
{
  int* long_word_buffer = reinterpret_cast<int*>(header);
  const int machine_stamp = long_word_buffer[53];
  
  // file endianness vs. host endianness
  // N.B. machine stamp field is not always implemented reliably
  //
  // This code defaults to little endian.
  if (this->host_is_big_endian_)
  {
    // big endian host, data read on little endian machine
    if (( (machine_stamp & MASK_LOWER_) == 0x44 ) || ( (machine_stamp & MASK_UPPER_) == 0x44000000 ))
    {
      this->swap_endian_ = true;
    }
    // big endian host, data read on big endian machine
    else if ( (machine_stamp &  MASK_LOWER_) == 0x11)
    {
      this->swap_endian_ = false;
    }
    else
    {
      // Guess endianness of data by checking upper bytes of nx:
      // assumes nx < 2^16
      char* char_buffer = reinterpret_cast<char*>(header);
      this->swap_endian_ = true;
      for (size_t j = MRC_LONG_WORD / 2; j < MRC_LONG_WORD; ++j) {
        
        if (char_buffer[j] != 0) {
          this->swap_endian_ = false;
          break;
        }
      }
    }
  }
  else // little endian (no other architectures supported by Core/Endian)
  {
    // little endian host, data read on little endian machine
    if (( (machine_stamp & MASK_LOWER_) == 0x44 ) || ( (machine_stamp & MASK_UPPER_) == 0x44000000 ))
    {
      this->swap_endian_ = false;
    }
    // little endian host, data read on big endian machine
    else if ( (machine_stamp &  MASK_LOWER_) == 0x11)
    {
      this->swap_endian_ = true;
    }
    else
    {
      // Guess endianness of data by checking lower bytes of nx:
      // assumes nx < 2^16
      char* char_buffer = reinterpret_cast<char*>(header);
      this->swap_endian_ = true;
      for (size_t j = 0; j < MRC_LONG_WORD / 2; ++j) {
        if (char_buffer[j] != 0) {
          this->swap_endian_ = false;
          break;
        }
      }
    }
  }
  
  // convert header to little endian before reading
  if (this->swap_endian_)
  {
    // code from DataBlock.cc, SwapEndian method
    unsigned char* ubuffer = reinterpret_cast<unsigned char*>(long_word_buffer);
    unsigned char tmp;
    const size_t SIZE8 = MRC_HEADER_LENGTH_LWORDS & ~(0x7);
    
    size_t i = 0;
    // swap word bytes in blocks of 32 bytes in place
    for(; i < SIZE8; i += 8)
    {
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;	
      
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
    }
    
    for(; i < MRC_HEADER_LENGTH_LWORDS; ++i)
    {
      tmp = ubuffer[ 0 ]; ubuffer[ 0 ] = ubuffer[ 3 ]; ubuffer[ 3 ] = tmp;
      tmp = ubuffer[ 1 ]; ubuffer[ 1 ] = ubuffer[ 2 ]; ubuffer[ 2 ] = tmp;
      ubuffer += 4;
    }
  }
  
  MRCHeader *h = reinterpret_cast<MRCHeader*>(header);
  
  // Force a sentinal null char
  h->map[MRC_LONG_WORD-1] = '\0';
  
  for (int j = 0; j < MRC_NUM_TEXT_LABELS; ++j)
  {
    // Also terminate labels with null char
    h->labels[j][MRC_SIZE_TEXT_LABELS-1] = '\0';
  }
  
  // minor correction
  if (h->nlabels > MRC_NUM_TEXT_LABELS)
  {
    h->nlabels = MRC_NUM_TEXT_LABELS;
  }
  
  return true;
}

std::string MRCUtil::export_header(const MRCHeader& header)
{
  std::ostringstream oss;
  oss.setf( std::ios::floatfield, std::ios_base::fixed );
  oss.precision(12);
  oss << header.nx << DELIM << header.ny << DELIM << header.nz << DELIM
  << header.mode << DELIM
  << header.nxstart << DELIM << header.nystart << DELIM << header.nzstart << DELIM
  << header.mx << DELIM << header.my << DELIM << header.mz << DELIM
  << header.xlen << DELIM << header.ylen << DELIM << header.zlen << DELIM
  << header.alpha << DELIM << header.beta << DELIM << header.gamma << DELIM
  << header.mapc << DELIM << header.mapr << DELIM << header.maps << DELIM
  << header.dmin << DELIM << header.dmax << DELIM<< header.dmean << DELIM
  << header.ispg << DELIM
  << header.nsymbt << DELIM;
  for (int i = 0; i < MRC_SIZE_EXTRA; ++i)
  {
    oss << header.extra[i] << DELIM;
  }
  oss << header.xorigin << DELIM << header.yorigin << DELIM << header.zorigin << DELIM
  << header.map << DELIM;
  oss << std::hex << std::showbase << header.machinestamp;
  oss << std::dec << DELIM << header.rms << DELIM
  << header.nlabels << DELIM;
  for (int i = 0; i < MRC_NUM_TEXT_LABELS; ++i)
  {
    oss << DELIM2 << header.labels[i];
  }
  
  return oss.str();
}

bool MRCUtil::import_header(const std::string& header_string, MRCHeader& header)
{
  if (header_string.size() == 0)
    return false;

  std::istringstream iss(header_string);
  iss.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
  iss.setf( std::ios::floatfield, std::ios_base::fixed );
  iss.precision(12);
  try
  {
    iss >> header.nx;
    iss >> header.ny;
    iss >> header.nz;
    iss >> header.mode;
    iss >> header.nxstart;
    iss >> header.nystart;
    iss >> header.nzstart;
    iss >> header.mx;
    iss >> header.my;
    iss >> header.mz;
    iss >> header.xlen;
    iss >> header.ylen;
    iss >> header.zlen;
    iss >> header.alpha;
    iss >> header.beta;
    iss >> header.gamma;
    iss >> header.mapc;
    iss >> header.mapr;
    iss >> header.maps;
    iss >> header.dmin;
    iss >> header.dmax;
    iss >> header.dmean;
    iss >> header.ispg;
    iss >> header.nsymbt;
    for (int i = 0; i < MRC_SIZE_EXTRA; ++i)
    {
      iss >> header.extra[i];
    }
    iss >> header.xorigin;
    iss >> header.yorigin;
    iss >> header.zorigin;
    iss >> header.map;
    iss >> std::hex >> header.machinestamp;
    iss >> std::dec >> header.rms;
    iss >> header.nlabels;
    
    for (int i = 0; i < MRC_NUM_TEXT_LABELS; ++i)
    {
      char* begin = &(header.labels[i][0]);
      char* end = &(header.labels[i][0]) +
      (sizeof(header.labels[i][0])/sizeof(char)) * MRC_SIZE_TEXT_LABELS;
      std::fill(begin, end, 0);
    }
    
    std::string h = header_string;
    std::vector<std::string> labels_list;
    
    while ( true )
    {
      std::string::size_type pos = h.find( DELIM2 );
      if ( pos >= h.size() )
      {
        labels_list.push_back( h );
        break;
      }
      labels_list.push_back( h.substr( 0, pos ) );
      h = h.substr( pos + DELIM2.size() );
    }
    if (labels_list.size() > MRC_NUM_TEXT_LABELS)
    {
      std::ostringstream oss;
      oss << "Number of parsed labels (" << labels_list.size()
      << ") is greater than the allowed number of labels (" << MRC_NUM_TEXT_LABELS << ").";
      error_ = oss.str(); 
    }
    
    // discard first string, since it is the first part of the header
    for (int i = 1; i < labels_list.size(); ++i)
    {
      if (labels_list[i].size() == 0)
        continue;
      
      int index = i-1;
      strncpy(header.labels[index], labels_list[i].c_str(), MRC_SIZE_TEXT_LABELS);
      header.labels[index][MRC_SIZE_TEXT_LABELS-1] = '\0';
    }
    
    return true;
  }
  catch ( std::istringstream::failure e )
  {
    error_ = e.what();
    return false;
  }
  catch (...)
  {
    error_ = "Could not parse header";
    return false;
  }
}
  
}
