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

// File         : the_utils.cxx
// Author       : Pavel A. Koshevoy
// Created      : Tue Nov  4 20:32:04 MST 2008
// Copyright    : (C) 2004-2008 University of Utah
// Description  : utility functions

// system includes:
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

// system includes:
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wchar.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <stdio.h>

// local includes:
#include <Core/ITKCommon/the_utils.hxx>


//----------------------------------------------------------------
// sleep_msec
// 
void
sleep_msec(size_t msec)
{
#ifdef WIN32
  Sleep((DWORD)(msec));
#else
  usleep(msec * 1000);
#endif
}

//----------------------------------------------------------------
// restore_console_stdio
// 
bool
restore_console_stdio()
{
#ifdef _WIN32
  AllocConsole();
  
#pragma warning(push)
#pragma warning(disable: 4996)
  
  freopen("conin$", "r", stdin);
  freopen("conout$", "w", stdout);
  freopen("conout$", "w", stderr);
  
#pragma warning(pop)
  
  HANDLE std_out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (std_out_handle == INVALID_HANDLE_VALUE)
  {
    return false;
  }
  
  COORD console_buffer_size;
  console_buffer_size.X = 80;
  console_buffer_size.Y = 9999;
  SetConsoleScreenBufferSize(std_out_handle,
                             console_buffer_size);
#endif
  
  return true;
}


namespace the
{
#ifdef _WIN32
//----------------------------------------------------------------
// utf8_to_utf16
// 
static void
utf8_to_utf16(const char * utf8, wchar_t *& utf16)
{
  int wcs_size =
  MultiByteToWideChar(CP_UTF8, // encoding (ansi, utf, etc...)
                      0,     // flags (precomposed, composite,... )
                      utf8,    // source multi-byte character string
                      -1,    // number of bytes in the source string
                      NULL,    // wide-character destination
                      0);    // destination buffer size
  
  utf16 = new wchar_t[wcs_size + 1];
  MultiByteToWideChar(CP_UTF8,
                      0,
                      utf8,
                      -1,
                      utf16,
                      wcs_size);
}
#endif
  
//----------------------------------------------------------------
// open_utf8
// 
int
open_utf8(const char * filename_utf8, int oflag, int pmode)
{
  int fd = -1;
  
#ifdef _WIN32
  // on windows utf-8 has to be converted to utf-16
  wchar_t * filename_utf16 = 0;
  utf8_to_utf16(filename_utf8, filename_utf16);
  
  int sflag = _SH_DENYNO;
  _wsopen_s(&fd, filename_utf16, oflag, sflag, pmode);
  delete [] filename_utf16;
  
#else
  // assume utf-8 is supported natively:
  fd = open(filename_utf8, oflag, pmode);
#endif
  
  return fd;
}

//----------------------------------------------------------------
// open_utf8
// 
void
open_utf8(std::fstream & fstream_to_open,
          const char * filename_utf8,
          std::ios_base::openmode mode)
{
#ifdef _WIN32
  // on windows utf-8 has to be converted to utf-16
  wchar_t * filename_utf16 = 0;
  utf8_to_utf16(filename_utf8, filename_utf16);
  
  fstream_to_open.open(filename_utf16, mode);
  delete [] filename_utf16;
  
#else
  // assume utf-8 is supported natively:
  fstream_to_open.open(filename_utf8, mode);
#endif
}

//----------------------------------------------------------------
// fopen_utf8
// 
FILE *
fopen_utf8(const char * filename_utf8, const char * mode)
{
  FILE * file = NULL;
  
#ifdef _WIN32
  wchar_t * filename_utf16 = NULL;
  utf8_to_utf16(filename_utf8, filename_utf16);
  
  wchar_t * mode_utf16 = NULL;
  utf8_to_utf16(mode, mode_utf16);
  
  _wfopen_s(&file, filename_utf16, mode_utf16);
  delete [] filename_utf16;
  delete [] mode_utf16;
#else
  file = fopen(filename_utf8, mode);
#endif
  
  return file;
}

//----------------------------------------------------------------
// rename_utf8
// 
int
rename_utf8(const char * old_utf8, const char * new_utf8)
{
#ifdef _WIN32
  wchar_t * old_utf16 = NULL;
  utf8_to_utf16(old_utf8, old_utf16);
  
  wchar_t * new_utf16 = NULL;
  utf8_to_utf16(new_utf8, new_utf16);
  
  int ret = _wrename(old_utf16, new_utf16);
  
  delete [] old_utf16;
  delete [] new_utf16;
#else
  
  int ret = rename(old_utf8, new_utf8);
#endif
  
  return ret;
}

//----------------------------------------------------------------
// remove_utf8
// 
int
remove_utf8(const char * filename_utf8)
{
#ifdef _WIN32
  wchar_t * filename_utf16 = NULL;
  utf8_to_utf16(filename_utf8, filename_utf16);
  
  int ret = _wremove(filename_utf16);
  delete [] filename_utf16;
#else
  
  int ret = remove(filename_utf8);
#endif
  
  return ret;
}

//----------------------------------------------------------------
// rmdir_utf8
// 
int
rmdir_utf8(const char * dir_utf8)
{
#ifdef _WIN32
  wchar_t * dir_utf16 = NULL;
  utf8_to_utf16(dir_utf8, dir_utf16);
  
  int ret = _wrmdir(dir_utf16);
  delete [] dir_utf16;
#else
  
  int ret = remove(dir_utf8);
#endif
  
  return ret;
}

//----------------------------------------------------------------
// mkdir_utf8
// 
int
mkdir_utf8(const char * path_utf8)
{
#ifdef _WIN32
  wchar_t * path_utf16 = NULL;
  utf8_to_utf16(path_utf8, path_utf16);
  
  int ret = _wmkdir(path_utf16);
  delete [] path_utf16;
#else
  
  int ret = mkdir(path_utf8, S_IRWXU);
#endif
  
  return ret;
}

//----------------------------------------------------------------
// fseek64
// 
int
fseek64(FILE * file, off_t offset, int whence)
{
#ifdef _WIN32
  int ret = _fseeki64(file, offset, whence);
#else
  int ret = fseeko(file, offset, whence);
#endif
  
  return ret;
}

//----------------------------------------------------------------
// ftell64
// 
off_t
ftell64(const FILE * file)
{
#ifdef _WIN32
  off_t pos = _ftelli64(const_cast<FILE *>(file));
#else
  off_t pos = ftello(const_cast<FILE *>(file));
#endif
  
  return pos;
}

}
