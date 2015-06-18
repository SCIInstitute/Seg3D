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
#include <boost/smart_ptr.hpp> // Needed for shared_ptr
#include <boost/utility.hpp> // Needed for noncopyable

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
  /// Compute isosurface.  quality_factor must be one of: {0.125, 0.25, 0.5, 1.0} 
  void compute( double quality_factor, bool capping_enabled, boost::function< bool () > check_abort );

  // GET_POINTS:
  /// Get 3D points for vertices, each stored only once
  /// NOTE: This function is not thread-safe, make sure you have the mutex
  /// allocated before using this array (use get_mutex())
  const std::vector< PointF >& get_points() const;

  // GET_FACES:
  /// Indices into vertices, 3 per face
  /// NOTE: This function is not thread-safe, make sure you have the mutex
  /// allocated before using this array (use get_mutex())
  const std::vector< unsigned int >& get_faces() const;

  // GET_NORMALS:
  /// Get one normal per vertex, interpolated
  /// NOTE: This function is not thread-safe, make sure you have the mutex
  /// allocated before using this array (use get_mutex())
  const std::vector< VectorF >& get_normals() const;

  // SURFACE_AREA:
  /// Return the area of the isosurface.
  float surface_area() const;

  // GET_VALUES:
  /// Get values per vertex.  Returns empty vector if use has not set values.
  /// NOTE: This function is not thread-safe, make sure you have the mutex
  /// allocated before using this array (use get_mutex())
  const std::vector< float >& get_values() const; 

  // SET_VALUES:
  /// Set values for all vertices.  Vector must be same size as points and normals vectors 
  /// or empty.  Returns true on success, false on failure.
  /// NOTE: This function is not thread-safe, make sure you have the mutex
  /// allocated before using this array (use get_mutex())
  bool set_values( const std::vector< float >& values );

  // SET_COLOR_MAP:
  /// Set mapping from vertex values to RGB colors.  
  /// NOTE: This function is not thread-safe. Passing handle since colormap is unlikely
  /// to be modified after creation. 
  void set_color_map( ColorMapHandle color_map );

  // GET_COLOR_MAP:
  /// Get mapping from vertex values to RGB colors
  ColorMapHandle get_color_map() const;

  // REDRAW:
  /// Render the isosurface.  This function doesn't work in isolation -- it must be called from the 
  /// Seg3D Renderer. 
  void redraw( bool use_colormap );

  // EXPORT_LEGACY_ISOSURFACE:
  /// Write points to .pts file, faces to .fac file, and values (if assigned) to .val file.  
  /// Returns true on success, false on failure.
  ///
  /// path: Path to existing directory where files should be written.
  /// file_prefix: File prefix to use for output files (no extension).
  /// 
  /// Format for .pts:
  /// x y z
  /// x y z
  /// ...
  /// 
  /// Format for .fac:
  /// p1 p2 p3
  /// p1 p2 p3
  /// ...
  ///
  /// Format for .val:
  /// v1
  /// v2
  /// ...
  ///
  /// Note: can't call this function "export" because it is reserved by the Visual C++ compiler.
  bool export_legacy_isosurface( const boost::filesystem::path& path,
                                 const std::string& file_prefix ); 

  // EXPORT_VTK_ISOSURFACE:
  /// Writes out an isosurface in VTK mesh format
  bool export_vtk_isosurface( const boost::filesystem::path& filename );

  // EXPORT_STL_ISOSURFACE:
  /// Writes out an isosurface in STL file format
  bool export_stl_isosurface( const boost::filesystem::path& filename, const std::string& name );

  typedef boost::signals2::signal< void (double) > update_progress_signal_type;

  // UPDATE_PROGRESS:
  /// When new information on progress is available this signal is triggered. If this signal is 
  /// triggered it should end with a value 1.0 indicating that progress reporting has finished.
  /// Progress is measured between 0.0 and 1.0.
  update_progress_signal_type update_progress_signal_;

  static const std::string EXPORT_FORMATS_C;

private:
  IsosurfacePrivateHandle private_;
};

} // end namespace Core

#endif