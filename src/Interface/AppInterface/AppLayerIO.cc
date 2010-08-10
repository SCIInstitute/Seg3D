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

// Boost includes
#include <boost/filesystem.hpp>

// Qt includes
#include <QMessageBox>
#include <QFileDialog>

// Application includes
#include <Application/LayerIO/LayerIO.h>

// Interface includes
#include <Interface/AppInterface/LayerImporterWidget.h>
#include <Interface/AppInterface/AppLayerIO.h>

namespace Seg3D
{

void AppLayerIO::ImportFiles( QMainWindow* main_window )
{
  // Step (1): Get the importer list from the LayerIO system
  LayerIO::importer_types_type importer_types = LayerIO::Instance()->get_importer_types();
  
  QStringList filters;
  for ( size_t j = 0; j < importer_types.size(); j++ )
  {
    filters << QString::fromStdString( importer_types[j] );
  }

  // Step (2): Bring up the file dialog
  QFileDialog import_dialog( main_window, QString( "Import Layer(s)... " ) );

  import_dialog.setNameFilters( filters );
  import_dialog.setAcceptMode( QFileDialog::AcceptOpen );
  import_dialog.setFileMode( QFileDialog::ExistingFiles );
  import_dialog.setViewMode( QFileDialog::Detail );
  import_dialog.exec();
  
  // Step (3): Get the selected filename and name filter
  QStringList file_list = import_dialog.selectedFiles();
  if( file_list.size() == 0) return;
  std::string filtername = import_dialog.selectedNameFilter().toStdString();
  
  std::vector< LayerImporterHandle > importers;

  for( int i = 0; i < file_list.size(); ++i )
  {

    LayerImporterHandle importer;
    if( ! ( LayerIO::Instance()->create_importer( file_list.at( i ).toStdString(), 
      importer, filtername ) ) )
    {
      std::string error_message = std::string("ERROR: No importer is available for file '") + 
        file_list.at( i ).toStdString() + std::string("'.");

      QMessageBox message_box( main_window );
      message_box.setWindowTitle( "Import Layer..." );
      message_box.addButton( QMessageBox::Ok );
      message_box.setIcon( QMessageBox::Critical );
      message_box.setText( QString::fromStdString( error_message ) );
      message_box.exec();
      return;
    }
    else
    {
      importers.push_back( importer );
    }
  }
  
  // Step (5): Open the importer dialog that issues the action to import the data file(s)
  LayerImporterWidget layer_import_dialog( importers, main_window );
  layer_import_dialog.exec();
}

void AppLayerIO::ImportFolder( QMainWindow* main_window )
{
  // Step (1): Get the importer list from the LayerIO system
  LayerIO::importer_types_type importer_types = LayerIO::Instance()->get_importer_types();

  QStringList filters;
  for ( size_t j = 0; j < importer_types.size(); j++ )
  {
    filters << QString::fromStdString( importer_types[j] );
  }

  // Step (2): Bring up the file dialog
  QString dir = QFileDialog::getExistingDirectory( main_window,  "Import Folder of Layers...",
    "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  boost::filesystem::path path = complete( boost::filesystem::path( 
    dir.toStdString().c_str(), boost::filesystem::native ) );

  // Step (3): Get the selected filename and name filter
  std::string filename = dir.toStdString();
  std::string filtername = "All Importers (*)";

  std::vector< LayerImporterHandle > importers;
  
  boost::filesystem::directory_iterator dir_end;
  for( boost::filesystem::directory_iterator dir_itr( path ); dir_itr != dir_end; ++dir_itr )
  {
    std::string file_path = ( path / dir_itr->leaf() ).string();
    LayerImporterHandle importer;
    if( ! ( LayerIO::Instance()->create_importer( file_path, importer, filtername  ) ) )
    {
      std::string error_message = std::string("ERROR: No importer is available for file '") + 
        filename + std::string("'.");

      QMessageBox message_box( main_window );
      message_box.setWindowTitle( "Import Layer..." );
      message_box.addButton( QMessageBox::Ok );
      message_box.setIcon( QMessageBox::Critical );
      message_box.setText( QString::fromStdString( error_message ) );
      message_box.exec();
    }
    else
    {
      importers.push_back( importer );
    }
  }

  // Step (5): Open the importer dialog that issues the action to import the data files
  LayerImporterWidget layer_import_dialog( importers, main_window );
  layer_import_dialog.exec();
}
  
void AppLayerIO::Export( QMainWindow* main_window )
{
  QFileDialog export_dialog( main_window, QString( "Export Layer ... " ) );

  export_dialog.setFileMode( QFileDialog::AnyFile );
  export_dialog.setViewMode( QFileDialog::Detail );
  export_dialog.setAcceptMode( QFileDialog::AcceptSave );
  export_dialog.exec();
}




} // end namespace Seg3D

