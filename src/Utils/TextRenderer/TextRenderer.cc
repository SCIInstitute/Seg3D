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

#include <cassert>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <Utils/TextRenderer/TextRenderer.h>

namespace Utils
{

TextRenderer::TextRenderer() :
  valid_( true )
{
  this->ft_library_ = FreeTypeLibraryFactory::CreateLibrary();

#if defined( _WIN32 )
  std::vector<char> buffer( MAX_PATH );
  GetWindowsDirectory( &buffer[0], MAX_PATH );
  this->font_file_.assign( &buffer[0] );
  this->font_file_ += "\\fonts\\arial.ttf";
  this->face_index_ = 0;
#elif defined( __APPLE__ )
  this->font_file_.assign( "/System/Library/Fonts/Helvetica.dfont" );
  this->face_index_ = 2;
#endif
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::render( const std::string& text, unsigned char* buffer, int width, 
    int height, int x_offset, int y_offset, unsigned int font_size, 
    float red, float green, float blue, float alpha, bool blend )
{
  assert( width > 0 && height > 0 );

  if ( !this->valid_ )
    return;

  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
  }

  FreeTypeFaceHandle face;
  face_map_type::iterator it = this->face_map_.find( font_size );
  if ( it != this->face_map_.end() )
  {
    face = ( *it ).second;
  }
  else
  {
    face = this->ft_library_->new_face_from_file( this->font_file_.c_str(), this->face_index_ );
    if ( !face )
    {
      this->valid_ = false;
      return;
    }
    face->set_pixel_sizes( font_size, font_size );
    this->face_map_[ font_size ] = face;
  }

  int pen_x = x_offset;
  int pen_y = y_offset;
  unsigned int previous_index = 0;
  bool use_kerning = face->has_kerning();
  for ( size_t i = 0; i < text_len; i++ )
  {
    unsigned int glyph_index = face->get_char_index( text[ i ] );
    FreeTypeGlyphHandle glyph = face->get_glyph_by_index( glyph_index );
    if ( !glyph )
    {
      continue;
    }

    if ( use_kerning && previous_index != 0 && glyph_index != 0 )
    {
      FT_Vector delta;
      face->get_kerning( previous_index, glyph_index, FT_KERNING_DEFAULT, &delta );
      pen_x += ( delta.x >> 6 );
    }
    FreeTypeBitmapGlyphHandle glyph_bitmap = glyph->render_to_bitmap();
    glyph_bitmap->draw( buffer, width, height, pen_x, pen_y, red, green, blue, alpha, blend );
    pen_x += glyph->glyph_->advance.x >> 16;
    previous_index = glyph_index;
  }
}

} // end namespace Utils