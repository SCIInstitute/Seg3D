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

#ifndef APPLICATION_PROJECT_INPUTFILESIMPORTER_H
#define APPLICATION_PROJECT_INPUTFILESIMPORTER_H

// STL includes
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Utils/Lockable.h>

// Application includes
#include <Application/Provenance/Provenance.h>

namespace Seg3D
{


// Forward declaration
class InputFilesImporter;
class InputFilesImporterPrivate;

typedef boost::shared_ptr<InputFilesImporter> InputFilesImporterHandle;
typedef boost::shared_ptr<InputFilesImporterPrivate> InputFilesImporterPrivateHandle;

// Class definition
class InputFilesImporter : public Core::Lockable
{
  // -- constructor / destructor --
public:
  InputFilesImporter( InputFilesID inputfiles_id );
  virtual ~InputFilesImporter();

  // --Add files that need to be copied --
public:

  /// ADD_FILENAME
  /// Add the name and path a file that needs to be copied
  void add_filename( const boost::filesystem::path& filename );
  
  /// SET_COPY_FILE_FUNCTION
  /// Set the function that will copy the file
  void set_copy_file_function( boost::function
    < bool( const boost::filesystem::path&, const boost::filesystem::path& ) > copy_file_function );
  
  /// GET_INPUTFILE_ID
  /// Get the inputfile_id that was assigned to the series of files
  InputFilesID get_inputfiles_id();
  
  /// COPY_FILES
  /// Copy the files to the destination location inside the project
  bool copy_files( boost::filesystem::path& project_cache_path ); 
        
  // -- internals --
public:
  InputFilesImporterPrivateHandle private_;
};

} // end namespace seg3D

#endif
