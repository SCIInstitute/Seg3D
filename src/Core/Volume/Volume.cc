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

#include <Core/Volume/Volume.h>

namespace Core
{

Volume::Volume( const GridTransform& grid_transform ) :
  grid_transform_( grid_transform )
{
  // Cache size for faster indexing into the volume
  this->nx_ = this->grid_transform_.get_nx();
  this->ny_ = this->grid_transform_.get_ny();
  this->nz_ = this->grid_transform_.get_nz();
  this->nxy_ = this->nx_ * this->ny_;

  // Compute the inverse transform asw well.
  this->inverse_transform_ = this->grid_transform_.transform().get_inverse();
}

Volume::~Volume()
{
}

Point Volume::apply_grid_transform( const Point& pt ) const
{
  return this->grid_transform_ * pt;
}

Point Volume::apply_inverse_grid_transform( const Point& pt ) const
{
  return this->inverse_transform_ * pt;
}

void Volume::set_grid_transform( const GridTransform& grid_transform, bool preserve_centering )
{
  bool originally_node_centered = this->grid_transform_.get_originally_node_centered();
  this->grid_transform_ = grid_transform;
  if( preserve_centering )
  {
    this->grid_transform_.set_originally_node_centered( originally_node_centered );
  }
}

} // end namespace Core

