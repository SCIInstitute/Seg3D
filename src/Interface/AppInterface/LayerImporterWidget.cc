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

// Utils includes
#include <Utils/Core/Log.h>

// Application includes
#include <Application/LayerManager/Actions/ActionImportLayer.h>

//Interface Includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/LayerImporterWidget.h>

// Qt Gui Includes
#include "ui_LayerImporterWidget.h"

namespace Seg3D
{

class LayerImporterWidgetPrivate
{
public:
  Ui::LayerImportWidget ui_;
  
  QIcon data_icon_;
  QIcon single_mask_icon_;
  QIcon bitplane_mask_icon_;
  QIcon label_mask_icon_;

  QIcon data_off_icon_;
  QIcon single_mask_off_icon_;
  QIcon bitplane_mask_off_icon_;
  QIcon label_mask_off_icon_;
  
  QString active_button_style_;
  QString inactive_button_style_;
  
  LayerImporterWidgetPrivate()
  {
    this->data_icon_.addFile( 
      QString::fromUtf8( ":/Images/DataVolume.png" ),
      QSize(48,48), QIcon::Normal, QIcon::Off );
    this->data_off_icon_.addFile( 
      QString::fromUtf8( ":/Images/DataVolumeOff.png" ),
      QSize(48,48), QIcon::Normal, QIcon::Off );
      
    this->single_mask_icon_.addFile( 
      QString::fromUtf8( ":/Images/SingleMask.png" ), 
      QSize(48,48), QIcon::Normal, QIcon::Off );
    this->single_mask_off_icon_.addFile( 
      QString::fromUtf8( ":/Images/SingleMaskOff.png" ), 
      QSize(48,48), QIcon::Normal, QIcon::Off );

    this->bitplane_mask_icon_.addFile( 
      QString::fromUtf8( ":/Images/BitPlaneMask.png" ), 
      QSize(48,48), QIcon::Normal, QIcon::Off );
    this->bitplane_mask_off_icon_.addFile( 
      QString::fromUtf8( ":/Images/BitPlaneMaskOff.png" ), 
      QSize(48,48), QIcon::Normal, QIcon::Off );

    this->label_mask_icon_.addFile( 
      QString::fromUtf8( ":/Images/LabelMask.png" ), 
      QSize(48,48), QIcon::Normal, QIcon::Off );  
    this->label_mask_off_icon_.addFile( 
      QString::fromUtf8( ":/Images/LabelMaskOff.png" ), 
      QSize(48,48), QIcon::Normal, QIcon::Off );  

    this->active_button_style_ = QString( "QToolButton{"\
      "background-color: rgb(233, 111, 53);"\
      "border-radius: 3px;"\
      "border: 2px solid rgb(142, 67, 32);}" );

    this->inactive_button_style_ = QString( "QToolButton{"\
      "background-color: rgb(150, 150, 150);"\
      "border-radius: 3px;"\
      "border: 2px solid rgb(30, 30, 30);}" );
  }
};

LayerImporterWidget::LayerImporterWidget( LayerImporterHandle importer, QWidget* parent ) :
  QDialog( parent ),
  importer_(importer),
  mode_(LayerImporterMode::INVALID_E)
{
  // Step (1): Ensure it will be the only focus in the pogram
  setWindowModality(  Qt::ApplicationModal );
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  setMinimumHeight( 100 );
  
  // Step (2): Make a new LayerImporterWidgetPrivateHandle object
  private_ = LayerImporterWidgetPrivateHandle( new LayerImporterWidgetPrivate );
  private_->ui_.setupUi( this );

  // Step (3): Hide the parts of the UI that cannot be used yet
  
  private_->ui_.file_info_->hide();
  private_->ui_.importer_options_->hide();
  private_->ui_.scanning_file_->show();
  private_->ui_.import_button_->setEnabled( false );

  adjustSize();

  // Step (4): Activate the cancel button
  connect( private_->ui_.cancel_button_, SIGNAL( released() ),
    this, SLOT( reject() ) );

  // Step (5): Asynchronously scan file, so UI stays interactive
  boost::thread( boost::bind( &LayerImporterWidget::ScanFile, 
    qpointer_type( this ), importer_ ) );
}

LayerImporterWidget::~LayerImporterWidget()
{
}

void LayerImporterWidget::list_import_options()
{
  // Step (1): Switch off options that this importer does not support
  int importer_modes = importer_->get_importer_modes();

  if ( importer_modes & LayerImporterMode::LABEL_MASK_E )
  {
    private_->ui_.label_mask_->show();
    private_->ui_.label_mask_label_->show();
    mode_ = LayerImporterMode::LABEL_MASK_E;
  }
  else
  {
    private_->ui_.label_mask_->hide();
    private_->ui_.label_mask_label_->hide();
  }

  if ( importer_modes & LayerImporterMode::BITPLANE_MASK_E )
  {
    private_->ui_.bitplane_mask_->show();
    private_->ui_.bitplane_mask_label_->show();
    mode_ = LayerImporterMode::BITPLANE_MASK_E;
  }
  else
  {
    private_->ui_.bitplane_mask_->hide();
    private_->ui_.bitplane_mask_label_->hide();
  }

  if ( importer_modes & LayerImporterMode::SINGLE_MASK_E )
  {
    private_->ui_.single_mask_->show();
    private_->ui_.single_mask_label_->show();
    mode_ = LayerImporterMode::SINGLE_MASK_E;
  }
  else
  {
    private_->ui_.single_mask_->hide();
    private_->ui_.single_mask_label_->hide();
  }

  if ( importer_modes &  LayerImporterMode::DATA_E )
  {
    private_->ui_.data_->show();
    private_->ui_.data_label_->show();
    mode_ = LayerImporterMode::DATA_E;
  }
  else
  {
    private_->ui_.data_->hide();
    private_->ui_.data_label_->hide();
  }

  // Step (2): Switch on the right icons
  update_icons();

  // Step (3): Add information from file scan to importer
  boost::filesystem::path full_filename( importer_->get_filename() );
  private_->ui_.filename_->setText( QString::fromStdString( full_filename.filename() ) );
  
  std::string data_type = Utils::ExportToString( importer_->get_data_type() );
  private_->ui_.data_type_->setText( QString::fromStdString( data_type ) );
  
  Utils::GridTransform grid_transform = importer_->get_grid_transform();
  private_->ui_.x_size_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.get_nx() ) ) );
  private_->ui_.y_size_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.get_ny() ) ) );
  private_->ui_.z_size_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.get_nz() ) ) );

  private_->ui_.x_spacing_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.spacing_x() ) ) );
  private_->ui_.y_spacing_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.spacing_y() ) ) );
  private_->ui_.z_spacing_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.spacing_z() ) ) );

  // Step (4): Add connections for selecting and importing the data
  connect( private_->ui_.data_, SIGNAL( released() ), 
    this, SLOT( set_data() ) );
    
  connect( private_->ui_.single_mask_, SIGNAL( released() ), 
    this, SLOT( set_single_mask() ) );
    
  connect( private_->ui_.bitplane_mask_, SIGNAL( released() ), 
    this, SLOT( set_bitplane_mask() ) );
    
  connect( private_->ui_.label_mask_, SIGNAL( released() ), 
    this, SLOT( set_label_mask() ) );

  connect( private_->ui_.import_button_, SIGNAL( released() ),
    this, SLOT( import() ) );

  // Step (5): Swap out visuals to allow the user to select the right option
  private_->ui_.file_info_->show();
  private_->ui_.importer_options_->show();
  private_->ui_.scanning_file_->hide();
  
  // Step (6): Make the import button the default option
  private_->ui_.import_button_->setEnabled( true );
  private_->ui_.cancel_button_->setDefault( false );
  private_->ui_.cancel_button_->setAutoDefault( false );
  private_->ui_.import_button_->setDefault( true );
  private_->ui_.import_button_->setAutoDefault( true );
  
  adjustSize();
}


