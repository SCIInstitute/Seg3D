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
#include <Core/Graphics/ColorMap.h>
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

  // COMPUTE:
  void compute( double quality_factor = 1.0 );

  // GET_POINTS:
  // Get 3D points for vertices, each stored only once
  const std::vector< PointF >& get_points() const;

  // GET_NORMALS:
  // Get one normal per vertex, interpolated
  const std::vector< VectorF >& get_normals() const;

  // GET_VALUES:
  // Get values per vertex.  Returns empty vector if use has not set values.
  const std::vector< float >& get_values() const; 

  // SET_VALUES:
  // Set values for all vertices.  Vector must be same size as points and normals vectors 
  // or empty.  Returns true on success, false on failure.
  bool set_values( const std::vector< float >& values );

  // GET_FACES:
  // Indices into vertices, 3 per face
  const std::vector< unsigned int >& get_faces() const;

  // REDRAW:
  // Render the isosurface.
  void redraw();

  typedef boost::signals2::signal< void (double) > update_progress_signal_type;

  // UPDATE_PROGRESS:
  // When new information on progress is available this signal is triggered. If this signal is 
  // triggered it should end with a value 1.0 indicating that progress reporting has finised.
  // Progress is measured between 0.0 and 1.0.
  update_progress_signal_type update_progress_signal_;

  // GETCOLORMAP
  // Get a copy of the color map.  Returns copy in order to avoid threading issues.  
  static void GetColorMap( ColorMap& color_map );

  // SETCOLORMAP
  // Set the colormap.  Makes a copy to avoid threading issues.  
  static void SetColorMap( const ColorMap& color_map );

private:
  IsosurfacePrivateHandle private_;
};

} // end namespace Core

#endif