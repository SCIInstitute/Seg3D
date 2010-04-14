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

#ifndef UTILS_TEXTRENDERER_TEXTRENDERER_H
#define UTILS_TEXTRENDERER_TEXTRENDERER_H

// STL includes
#include <map>
#include <string>

// boost includes
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

#include <Utils/Core/EnumClass.h>
#include <Utils/TextRenderer/FreeTypeLibrary.h>

namespace Utils
{

class TextRenderer;
typedef boost::shared_ptr< TextRenderer > TextRendererHandle;

SCI_ENUM_CLASS
(
  TextHAlignmentType,
  LEFT_E,
  RIGHT_E,
  CENTER_E
)

SCI_ENUM_CLASS
(
  TextVAlignmentType,
  BOTTOM_E,
  TOP_E,
  CENTER_E
)

class TextRenderer : public boost::noncopyable
{
public:
  TextRenderer();
  ~TextRenderer();

  // RENDER
  // Renders the given text to the target buffer with the specified font size. 
  // The result in the buffer is a 1-channel 8-bit gray scale bitmap.
  void render( const std::string& text, unsigned char* buffer, int width,
    int height, int x_offset, int y_offset, unsigned int font_size );

  // RENDER
  // Render multiple lines of text onto the buffer, with the specified line spacing.
  void render( const std::vector< std::string >& text, unsigned char* buffer, int width,
    int height, int x_offset, int y_offset, unsigned int font_size, int line_spacing );

  // RENDER
  // Render the given text to the target buffer with the specified font size and alignment
  void render_aligned( const std::string& text, unsigned char* buffer, int width, int height, 
    unsigned int font_size, TextHAlignmentType halign = TextHAlignmentType::LEFT_E,
    TextVAlignmentType valign = TextVAlignmentType::BOTTOM_E );

  // COMPUTE_SIZE
  // Compute the size of the given text string when rendered in the specified font size
  void compute_size( const std::string& text, unsigned int font_size, 
    int& width, int& height, int& x_offset, int& y_offset );

private:

  FreeTypeFaceHandle get_face( unsigned int font_size );

  void render( const std::string& text, unsigned char* buffer, int width,
    int height, int x_offset, int y_offset, FreeTypeFaceHandle face );

  void compute_bbox( const std::string& text, FreeTypeFaceHandle face, FT_BBox& bbox );

private:
  FreeTypeLibraryHandle ft_library_;

  typedef std::map< unsigned int, FreeTypeFaceHandle > face_map_type;
  // The cache of faces indexed by their sizes
  face_map_type face_map_;

  std::string font_file_;
  long face_index_;
  bool valid_;
};

} // end namespace Utils

#endif