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

// Core includes
#include <Core/Utils/Log.h>

// Application includes
#include <Application/LayerManager/Actions/ActionImportLayer.h>

// QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtDoubleClickPushButton.h>

// Interface Includes
#include <Interface/AppInterface/StyleSheet.h>
#include <Interface/AppInterface/LayerImporterWidget.h>

// Qt Gui Includes
#include "ui_LayerImporterWidget.h"

namespace Seg3D
{




class LayerImporterWidgetPrivate
{
public:
  LayerImporterWidgetPrivate() : 
    mode_( LayerImporterMode::INVALID_E )
  {
  }
  
  Ui::LayerImporterWidget ui_;
  
  QtUtils::QtDoubleClickPushButton *data_volume_button_;
  QtUtils::QtDoubleClickPushButton *mask_single_button_;
  QtUtils::QtDoubleClickPushButton *mask_1234_button_;
  QtUtils::QtDoubleClickPushButton *mask_1248_button_;
  QButtonGroup *type_button_group_;

  QButtonGroup *itk_import_type_button_group_;

  
  // The importer(s) that were chosen in the filedialog
  std::vector< LayerImporterHandle > importers_;
  std::vector< std::string > files_;
  std::string importer_name_;
  bool series_;

  // The current active mode
  LayerImporterMode mode_;
};

LayerImporterWidget::LayerImporterWidget( std::vector< LayerImporterHandle > importers, 
  std::vector< std::string > files, QWidget *parent ) :
  QDialog( parent ),
  private_( new LayerImporterWidgetPrivate )
{
  this->setup_ui();

  this->private_->importers_ = importers;
  this->private_->files_ = files;
  this->private_->importer_name_ = this->private_->importers_[ 0 ]->name();

  boost::filesystem::path full_filename( this->private_->importers_[ 0 ]->get_filename() );
  this->private_->ui_.file_name_label_->setText( QString::fromUtf8( "Scanning: " ) +
    QString::fromStdString( full_filename.leaf() ) );

  boost::thread( boost::bind( &LayerImporterWidget::ScanFile, 
    qpointer_type( this ), this->private_->importers_[ 0 ] ) );
  
  this->private_->ui_.horizontalLayout_20->setAlignment( Qt::AlignCenter );
  this->private_->ui_.swap_x_y_spacing_widget_->hide();
  }


LayerImporterWidget::~LayerImporterWidget()
{
}

void LayerImporterWidget::list_import_options()
{
  this->setUpdatesEnabled( false );

  // We are done scanning, so we hide the scanning widget
  this->private_->ui_.scanning_file_->hide();

  // Step (1): Switch off options that this importer does not support
  int importer_modes = this->private_->importers_[ 0 ]->get_importer_modes();

  if( importer_modes & LayerImporterMode::LABEL_MASK_E )
  {
    this->private_->ui_.label_mask_->show();
    this->private_->mask_1234_button_->setMinimumSize( this->private_->ui_.bitplane_mask_->size() );
    this->private_->mask_1234_button_->setChecked( true );
    this->private_->mode_ = LayerImporterMode::LABEL_MASK_E;
  }
  else
  {
    this->private_->ui_.label_mask_->hide();
  }

  if( importer_modes & LayerImporterMode::BITPLANE_MASK_E )
  {
    this->private_->ui_.bitplane_mask_->show();
    this->private_->mask_1248_button_->setMinimumSize( this->private_->ui_.label_mask_->size());
    this->private_->mask_1248_button_->setChecked( true );
    this->private_->mode_ = LayerImporterMode::BITPLANE_MASK_E;
  }
  else
  {
    this->private_->ui_.bitplane_mask_->hide();
  }

  if( importer_modes & LayerImporterMode::SINGLE_MASK_E )
  {
    this->private_->ui_.single_mask_->show();
    this->private_->mask_single_button_->setMinimumSize( this->private_->ui_.single_mask_->size() );
    this->private_->mask_single_button_->setChecked( true );
    this->private_->mode_ = LayerImporterMode::SINGLE_MASK_E;
  }
  else
  {
    this->private_->ui_.single_mask_->hide();
    
  }

  if( importer_modes &  LayerImporterMode::DATA_E )
  {
    this->private_->ui_.data_->show();
    this->private_->data_volume_button_->setMinimumSize( this->private_->ui_.data_->size() );
    this->private_->data_volume_button_->setChecked( true );
    this->private_->mode_ = LayerImporterMode::DATA_E;
    std::string extension = boost::filesystem::path( this->private_->files_[ 0 ] ).extension();
    if( ( extension == ".dcm" ) || ( extension == "" ) )
    {
      this->private_->ui_.swap_x_y_spacing_widget_->show();
    }
  }
  else
  {
    this->private_->ui_.data_->hide();
  }

  this->private_->ui_.file_name_table_->show();

  //Step (3): Add information from importer to the table
  for( int i = 0; i < static_cast< int >( this->private_->files_.size() ); ++i )
  {
    QTableWidgetItem *new_item;
    new_item = new QTableWidgetItem( QString::fromStdString( 
      boost::filesystem::path( this->private_->files_[ i ] ).filename() ) );
    this->private_->ui_.file_name_table_->insertRow( i );
    this->private_->ui_.file_name_table_->setItem( i, 0, new_item );

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
  this->private_->ui_.cancel_button_->setDefault( false );
  this->private_->ui_.cancel_button_->setAutoDefault( false );
  this->private_->ui_.import_button_->setDefault( true );
  this->private_->ui_.import_button_->setAutoDefault( true );
  
  this->adjustSize();
  this->center_widget_on_screen( this );
  this->setUpdatesEnabled( true );
  this->update();
}

  
void LayerImporterWidget::center_widget_on_screen( QWidget *widget ) 
{
  QRect rect = QApplication::desktop()->availableGeometry();
  
  widget->move( rect.center() - widget->rect().center() );
}
  

void LayerImporterWidget::set_type( int file_type )
{
  switch( file_type )
  {
  case 0:
    set_mode( LayerImporterMode::DATA_E );
    break;

  case 1:
    set_mode( LayerImporterMode::SINGLE_MASK_E );
    break;

  case 2:
    set_mode( LayerImporterMode::BITPLANE_MASK_E ); 
    break;

  case 3:
    set_mode( LayerImporterMode::LABEL_MASK_E );
    break;
  default:
    break;
  }
}

void LayerImporterWidget::set_mode( LayerImporterMode mode )
{
  this->private_->mode_ = mode;
}

void LayerImporterWidget::import()
{
  this->setUpdatesEnabled( false );
  this->private_->ui_.scanning_file_->show();
  this->private_->ui_.importer_options_->hide();
  this->private_->ui_.file_name_table_->hide();
  this->private_->ui_.swap_x_y_spacing_widget_->hide();
  this->setMaximumHeight( 80 );
  this->setUpdatesEnabled( true );
  
  this->repaint();

  this->private_->ui_.import_button_->setEnabled( false );

  // Step (2): Show Scanning Widget
  
  this->center_widget_on_screen( this );

  this->repaint();


  for( size_t i = 1; i < this->private_->importers_.size(); ++i )
  {
    boost::filesystem::path full_filename( this->private_->importers_[ i ]->get_filename() );
    this->private_->ui_.file_name_label_->setText( QString::fromUtf8( "Scanning: " ) +
      QString::fromStdString( full_filename.leaf() ) );
    this->repaint();
    this->private_->importers_[ i ]->import_header();
    
  }
  
  for( size_t i = 0; i < this->private_->importers_.size(); ++i )
  {
    this->private_->importers_[ i ]->set_swap_xy_spacing( 
      this->private_->ui_.swap_spacing_checkbox_->isChecked() );
    ActionImportLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->importers_[ i ], this->private_->mode_, this->private_->series_ );
  }
  accept();
}

void LayerImporterWidget::ScanFile( qpointer_type qpointer, LayerImporterHandle importer )
{
  // Step (1) : Import the file header or in some cases the full file
  bool success = importer->import_header();

  // Step (1a): If import was a success but no import modes are available we cannot proceed
  // hence success of scanning is false.
  if( importer->get_importer_modes() == 0) success = false;
  
  // Step (2) : Update the widget if it still exists
  if( success )
  {
    if( ( qpointer->private_->files_.size() > 1 ) &&
      ( importer->set_file_list( qpointer->private_->files_ ) ) )
    {
      qpointer->private_->series_ = true;
    }
    else
    {
      qpointer->private_->series_ = false;
    }
      
    importer->set_file_list( qpointer->private_->files_ );

    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerImporterWidget::ListImportOptions, qpointer, importer ) ); 
  }
  else
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerImporterWidget::ReportImportError, qpointer, importer ) );
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
  
    std::string error_message = std::string("ERROR: Could not read file '") + 
      importer->get_filename() + std::string("'.");
    std::string detailed_message = importer->get_error(); 
      
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

  // Step (6): Show Scanning Widget then Asynchronously scan file, so UI stays interactive
  this->private_->ui_.scanning_file_->show();

}

}  // end namespace Seg3D
