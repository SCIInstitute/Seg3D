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

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>
#include <Core/TextRenderer/FreeTypeFace.h>

namespace Core
{

FreeTypeFace::FreeTypeFace( FT_Face face ) :
  face_( face )
{
}

FreeTypeFace::~FreeTypeFace()
{
  // NOTE: FreeType library handles the destruction of face
}

void FreeTypeFace::set_char_size( FT_F26Dot6 width, FT_F26Dot6 height, 
    unsigned int horz_resolution, unsigned int vert_resolution )
{
  FT_Set_Char_Size( this->face_, width, height, horz_resolution, vert_resolution );
  this->glyph_map_.clear();
}

void FreeTypeFace::set_pixel_sizes( unsigned int width, unsigned int height )
{
  FT_Set_Pixel_Sizes( this->face_, width, height );
  this->glyph_map_.clear();
}

unsigned int FreeTypeFace::get_char_index( unsigned long charcode )
{
  return FT_Get_Char_Index( this->face_, charcode );
}

FreeTypeGlyphConstHandle FreeTypeFace::get_glyph_by_char( unsigned long charcode )
{
  return this->get_glyph_by_index( this->get_char_index( charcode ) );
}

FreeTypeGlyphConstHandle FreeTypeFace::get_glyph_by_index( unsigned int index )
{
  glyph_map_type::iterator it = this->glyph_map_.find( index );
  if ( it != this->glyph_map_.end() )
  {
    return ( *it ).second;
  }

  FT_Error err = FT_Load_Glyph( this->face_, index, FT_LOAD_DEFAULT );
  if ( err != 0 )
  {
    return FreeTypeGlyphHandle();
  }

  FT_Glyph glyph;
  FT_Get_Glyph( this->face_->glyph, &glyph );
  FreeTypeGlyphHandle glyph_handle( new FreeTypeGlyph( glyph ) );
  this->glyph_map_[ index ] = glyph_handle;

  return glyph_handle;
}

bool FreeTypeFace::has_kerning()
{
  return FT_HAS_KERNING( this->face_ ) != 0;
}

void FreeTypeFace::get_kerning( unsigned int left_index, unsigned int right_index,
    unsigned int kern_mode, FT_Vector* kerning )
{
  FT_Get_Kerning( this->face_, left_index, right_index, kern_mode, kerning );
}

} // end namespace Core