void LayerImporterWidget::update_icons()
{
  setUpdatesEnabled( false );
  
  if ( mode_ == LayerImporterMode::DATA_E )
  {
    private_->ui_.data_->setIcon( private_->data_icon_ ); 
    private_->ui_.data_->setStyleSheet( private_->active_button_style_ );
  }
  else
  {
    private_->ui_.data_->setIcon( private_->data_off_icon_ );
    private_->ui_.data_->setStyleSheet( private_->inactive_button_style_ );
  }
  
  if ( mode_ == LayerImporterMode::SINGLE_MASK_E )
  {
    private_->ui_.single_mask_->setIcon( private_->single_mask_icon_ );
    private_->ui_.single_mask_->setStyleSheet( private_->active_button_style_ );
  }
  else
  {
    private_->ui_.single_mask_->setIcon( private_->single_mask_off_icon_ );
    private_->ui_.single_mask_->setStyleSheet( private_->inactive_button_style_ );
  }
  
  if ( mode_ == LayerImporterMode::BITPLANE_MASK_E )
  {
    private_->ui_.bitplane_mask_->setIcon( private_->bitplane_mask_icon_ );
    private_->ui_.bitplane_mask_->setStyleSheet( private_->active_button_style_ );
  }
  else
  {
    private_->ui_.bitplane_mask_->setIcon( private_->bitplane_mask_off_icon_ );
    private_->ui_.bitplane_mask_->setStyleSheet( private_->inactive_button_style_ );
  }
  
  if ( mode_ == LayerImporterMode::LABEL_MASK_E )
  {
    private_->ui_.label_mask_->setIcon( private_->label_mask_icon_ );
    private_->ui_.label_mask_->setStyleSheet( private_->active_button_style_ );
  }
  else
  {
    private_->ui_.label_mask_->setIcon( private_->label_mask_off_icon_ );
    private_->ui_.label_mask_->setStyleSheet( private_->inactive_button_style_ );
  }
  
  setUpdatesEnabled( true );
  
  // issue an update onto the Qt event queue
  update(); 
}

