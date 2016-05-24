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

#include <Core/RenderResources/RenderResources.h>
#include <Core/Volume/DataVolumeBrick.h>
#include <Core/Graphics/PixelBufferObject.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class DataVolumeBrickPrivate
//////////////////////////////////////////////////////////////////////////
  
class DataVolumeBrickPrivate
{
public:
  BBox brick_bbox_;
  BBox tex_bbox_;
  Vector texel_size_;
  Texture3DHandle tex_;
};

//////////////////////////////////////////////////////////////////////////
// Class DataVolumeBrick
//////////////////////////////////////////////////////////////////////////

const unsigned int DataVolumeBrick::TEXTURE_DATA_TYPE_C = GL_UNSIGNED_SHORT;
const unsigned int DataVolumeBrick::TEXTURE_FORMAT_C = GL_ALPHA16;

DataVolumeBrick::DataVolumeBrick( const BBox& brick_bbox, const BBox& tex_bbox, 
                 const Vector& texel_size, Texture3DHandle tex ) :
  private_( new DataVolumeBrickPrivate )
{
  this->private_->brick_bbox_ = brick_bbox;
  this->private_->tex_bbox_ = tex_bbox;
  this->private_->texel_size_ = texel_size;
  this->private_->tex_ = tex;
}

DataVolumeBrick::~DataVolumeBrick()
{
}

Core::Texture3DHandle DataVolumeBrick::get_texture() const
{
  return this->private_->tex_;
}

Core::BBox DataVolumeBrick::get_brick_bbox() const
{
  return this->private_->brick_bbox_;
}

Core::BBox DataVolumeBrick::get_texture_bbox() const
{
  return this->private_->tex_bbox_;
}

Core::Vector DataVolumeBrick::get_texel_size() const
{
  return this->private_->texel_size_;
}

} // end namespace Core
