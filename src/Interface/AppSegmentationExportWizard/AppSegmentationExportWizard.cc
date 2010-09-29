/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software" ),
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

// Interface includes
#include <Interface/AppSegmentationExportWizard/AppSegmentationExportWizard.h>



// Application includes
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionExportLayer.h>


namespace Seg3D
{

AppSegmentationExportWizard::AppSegmentationExportWizard( QWidget *parent ) :
    QWizard( parent )
{
  this->addPage( new SegmentationSelectionPage( this ) );
    this->addPage( new SegmentationSummaryPage( this ) );
  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );
  
  this->setWindowTitle( tr( "Segmentation Export Wizard" ) );
}

AppSegmentationExportWizard::~AppSegmentationExportWizard()
{
}

void AppSegmentationExportWizard::accept()
{
    QDialog::accept();
}

SegmentationSelectionPage::SegmentationSelectionPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setSubTitle( "Choose the layers that you would like to export as a segmentation." );

  file_name_lineedit_ = new QLineEdit( this );
  file_name_lineedit_->hide();
  
  this->mask_list_ = new QLineEdit( this );
  this->mask_list_->hide();

  this->main_layout_ = new QVBoxLayout( this );
  this->main_layout_->setContentsMargins(6, 6, 26, 6);
  this->main_layout_->setObjectName( QString::fromUtf8( "main_layout_" ) );

  this->group_with_masks_tree_ = new QTreeWidget( this );
  this->group_with_masks_tree_->setObjectName( QString::fromUtf8( "group_with_masks_tree_" ) );
  this->group_with_masks_tree_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->group_with_masks_tree_->setProperty( "showDropIndicator", QVariant( false ) );
  this->group_with_masks_tree_->setAlternatingRowColors( true );
  this->group_with_masks_tree_->setIndentation( 15 );
  this->group_with_masks_tree_->setItemsExpandable( true );
  this->group_with_masks_tree_->setHeaderHidden( true );

  this->main_layout_->addWidget( group_with_masks_tree_ );
  
  this->single_or_multiple_files_widget_ = new QWidget( this );
  this->single_or_multiple_files_widget_->setObjectName( 
    QString::fromUtf8( "single_or_multiple_files_widget_" ) );

  this->radio_button_group_ = new QButtonGroup( this );
  this->radio_button_group_->setExclusive( true );

  this->single_or_multiple_files_widget_->setMinimumSize( QSize( 0, 32 ) );
  this->single_or_multiple_files_widget_->setMaximumSize( QSize( 16777215, 32) );
  this->horizontalLayout_1 = new QHBoxLayout( this->single_or_multiple_files_widget_ );
  this->horizontalLayout_1->setSpacing( 0 );
  this->horizontalLayout_1->setContentsMargins( 0, 0, 0, 0 );
  this->horizontalLayout_1->setObjectName( QString::fromUtf8( "horizontalLayout" ) );
  this->single_file_widget_ = new QWidget( this->single_or_multiple_files_widget_ );
  this->single_file_widget_->setObjectName( QString::fromUtf8( "single_file_widget_" ) );
  this->horizontalLayout_4 = new QHBoxLayout( this->single_file_widget_ );
  this->horizontalLayout_4->setSpacing( 0 );
  this->horizontalLayout_4->setContentsMargins( 4, 4, 4, 4 );
  this->horizontalLayout_4->setObjectName( QString::fromUtf8( "horizontalLayout_2" ) );
  this->single_file_radio_button_ = new QRadioButton( this->single_file_widget_ );
  this->single_file_radio_button_->setObjectName( 
    QString::fromUtf8( "single_file_radio_button_" ) );
  this->single_file_radio_button_->setText( QString::fromUtf8( "Save masks as a single file" ) );
  this->single_file_radio_button_->setChecked( true );
  this->radio_button_group_->addButton( this->single_file_radio_button_, 0 );

  this->horizontalLayout_4->addWidget( this->single_file_radio_button_ );
  this->horizontalLayout_1->addWidget( this->single_file_widget_ );

  this->multiple_files_widget_ = new QWidget( this->single_or_multiple_files_widget_ );
  this->multiple_files_widget_->setObjectName( QString::fromUtf8( "multiple_files_widget_" ) );

  this->horizontalLayout_5 = new QHBoxLayout( this->multiple_files_widget_ );
  this->horizontalLayout_5->setSpacing( 0);
  this->horizontalLayout_5->setContentsMargins( 4, 4, 4, 4 );
  this->horizontalLayout_5->setObjectName( QString::fromUtf8( "horizontalLayout_3" ) );
  this->individual_files_radio_button_ = new QRadioButton( this->multiple_files_widget_ );
  this->individual_files_radio_button_->setObjectName( 
    QString::fromUtf8( "individual_files_radio_button_" ) );
  this->individual_files_radio_button_->setText( 
    QString::fromUtf8( "Save masks as individual files" ) );
  this->radio_button_group_->addButton( this->individual_files_radio_button_, 1 );

  this->horizontalLayout_5->addWidget( this->individual_files_radio_button_ );
  this->horizontalLayout_1->addWidget( this->multiple_files_widget_ );

  this->main_layout_->addWidget( this->single_or_multiple_files_widget_ );
  

  registerField( "maskList", this->mask_list_ );
  registerField( "segmentationPath", this->file_name_lineedit_ );
  registerField( "singleFile", this->single_file_radio_button_ );

}
  
