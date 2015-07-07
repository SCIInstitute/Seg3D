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
#include <boost/thread.hpp>

// Qt includes
#include <QtGui/QDesktopWidget>
#include <QtGui/QMessageBox>

// Core includes
#include <Core/Utils/Log.h>

// Application includes
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerIO/Actions/ActionImportLayer.h>
#include <Application/LayerIO/Actions/ActionImportSeries.h>

// QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtDoubleClickPushButton.h>

// Interface Includes
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/LayerImporterWidget.h>

// Qt Gui Includes
#include "ui_LayerImporterWidget.h"

namespace Seg3D
{

class LayerImporterWidgetPrivate
{
public:
  LayerImporterWidgetPrivate()
  {
  }
  
  Ui::LayerImporterWidget ui_;
  
  QtUtils::QtDoubleClickPushButton *data_volume_button_;
  QtUtils::QtDoubleClickPushButton *mask_single_button_;
  QtUtils::QtDoubleClickPushButton *mask_1234_button_;
  QtUtils::QtDoubleClickPushButton *mask_1248_button_;

  QButtonGroup *type_button_group_;
  QButtonGroup *itk_import_type_button_group_;

  // The importers that were provided for this data
  std::vector< LayerImporterHandle > importers_;
  
  // File information for the first file
  LayerImporterFileInfoHandle info_;
  
  // The current active mode
  std::string mode_;
};


LayerImporterWidget::LayerImporterWidget( std::vector< LayerImporterHandle > importers, 
  QWidget *parent ) :
  QDialog( parent ),
  private_( new LayerImporterWidgetPrivate )
{
  // Copy importers to private class
  this->private_->importers_ = importers;
  // Set data as default importer
  this->private_->mode_ = LayerIO::DATA_MODE_C;

  this->setup_ui();

  // Tell the user which file we are scanning and start the actual scanning process.
  // The first file is scanned to use as a template for importing the others.
  // The scan is run on a separate thread, as loading can take a while and some
  // importers need to scan the full file to read header information.
  if ( this->private_->importers_.size() )
  {
    this->private_->ui_.file_name_label_->setText( QString::fromUtf8( "Scanning: " ) +
      QString::fromStdString( this->private_->importers_[ 0 ]->get_file_tag() ) );
    boost::thread( boost::bind( &LayerImporterWidget::ScanFirstFile, qpointer_type( this ) ) );
  }
  
  this->private_->ui_.swap_x_y_spacing_widget_layout_->setAlignment( Qt::AlignCenter );
  this->private_->ui_.swap_x_y_spacing_widget_->hide();
  this->private_->ui_.series_warning_widget_->hide();

  // We warn the user if only one file was selected and it is a series importer. Most likely
  // the scanning of similar files failed.
  if( importers.size() > 0 && importers[ 0 ]->get_type() == LayerImporterType::FILE_SERIES_E )
  {
    if ( importers[ 0 ]->get_filenames().size() < 2 )
    {
      this->private_->ui_.series_warning_widget_->setVisible( true );
    }
  } 
}

LayerImporterWidget::~LayerImporterWidget()
{
}

void LayerImporterWidget::setup_ui()
{
  // Step (1): Ensure it will be the only focus in the program
  this->setWindowModality(  Qt::ApplicationModal );

  // Step (2): Make a new LayerImporterWidgetPrivateHandle object
  this->private_->ui_.setupUi( this );

  // Step (3): Add the buttons
  this->private_->type_button_group_ = new QButtonGroup( this );
  this->private_->type_button_group_->setExclusive( true );

  this->private_->data_volume_button_ = new QtUtils::QtDoubleClickPushButton( this->private_->ui_.data_ );
  this->private_->data_volume_button_->setObjectName( "data_volume_button" );
  this->private_->data_volume_button_->setFlat( true );
  this->private_->data_volume_button_->setCheckable( true );
  this->private_->type_button_group_->addButton( this->private_->data_volume_button_, 0 );

  this->private_->mask_single_button_ = new QtUtils::QtDoubleClickPushButton( this->private_->ui_.single_mask_ );
  this->private_->mask_single_button_->setObjectName( "mask_single_button" );
  this->private_->mask_single_button_->setFlat( true );
  this->private_->mask_single_button_->setCheckable( true );
  this->private_->type_button_group_->addButton( this->private_->mask_single_button_, 1 );

  this->private_->mask_1234_button_ = new QtUtils::QtDoubleClickPushButton( this->private_->ui_.bitplane_mask_ );
  this->private_->mask_1234_button_->setObjectName( "mask_1234_button" );
  this->private_->mask_1234_button_->setFlat( true );
  this->private_->mask_1234_button_->setCheckable( true );
  this->private_->type_button_group_->addButton( this->private_->mask_1234_button_, 2 );

  this->private_->mask_1248_button_ = new QtUtils::QtDoubleClickPushButton( this->private_->ui_.label_mask_ );
  this->private_->mask_1248_button_->setObjectName( "mask_1248_button" );
  this->private_->mask_1248_button_->setFlat( true );
  this->private_->mask_1248_button_->setCheckable( true );
  this->private_->type_button_group_->addButton( this->private_->mask_1248_button_, 3 );

  // Step (4): Hide the parts of the UI that cannot be used yet
  this->private_->ui_.importer_options_->hide();
  this->private_->ui_.file_name_table_->hide();
  this->private_->ui_.scanning_file_->hide();

  this->resize( 10, 10 );
  this->private_->ui_.import_button_->setEnabled( false );

  // Step (5): Activate the cancel button
  connect( this->private_->ui_.cancel_button_, SIGNAL( released() ),
    this, SLOT( reject() ) );

  this->setStyleSheet( StyleSheet::LAYERIO_C );


  // Step (6): Show Scanning Widget then Asynchronously scan file, so UI stays interactive
  this->private_->ui_.scanning_file_->show();
}
  
void LayerImporterWidget::set_type( int file_type )
{
  switch( file_type )
  {
  case 0:
    this->private_->mode_ = LayerIO::DATA_MODE_C;
    break;

  case 1:
    this->private_->mode_ = LayerIO::SINGLE_MASK_MODE_C;
    break;

  case 2:
    this->private_->mode_ = LayerIO::BITPLANE_MASK_MODE_C;
    break;

  case 3:
    this->private_->mode_ = LayerIO::LABEL_MASK_MODE_C;
    break;
  default:
    break;
  }
}

void LayerImporterWidget::import()
{
  // Run through the importers and send an action to the application thread
  for( size_t i = 0; i < this->private_->importers_.size(); ++i )
  {   
    this->private_->importers_[ i ]->set_dicom_swap_xyspacing_hint( 
      this->private_->ui_.swap_spacing_checkbox_->isChecked() );
    
    // File series have a different action hence dispatch that one here
    if( this->private_->importers_[ i ]->get_type() == LayerImporterType::FILE_SERIES_E )
    {
      ActionImportSeries::Dispatch( Core::Interface::GetWidgetActionContext(), 
        this->private_->importers_[ i ], this->private_->mode_ );
    }
    else
    {
      // Single file importer path.
      ActionImportLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
        this->private_->importers_[ i ], this->private_->mode_ );
    } 
  }
  this->accept();
}

