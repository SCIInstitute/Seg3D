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

#ifndef INTERFACE_APPLICATION_LAYERIOFUNCTIONS_H
#define INTERFACE_APPLICATION_LAYERIOFUNCTIONS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

#include <QMainWindow>

#endif

namespace Seg3D
{

class LayerIOFunctions
{
public:
  /// IMPORTSINGLEFILE:
  /// Import a layer into the LayerManager from a single file, returns its success/failure
  static bool ImportFiles( QMainWindow* main_window, const std::string& file_to_open );

  /// IMPORTSERIES:
  /// Import a data volume from a series of files using the ITK importer.
  static void ImportSeries( QMainWindow* main_window );


  /// IMPORTLARGEVOLUME:
  static bool ImportLargeVolume( QMainWindow* main_window );

  /// EXPORTLAYER:
  /// Export the current layer to file
  static void ExportLayer( QMainWindow* main_window );
  
  /// EXPORTSEGMENTATION:
  /// Export selected mask layers to file
  static void ExportSegmentation( QMainWindow* main_window );

  /// EXPORTISOSURFACE
  /// Export isosurface generated from selected mask layer to file
  static void ExportIsosurface( QMainWindow* main_window );
};

} // end namespace Seg3D

#endif