void SegmentationSelectionPage::initializePage()
{
  std::vector< LayerGroupHandle > groups;
  LayerManager::Instance()->get_groups( groups );

  QStringList masks;
  
  int group_with_active_layer = -1;
  
  for( int i = 0; i < static_cast< int >( groups.size() ); ++i )
  {
    bool mask_found = false;

    QTreeWidgetItem *group = new QTreeWidgetItem( this->group_with_masks_tree_ );
    group->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate );
    group->setCheckState( 0, Qt::Checked );
    group->setCheckState( 0, Qt::Unchecked );
      
    std::string group_name = Core::ExportToString( 
    groups[ i ]->get_grid_transform().get_nx() ) + " x " +
    Core::ExportToString( groups[ i ]->get_grid_transform().get_ny() ) + " x " +
    Core::ExportToString( groups[ i ]->get_grid_transform().get_nz() );
    group->setText( 0,  QString::fromUtf8( "Group - " ) + 
      QString::fromStdString( group_name ) );
    group->setExpanded( true );

    layer_list_type layers = groups[ i ]->get_layer_list();
    layer_list_type::iterator it = layers.begin();
    while( it != layers.end() )
    {
      if( ( *it ) == LayerManager::Instance()->get_active_layer() )
      {
        group_with_active_layer = i;
      }
      if( ( *it )->type() == Core::VolumeType::MASK_E )
      {
        QTreeWidgetItem *mask = new QTreeWidgetItem( group );
        mask->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        mask->setCheckState( 0, Qt::Checked );
        mask->setCheckState( 0, Qt::Unchecked );
        mask->setText( 0, QString::fromStdString( ( *it )->name_state_->get() ) );

        mask_found = true;
      }
      ++it;
    }

    if( !mask_found )
    {
      group->setCheckState( 0, Qt::Unchecked );
      group->setDisabled( true );
    }
  }
  
  this->group_with_masks_tree_->topLevelItem( group_with_active_layer )->
    setCheckState( 0, Qt::Checked );
  
}

