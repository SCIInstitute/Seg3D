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

#include <cassert>
#include <numeric>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <Core/Math/MathFunctions.h>
#include <Core/TextRenderer/TextRenderer.h>
#include <Core/Utils/Log.h>

#include <boost/filesystem.hpp>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace Core
{

TextRenderer::TextRenderer() :
  ft_library_(FreeTypeLibraryFactory::CreateLibrary()),
  face_index_(0),
  valid_( true )
{
#if defined( _WIN32 )
  std::vector<char> buffer( MAX_PATH );
  GetWindowsDirectory( &buffer[0], MAX_PATH );
  this->font_file_.assign( &buffer[0] );
  this->font_file_ += "\\fonts\\arial.ttf";
#elif defined( __APPLE__ )
  this->font_file_.assign( "/System/Library/Fonts/Helvetica.dfont" );
    if ( !boost::filesystem::exists( this->font_file_ ) )
    {
      this->font_file_.assign( "/System/Library/Fonts/Helvetica.ttc");
    }
  this->face_index_ = 2;
#else
  const char* font_path = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf";
  if (! boost::filesystem::exists(font_path) )
  {
    // alternative path to freefont paths on some linux distros (OpenSuSE for example)
    font_path = "/usr/share/fonts/truetype/FreeSerif.ttf";

    if (! boost::filesystem::exists(font_path) )
    {

      // alternative path to freefont paths on some other linux distros (CentOS for example)
      font_path = "/usr/share/fonts/gnu-free/FreeSerif.ttf";

      if (! boost::filesystem::exists(font_path) )
      {

        // TODO: would it make sense to throw an exception here?
        // object will be incompletely constructed if font path cannot be found
        //
        // alternatively, configure location of font path on program startup, so this check
        // does not have to be run every time the object is constructed? (AYK)
        CORE_LOG_ERROR( "Unable to locate FreeSerif.ttf in /usr/share/fonts/truetype/freefont or /usr/share/fonts/truetype or /usr/share/fonts/gnu-free." );
        return;
      }
    }
  }
  this->font_file_.assign( font_path );
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

  std::vector< FreeTypeGlyphHandle > glyphs;
  this->get_glyphs( text, face, glyphs );
  this->render( glyphs, buffer, width, height, x_offset, y_offset );
}

void TextRenderer::render( const std::string& text, unsigned char* buffer, int width,
  int height, int x_offset, int y_offset, unsigned int font_size, double angle )
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

  std::vector< FreeTypeGlyphHandle > glyphs;
  this->get_glyphs( text, face, angle, glyphs );
  this->render( glyphs, buffer, width, height, x_offset, y_offset );
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
    std::vector< FreeTypeGlyphHandle > glyphs;
    this->get_glyphs( text[ i ], face, glyphs );
    this->render( glyphs, buffer, width, height, x_offset, y_offset );
    y_offset -= ( ( face->face_->size->metrics.height >> 6 ) + line_spacing );
  }
}

void TextRenderer::render_aligned( const std::string& text, unsigned char* buffer, 
  int width, int height, unsigned int font_size, 
  TextHAlignmentType halign, TextVAlignmentType valign, 
  int left_margin, int right_margin, int bottom_margin, int top_margin )
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

  std::vector< FreeTypeGlyphHandle > glyphs;
  FT_BBox bbox;
  this->get_glyphs( text, face, glyphs );
  this->compute_bbox( glyphs, bbox );

  int x_offset = 0;
  int y_offset = 0;
  this->compute_offset( width, height, bbox, halign, valign, x_offset, y_offset,
    left_margin, right_margin, bottom_margin, top_margin );
  this->render( glyphs, buffer, width, height, x_offset, y_offset );
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

  FT_BBox bbox;
  std::vector< FreeTypeGlyphHandle > glyphs;
  this->get_glyphs( text, face, angle, glyphs );
  this->compute_bbox( glyphs, bbox );

  int x_offset = 0;
  int y_offset = 0;
  this->compute_offset( width, height, bbox, halign, valign, x_offset, y_offset );

  this->render( glyphs, buffer, width, height, x_offset, y_offset );
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

  std::vector< FreeTypeGlyphHandle > glyphs;
  this->get_glyphs( text, face, glyphs );

  FT_BBox string_bbox;
  this->compute_bbox( glyphs, string_bbox );

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

void TextRenderer::get_glyphs( const std::string& text, FreeTypeFaceHandle face,
  std::vector< FreeTypeGlyphHandle >& glyphs )
{
  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
  }
  
  FT_Vector pen_pos;
  pen_pos.x = 0;
  pen_pos.y = 0;

  unsigned int previous_index = 0;
  bool use_kerning = face->has_kerning();
  for ( size_t i = 0; i < text_len; i++ )
  {
    unsigned int glyph_index = face->get_char_index( text[ i ] );
    FreeTypeGlyphConstHandle glyph = face->get_glyph_by_index( glyph_index );
    if ( !glyph )
    {
      continue;
    }

    if ( use_kerning && previous_index != 0 && glyph_index != 0 )
    {
      FT_Vector delta;
      face->get_kerning( previous_index, glyph_index, FT_KERNING_DEFAULT, &delta );
      pen_pos.x += delta.x;
    }

    FreeTypeGlyphHandle tmp_glyph = FreeTypeGlyphHandle( new FreeTypeGlyph( *glyph ) );
    tmp_glyph->transform( 0, &pen_pos );
    glyphs.push_back( tmp_glyph );

    pen_pos.x += tmp_glyph->glyph_->advance.x >> 10;
    previous_index = glyph_index;
  }
}