void LayerImporterWidget::list_import_options()
{
  // Security check: this one should have been set, if not just fail.
  if ( ! this->private_->info_ ) return;

  // Do not update immediately, but consolidate changes.
  this->setUpdatesEnabled( false );

  // We are done scanning, so we hide the scanning widget.
  this->private_->ui_.scanning_file_->hide();

  // Show the data importer option
  this->private_->ui_.data_->show();
  this->private_->data_volume_button_->setMinimumSize( this->private_->ui_.data_->size() );
  this->private_->data_volume_button_->setChecked( this->private_->mode_ == LayerIO::DATA_MODE_C );
    
  if ( this->private_->info_->get_mask_compatible() )
  {
    // Show the mask options.
    this->private_->ui_.label_mask_->show();
    this->private_->ui_.bitplane_mask_->show();
    this->private_->ui_.single_mask_->show();

    // Ensure that the widgets have the right size
    this->private_->mask_1234_button_->setMinimumSize( this->private_->ui_.bitplane_mask_->size() );
    this->private_->mask_1234_button_->setChecked( this->private_->mode_ == LayerIO::LABEL_MASK_MODE_C );
    
    this->private_->mask_1248_button_->setMinimumSize( this->private_->ui_.label_mask_->size());
    this->private_->mask_1248_button_->setChecked( this->private_->mode_ == LayerIO::BITPLANE_MASK_MODE_C );

    this->private_->mask_single_button_->setMinimumSize( this->private_->ui_.single_mask_->size() );
    this->private_->mask_single_button_->setChecked( this->private_->mode_ == LayerIO::SINGLE_MASK_MODE_C );
  }
  else
  {
    // Otherwise hide the mask buttons
    this->private_->ui_.label_mask_->hide();
    this->private_->ui_.bitplane_mask_->hide();
    this->private_->ui_.single_mask_->hide();
  }

  if ( this->private_->info_->get_file_type() == "dicom" )
  {
    this->private_->ui_.swap_x_y_spacing_widget_->show();
  }

  this->private_->ui_.file_name_table_->show();

  //Step (3): Add information from importer to the table
  int row = 0;
  for ( size_t j = 0; j < this->private_->importers_.size(); j++ )
  {
    std::vector<std::string> filenames = this->private_->importers_[ j ]->get_filenames();
  
    for( size_t i = 0; i < filenames.size(); i++, row++ )
    {
      boost::filesystem::path path( filenames[ i ] );
      QTableWidgetItem *new_item = new QTableWidgetItem( QString::fromStdString( 
          path.filename().string() ) );
      this->private_->ui_.file_name_table_->insertRow( row );
      this->private_->ui_.file_name_table_->setItem( row, 0, new_item );
    }
  }

  this->private_->ui_.file_name_table_->verticalHeader()->resizeSection( 0, 24 );

  // Step (4): connect the buttons
  connect( this->private_->type_button_group_, SIGNAL( buttonClicked( int ) ), 
    this, SLOT( set_type( int ) ) );

  connect( this->private_->ui_.import_button_, SIGNAL( clicked() ),
    this, SLOT( import() ) );
    
  this->private_->data_volume_button_->set_double_click_function( 
    boost::bind( &LayerImporterWidget::import, this ) );
  this->private_->mask_single_button_->set_double_click_function( 
    boost::bind( &LayerImporterWidget::import, this ) );
  this->private_->mask_1234_button_->set_double_click_function( 
    boost::bind( &LayerImporterWidget::import, this ) );
  this->private_->mask_1248_button_->set_double_click_function( 
    boost::bind( &LayerImporterWidget::import, this ) );


  // Step (5): Swap out visuals to allow the user to select the right option
  this->resize( 10, 10 );
  this->private_->ui_.importer_options_->show();
  
  // Step (6): Make the import button the default option
  this->private_->ui_.import_button_->setEnabled( true );
  this->private_->ui_.import_button_->setDefault( true );
  this->private_->ui_.import_button_->setAutoDefault( true );
  this->private_->ui_.cancel_button_->setDefault( false );
  this->private_->ui_.cancel_button_->setAutoDefault( false );
  
  this->adjustSize();
  // Center on screen
  this->move( QApplication::desktop()->availableGeometry().center() - this->rect().center() );
  this->setUpdatesEnabled( true );
  this->update();
}


