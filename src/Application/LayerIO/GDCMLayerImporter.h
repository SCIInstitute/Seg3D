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

#ifndef APPLICATION_LAYERIO_GDCMLAYERIMPORTER_H
#define APPLICATION_LAYERIO_GDCMLAYERIMPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif 

// Application includes
#include <Application/LayerIO/LayerFileSeriesImporter.h>

namespace Seg3D
{

class GDCMLayerImporterPrivate;
typedef boost::shared_ptr< GDCMLayerImporterPrivate > GDCMLayerImporterPrivateHandle;

class GDCMLayerImporter : public LayerFileSeriesImporter
{
  SEG3D_IMPORTER_TYPE( "GDCM Dicom Importer", "*;.dcm;.dicom;.ima;.DCM;.DICOM;.IMA", 25 )
  // -- Constructor/Destructor --
public:
  /// Construct a new layer file importer
  GDCMLayerImporter();
  virtual ~GDCMLayerImporter();

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
  
  // -- Addional hints to compensate for file formats where user input is needed --
public:
  /// SET_DICOM_SWAP_XYSPACING_HINT
  /// Some dicom writers interpret X/Y spacing incorrectly. 
  /// The DICOM standard says that pixel spacing is in Y/X order, 
  /// but some scanners put it in X/Y order. Hence a hint can be given how to interpret the data
  virtual void set_dicom_swap_xyspacing_hint( bool value ); 

private:
  GDCMLayerImporterPrivateHandle private_;
};

} // end namespace seg3D

#endif
