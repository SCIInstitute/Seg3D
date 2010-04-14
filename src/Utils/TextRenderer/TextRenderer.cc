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
#include <numeric>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <Utils/Math/MathFunctions.h>
#include <Utils/TextRenderer/TextRenderer.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

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
    int height, int x_offset, int y_offset, unsigned int font_size )
{
  assert( width > 0 && height > 0 );

  if ( !this->valid_ )
    return;

  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
  }

  FreeTypeFaceHandle face = this->get_face( font_size );
  if ( !face )
  {
    return;
  }

  this->render( text, buffer, width, height, x_offset, y_offset, face );
}

void TextRenderer::render( const std::vector< std::string >& text, unsigned char* buffer, 
  int width, int height, int x_offset, int y_offset, unsigned int font_size, int line_spacing )
{
  assert( width > 0 && height > 0 );

  if ( !this->valid_ )
    return;

  size_t num_of_strings = text.size();
  if ( num_of_strings == 0 )
  {
    return;
  }

  FreeTypeFaceHandle face = this->get_face( font_size );
  if ( !face )
  {
    return;
  }

  for ( size_t i = 0; i < num_of_strings; i++ )
  {
    this->render( text[ i ], buffer, width, height, x_offset, y_offset, face );
    y_offset -= ( ( face->face_->size->metrics.height >> 6 ) + line_spacing );
  }
}

void TextRenderer::render_aligned( const std::string& text, unsigned char* buffer, 
  int width, int height, unsigned int font_size, 
  TextHAlignmentType halign, TextVAlignmentType valign )
{
  assert( width > 0 && height > 0 );

  if ( !this->valid_ )
    return;

  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
  }

  FreeTypeFaceHandle face = this->get_face( font_size );
  if ( !face )
  {
    return;
  }

  int x_offset;
  int y_offset;
  FT_BBox bbox;
  this->compute_bbox( text, face, bbox );
  int text_width = static_cast< int >( bbox.xMax - bbox.xMin );
  int text_height = static_cast< int >( bbox.yMax - bbox.yMin );

  switch( halign )
  {
  case TextHAlignmentType::LEFT_E:
    x_offset = -bbox.xMin;
    break;
  case TextHAlignmentType::RIGHT_E:
    x_offset = width - text_width - bbox.xMin;
    break;
  case TextHAlignmentType::CENTER_E:
    x_offset = ( width - text_width ) / 2 - bbox.xMin;
    break;
  default:
    assert( false );
    break;
  }

  switch( valign )
  {
  case TextVAlignmentType::BOTTOM_E:
    y_offset = -bbox.yMin;
    break;
  case TextVAlignmentType::TOP_E:
    y_offset = height - text_height - bbox.yMin;
    break;
  case TextVAlignmentType::CENTER_E:
    y_offset = ( height - text_height ) / 2 - bbox.yMin;
    break;
  default:
    assert( false );
    break;
  }

  this->render( text, buffer, width, height, x_offset, y_offset, face );
}

void TextRenderer::render_aligned( const std::string& text, unsigned char* buffer, 
  int width, int height, unsigned int font_size, double angle, 
  TextHAlignmentType halign, TextVAlignmentType valign )
{
  assert( width > 0 && height > 0 );

  if ( !this->valid_ )
    return;

  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
  }

  FreeTypeFaceHandle face = this->get_face( font_size );
  if ( !face )
  {
    return;
  }

  int x_offset;
  int y_offset;
  FT_BBox bbox;
  std::vector< FreeTypeGlyphHandle > glyphs;
  this->compute_bbox( text, face, angle, bbox, glyphs );
  int text_width = static_cast< int >( bbox.xMax - bbox.xMin );
  int text_height = static_cast< int >( bbox.yMax - bbox.yMin );

  switch( halign )
  {
  case TextHAlignmentType::LEFT_E:
    x_offset = -bbox.xMin;
    break;
  case TextHAlignmentType::RIGHT_E:
    x_offset = width - text_width - bbox.xMin;
    break;
  case TextHAlignmentType::CENTER_E:
    x_offset = ( width - text_width ) / 2 - bbox.xMin;
    break;
  default:
    assert( false );
    break;
  }

  switch( valign )
  {
  case TextVAlignmentType::BOTTOM_E:
    y_offset = -bbox.yMin;
    break;
  case TextVAlignmentType::TOP_E:
    y_offset = height - text_height - bbox.yMin;
    break;
  case TextVAlignmentType::CENTER_E:
    y_offset = ( height - text_height ) / 2 - bbox.yMin;
    break;
  default:
    assert( false );
    break;
  }

  size_t num_of_glyphs = glyphs.size();
  for ( size_t i = 0; i < num_of_glyphs; i++ )
  {
    FreeTypeGlyphHandle glyph = glyphs[ i ];
    FreeTypeBitmapGlyphHandle glyph_bitmap = glyph->render_to_bitmap();
    glyph_bitmap->draw( buffer, width, height, x_offset, y_offset );
  }
}