void LayerImporterWidget::scan_first_file()
{
  // We should not get into this loop, but just in case we did not get any importers
  // just bail out and report an error.
  if ( this->private_->importers_.size() == 0 )
  {
    // Post an importer error.
    LayerImporterHandle empty_handle;
    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerImporterWidget::ReportImportError, qpointer_type( this ), empty_handle ) );
    return; 
  }
  
  // Get the first importer
  LayerImporterHandle importer = this->private_->importers_[ 0 ];
  if ( ! importer->get_file_info( this->private_->info_ ) )
  {
    // Post an importer error, since we cannot read the first file
    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerImporterWidget::ReportImportError, qpointer_type( this ), importer ) );
    return;
  }
  else
  {
    // We scanned the first file, hence the widget can now query the information needed
    // to show the next menu.
    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerImporterWidget::ListImportOptions, qpointer_type( this ), importer ) );
    return;
  } 
}


void LayerImporterWidget::ScanFirstFile( qpointer_type qpointer )
{
  if( qpointer.data() ) 
  {
    qpointer->scan_first_file();
  }
}

void LayerImporterWidget::ListImportOptions( qpointer_type qpointer, LayerImporterHandle importer )
{
  if( qpointer.data() ) 
  {
    qpointer->list_import_options();
  }
}

void LayerImporterWidget::ReportImportError( qpointer_type qpointer, LayerImporterHandle importer )
{
  if( qpointer.data() )
  {
    QWidget* parent = dynamic_cast<QWidget*>( qpointer->parent() );
    qpointer->reject();
  
    std::string error_message = "Could not create file";
    std::string detailed_message = "Could not create suitable importer fo file.";
    
    if ( importer )
    {
      error_message = std::string( "ERROR: Could not read file '" ) + 
        importer->get_filename() + std::string( "'." );
      detailed_message = importer->get_error(); 
    }
    
    QMessageBox message_box( parent );
    message_box.setWindowTitle( "Import Layer..." );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( QString::fromStdString( error_message ) );
    message_box.setDetailedText( QString::fromStdString ( detailed_message ) );
    message_box.exec();
    return; 
  }
}

}  // end namespace Seg3D
