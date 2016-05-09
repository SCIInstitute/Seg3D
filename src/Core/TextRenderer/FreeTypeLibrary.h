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

#ifndef CORE_TEXTRENDERER_FREETYPELIBRARY_H
#define CORE_TEXTRENDERER_FREETYPELIBRARY_H

// boost includes
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Core include
#include <Core/Utils/Lockable.h>
#include <Core/Utils/Singleton.h>
#include <Core/TextRenderer/FreeTypeFace.h>

namespace Core
{

class FreeTypeLibrary;
class FreeTypeLibraryFactory;
typedef boost::shared_ptr< FreeTypeLibrary > FreeTypeLibraryHandle;

class FreeTypeLibrary : public boost::noncopyable
{
private:
  friend class FreeTypeLibraryFactory;
  FreeTypeLibrary( FT_Library library );

public:
  ~FreeTypeLibrary();

  FreeTypeFaceHandle new_face_from_file( const char* file_name, long index = 0 );
  FreeTypeFaceHandle new_face_frome_buffer( const unsigned char* buffer, 
    long size, long index = 0 );

private:
  FT_Library library_;
};

class FreeTypeLibraryFactory : private RecursiveLockable
{
  CORE_SINGLETON( FreeTypeLibraryFactory );

private:
  friend class FreeTypeLibrary;
  FreeTypeLibraryFactory();
  virtual ~FreeTypeLibraryFactory();

public:
  FreeTypeLibraryHandle create_library();

private:
  void destroy_library( FT_Library library );

public:
  static FreeTypeLibraryHandle CreateLibrary();

private:
  static void DestroyLibrary( FT_Library library );
};

} // end namespace Core

#endif