void TextRenderer::compute_size( const std::string& text, unsigned int font_size, 
  int& width, int& height, int& x_offset, int& y_offset )
{
  size_t text_len = text.size();
  if ( !this->valid_ || text_len == 0 )
  {
    width = height = 0;
    return;
  }

  FreeTypeFaceHandle face = this->get_face( font_size );
  if ( !face )
  {
    width = height = 0;
    return;
  }

  FT_BBox string_bbox;
  this->compute_bbox( text, face, string_bbox );
  width = static_cast< int >( string_bbox.xMax - string_bbox.xMin );
  height = static_cast< int >( string_bbox.yMax - string_bbox.yMin );
  x_offset = static_cast< int >( -string_bbox.xMin );
  y_offset = static_cast< int >( -string_bbox.yMin );
}

FreeTypeFaceHandle TextRenderer::get_face( unsigned int font_size )
{
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
    }
    else
    {
      face->set_pixel_sizes( font_size, font_size );
      this->face_map_[ font_size ] = face;
    }
  }

  return face;
}

void TextRenderer::render( const std::string& text, unsigned char* buffer, int width,
    int height, int x_offset, int y_offset, FreeTypeFaceHandle face )
{
  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
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
    glyph_bitmap->draw( buffer, width, height, pen_x, pen_y );
    pen_x += glyph->glyph_->advance.x >> 16;
    previous_index = glyph_index;
  }
}

void TextRenderer::compute_bbox( const std::string& text, FreeTypeFaceHandle face, 
  FT_BBox& bbox )
{
  bbox.xMax = bbox.yMax = std::numeric_limits< FT_Pos >::min();
  bbox.xMin = bbox.yMin = std::numeric_limits< FT_Pos >::max();

  size_t text_len = text.size();
  int pen_x = 0;
  int pen_y = 0;
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

    FT_BBox glyph_bbox;
    glyph->get_cbox( &glyph_bbox );
    glyph_bbox.xMin += pen_x;
    glyph_bbox.xMax += pen_x;
    glyph_bbox.yMin += pen_y;
    glyph_bbox.yMax += pen_y;

    bbox.xMin = Utils::Min( bbox.xMin, glyph_bbox.xMin );
    bbox.xMax = Utils::Max( bbox.xMax, glyph_bbox.xMax );
    bbox.yMin = Utils::Min( bbox.yMin, glyph_bbox.yMin );
    bbox.yMax = Utils::Max( bbox.yMax, glyph_bbox.yMax );
    
    pen_x += glyph->glyph_->advance.x >> 16;
    previous_index = glyph_index;
  }

  if ( bbox.xMin > bbox.xMax )
  {
    bbox.xMin = 0;
    bbox.yMin = 0;
    bbox.xMax = 0;
    bbox.yMax = 0;
  }
}

void TextRenderer::compute_bbox( const std::string &text, Utils::FreeTypeFaceHandle face,
  double angle, FT_BBox &bbox, std::vector<FreeTypeGlyphHandle>& glyphs )
{
  bbox.xMax = bbox.yMax = std::numeric_limits< FT_Pos >::min();
  bbox.xMin = bbox.yMin = std::numeric_limits< FT_Pos >::max();

  angle = Utils::DegreeToRadian( angle );
  double sin_angle = Utils::Sin( angle );
  double cos_angle = Utils::Cos( angle );

  FT_Matrix rot_mat;
  rot_mat.xx = static_cast< FT_Fixed >( cos_angle * 0x10000L );
  rot_mat.xy = static_cast< FT_Fixed >( -sin_angle * 0x10000L );
  rot_mat.yx = -rot_mat.xy;
  rot_mat.yy = rot_mat.xx;

  FT_Vector pen_pos;
  pen_pos.x = 0;
  pen_pos.y = 0;

  glyphs.clear();
  size_t text_len = text.size();
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
      FT_Vector rotated_delta;
      rotated_delta.x = static_cast< FT_Pos >( cos_angle * delta.x - sin_angle * delta.y );
      rotated_delta.y = static_cast< FT_Pos >( sin_angle * delta.x + cos_angle * delta.y );
      pen_pos.x += ( rotated_delta.x );
      pen_pos.y += ( rotated_delta.y );
    }

    // Make a copy of the glyph so the original one remains untouched.
    FreeTypeGlyphHandle tmp_glyph = FreeTypeGlyphHandle( new FreeTypeGlyph( *glyph ) );
    // Transform the glyph
    tmp_glyph->transform( &rot_mat, &pen_pos );

    glyphs.push_back( tmp_glyph );

    FT_BBox glyph_bbox;
    tmp_glyph->get_cbox( &glyph_bbox );

    bbox.xMin = Utils::Min( bbox.xMin, glyph_bbox.xMin );
    bbox.xMax = Utils::Max( bbox.xMax, glyph_bbox.xMax );
    bbox.yMin = Utils::Min( bbox.yMin, glyph_bbox.yMin );
    bbox.yMax = Utils::Max( bbox.yMax, glyph_bbox.yMax );

    pen_pos.x += tmp_glyph->glyph_->advance.x >> 10;
    pen_pos.y += tmp_glyph->glyph_->advance.y >> 10;
    previous_index = glyph_index;
  }

  if ( bbox.xMin > bbox.xMax )
  {
    bbox.xMin = 0;
    bbox.yMin = 0;
    bbox.xMax = 0;
    bbox.yMax = 0;
  }
}

} // end namespace Utils