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

// Application includes
#include <Application/LayerIO/LayerIO.h>

// Interface includes
#include <Interface/AppInterface/AppLayerIO.h>

namespace Seg3D
{

void AppLayerIO::Import( QMainWindow* main_window )
{
  // Step (1): Get the importer list from the LayerIO system
  LayerIO::importer_types_type importer_types = LayerIO::Instance()->get_importer_types();
  
  QStringList filters;
  for ( size_t j = 0; j < importer_types.size(); j++ )
  {
    filters << QString::fromStdString( importer_types[j] );
  }
  filters << "All Importers (*)";

  // Step (2): Bring up the file dialog
  QFileDialog import_dialog( main_window, QString( "Import Layer ... " ) );

  import_dialog.setNameFilters( filters );
  import_dialog.setAcceptMode( QFileDialog::AcceptOpen );
  import_dialog.setFileMode( QFileDialog::ExistingFile );
  import_dialog.setViewMode( QFileDialog::Detail );
  import_dialog.exec();
  
  // Step (3): Get the selected filename
  QStringList file_list = import_dialog.selectedFiles();
  if ( file_list.size() == 0) return;

  std::string filename = file_list[0].toStdString();
  
  
  
}
  
void AppLayerIO::Export( QMainWindow* main_window )
{
  QFileDialog* export_dialog = new QFileDialog( main_window, 
    QString( "Export Layer" ));

  export_dialog->setFileMode( QFileDialog::AnyFile );
  export_dialog->setViewMode( QFileDialog::Detail );
  export_dialog->setAcceptMode( QFileDialog::AcceptSave );
  export_dialog->exec();
}

} // end namespace Seg3D

