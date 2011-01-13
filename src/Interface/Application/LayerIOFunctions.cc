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

// STL includes
#include <algorithm>
#include <utility>
#include <vector>
#include <set>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/algorithm/minmax_element.hpp>

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
#include <Interface/Application/LayerImporterWidget.h>
#include <Interface/Application/LayerIOFunctions.h>
#include <Interface/Application/SegmentationExportWizard.h>

namespace Seg3D
{

void LayerIOFunctions::ImportFiles( QMainWindow* main_window, std::string file_to_open )
{
  QStringList file_list;
  std::string filtername;
  if( file_to_open != "" )
  {
    file_list << QString::fromStdString( file_to_open );
    filtername = "All Importers (*)";
  }
  else
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
    if( !import_dialog.exec() ) return;
    
    // Step (3): Get the selected filename and name filter
    file_list = import_dialog.selectedFiles();
    if( file_list.size() == 0) return;
    filtername = import_dialog.selectedNameFilter().toStdString();
  }

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

void LayerIOFunctions::ImportSeries( QMainWindow* main_window )
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
  if( !import_dialog.exec() ) return;

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
    
    if( !boost::filesystem::exists( full_filename ) ) return;
    
    // Step 3: now we want to see if we can figure out the file name pattern.  We will start by
    // checking to see if the sequence numbers are at the end of the file name.  
    std::string filename = boost::filesystem::basename( full_filename );
    
    std::vector<std::pair<size_t,size_t> > numbers;
    
    size_t j = 0;
    while ( j < filename.size() )
    {
      size_t start, end;
      while ( j < filename.size() && ( filename[ j ] < '0' || filename[ j ] > '9' ) ) j++;
      start = j;
      while ( j < filename.size() && ( filename[ j ] >= '0' && filename[ j ] <= '9' ) ) j++;
      end = j;
      
      if ( start != end ) numbers.push_back( std::make_pair<size_t,size_t>( start, end ) );
    }
    
    std::vector<std::string> dir_files;
    std::vector< boost::filesystem::path > dir_full_filenames;
    if( boost::filesystem::exists( full_filename.parent_path() ) )
    {
      boost::filesystem::directory_iterator dir_end;
      for( boost::filesystem::directory_iterator dir_itr( full_filename.parent_path() ); 
        dir_itr != dir_end; ++dir_itr )
      {
        dir_full_filenames.push_back( *dir_itr );
        dir_files.push_back( boost::filesystem::basename( dir_itr->filename() ) );
      }
    }

    size_t index = 0;
    size_t num_files = 0;

    for ( size_t j = 0; j < numbers.size(); j++ )
    {
      size_t start = numbers[ j ].first;
      size_t end = filename.size() - numbers[ j ].second;
      
      std::string filename_prefix = filename.substr( 0, start );
      std::string filename_postfix = filename.substr( filename.size() - end );
      
      std::set<size_t> indices;
      
      for ( size_t k = 0; k < dir_files.size(); k++ )
      {
        if ( dir_files[ k ].substr( 0, start ) == filename_prefix && 
          dir_files[ k ].substr( dir_files[ k ].size() - end ) == filename_postfix )
        {
          size_t val;
          if ( Core::ImportFromString( dir_files[ k ].substr( start, 
            dir_files[ k ].size() - end - start ), val ) )
          {
            indices.insert( val );
          }
        }
      }
      
      if ( num_files < indices.size() )
      {
        index = j;
        num_files = indices.size();
      }
    }