bool SegmentationSelectionPage::validatePage()
{
  QString selected_masks = "";
  for( int i = 0; i < this->group_with_masks_tree_->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem* group = this->group_with_masks_tree_->topLevelItem( i );
    for( int j = 0; j < group->childCount(); ++j )
    {
      QTreeWidgetItem* mask = group->child( j );
      if( mask->checkState( 0 ) == Qt::Checked )
      {
        if( selected_masks == "" )
        {
          selected_masks = mask->text( 0 );
        }
        else
        {
          selected_masks = selected_masks + QString::fromUtf8( "|" ) + mask->text( 0 );
        }
      }
    }
  }
  
  boost::filesystem::path desktop_path;
  Core::Application::Instance()->get_user_desktop_directory( desktop_path );
  
  QString filename;
  
  if( this->single_file_radio_button_->isChecked() )
  {
    filename = QFileDialog::getSaveFileName( this, "Export Segmentation As... ",
      QString::fromStdString( desktop_path.string() ),"NRRD files (*.nrrd)" );
  }
  else
  {
    filename = QFileDialog::getExistingDirectory( this, tr( "Choose Directory for Export..." ),
      QString::fromStdString( desktop_path.string() ),
      QFileDialog::ShowDirsOnly
      | QFileDialog::DontResolveSymlinks );
  }
  
  if( !boost::filesystem::exists( boost::filesystem::path( filename.toStdString() ).parent_path() ) )
  {
    return false;
  }
  
  this->file_name_lineedit_->setText( filename );
  
  setField( "maskList", selected_masks );
  setField( "segmentationPath", this->file_name_lineedit_->text() );
  setField( "singleFile", this->single_file_radio_button_->isChecked() );
  return true;
}

SegmentationSummaryPage::SegmentationSummaryPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Export to Segmentation Summary" );

  this->setFinalPage( true );
  this->main_layout_ = new QVBoxLayout( this );
  this->main_layout_->setContentsMargins( 6, 6, 26, 6 );
  this->main_layout_->setSpacing( 6 );
  this->main_layout_->setObjectName( QString::fromUtf8( "main_layout_" ) );

  this->description_ = new QLabel( this );
  this->description_->setWordWrap( true );
  this->main_layout_->addWidget( this->description_ );

  this->mask_scroll_area_ = new QScrollArea( this );
  this->mask_scroll_area_->setObjectName( QString::fromUtf8( "mask_scroll_area_" ) );
  this->mask_scroll_area_->setWidgetResizable( true );
  this->mask_scroll_area_->setAlignment( Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop );
  this->mask_scroll_area_->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  this->mask_scroll_area_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  
  this->layers_ = new QWidget();
  this->layers_->setObjectName( QString::fromUtf8( "layers_" ) );
  this->layers_->setStyleSheet( QString::fromUtf8( "background-color: white;" ) );
  this->masks_layout_ = new QVBoxLayout( this->layers_ );
  this->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->mask_scroll_area_->setWidget( this->layers_ );

  this->main_layout_->addWidget( this->mask_scroll_area_ );

  this->setLayout( this->main_layout_ );

  
}

