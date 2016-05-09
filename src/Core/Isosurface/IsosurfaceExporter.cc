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

#include <Core/Isosurface/IsosurfaceExporter.h>
#include <Core/Geometry/Point.h>

#include <boost/filesystem.hpp>
#include <boost/shared_array.hpp>

#include <fstream>
#include <string>

namespace Core
{

boost::shared_array<float>
computeFaceNormal(const PointF& p1, const PointF& p2, const PointF& p3)
{
  // compute face normal:
  //   U = p2 - p1
  //   V = p3 - p1
  //   Ni = UyVz - UzVy
  //   Nj = UzVx - UxVz
  //   Nk = UxVy - UyVx

  VectorF U = p2 - p1;
  VectorF V = p3 - p1;

  boost::shared_array<float> normal(new float[3]);
  normal[0] = U.y() * V.z() - U.z() * V.y();
  normal[1] = U.z() * V.x() - U.x() * V.z();
  normal[2] = U.x() * V.y() - U.y() * V.x();
  return normal;
}


bool IsosurfaceExporter::ExportLegacy( const boost::filesystem::path& path,
                                       const std::string& file_prefix,
                                       const PointFVector& points,
                                       const UIntVector& faces,
                                       const FloatVector& values
                                     )
{
  // Write points to .pts file
  boost::filesystem::path points_path = path / ( file_prefix + ".pts" );
  std::ofstream pts_file( points_path.string().c_str() );
  if ( ! pts_file.is_open() )
  {
    return false;
  }

  for ( auto &pt: points )
  {
    pts_file << pt.x() << " " << pt.y() << " " << pt.z() << std::endl;
  }
  pts_file.close();

  // Write faces to .fac file
  boost::filesystem::path faces_path = path / ( file_prefix + ".fac" );
  std::ofstream fac_file( faces_path.string().c_str() );
  if ( ! fac_file.is_open() )
  {
    return false;
  }

  for ( size_t i = 0; i + 2 < faces.size(); i += 3 )
  {
    fac_file << faces[ i ] << " " <<
                faces[ i + 1 ] << " " <<
                faces[ i + 2 ] << std::endl;
  }
  fac_file.close();

  // Write values to .val file
  if ( values.size() > 0 )
  {
    boost::filesystem::path values_path = path / ( file_prefix + ".val" );
    std::ofstream val_file( values_path.string().c_str() );
    if( ! val_file.is_open() )
    {
      return false;
    }

//    for ( size_t i = 0; i < this->private_->values_.size(); i++ )
    for ( auto &value: values )
    {
      val_file << value << std::endl;
    }
    val_file.close();
  }

  return true;
}

// Legacy VTK file format (http://vtk.org/VTK/img/file-formats.pdf)
bool IsosurfaceExporter::ExportVTKASCII( const boost::filesystem::path& filename,
                                         const PointFVector& points,
                                         const UIntVector& faces
                                        )
{
  std::ofstream vtk_file( filename.string().c_str() );
  if ( ! vtk_file.is_open() )
  {
    return false;
  }

  // write header
  vtk_file << "# vtk DataFile Version 3.0\n";
  vtk_file << "vtk output\n";

  vtk_file << "ASCII\n";
  vtk_file << "DATASET POLYDATA\n";
  vtk_file << "POINTS " << points.size() << " float\n";

  for ( auto &pt : points )
  {
    vtk_file << pt.x() << " " << pt.y() << " " << pt.z() << std::endl;
  }

  unsigned int num_triangles = faces.size() / 3;
  unsigned int triangle_list_size = num_triangles * 4;

  vtk_file << "\nPOLYGONS " << num_triangles << " " << triangle_list_size << std::endl;

  for( size_t i = 0; i + 2 < faces.size(); i += 3 )
  {
    vtk_file << "3 " << faces[ i ] << " " <<
                        faces[ i + 1 ] << " " <<
                        faces[ i + 2 ] << std::endl;
  }

  vtk_file.close();

  return true;
}

// ASCII STL format: https://en.wikipedia.org/wiki/STL_(file_format)
bool IsosurfaceExporter::ExportSTLASCII( const boost::filesystem::path& filename,
                                         const std::string& name,
                                         const PointFVector& points,
                                         const UIntVector& faces
                                       )
{
  std::ofstream stl_file( filename.string().c_str() );
  if ( ! stl_file.is_open() )
  {
    return false;
  }

  const std::string delim(" ");
  const std::string indent_level1("  ");
  const std::string indent_level2("    ");
  const std::string indent_level3("      ");
  stl_file << "solid " << name << std::endl;

  for( size_t i = 0; i + 2 < faces.size(); i += 3 )
  {
    size_t vertex_index1 = faces[ i ];
    size_t vertex_index2 = faces[ i + 1 ];
    size_t vertex_index3 = faces[ i + 2 ];

    // Get vertices of face
    PointF p1 = points[ vertex_index1 ];
    PointF p2 = points[ vertex_index2 ];
    PointF p3 = points[ vertex_index3 ];

    boost::shared_array<float> normal = computeFaceNormal(p1, p2, p3);

    stl_file << indent_level1 << "facet normal " << std::fixed <<
                                 normal[0] << delim <<
                                 normal[1] << delim <<
                                 normal[2] << std::endl;
    stl_file << indent_level2 << "outer loop" << std::endl;
    stl_file << indent_level3 << "vertex " << std::fixed << p1.x() << delim <<
                                                            p1.y() << delim <<
                                                            p1.z() << std::endl;
    stl_file << indent_level3 << "vertex " << std::fixed << p2.x() << delim <<
                                                            p2.y() << delim <<
                                                            p2.z() << std::endl;
    stl_file << indent_level3 << "vertex " << std::fixed << p3.x() << delim <<
                                                            p3.y() << delim <<
                                                            p3.z() << std::endl;
    stl_file << indent_level2 << "endloop" << std::endl;
    stl_file << indent_level1 << "endfacet" << std::endl;
  }
  stl_file << "endsolid" << std::endl;

  stl_file.close();
  
  return true;
}

// Binary STL format: https://en.wikipedia.org/wiki/STL_(file_format)
bool IsosurfaceExporter::ExportSTLBinary( const boost::filesystem::path& filename,
                                          const std::string& name,
                                          const PointFVector& points,
                                          const UIntVector& faces
                                        )
{
  // 80 byte header, usually ignored
  const unsigned short STL_HEADER_LENGTH = 80;
  // STL binary contains unsigned ints, floats
  const unsigned short STL_FIELD_LENGTH = 4;
  const unsigned short POINT_LEN = 3;
  const unsigned short FIELD_LEN = 12;
  const unsigned short ATTRIBUTE_BYTE_COUNT = 2;

  std::ofstream stl_file( filename.string().c_str() );
  if ( ! stl_file.is_open() )
  {
    return false;
  }

  std::string header("STL header: Seg3D isosurface to STL Binary export");
  char* headerBuffer = const_cast<char*>(header.c_str());
  stl_file.write(headerBuffer, STL_HEADER_LENGTH);

  unsigned int numTriangles = faces.size() / 3;
  stl_file.write(reinterpret_cast<char*>(&numTriangles), STL_FIELD_LENGTH);

  // 0 is an acceptable value for this field
  unsigned short byteAttributeCount = 0;

  for ( size_t i = 0; i + 2 < faces.size(); i += 3 )
  {
    size_t vertex_index1 = faces[ i ];
    size_t vertex_index2 = faces[ i + 1 ];
    size_t vertex_index3 = faces[ i + 2 ];

    // Get vertices of face
    PointF p1 = points[ vertex_index1 ];
    PointF p2 = points[ vertex_index2 ];
    PointF p3 = points[ vertex_index3 ];

    boost::shared_array<float> normal = computeFaceNormal(p1, p2, p3);
    stl_file.write(reinterpret_cast<char*>(normal.get()), FIELD_LEN);

    boost::shared_array<float> vertex1(new float[POINT_LEN]);
    vertex1[0] = p1.x();
    vertex1[1] = p1.y();
    vertex1[2] = p1.z();
    stl_file.write(reinterpret_cast<char*>(vertex1.get()), FIELD_LEN);

    boost::shared_array<float> vertex2(new float[POINT_LEN]);
    vertex2[0] = p2.x();
    vertex2[1] = p2.y();
    vertex2[2] = p2.z();
    stl_file.write(reinterpret_cast<char*>(vertex2.get()), FIELD_LEN);

    boost::shared_array<float> vertex3(new float[POINT_LEN]);
    vertex3[0] = p3.x();
    vertex3[1] = p3.y();
    vertex3[2] = p3.z();
    stl_file.write(reinterpret_cast<char*>(vertex3.get()), FIELD_LEN);

    stl_file.write(reinterpret_cast<char*>(&byteAttributeCount), ATTRIBUTE_BYTE_COUNT);
  }
  stl_file.close();

  return true;
}

}
