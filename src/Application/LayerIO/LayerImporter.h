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

#ifndef APPLICATION_LAYERIO_LAYERIMPORTER_H
#define APPLICATION_LAYERIO_LAYERIMPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <vector>
#include <string>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Application includes
#include <Application/Project/InputFilesImporter.h>
#include <Application/LayerIO/LayerImporterFileInfo.h>
#include <Application/LayerIO/LayerImporterFileData.h>

namespace Seg3D
{

/// CLASS LayerImporter
/// This class is the base type of the importers that import layers into the program.
/// Two classes are derived from this class LayerSingleFileImporter and LayerFileSeriesImporter.
/// The first one just imports one file and the second imports a series of files.
/// These two have been split to make it easier to provide the functionality for both cases.

// Forward declarations
class LayerImporter;
class LayerImporterPrivate;
typedef boost::shared_ptr< LayerImporter > LayerImporterHandle;
typedef boost::shared_ptr< LayerImporterPrivate > LayerImporterPrivateHandle;

// Types of importers
CORE_ENUM_CLASS
(
  LayerImporterType,
  /// Importer that only imports one file
  SINGLE_FILE_E = 1,
  /// Importer that imports a series of related files
  FILE_SERIES_E = 2
)

// class definition
class LayerImporter : public boost::noncopyable
{
  // -- Constructor/Destructor --
public:
  LayerImporter();
  virtual ~LayerImporter();

  // -- Get the properties of this importer --
public:
  /// GET_NAME
  /// Name of the importer (to be listed in the file filter dialog)
  virtual std::string get_name() const = 0;

  /// GET_FILE_TYPES
  /// The file types (extensions that the importer handles)
  virtual std::string get_file_types() const = 0;

  /// GET_PRIORITY
  /// Priority flag that resolves which importer needs to be used if multiple are available for
  /// a similar file type
  virtual int get_priority() const = 0;

  /// GET_TYPE
  /// Get the type of the importer
  virtual LayerImporterType get_type() const = 0;

  // -- Error handling --
public:
  /// GET_ERROR
  /// Get the last error recorded in the importer
  std::string get_error() const;
  
  /// SET_ERROR
  /// Set the error message
  void set_error( const std::string& error );

  /// GET_WARNING
  /// Get the last warning recorded in the importer
  std::string get_warning() const;
  
  /// SET_WARNING
  /// Set the warning message
  void set_warning( const std::string& warning );

  // -- file_importer_id handling --
public:
  /// GET_INPUTFILES_ID:
  /// Get an id from the project that will be used to catalogue an import of files into the project
  InputFilesID get_inputfiles_id();
  
  // -- File tags --
public:
  /// GET_FILENAME
  /// Get the name of the file that needs to imported. If multiple files are specified,
  /// it uses the first one
  /// NOTE: This is implemented in the derived class, no need to implement this in the
  /// final importer class
  virtual std::string get_filename() const = 0;
  
  /// GET_FILENAMES
  /// Get the names of the files that need to imported
  /// NOTE: This is implemented in the derived class, no need to implement this in the
  /// final importer class
  virtual std::vector<std::string> get_filenames() const = 0;
      
  /// GET_FILE_TAG:
  /// Get the tag of the filename or directory we are scanning
  virtual std::string get_file_tag() const = 0;     
      
  // -- Check whether file header --
public:
  /// CHECK_FILES
  /// Check the file headers to see if we could import it.
  /// NOTE: This function does not need to be implemented if a check of the file extension
  /// is deemed enough. The file extension check is in the derived class that handles the filenames.
  virtual bool check_files() = 0;
  
  // -- Import information from file --
public:
  /// GET_FILE_INFO
  /// Get the information about the file we are currently importing.
  /// NOTE: This function often causes the file to be loaded in its entirety
  /// Hence it is best to run this on a separate thread if needed ( from the GUI ).
  virtual bool get_file_info( LayerImporterFileInfoHandle& info ) = 0;

  // -- Import data from file --  
public: 
  /// GET_FILE_DATA
  /// Get the file data from the file/ file series
  /// NOTE: The information is generated again, so that hints can be processed
  virtual bool get_file_data( LayerImporterFileDataHandle& data ) = 0;
  
  // -- Copy files --
public:
  /// GET_INPUTFILES_IMPORTER
  /// For provenance files need to be copied into the project cache. As some files need special
  /// attention: for example mhd and nhdr files actually list where there data is stored, this
  /// function can be overloaded with a specific function that copies the files. Otherwise a
  /// default implementation is given in the two derived classes.
  /// NOTE: This function does not need to be implemented if a check of the file extension
  /// is deemed enough. The file extension check is in the derived class that handles the filenames.
  virtual InputFilesImporterHandle get_inputfiles_importer() = 0;

  // -- Addional hints to compensate for file formats where user input is needed --
public:
  /// SET_DICOM_SWAP_XYSPACING_HINT
  /// Some dicom writers interpret X/Y spacing incorrectly. 
  /// The DICOM standard says that pixel spacing is in Y/X order, 
  /// but some scanners put it in X/Y order. Hence a hint can be given how to interpret the data
  virtual void set_dicom_swap_xyspacing_hint( bool value );

  // -- Internals of the importer --
private:
  LayerImporterPrivateHandle private_;
};

/// SEG3D_IMPORTER_TYPE:
/// This macro should be defined at the top of each importer. It renders 
/// code that allows both the class as well as the object to determine what its 
/// properties are. By defining class specific static functions the class 
/// properties can be queried without instantiating the object. On the other
/// hand you want to query these properties from the object as well, even when
/// we only have a pointer to the base object. Hence we need virtual functions
/// as well. 

/// Note: one would expect to use virtual static functions, but those are not
/// allowed in C++, hence this macro ensures a consistent definition.

#define SEG3D_IMPORTER_TYPE(importer_name,importer_file_types,importer_priority) \
  public: \
    static std::string GetName()          { return importer_name; }\
    static std::string GetFileTypes()       { return importer_file_types; }\
    static int GetPriority()            { return importer_priority; }\
  \
    \
    virtual std::string get_name() const      { return GetName(); } \
    virtual std::string get_file_types() const    { return GetFileTypes(); } \
    virtual int get_priority() const        { return GetPriority(); } \
  virtual LayerImporterType get_type() const    { return GetType(); }

} // end namespace seg3D

#endif
