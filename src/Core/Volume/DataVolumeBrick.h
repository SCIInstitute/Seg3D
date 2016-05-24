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

#ifndef CORE_VOLUME_DATAVOLUMEBRICK_H
#define CORE_VOLUME_DATAVOLUMEBRICK_H

#include <Core/Geometry/BBox.h>
#include <Core/Graphics/Texture.h>

namespace Core
{

class DataVolumeBrick;
typedef boost::shared_ptr< DataVolumeBrick > DataVolumeBrickHandle;

class DataVolumeBrickPrivate;
typedef boost::shared_ptr< DataVolumeBrickPrivate > DataVolumeBrickPrivateHandle;

class DataVolumeBrick
{
public:
  typedef unsigned short data_type;

  DataVolumeBrick( const BBox& brick_bbox, const BBox& tex_bbox, 
    const Vector& texel_size, Texture3DHandle tex );
  ~DataVolumeBrick();

  Texture3DHandle get_texture() const;
  BBox get_brick_bbox() const;
  BBox get_texture_bbox() const;
  Vector get_texel_size() const;

private:
  DataVolumeBrickPrivateHandle private_;

public:
  /// An array of GLenum's for data types, indexed by data_type values
  const static unsigned int TEXTURE_DATA_TYPE_C;

  /// An array of GLenum's for GL internal texture formats, indexed by data_type values.
  /// These formats are picked to best match the data type.
  const static unsigned int TEXTURE_FORMAT_C;
};

} // end namespace Core

#endif
