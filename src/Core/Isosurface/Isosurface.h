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

#ifndef CORE_ISOSURFACE_ISOSURFACE_H
#define CORE_ISOSURFACE_ISOSURFACE_H

// STL includes
#include <vector>

// Boost includes
#include <boost/utility.hpp> // Needed for noncopyable
#include <boost/smart_ptr.hpp> // Needed for shared_ptr

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Volume/MaskVolume.h>
#include <Core/Utils/Lockable.h>

namespace Core
{

class Isosurface;
typedef boost::shared_ptr< Isosurface > IsosurfaceHandle;

// Hide header includes, private interface and implementation
class IsosurfacePrivate;
typedef boost::shared_ptr< IsosurfacePrivate > IsosurfacePrivateHandle;

// Isosurface geometry and computation code 
class Isosurface : public Core::RecursiveLockable
{
public:
  Isosurface( const MaskVolumeHandle& mask_volume );  

  void compute();

  // Vertices, each stored only once
  const std::vector< PointF >&    get_points() const;
  // One normal per vertex, interpolated
  const std::vector< VectorF >&   get_normals() const;
  // Indices into points and normals, 3 per face
  const std::vector< unsigned int >&  get_faces() const;

  // Render the isosurface.
  void redraw();

private:
  IsosurfacePrivateHandle private_;
};

} // end namespace Core

#endif