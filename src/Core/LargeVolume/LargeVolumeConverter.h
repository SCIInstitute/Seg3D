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

#ifndef CORE_LARGEVOLUME_LARGEVOLUMECONVERTER_H
#define CORE_LARGEVOLUME_LARGEVOLUMECONVERTER_H

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>


// Geometry
#include <Core/Geometry/IndexVector.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>
#include <Core/DataBlock/DataType.h>
#include <Core/DataBlock/DataBlock.h>

#include <Core/LargeVolume/LargeVolumeSchema.h>



namespace Core
{

// Internals are separated from the interface
class LargeVolumeConverterPrivate;
typedef boost::shared_ptr< LargeVolumeConverterPrivate > LargeVolumeConverterPrivateHandle;

class LargeVolumeConverter;
typedef boost::shared_ptr< LargeVolumeConverter > LargeVolumeConverterHandle;


class LargeVolumeConverter
{
  // -- constructor --
public:
  LargeVolumeConverter();

  // -- phase 1 functions --
public:
  /// SET_OUTPUT_DIR
  /// Set the output directory for the file
  void set_output_dir( const boost::filesystem::path& dir );

  /// SET_FIRST_FILE
  // Set first file of the image series
  void set_first_file( const boost::filesystem::path& file );

  /// SET_SCHEMA_PARAMETERS
  /// Upload parameters for schema
  void set_schema_parameters( const Vector& spacing, const Point& origin, const IndexVector& brick_size, size_t overlap );

  /// RUN_PHASE1
  /// Check files and determine size
  bool run_phase1( std::string& error );

  /// SET_MEM_LIMIT
  /// How much meory to devote to the conversion process
  void set_mem_limit( long long mem_limit );

  /// RUN_PHASE2
  /// Downsample and build bricks
  bool run_phase2( std::string& error );

    /// RUN_PHASE3
    /// Compress bricks
    bool run_phase3( std::string& error );

  /// GET_SCHEMA
  /// Get information about bricking schema
  LargeVolumeSchemaHandle get_schema() const;

  // -- internals --
private:
  LargeVolumeConverterPrivateHandle private_;

};

} // end namespace Core

#endif
