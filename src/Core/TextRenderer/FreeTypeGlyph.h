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

#ifndef CORE_TEXTRENDERER_FREETYPEGLYPH_H
#define CORE_TEXTRENDERER_FREETYPEGLYPH_H

// boost includes
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// Core include
#include <Core/Utils/Lockable.h>
#include <Core/Utils/Singleton.h>

namespace Core
{

class FreeTypeGlyph;
class FreeTypeBitmapGlyph;
typedef boost::shared_ptr< FreeTypeGlyph > FreeTypeGlyphHandle;
typedef boost::shared_ptr< const FreeTypeGlyph > FreeTypeGlyphConstHandle;
typedef boost::shared_ptr< FreeTypeBitmapGlyph > FreeTypeBitmapGlyphHandle;

class FreeTypeGlyph
{
private:
  friend class FreeTypeFace;
  friend class TextRenderer;
  FreeTypeGlyph( FT_Glyph glyph );

public:
  FreeTypeGlyph( const FreeTypeGlyph& copy );
  ~FreeTypeGlyph();

  void transform( FT_Matrix* matrix, FT_Vector* delta );

  /// Get the CBox of the glyph in pixel coordinates
  void get_cbox( FT_BBox* cbox ) const;

  /// Render the glyph to a 8-bit anti-aliased bitmap
  FreeTypeBitmapGlyphHandle render_to_bitmap( FT_Vector* origin = 0 ) const;

private:
  FreeTypeGlyph& operator=(  const FreeTypeGlyph& copy );

private:
  FT_Glyph glyph_;
};

class FreeTypeBitmapGlyph : public boost::noncopyable
{
private:
  friend class FreeTypeGlyph;
  FreeTypeBitmapGlyph( FT_BitmapGlyph glyph_bitmap );

public:
  ~FreeTypeBitmapGlyph();

  void draw( unsigned char* target, const int width, const int height, 
    const int x_offset, const int y_offset ) const;

private:
  FT_BitmapGlyph glyph_bitmap_;
};

} // end namespace Core

#endif