void TextRenderer::get_glyphs( const std::string& text, FreeTypeFaceHandle face,
  double angle, std::vector< FreeTypeGlyphHandle >& glyphs )
{
  size_t text_len = text.size();
  if ( text_len == 0 )
  {
    return;
  }
  
  angle = Core::DegreeToRadian( angle );
  double sin_angle = Core::Sin( angle );
  double cos_angle = Core::Cos( angle );

  FT_Matrix rot_mat;
  rot_mat.xx = static_cast< FT_Fixed >( cos_angle * 0x10000L );
  rot_mat.xy = static_cast< FT_Fixed >( -sin_angle * 0x10000L );
  rot_mat.yx = -rot_mat.xy;
  rot_mat.yy = rot_mat.xx;

  FT_Vector pen_pos;
  pen_pos.x = 0;
  pen_pos.y = 0;

  unsigned int previous_index = 0;
  bool use_kerning = face->has_kerning();
  for ( size_t i = 0; i < text_len; i++ )
  {
    unsigned int glyph_index = face->get_char_index( text[ i ] );
    FreeTypeGlyphConstHandle glyph = face->get_glyph_by_index( glyph_index );
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
      pen_pos.x += rotated_delta.x;
      pen_pos.y += rotated_delta.y;
    }

    // Make a copy of the glyph so the original one remains untouched.
    FreeTypeGlyphHandle tmp_glyph = FreeTypeGlyphHandle( new FreeTypeGlyph( *glyph ) );
    // Transform the glyph: rotation plus translating the glyph to the current pen position
    tmp_glyph->transform( &rot_mat, &pen_pos );

    glyphs.push_back( tmp_glyph );

    pen_pos.x += tmp_glyph->glyph_->advance.x >> 10;
    pen_pos.y += tmp_glyph->glyph_->advance.y >> 10;
    previous_index = glyph_index;
  }
}

void TextRenderer::render( const std::vector< FreeTypeGlyphHandle >& glyphs, 
  unsigned char* buffer, int width, int height, int x_offset, int y_offset )
{
  size_t num_of_glyphs = glyphs.size();
  for ( size_t i = 0; i < num_of_glyphs; i++ )
  {
    FreeTypeGlyphHandle glyph = glyphs[ i ];
    FreeTypeBitmapGlyphHandle glyph_bitmap = glyph->render_to_bitmap();
    glyph_bitmap->draw( buffer, width, height, x_offset, y_offset );
  }
}

void TextRenderer::compute_bbox( const std::vector< FreeTypeGlyphHandle >& glyphs, 
  FT_BBox& bbox )
{
  bbox.xMax = bbox.yMax = std::numeric_limits< FT_Pos >::min();
  bbox.xMin = bbox.yMin = std::numeric_limits< FT_Pos >::max();

  size_t num_of_glyphs = glyphs.size();
  for ( size_t i = 0; i < num_of_glyphs; i++ )
  {
    FT_BBox glyph_bbox;
    glyphs[ i ]->get_cbox( &glyph_bbox );

    bbox.xMin = Core::Min( bbox.xMin, glyph_bbox.xMin );
    bbox.xMax = Core::Max( bbox.xMax, glyph_bbox.xMax );
    bbox.yMin = Core::Min( bbox.yMin, glyph_bbox.yMin );
    bbox.yMax = Core::Max( bbox.yMax, glyph_bbox.yMax );
  }

  if ( bbox.xMin > bbox.xMax )
  {
    bbox.xMin = 0;
    bbox.yMin = 0;
    bbox.xMax = 0;
    bbox.yMax = 0;
  }
}

void TextRenderer::compute_offset( int width, int height, const FT_BBox& bbox, 
  TextHAlignmentType halign, TextVAlignmentType valign, int& x_offset, int& y_offset,
  int left_margin, int right_margin, int bottom_margin, int top_margin )
{
  switch( halign )
  {
  case TextHAlignmentType::LEFT_E:
    x_offset = -bbox.xMin + left_margin;
    break;
  case TextHAlignmentType::RIGHT_E:
    x_offset = width - bbox.xMax - right_margin;
    break;
  case TextHAlignmentType::CENTER_E:
    x_offset = ( width - bbox.xMax - bbox.xMin + left_margin - right_margin ) / 2;
    break;
  default:
    assert( false );
    break;
  }

  switch( valign )
  {
  case TextVAlignmentType::BOTTOM_E:
    y_offset = -bbox.yMin + bottom_margin;
    break;
  case TextVAlignmentType::TOP_E:
    y_offset = height - bbox.yMax - top_margin;
    break;
  case TextVAlignmentType::CENTER_E:
    y_offset = ( height - bbox.yMax - bbox.yMin + bottom_margin - top_margin ) / 2;
    break;
  default:
    assert( false );
    break;
  }
}

} // end namespace Core
