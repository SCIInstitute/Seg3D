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

#ifndef UTILS_TEXTRENDERER_FREETYPEFACE_H
#define UTILS_TEXTRENDERER_FREETYPEFACE_H

// STL includes
#include <map>

// boost includes
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Utils include
#include <Utils/Core/Lockable.h>
#include <Utils/Core/Singleton.h>
#include <Utils/TextRenderer/FreeTypeGlyph.h>

namespace Utils
{

class FreeTypeFace;
typedef boost::shared_ptr< FreeTypeFace > FreeTypeFaceHandle;

class FreeTypeFace
{
private:
  friend class FreeTypeLibrary;
  FreeTypeFace( FT_Face face );

public:
  ~FreeTypeFace();

  // NOTE: The following two functions only work for scalable fonts, and they don't
  // check for errors.

  void set_char_size( FT_F26Dot6 width, FT_F26Dot6 height, 
    unsigned int horz_resolution, unsigned int vert_resolution );
  void set_pixel_sizes( unsigned int width, unsigned int height );

public:
  unsigned int get_char_index( unsigned long charcode );
  FreeTypeGlyphHandle get_glyph_by_char( unsigned long charcode );
  FreeTypeGlyphHandle get_glyph_by_index( unsigned int index );

  bool has_kerning();
  void get_kerning( unsigned int left_index, unsigned int right_index,
    unsigned int kern_mode, FT_Vector* kerning );

private:
  FT_Face face_;

  typedef std::map< unsigned int, FreeTypeGlyphHandle > glyph_map_type;
  // Cache of glyphs indexed by their char index
  glyph_map_type glyph_map_;
};

} // end namespace Utils

#endif