    if ( numbers.size() && num_files ) 
    {
      std::vector<std::pair<size_t,size_t> > order( num_files );
      
      size_t start = numbers[ index ].first;
      size_t end = filename.size() - numbers[ index ].second;
      
      std::string filename_prefix = filename.substr( 0, start );
      std::string filename_postfix = filename.substr( filename.size() - end );
      
      size_t  j = 0;
      
      for ( size_t k = 0; k < dir_files.size(); k++ )
      {
        if ( dir_files[ k ].substr( 0, start ) == filename_prefix && 
          dir_files[ k ].substr( dir_files[ k ].size() - end ) == filename_postfix )
        {
          size_t val;
          if ( Core::ImportFromString( dir_files[ k ].substr( start, 
            dir_files[ k ].size() - end - start ), val ) )
          {
            order[ j ].first = val;
            order[ j ].second = j;
            j++;
            files.push_back( dir_full_filenames[ k ].string() );
          }
        }
      }
    
      std::sort( order.begin(), order.end() );
      
      std::vector<std::string> old_files = files;
      for ( size_t j = 0; j < order.size(); j++ )
      {
        files[ j ] = old_files[ order[ j ].second ];
      }   
    }
    else
    {
      files.push_back( full_filename.string() );
    }
    
  }
  else
  {
    QStringList::iterator it = file_list.begin();
    QStringList::iterator it_end = file_list.end();
    
    std::vector<std::vector<size_t> > file_numbers;
    while ( it != it_end )
    {
      std::string filename = (*it).toStdString();
      files.push_back( filename );
      
      for ( size_t j = 0; j < filename.size(); j++ )
      {
        if ( filename[ j ] < '0' || filename[ j ] > '9' ) filename[ j ] = ' ';
      }

      std::vector<size_t> filename_numbers;
      Core::ImportFromString( filename, filename_numbers );
      file_numbers.push_back( filename_numbers );
      ++it;
    }
    
    size_t max_size = 0;
    for ( size_t j = 0; j < file_numbers.size(); j++ )
    {
      if ( file_numbers[ j ].size() > max_size ) max_size = file_numbers[ j ].size();
    }

    if ( max_size )
    {
      std::vector<std::set<size_t> > numbers( 2*max_size );
      
      for ( size_t j = 0; j < file_numbers.size(); j++ )
      {
        for ( size_t k = 0; k < file_numbers[ j ].size(); k++ )
        {
          numbers[ k ].insert( file_numbers[ j ][ k ] );
          numbers[ max_size + file_numbers[ j ].size() - 1 - k ].insert( file_numbers[ j ][ k ] );
        }
      }

      size_t index = 0;
      std::pair< std::set<size_t>::iterator, std::set<size_t>::iterator > min_max = 
        boost::minmax_element( numbers[ 0 ].begin(), numbers[ 0 ].end() );
      size_t index_range = (*min_max.second) - (*min_max.first);
      size_t index_size = numbers[ 0 ].size();
      
      for ( size_t j = 0; j < numbers.size(); j++ )
      {
        min_max = boost::minmax_element( numbers[ j ].begin(), numbers[ j ].end() );
        size_t range = (*min_max.second) - (*min_max.first);
        size_t size = numbers[ j ].size();
        
        if ( ( size > index_size ) || ( size == index_size && range < index_range ) )
        {
          index = j;
          index_size = size;
          index_range = range;
        }
      }
      
      if ( index_size != files.size() )
      {
        // Not enough for sorting, use alphabetical order
        std::sort( files.begin(), files.end() );
      }
      else
      {
        std::vector<std::pair<size_t,size_t> > order( files.size() );
        if ( index < max_size )
        {
          for ( size_t j = 0; j < order.size(); j++ )
          {
            order[ j ].second = j;
            order[ j ].first = file_numbers[ j ][ index ];
          }
        }
        else
        {
          for ( size_t j = 0; j < order.size(); j++ )
          {
            order[ j ].second = j;
            order[ j ].first = file_numbers[ j ][ file_numbers[ j ].size() - 1 - ( index - max_size ) ];
          }
        }
        
        std::sort( order.begin(), order.end() );
        
        std::vector<std::string> old_files = files;
        for ( size_t j = 0; j < order.size(); j++ )
        {
          files[ j ] = old_files[ order[ j ].second ];
        }
      }
    }
    else
    {
      // no numbers for sorting use alphabetical order
      std::sort( files.begin(), files.end() );
    }
    
    importers.push_back( importer );
  }

  importers[ 0 ]->set_file_list( files );

  LayerImporterWidget layer_import_dialog( importers, files, main_window, true );
  layer_import_dialog.exec();
}
  
void LayerIOFunctions::ExportLayer( QMainWindow* main_window )
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

  boost::filesystem::path file_path = boost::filesystem::path( 
    PreferencesManager::Instance()->export_path_state_->get() ) / layer_handles[ 0 ]->get_layer_name();

  QString filename = QFileDialog::getSaveFileName( main_window, "Export Data Layer As... ",
    QString::fromStdString( file_path.string() ),
    "NRRD files (*.nrrd);;DICOM files (*.dcm);;TIFF files (*.tiff);;PNG files (*.png)" );
  
  if( filename == "" ) return;
  
  if( boost::filesystem::exists( boost::filesystem::path( filename.toStdString() ).parent_path() ) )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      PreferencesManager::Instance()->export_path_state_, 
      boost::filesystem::path( filename.toStdString() ).parent_path().string() );
  }
    
  std::string extension = boost::filesystem::path( filename.toStdString() ).extension(); 
  std::string exportername;
  
  if( extension == ".nrrd" ) exportername = "NRRD Exporter";
  else if( extension != "" ) exportername = "ITK Data Exporter";
    
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

void LayerIOFunctions::ExportSegmentation( QMainWindow* main_window )
{
  QPointer< SegmentationExportWizard > export_segmentation_wizard_ = 
    new SegmentationExportWizard( main_window );
  export_segmentation_wizard_->show();
}

} // end namespace Seg3D
