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

#ifndef APPLICATION_LAYERIO_LAYERFILESERIESIMPORTER_H
#define APPLICATION_LAYERIO_LAYERFILESERIESIMPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Application includes
#include <Application/LayerIO/LayerImporter.h>

namespace Seg3D
{

/// CLASS LayerSingleFileImporter
/// Base class for importer for a single file
class LayerFileSeriesImporter;
class LayerFileSeriesImporterPrivate;

typedef boost::shared_ptr< LayerFileSeriesImporter > LayerFileSeriesImporterHandle;
typedef boost::shared_ptr< LayerFileSeriesImporterPrivate > LayerFileSeriesImporterPrivateHandle;

class LayerFileSeriesImporter : public LayerImporter
{
  // -- Constructor/Destructor --
public:
  /// Create an importer for a given file
  LayerFileSeriesImporter();
  virtual ~LayerFileSeriesImporter();

  // -- Filename accessor --
public:
  /// GET_FILENAME
  /// Get the name of the file that needs to imported
  virtual std::string get_filename() const;
  
  /// GET_FILENAMES
  /// Get the name of the files that need to imported
  virtual std::vector< std::string > get_filenames() const;

  /// GET_FILE_TAG:
  /// Get the tag of the filename or directory we are scanning
  virtual std::string get_file_tag() const;   

protected:
  friend class LayerIO;
  /// SET_FILENAMES
  /// Set the name of the file that needs to be importerd
  void set_filenames( const std::vector< std::string >& filename );
      
public:
  /// CHECK_FILES
  /// Check the file headers to see if we could import it.
  virtual bool check_files();
  
  /// GET_INPUTFILES_IMPORTER
  /// For provenance files need to be copied into the project cache. As some files need special
  /// attention: for example mhd and nhdr files actually list where there data is stored, this
  /// function can be overloaded with a specific function that copies the files. Otherwise a
  /// default implementation is given in the two derived classes.
  virtual InputFilesImporterHandle get_inputfiles_importer();
  
  // -- internals --
private:
  LayerFileSeriesImporterPrivateHandle private_;
  
  // -- importer type --
public:
  static LayerImporterType GetType();
};

} // end namespace seg3D

#endif
  
