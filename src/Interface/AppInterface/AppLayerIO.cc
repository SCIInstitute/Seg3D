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
#include <QPointer>

// Core includes
#include <Core/State/Actions/ActionSet.h>

// Application includes
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionExportLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// Interface includes
#include <Interface/AppInterface/LayerImporterWidget.h>
#include <Interface/AppInterface/AppLayerIO.h>
#include <Interface/AppSegmentationExportWizard/AppSegmentationExportWizard.h>

namespace Seg3D
{

void AppLayerIO::ImportFiles( QMainWindow* main_window )
{
  // Step (1): Get the importer list from the LayerIO system
  LayerIO::importer_types_type importer_types = LayerIO::Instance()->get_importer_types();
  
  QStringList filters;
  for ( size_t j = 0; j < importer_types.size(); j++ )
  {
    filters << QString::fromStdString( importer_types[ j ] );
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
  std::vector< std::string > files;

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

      boost::filesystem::path full_filename( importer->get_filename() );
      files.push_back( full_filename.string() );
    }
  }

  importers[ 0 ]->set_file_list( files );

  // Step (5): Open the importer dialog that issues the action to import the data file(s)
  LayerImporterWidget layer_import_dialog( importers, files, main_window );
  layer_import_dialog.exec();
}

void AppLayerIO::ImportSeries( QMainWindow* main_window )
{
  // Step (1): Get the importer list from the LayerIO system
  LayerIO::importer_types_type importer_types = LayerIO::Instance()->get_series_importer_types();

  QStringList filters;
  for ( size_t j = 0; j < importer_types.size(); j++ )
  {
    filters << QString::fromStdString( importer_types[j] );
  }

  // Step (2): Bring up the file dialog
  QFileDialog import_dialog( main_window, QString( "Select a file from the series... " ) );

  import_dialog.setNameFilters( filters );
  import_dialog.setAcceptMode( QFileDialog::AcceptOpen );
  import_dialog.setFileMode( QFileDialog::ExistingFiles );
  import_dialog.setViewMode( QFileDialog::Detail );
  import_dialog.exec();

  QStringList file_list = import_dialog.selectedFiles();
  if( file_list.size() == 0) return;
  std::string filtername = import_dialog.selectedNameFilter().toStdString();

  std::vector< LayerImporterHandle > importers;
  std::vector< std::string > files;

  LayerImporterHandle importer;
  if( ! ( LayerIO::Instance()->create_importer( file_list.at( 0 ).toStdString(), 
    importer, filtername ) ) )
  {
    // IF we are unable to create an importer we pop up an error message box
    std::string error_message = std::string("ERROR: No importer is available for file '") + 
      file_list.at( 0 ).toStdString() + std::string("'.");

    QMessageBox message_box( main_window );
    message_box.setWindowTitle( "Import Layer Error." );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( QString::fromStdString( error_message ) );
    message_box.exec();
    return;
  }
  else if ( file_list.size() == 1 )
  {
    // If we are able to create an importer, we then need to figure out which files are part
    // of the series.
    
    // Step 1: we add the importer to the importers vector, in this case we only have a single
    // importer in the vector.
    importers.push_back( importer );

    // Step 2: we get the get a boost::filesystem::path version of the file name so we can 
    // take advantage of the filesystem functionality
    boost::filesystem::path full_filename( importer->get_filename() );

    // Step 3: now we want to see if we can figure out the file name pattern.  We will start by
    // checking to see if the sequence numbers are at the end of the file name.  
    std::string filename = boost::filesystem::basename( full_filename );
    
    std::string numbers( "0123456789" );
    size_t length =  filename.find_last_not_of( numbers ) + 1;

    // we check to see if we have found a series number at the end. length == 0 if we haven't.
    if( length == 0 ) return;

    std::vector< std::string > files1;

    // if length was > 0 then chances are that we found a sequence pattern
    std::string file_pattern = filename.substr( 0, length ); 

    // Step 4: parse through the directory
    if( boost::filesystem::exists( full_filename.parent_path() ) )
    {
      boost::filesystem::directory_iterator dir_end;
      for( boost::filesystem::directory_iterator dir_itr( full_filename.parent_path() ); 
        dir_itr != dir_end; ++dir_itr )
      {
        std::string potential_match = dir_itr->filename().substr( 0, length );
        if( potential_match == file_pattern )
        {
          files1.push_back( dir_itr->string() );
        }
      }
    }
    files = files1;
  }
  else
  {
    QStringList::iterator it = file_list.begin();
    QStringList::iterator it_end = file_list.end();
    while ( it != it_end )
    {
      files.push_back( (*it).toStdString() );
      ++it;
    }
    
    std::sort( files.begin(), files.end() );
    
    importers.push_back( importer );
  }

  importers[ 0 ]->set_file_list( files );

  LayerImporterWidget layer_import_dialog( importers, files, main_window );
  layer_import_dialog.exec();
}
  
void AppLayerIO::ExportLayer( QMainWindow* main_window )
{
  std::vector< LayerHandle > layer_handles;
  layer_handles.push_back( LayerManager::Instance()->get_active_layer() );
  if( !layer_handles[ 0 ] ) return;

  if( layer_handles[ 0 ]->get_type() != Core::VolumeType::DATA_E )
  {
    std::string error_message = 
      std::string( "ERROR: A Data layer is not set as the active layer" );

    QMessageBox message_box( main_window );
    message_box.setWindowTitle( "Export Layer Error." );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( QString::fromStdString( error_message ) );
    message_box.exec();
    return;
  }

  QString filename = QFileDialog::getSaveFileName( main_window, "Export Data Layer As... ",
    QString::fromStdString( PreferencesManager::Instance()->export_path_state_->get() ),
    "NRRD files (*.nrrd);;DICOM files (*.dcm)" );
    
  if( boost::filesystem::exists( boost::filesystem::path( filename.toStdString() ).parent_path() ) )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      PreferencesManager::Instance()->export_path_state_, 
      boost::filesystem::path( filename.toStdString() ).parent_path().string() );
  }
    
  std::string extension = boost::filesystem::path( filename.toStdString() ).extension(); 
  std::string exportername;
  
  if( extension == ".nrrd" ) exportername = "NRRD Exporter";
  else if( extension == ".dcm" ) exportername = "ITK Exporter";
  else return;
    
  LayerExporterHandle exporter;
  if( ! ( LayerIO::Instance()->create_exporter( exporter, layer_handles, exportername, extension ) ) )
  {
    std::string error_message = std::string("ERROR: No exporter is available for file '") + 
      filename.toStdString() + std::string("'.");

    QMessageBox message_box( main_window );
    message_box.setWindowTitle( "Import Layer..." );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( QString::fromStdString( error_message ) );
    message_box.exec();
    return;
  }
    
  ActionExportLayer::Dispatch( Core::Interface::GetWidgetActionContext(), exporter,
    filename.toStdString() );
}

void AppLayerIO::ExportSegmentation( QMainWindow* main_window )
{
  QPointer< AppSegmentationExportWizard > export_segmentation_wizard_ = 
    new AppSegmentationExportWizard( main_window );
  export_segmentation_wizard_->show();
}

} // end namespace Seg3D

