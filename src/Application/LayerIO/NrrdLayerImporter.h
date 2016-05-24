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

#ifndef APPLICATION_LAYERIO_NRRDLAYERIMPORTER_H
#define APPLICATION_LAYERIO_NRRDLAYERIMPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Core includes
#include <Core/DataBlock/NrrdData.h>
#include <Core/DataBlock/NrrdDataBlock.h>

// Application includes
#include <Application/LayerIO/LayerSingleFileImporter.h>
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{

// Forward declaration for internals of this class
class NrrdLayerImporterPrivate;
typedef boost::shared_ptr<NrrdLayerImporterPrivate> NrrdLayerImporterPrivateHandle;

/// CLASS NrrdLayerImporter
/// This class imports nrrds, the default file type supported by Seg3D. Hence it has the
/// highest priority as this importer should be used by default for nrrd files.
class NrrdLayerImporter : public LayerSingleFileImporter
{
  SEG3D_IMPORTER_TYPE( "Teem Importer", ".nrrd;.nhdr", 50 )

  // -- Constructor/Destructor --
public:
  NrrdLayerImporter();
  virtual ~NrrdLayerImporter();

  // -- Import information from file --
public:
  /// GET_FILE_INFO
  /// Get the information about the file we are currently importing.
  /// NOTE: This function often causes the file to be loaded in its entirety
  /// Hence it is best to run this on a separate thread if needed ( from the GUI ).
  virtual bool get_file_info( LayerImporterFileInfoHandle& info );

  // -- Import data from file --  
public: 
  /// GET_FILE_DATA
  /// Get the file data from the file/ file series
  /// NOTE: The information is generated again, so that hints can be processed
  virtual bool get_file_data( LayerImporterFileDataHandle& data );

  // -- Copy files --
public:
  /// GET_INPUTFILES_IMPORTER
  /// For provenance files need to be copied into the project cache. As some files need special
  /// attention: for example mhd and nhdr files actually list where there data is stored, this
  /// function can be overloaded with a specific function that copies the files. Otherwise a
  /// default implementation is given in the two derived classes.
  
  /// NOTE: This function has to be implemented as nhdr files can refer to other files on the
  /// file system. Hence copying NRRD files needs special logic, as it needs to copy the depending
  /// files as well and may need to change file references in the header files.
  virtual InputFilesImporterHandle get_inputfiles_importer();
  
  // --internals --
public:
  NrrdLayerImporterPrivateHandle private_;
  
};

} // end namespace seg3D

#endif
