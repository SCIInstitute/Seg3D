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

// Core includes
#include <Core/Graphics/ColorMap.h>
#include <Core/RenderResources/RenderResources.h>

namespace Core 
{

class ColorMapPrivate : public Lockable
{
public:
  std::vector< Color > colors_;
  float lookup_min_;
  float lookup_max_;
  Texture1DHandle texture_;
  bool changed_;

  const static Color DEFAULT_COLOR_C;
};

const Color ColorMapPrivate::DEFAULT_COLOR_C = Color( 0, 0, 0 );

ColorMap::ColorMap()
: private_( new ColorMapPrivate )
{
  this->private_->lookup_min_ = 0;
  this->private_->lookup_max_ = 1;
  this->private_->changed_ = true;

  // Default to Rainbow
  float scale = 1.0f / 255.0f;
  this->private_->colors_.push_back( Color( 0, 0, 255 ) * scale );
  this->private_->colors_.push_back( Color( 0, 52, 255 ) * scale );
  this->private_->colors_.push_back( Color( 1, 80, 255 ) * scale );
  this->private_->colors_.push_back( Color( 3, 105, 255 ) * scale );
  this->private_->colors_.push_back( Color( 5, 132, 255 ) * scale );
  this->private_->colors_.push_back( Color( 9, 157, 243 ) * scale );
  this->private_->colors_.push_back( Color( 11, 177, 213 ) * scale );
  this->private_->colors_.push_back( Color( 15, 193, 182 ) * scale );
  this->private_->colors_.push_back( Color( 21, 210, 152 ) * scale );
  this->private_->colors_.push_back( Color( 30, 225, 126 ) * scale );
  this->private_->colors_.push_back( Color( 42, 237, 102 ) * scale );
  this->private_->colors_.push_back( Color( 60, 248, 82 ) * scale );
  this->private_->colors_.push_back( Color( 87, 255, 62 ) * scale);
  this->private_->colors_.push_back( Color( 116, 255, 49 ) * scale );
  this->private_->colors_.push_back( Color( 148, 252, 37 ) * scale );
  this->private_->colors_.push_back( Color( 178, 243, 27 ) * scale );
  this->private_->colors_.push_back( Color( 201, 233, 19 ) * scale );
  this->private_->colors_.push_back( Color( 220, 220, 14 ) * scale );
  this->private_->colors_.push_back( Color( 236, 206, 10 ) * scale );
  this->private_->colors_.push_back( Color( 247, 185, 8 ) * scale );
  this->private_->colors_.push_back( Color( 253, 171, 5 ) * scale );
  this->private_->colors_.push_back( Color( 255, 151, 3 ) * scale );
  this->private_->colors_.push_back( Color( 255, 130, 2 ) * scale );
  this->private_->colors_.push_back( Color( 255, 112, 1 ) * scale );
  this->private_->colors_.push_back( Color( 255, 94, 0 ) * scale );
  this->private_->colors_.push_back( Color( 255, 76, 0 ) * scale );
  this->private_->colors_.push_back( Color( 255, 55, 0 ) * scale );
  this->private_->colors_.push_back( Color( 255, 0, 0 ) * scale );
}

ColorMap::~ColorMap()
{
}

ColorMap::ColorMap( const std::vector< Color >& colors, float lookup_min, float lookup_max )
: private_( new ColorMapPrivate )
{
  this->private_->colors_ = colors;
  this->private_->lookup_min_ = lookup_min;
  this->private_->lookup_max_ = lookup_max;
  this->private_->changed_ = true;
}

const std::vector< Color >& ColorMap::get_colors() const
{
  return this->private_->colors_;
}

void ColorMap::set_color( size_t index, const Color& color )
{
  assert( index < this->private_->colors_.size() - 1 );
  this->private_->colors_[ index ]= color;
  this->private_->changed_ = true;
}

size_t ColorMap::get_size() const
{
  return this->private_->colors_.size();
}

void ColorMap::resize( size_t size )
{
  if( size != this->get_size() )
  {
    this->private_->colors_.resize( size, ColorMapPrivate::DEFAULT_COLOR_C );
    this->private_->changed_ = true;
  }
}

void ColorMap::set_lookup_range( float lookup_min, float lookup_max )
{
  this->private_->lookup_min_ = lookup_min;
  this->private_->lookup_max_ = lookup_max;
}

void ColorMap::get_lookup_range( float& lookup_min, float& lookup_max ) const
{
  lookup_min = this->private_->lookup_min_;
  lookup_max = this->private_->lookup_max_;
}

Texture1DHandle ColorMap::get_texture()
{
  ColorMapPrivate::lock_type lock( this->private_->get_mutex() );

  if( this->private_->changed_ )
  {
    // Upload colormap data to 1D graphics texture.
    this->private_->texture_.reset( new Texture1D );
    this->private_->texture_->set_min_filter( GL_LINEAR );
    this->private_->texture_->set_mag_filter( GL_LINEAR );
    this->private_->texture_->set_image( static_cast< int >( this->get_size() ), GL_RGB, 
      &this->private_->colors_[ 0 ], GL_RGB, GL_FLOAT );
    CORE_CHECK_OPENGL_ERROR();
    this->private_->changed_ = false;
  }
  return this->private_->texture_; 
}

} // End namespace Core


