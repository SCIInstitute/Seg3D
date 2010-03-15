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
  mode_(LayerImporterMode::DATA_E)
{
  // Ensure we have all the data of the import process
  importer->import_header();

  // Ensure it will be the only focus in the pogram
  setWindowModality(  Qt::ApplicationModal );

  // Make a new LayerImporterWidgetPrivateHandle object
  private_ = LayerImporterWidgetPrivateHandle( new LayerImporterWidgetPrivate );
  private_->ui_.setupUi( this );

  // Switch off options that this importer does not support
  if ( !( importer_->has_importer_mode( LayerImporterMode::DATA_E ) ) )
  {
    private_->ui_.data_->hide();
    private_->ui_.data_label_->hide();
  }

  if ( !( importer_->has_importer_mode( LayerImporterMode::SINGLE_MASK_E ) ) )
  {
    private_->ui_.single_mask_->hide();
    private_->ui_.single_mask_label_->hide();
  }

  if ( !( importer_->has_importer_mode( LayerImporterMode::BITPLANE_MASK_E ) ) )
  {
    private_->ui_.bitplane_mask_->hide();
    private_->ui_.bitplane_mask_label_->hide();
  }

  if ( !( importer_->has_importer_mode( LayerImporterMode::LABEL_MASK_E ) ) )
  {
    private_->ui_.label_mask_->hide();
    private_->ui_.label_mask_label_->hide();
  }

  // Switch on the right icons
  update_icons();

  // Add information to importer
  boost::filesystem::path full_filename( importer_->get_filename() );
  private_->ui_.filename_->setText( QString::fromStdString( full_filename.filename() ) );
  
  std::string data_type = Utils::ExportToString( importer_->get_data_type() );
  private_->ui_.data_type_->setText( QString::fromStdString( data_type ) );
  
  Utils::GridTransform grid_transform = importer_->get_grid_transform();
  private_->ui_.x_size_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.nx() ) ) );
  private_->ui_.y_size_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.ny() ) ) );
  private_->ui_.z_size_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.nz() ) ) );

  SCI_LOG_DEBUG( std::string( "Transform = " ) + Utils::export_to_string(grid_transform) );

  private_->ui_.x_spacing_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.spacing_x() ) ) );
  private_->ui_.y_spacing_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.spacing_y() ) ) );
  private_->ui_.z_spacing_->setText( 
    QString::fromStdString( Utils::to_string( grid_transform.spacing_z() ) ) );

  // Create the notes:
  std::string notes;
  if ( !( grid_transform.is_axis_aligned() ) )
  {
    notes += "Converting data to be axis align."; 
  }
  
  if ( notes == "" )
  {
    notes = "No messages.";
  }
  
  private_->ui_.import_notes_->setText( QString::fromStdString(notes) );

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
  connect( private_->ui_.cancel_button_, SIGNAL( released() ),
    this, SLOT( reject() ) );
}

LayerImporterWidget::~LayerImporterWidget()
{
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
}

void LayerImporterWidget::set_mode( LayerImporterMode mode )
{
  mode_ = mode;
  update_icons();
  update(); 
}

void LayerImporterWidget::import()
{
  ActionImportLayer::Dispatch( importer_, mode_ );
  accept();
}

}  // end namespace Seg3D
