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

#ifndef CORE_ISOSURFACE_ISOSURFACEEXPORTER_H
#define CORE_ISOSURFACE_ISOSURFACEEXPORTER_H

#include <Core/Isosurface/Isosurface.h>

namespace Core
{

class IsosurfaceExporter
{
  friend class Isosurface;

  static bool ExportLegacy( const boost::filesystem::path& path,
                            const std::string& file_prefix,
                            const PointFVector& points,
                            const UIntVector& faces,
                            const FloatVector& values
                          );

  static bool ExportOBJ( const boost::filesystem::path& filename,
                              const PointFVector& points,
                              const UIntVector& faces
                            );
    
  static bool ExportVTKASCII( const boost::filesystem::path& filename,
                              const PointFVector& points,
                              const UIntVector& faces
                            );

  static bool ExportSTLASCII( const boost::filesystem::path& filename,
                              const std::string& name,
                              const PointFVector& points,
                              const UIntVector& faces
                            );

  static bool ExportSTLBinary( const boost::filesystem::path& filename,
                               const std::string& name,
                               const PointFVector& points,
                               const UIntVector& faces
                             );
};

}



#endif