void SegmentationSummaryPage::initializePage()
{
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove('&');
    
  this->masks_.clear();
    this->layers_->deleteLater();
  
  QString masks = field( "maskList" ).toString();
  std::vector< std::string > selected_masks_vector = 
    Core::SplitString( masks.toStdString(), "|" );
    
  this->layers_ = new QWidget();
  this->layers_->setObjectName( QString::fromUtf8( "layers_" ) );
  this->layers_->setStyleSheet( QString::fromUtf8( "background-color: white;" ) );
    
  this->masks_layout_ = new QVBoxLayout( this->layers_ );
  this->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->masks_layout_->setContentsMargins( 0, 0, 0, 0 );
  this->masks_layout_->setSpacing( 0 );
  this->masks_layout_->setAlignment( Qt::AlignTop );
  this->mask_scroll_area_->setWidget( this->layers_ );
  
  bool single_file = ( field( "singleFile" ).toString().toStdString() == "true" );
  
  // insert the background layer settings
  if( single_file )
  {
    QtLayerListWidget* new_mask = new QtLayerListWidget( this->layers_ );
    new_mask->set_mask_name( "Background" );
    new_mask->set_mask_index( 0 );
    this->masks_layout_->addWidget( new_mask );
    this->masks_.push_back( new_mask );
    
    connect( new_mask, SIGNAL( index_changed_signal() ), this, SIGNAL( completeChanged() ) ); 
  }
  
  for( int i = 0; i < static_cast< int >( selected_masks_vector.size() ); ++i )
  {
    QtLayerListWidget* new_mask = new QtLayerListWidget( this->layers_ );
    new_mask->set_mask_name( selected_masks_vector[ i ] );
    new_mask->set_mask_index( i + 1 );
    new_mask->hide_counter( !single_file );
    this->masks_layout_->addWidget( new_mask );
    this->masks_.push_back( new_mask );
    connect( new_mask, SIGNAL( index_changed_signal() ), this, SIGNAL( completeChanged() ) ); 
  }

  this->description_->setText(  QString::fromUtf8( "Please verify that you want to export the "
    "following mask layers as a segmentation " ) );
    
}

bool SegmentationSummaryPage::validatePage()
{
  QString selected_masks = "";
  for( int i = 0; i < this->masks_.size(); ++i )
  {
    if( selected_masks == "" )
    {
      selected_masks = this->masks_[ i ]->get_label() + QString::fromUtf8( "," ) + 
        QString::number( this->masks_[ i ]->get_value() );
    }
    else
    {
      selected_masks = selected_masks + QString::fromUtf8( "|" ) + 
        this->masks_[ i ]->get_label() + QString::fromUtf8( "," ) + 
        QString::number( this->masks_[ i ]->get_value() );
    }
  }
  
  bool single_file = ( field( "singleFile" ).toString().toStdString() == "true" ); 
  
  boost::filesystem::path file_name_and_path = 
    field( "segmentationPath" ).toString().toStdString();
  
  std::vector< LayerHandle > layers;
  std::vector< double > values;
  for( int i = 0; i < this->masks_.size(); ++i )
  {
    layers.push_back( LayerManager::Instance()->get_layer_by_name( 
      this->masks_[ i ]->get_label().toStdString() ) );
    values.push_back( this->masks_[ i ]->get_value() );
  }
  
  LayerExporterHandle exporter;
  if( ! ( LayerIO::Instance()->create_exporter( exporter, layers, "NRRD Exporter", ".nrrd" ) ) )
  {
    std::string error_message = std::string("ERROR: No importer is available for file '") + 
      file_name_and_path.string() + std::string("'.");

    QMessageBox message_box( this );
    message_box.setWindowTitle( "Import Layer..." );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( QString::fromStdString( error_message ) );
    message_box.exec();
    return false;
  
  }

  if( single_file )
  { 
    exporter->set_label_layer_values( values );
    ActionExportLayer::Dispatch( Core::Interface::GetWidgetActionContext(), exporter,
      LayerExporterMode::LABEL_MASK_E, file_name_and_path.string() );
  }
  else
  {
    ActionExportLayer::Dispatch( Core::Interface::GetWidgetActionContext(), exporter,
      LayerExporterMode::SINGLE_MASK_E, file_name_and_path.string() );
  }
    
  return true;
}

bool SegmentationSummaryPage::isComplete() const
{
  bool valid = true;
  for( int i = 0; i < this->masks_.size(); ++i )
  {
    bool found = false;
    for( int j = 0; j < this->masks_.size(); ++j )
    {
      if( i == j ) continue;
      if( this->masks_[ i ]->get_value() == this->masks_[ j ]->get_value() )
      {
        found = true;
        break;
      }
    }
    if( found ) 
    {
      this->masks_[ i ]->set_validity( false );
      valid = false;
    }
    else this->masks_[ i ]->set_validity( true );
  }
  return valid;
}

} // end namespace Seg3D