void LayerImporterWidget::set_data() 
{ 
  set_mode( LayerImporterMode::DATA_E ); 
}

void LayerImporterWidget::set_single_mask() 
{ 
  set_mode( LayerImporterMode::SINGLE_MASK_E );
}

void LayerImporterWidget::set_bitplane_mask() 
{ 
  set_mode( LayerImporterMode::BITPLANE_MASK_E ); 
}

void LayerImporterWidget::set_label_mask() 
{ 
  set_mode( LayerImporterMode::LABEL_MASK_E ); 
}

void LayerImporterWidget::set_mode( LayerImporterMode mode )
{
  mode_ = mode;
  update_icons();
}

void LayerImporterWidget::import()
{
  ActionImportLayer::Dispatch( importer_, mode_ );
  accept();
}

void LayerImporterWidget::ScanFile( qpointer_type qpointer, LayerImporterHandle importer )
{
  // Step (1) : Import the file header or in some cases the full file
  bool success = importer->import_header();
  
  // Step (1a): If import was a success but no import modes are available we cannot proceed
  // hence success of scanning is false.
  if ( importer->get_importer_modes() == 0) success = false;
  
  // Step (2) : Update the widget if it still exists
  if ( success )
  {
    Interface::Instance()->post_event( boost::bind( &LayerImporterWidget::ListImportOptions,
      qpointer, importer ) ); 
  }
  else
  {
    Interface::Instance()->post_event( boost::bind( &LayerImporterWidget::ReportImportError,
      qpointer, importer ) );
  }
}

void LayerImporterWidget::ListImportOptions( qpointer_type qpointer, LayerImporterHandle importer )
{
  if ( qpointer.data() ) 
  {
    qpointer->list_import_options();
  }
}

void LayerImporterWidget::ReportImportError( qpointer_type qpointer, LayerImporterHandle importer )
{
  if ( qpointer.data() )
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

}  // end namespace Seg3D
