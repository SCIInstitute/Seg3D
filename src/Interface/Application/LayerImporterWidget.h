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

#ifndef INTERFACE_APPLICATION_LAYERIMPORTERWIDGET_H
#define INTERFACE_APPLICATION_LAYERIMPORTERWIDGET_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#ifndef Q_MOC_RUN

// Qt includes
#include <QtCore/QPointer>
#include <QDialog>

// boost includes
#include <boost/smart_ptr.hpp>

// Application Includes
#include <Application/LayerIO/LayerImporter.h>

#endif

namespace Seg3D
{

// Forward declaration for class describing the internals of the ImporterWidget
class LayerImporterWidgetPrivate;
typedef boost::shared_ptr< LayerImporterWidgetPrivate > LayerImporterWidgetPrivateHandle;

// CLASS LAYERIMPORTERWIDGET:
// Dialog that is opened after the user has selected a filename

class LayerImporterWidget : public QDialog
{
  // Needed to make it a Qt object
Q_OBJECT

  // -- constructor/destructor --
public:
  typedef QPointer< LayerImporterWidget > qpointer_type;

  LayerImporterWidget( std::vector< LayerImporterHandle > importers, QWidget *parent = 0 );
  virtual ~LayerImporterWidget();
    
private Q_SLOTS:

  /// SLOTS: These functions connect to the buttons for importing the data as a specific
  /// layer type.
  void set_type( int file_type );

  /// SLOT IMPORT: execute this function when the import button is selected
  void import();
  
private:
  /// UPDATE_ICONS:
  /// Update the icons to reflect active mode
  void setup_ui();
  
  /// LIST_IMPORT_OPTIONS:
  /// Prompt for the import options that are available
  void list_import_options();

  /// SCAN_FIRST_FILE:
  /// Scan the first file
  void scan_first_file();

private:
  /// The private information for rendering the widget
  LayerImporterWidgetPrivateHandle private_;
  
private:  
  /// SCANFIRSTFILE:
  /// Function that needs to be run asynchronously to scan the file contents
  static void ScanFirstFile( qpointer_type qpointer ); 

  /// LIST_IMPORT_OPTIONS:
  /// List the import options in the dialog
  static void ListImportOptions ( qpointer_type qpointer, LayerImporterHandle importer );
  
  /// REPORT_IMPORT_ERROR:
  /// Close the dialog and show an error dialog
  static void ReportImportError( qpointer_type qpointer, LayerImporterHandle importer );
  
};

} //endnamespace Seg3D

#endif
