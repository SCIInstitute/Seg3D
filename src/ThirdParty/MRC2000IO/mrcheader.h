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
 * MRC file format reader: http://emdatabank.org/conventions.html
   and http://www2.mrc-lmb.cam.ac.uk/image2000.html
 * Also helpful: http://www.ccp4.ac.uk/html/maplib.html
 * Implementation follows EMAN2 and Chimera.
 */


#ifndef CORE_ALGORITHMS_DATAIO_MRCHEADER_H
#define CORE_ALGORITHMS_DATAIO_MRCHEADER_H 1


// MRC header size is 1024 bytes
#define MRC_HEADER_LENGTH 1024
// MRC header size is 256 long words
#define MRC_HEADER_LENGTH_LWORDS 256
// MRC format allows for 10 80 byte text labels
#define MRC_NUM_TEXT_LABELS 10
#define MRC_SIZE_TEXT_LABELS 80
#define MRC_LONG_WORD 4
#define MRC_SIZE_EXTRA 25

namespace MRC2000IO {

// MRC 2000 header
struct MRCHeader {
  int nx;				/* 1 Number of columns. */
  int ny;				/* 2 Number of rows. */
  int nz;				/* 3 Number of sections. */
  
  int mode;			/* 4 Data type :
                       0  image : signed 8-bit bytes range -128 to 127
                       1  image : 16-bit halfwords
                       2  image : 32-bit reals
                       3  transform : complex 16-bit integers (not supported)
                       4  transform : complex 32-bit reals (not supported) */
  
  int nxstart;		/* 5 Number of first column in map (default = 0). */
  int nystart;		/* 6 Number of first row in map. */
  int nzstart;		/* 7 Number of first section in map. */
  
  /* Grid size */
  int mx;				/* 8 Number of intervals along X. */
  int my;				/* 9 Number of intervals along Y. */
  int mz;				/* 10 Number of intervals along Z. */

  /* Cell: treat a whole 2D image as a cell */
  float xlen;			/* 11 Cell dimensions in angstroms. */
  float ylen;			/* 12 Cell dimensions in angstroms. */
  float zlen;			/* 13 Cell dimensions in angstroms. */

  /* Cell angles: only used by crystallographic maps */
  float alpha;		/* 14 Cell angles in degrees. */
  float beta;			/* 15 Cell angles in degrees. */
  float gamma;		/* 16 Cell angles in degrees. */
  
  int mapc;			/* 17 Axis corresponding to columns (X=1, Y=2 and Z=3). */
  int mapr;			/* 18 Axis corresponding to rows (X=1, Y=2 and Z=3). */
  int maps;			/* 19 Axis corresponding to sections (X=1, Y=2 and Z=3). */
  
  float dmin;			/* 20 Minimum density value. */
  float dmax;			/* 21 Maximum density value. */
  float dmean;		/* 22 Mean density value.    */
  
  int ispg;			    /* 23 Space group number 0 or 1 (default (images) = 0). */
  
  int nsymbt;		  	/* 24 Number of chars used for optionally storing symmetry
                          operators (crystallographic maps only). */
  
  int extra[MRC_SIZE_EXTRA];    /* 25-49 Extra space used for symmetry operators, or other unspecified (0 by default). */
  
  float xorigin;	/* 50 Origin in X, Y, Z used for transformations. */
  float yorigin;	/* 51 Origin in X, Y, Z used for transformations. */
  float zorigin;	/* 52 Origin in X, Y, Z used for transformations. */
  
  char map[MRC_LONG_WORD];		  /* 53 Character string "MAP " used to distinguish file type from older formats. */
  
  int machinestamp;	/* 54 Machine stamp in CCP4 convention:
                          big endian=0x00001111
                          little endian=0x44440000 or 0x00004444 or 0x00004144 or 0x11111111 & 4
                          There is an ambiguity in the specification - not reliably implemented.
                     
                          Machine stamp in MRC from
                          http://emdatabank.org/mapformat.html
                     
                          [10] MACHST is 0x44,0x41,0x00,0x00 (written/read as 4 hex byte sequence) for
                          little endian machines and 0x11,0x11,0x00,0x00 (written/read as 4 hex byte sequence) for
                          big endian machines.
                     */
  
  float rms;			/* 55 Rms deviation of map from mean density. */
  
  int nlabels;		/* 56 Number of labels being used. */
  char labels[MRC_NUM_TEXT_LABELS][MRC_SIZE_TEXT_LABELS]; /* 57-256 10 80-character text labels */
};

enum MRC_mode {
  // standard modes
  MRC_CHAR = 0,
  MRC_SHORT,
  MRC_FLOAT,
  MRC_SHORT_COMPLEX,
  MRC_FLOAT_COMPLEX,
};

const int LITTLE_ENDIAN_STAMP = 0x00004444;
const int BIG_ENDIAN_STAMP = 0x00001111;
  
}

#endif // CORE_ALGORITHMS_DATAIO_MRCHEADER_H

