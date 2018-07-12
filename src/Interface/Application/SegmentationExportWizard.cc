/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

// Qt includes
#include <QtCore/QVariant>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QTreeWidget>
#include <QScrollArea>

// Core includes
#include <Core/State/Actions/ActionSet.h>

// Application includes
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/LayerIO/Actions/ActionExportSegmentation.h>
#include <Application/ProjectManager/ProjectManager.h>

// Interface includes
#include <Interface/Application/SegmentationExportWizard.h>
#include <Interface/Application/StyleSheet.h>

namespace Seg3D
{
  
class SegmentationPrivate
{
public:
  QVector< QtLayerListWidget* > masks_;
  QWidget* bitmap_widget_;
  QLabel* export_label_;
  QComboBox* export_selector_;
  QHBoxLayout* bitmap_layout_;
  
  
  // SegmentationSelectionPage
  QVBoxLayout *selection_main_layout_;
  QWidget *segmentation_top_widget_;
  QVBoxLayout *verticalLayout;
  QWidget *segmentation_name_widget_;
  QHBoxLayout *horizontalLayout;
  QLabel *segmentation_name_label_;
  QTreeWidget *group_with_masks_tree_;
  QLabel *warning_message_;

  std::string file_name_;
    
  QWidget *single_or_multiple_files_widget_;
  QHBoxLayout *horizontalLayout_1;
  QWidget *single_file_widget_;
  QHBoxLayout *horizontalLayout_4;
  QRadioButton *single_file_radio_button_;
  QWidget *multiple_files_widget_;
  QHBoxLayout *horizontalLayout_5;
  QRadioButton *individual_files_radio_button_;
  QButtonGroup *radio_button_group_;
  QLabel *filename_path_label_;
  QLabel *filename_name_label_;
  QPushButton *choose_filename_loc_button_;
  QWidget *path_widget_;
  QWidget *name_widget_;
  QHBoxLayout *path_layout_;
  QHBoxLayout *name_layout_;
  QLineEdit *filename_name_lineEdit_;
  QLineEdit *filename_path_lineEdit_;
  
  //SegmentationSummaryPage

  QLabel *description_;
  QVBoxLayout *summary_main_layout_;
  QScrollArea *mask_scroll_area_;
  QWidget *layers_;
  QVBoxLayout *masks_layout_;
  
};
  SegmentationExportWizard::SegmentationExportWizard( QWidget *parent ) :
    QWizard( parent ),
  private_( new SegmentationPrivate )
{
  this->setMinimumSize( 660, 400 );
  this->addPage( new SegmentationSelectionPage( private_, this ) );
    this->addPage( new SegmentationSummaryPage( private_, this ) );
  this->setWizardStyle( QWizard::MacStyle );
  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );
  this->setWindowTitle( tr( "Segmentation Export Wizard" ) );
}

SegmentationExportWizard::~SegmentationExportWizard()
{
}

void SegmentationExportWizard::accept()
{
    QDialog::accept();
}

SegmentationSelectionPage::SegmentationSelectionPage( SegmentationPrivateHandle private_handle, QWidget *parent )
    : QWizardPage( parent )
{
  this->private_ = private_handle;
    this->setSubTitle( "Choose the layers that you would like to export as a segmentation." );

  this->private_->selection_main_layout_ = new QVBoxLayout( this );
  this->private_->selection_main_layout_->setContentsMargins(6, 6, 26, 6);
  this->private_->selection_main_layout_->setObjectName( QString::fromUtf8( "selection_main_layout_" ) );

  //Mask tree
  this->private_->group_with_masks_tree_ = new QTreeWidget( this );
  this->private_->group_with_masks_tree_->setObjectName( QString::fromUtf8( "group_with_masks_tree_" ) );
  this->private_->group_with_masks_tree_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->private_->group_with_masks_tree_->setProperty( "showDropIndicator", QVariant( false ) );
  this->private_->group_with_masks_tree_->setAlternatingRowColors( true );
  this->private_->group_with_masks_tree_->setIndentation( 15 );
  this->private_->group_with_masks_tree_->setItemsExpandable( true );
  this->private_->group_with_masks_tree_->setHeaderHidden( true );
  this->private_->group_with_masks_tree_->setStyleSheet( StyleSheet::SEGMENTATION_EXPORT_C );

  this->private_->selection_main_layout_->addWidget( this->private_->group_with_masks_tree_ );
  
  //Single file or multiple file radio buttons
  this->private_->single_or_multiple_files_widget_ = new QWidget( this );
  this->private_->single_or_multiple_files_widget_->setObjectName( 
    QString::fromUtf8( "single_or_multiple_files_widget_" ) );

  this->private_->radio_button_group_ = new QButtonGroup( this );
  this->private_->radio_button_group_->setExclusive( true );

  this->private_->single_or_multiple_files_widget_->setMinimumSize( QSize( 0, 30 ) );
  this->private_->single_or_multiple_files_widget_->setMaximumSize( QSize( 16777215, 30 ) );
  this->private_->horizontalLayout_1 = new QHBoxLayout( this->private_->single_or_multiple_files_widget_ );
  this->private_->horizontalLayout_1->setSpacing( 0 );
  this->private_->horizontalLayout_1->setContentsMargins( 0, 0, 0, 0 );
  this->private_->horizontalLayout_1->setObjectName( QString::fromUtf8( "horizontalLayout" ) );
  this->private_->single_file_widget_ = new QWidget( this->private_->single_or_multiple_files_widget_ );
  this->private_->single_file_widget_->setObjectName( QString::fromUtf8( "single_file_widget_" ) );
  this->private_->horizontalLayout_4 = new QHBoxLayout( this->private_->single_file_widget_ );
  this->private_->horizontalLayout_4->setSpacing( 0 );
  this->private_->horizontalLayout_4->setContentsMargins( 4, 4, 4, 4 );
  this->private_->horizontalLayout_4->setObjectName( QString::fromUtf8( "horizontalLayout_2" ) );
  this->private_->single_file_radio_button_ = new QRadioButton( this->private_->single_file_widget_ );
  this->private_->single_file_radio_button_->setObjectName( 
    QString::fromUtf8( "single_file_radio_button_" ) );
  this->private_->single_file_radio_button_->setText( QString::fromUtf8( "Save segmentation as a single file" ) );
  this->private_->single_file_radio_button_->setChecked( true );
  this->private_->radio_button_group_->addButton( this->private_->single_file_radio_button_, 0 );

  this->private_->horizontalLayout_4->addWidget( this->private_->single_file_radio_button_ );
  this->private_->horizontalLayout_1->addWidget( this->private_->single_file_widget_ );

  this->private_->multiple_files_widget_ = new QWidget( this->private_->single_or_multiple_files_widget_ );
  this->private_->multiple_files_widget_->setObjectName( QString::fromUtf8( "multiple_files_widget_" ) );

  this->private_->horizontalLayout_5 = new QHBoxLayout( this->private_->multiple_files_widget_ );
  this->private_->horizontalLayout_5->setSpacing( 0);
  this->private_->horizontalLayout_5->setContentsMargins( 4, 4, 4, 4 );
  this->private_->horizontalLayout_5->setObjectName( QString::fromUtf8( "horizontalLayout_3" ) );
  this->private_->individual_files_radio_button_ = new QRadioButton( this->private_->multiple_files_widget_ );
  this->private_->individual_files_radio_button_->setObjectName( 
    QString::fromUtf8( "individual_files_radio_button_" ) );
  this->private_->individual_files_radio_button_->setText( 
    QString::fromUtf8( "Save segmentation as individual files" ) );
  this->private_->radio_button_group_->addButton( this->private_->individual_files_radio_button_, 1 );

  this->private_->horizontalLayout_5->addWidget( this->private_->individual_files_radio_button_ );
  this->private_->horizontalLayout_1->addWidget( this->private_->multiple_files_widget_ );
    
  connect( this->private_->single_file_radio_button_, SIGNAL( clicked() ), SLOT(radio_button_change_path() ) );
  connect( this->private_->individual_files_radio_button_, SIGNAL( clicked() ), SLOT(radio_button_change_path() ) );

  //Segmentation Name
  this->private_->name_widget_ = new QWidget( this );
  this->private_->name_widget_->setMinimumSize( QSize( 0, 40) );
  this->private_->name_widget_->setMaximumSize( QSize( 16777215, 40 ) );
  this->private_->name_layout_ = new QHBoxLayout( this->private_->name_widget_ );
  this->private_->name_layout_->setSpacing( 6 );
  this->private_->name_layout_->setContentsMargins( 4, 4, 4, 4 );
    
  this->private_->filename_name_label_ = new QLabel( QString::fromUtf8( "Segmentation Name: " ),
                                                      this->private_->name_widget_ );
  this->private_->name_layout_->addWidget( this->private_->filename_name_label_ );
    
  this->private_->filename_name_lineEdit_ = new QLineEdit();
  this->private_->name_layout_->addWidget( this->private_->filename_name_lineEdit_ );
    
  //Segmentation Path
  this->private_->path_widget_ = new QWidget( this );
  this->private_->path_widget_->setMinimumSize( QSize( 0, 40) );
  this->private_->path_widget_->setMaximumSize( QSize( 16777215, 40 ) );
  this->private_->path_layout_ = new QHBoxLayout( this->private_->path_widget_ );
  this->private_->path_layout_->setSpacing( 6 );
  this->private_->path_layout_->setContentsMargins( 4, 4, 4, 4 );

  this->private_->filename_path_label_ = new QLabel( QString::fromUtf8( "Segmentation Path: " ),
                                        this->private_->path_widget_ );
  this->private_->path_layout_->addWidget( this->private_->filename_path_label_ );
    
  this->private_->filename_path_lineEdit_ = new QLineEdit();
  this->private_->path_layout_->addWidget( this->private_->filename_path_lineEdit_ );
    
  this->private_->choose_filename_loc_button_ = new QPushButton( "Browse..." );
  connect( this->private_->choose_filename_loc_button_, SIGNAL( clicked() ), SLOT( set_export_path() ) );
  this->private_->choose_filename_loc_button_->setFocusPolicy( Qt::NoFocus );
  this->private_->path_layout_->addWidget( this->private_->choose_filename_loc_button_ );
    
  //Export Segmentation
  this->private_->bitmap_widget_ = new QWidget( this );
  this->private_->bitmap_widget_->setMinimumSize( QSize( 0, 30) );
  this->private_->bitmap_widget_->setMaximumSize( QSize( 16777215, 30 ) );
  this->private_->bitmap_layout_ = new QHBoxLayout( this->private_->bitmap_widget_ );
  this->private_->bitmap_layout_->setSpacing( 6 );
  this->private_->bitmap_layout_->setContentsMargins( 4, 4, 4, 4 );
  
  this->private_->export_label_ = new QLabel( QString::fromUtf8( "Export segmentation as: " ),
    this->private_->bitmap_widget_ );
  this->private_->bitmap_layout_->addWidget( this->private_->export_label_ );
  
  this->private_->export_selector_ = new QComboBox( this->private_->bitmap_widget_ );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".nrrd" ) );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".mat" ) );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".mrc" ) );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".tiff" ) );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".bmp" ) );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".png" ) );
  this->private_->export_selector_->addItem( QString::fromUtf8( ".dcm" ) );
  this->private_->export_selector_->setCurrentIndex( 0 );
  this->private_->export_selector_->setEnabled( true );
  connect( this->private_->export_selector_, SIGNAL( currentIndexChanged(int) ), SLOT( radio_button_change_path() ) );
  this->private_->bitmap_layout_->addWidget( this->private_->export_selector_ );
  
  //Warning message
  this->private_->warning_message_ = new QLabel( QString::fromUtf8( "This location does not exist, please choose a valid location." ) );
  this->private_->warning_message_->setObjectName( QString::fromUtf8( "message_" ) );
  this->private_->warning_message_->setWordWrap( true );
  this->private_->warning_message_->setStyleSheet( StyleSheet::MAIN_STYLE_C );
  this->private_->warning_message_->hide();
  
  //Main layout
  this->private_->selection_main_layout_->addWidget( this->private_->single_or_multiple_files_widget_ );
  this->private_->selection_main_layout_->addWidget( this->private_->warning_message_ );
  this->private_->selection_main_layout_->addWidget( this->private_->path_widget_ );
  this->private_->selection_main_layout_->addWidget( this->private_->name_widget_ );
  this->private_->selection_main_layout_->addWidget( this->private_->bitmap_widget_ );

  this->private_->single_file_radio_button_->setChecked( true );
}
  
void SegmentationSelectionPage::change_type_text( int index )
{
  if( index == 0 ) 
  {
    this->private_->export_label_->setText( QString::fromUtf8( "Export segmentation as: " ) );
  }
  else
  {
    this->private_->export_label_->setText( QString::fromUtf8( "Export segmentation as a series of: " ) );
  }
}
  
void SegmentationSelectionPage::initializePage()
{
  //Export path intialize
  std::string file_type = this->private_->export_selector_->currentText().toStdString();
    
  this->private_->filename_name_lineEdit_->setText(QString::fromStdString( "Untitled" + file_type ) );
  
  this->private_->filename_path_lineEdit_->setText( QString::fromStdString(
    ProjectManager::Instance()->get_current_project_folder().string() ) );
    
  this->registerField( "projectName", this->private_->filename_name_lineEdit_ );
  this->registerField( "projectPath", this->private_->filename_path_lineEdit_ );
    
  //Mask initialize
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  std::vector< LayerGroupHandle > groups;
  LayerManager::Instance()->get_groups( groups );

  QStringList masks;
  
  int group_with_active_layer = -1;
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  
  for( int i = 0; i < static_cast< int >( groups.size() ); ++i )
  {
    bool mask_found = false;

    QTreeWidgetItem *group = new QTreeWidgetItem( this->private_->group_with_masks_tree_ );
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

    LayerVector layers;
    groups[ i ]->get_layers( layers );
    LayerVector::iterator it = layers.begin();
    while( it != layers.end() )
    {
      if( ( *it ) == active_layer )
      {
        group_with_active_layer = i;
      }
      if( ( *it )->get_type() == Core::VolumeType::MASK_E )
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
      group->setCheckState( 0, Qt::Checked );
      group->setCheckState( 0, Qt::Unchecked );
      group->setDisabled( true );
    }
  }
  
  if ( group_with_active_layer >= 0 )
  { 
    this->private_->group_with_masks_tree_->topLevelItem( group_with_active_layer )->
      setCheckState( 0, Qt::Checked );
  }
}

bool SegmentationSelectionPage::validatePage()
{
  //Masks
  this->private_->warning_message_->hide();
  this->private_->masks_.clear();
  for ( int i = 0; i < this->private_->group_with_masks_tree_->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem* group = this->private_->group_with_masks_tree_->topLevelItem( i );
    for ( int j = 0; j < group->childCount(); ++j )
    {
      QTreeWidgetItem* mask = group->child( j );
      if ( mask->checkState( 0 ) == Qt::Checked )
      {
        QtLayerListWidget* new_mask = new QtLayerListWidget( this );
        new_mask->set_mask_name( mask->text( 0 ).toStdString() );
        new_mask->set_mask_index( 0 );
        this->private_->masks_.push_front( new_mask );
      }
    }
  }
  
  //Error checks for segmentation export
  QString filename = this->private_->filename_path_lineEdit_->text();
  boost::filesystem::path full_path =
  boost::filesystem::path( filename.toStdString() ) /
  boost::filesystem::path( this->private_->filename_name_lineEdit_->text().toStdString() );
    
  if( boost::filesystem::exists( full_path ) )
  {
    int ret = QMessageBox::warning( this,
      "A project with this name already exists!",
      "A project with this name already exists!\n"
      "Please rename the file or delete the existing file.\n",
      QMessageBox::Ok);
        
    if( ret == QMessageBox::Ok )
    {
      return false;
    }
  }

    
  if( !QFileInfo( filename ).exists() )
  {
    this->private_->warning_message_->setText( QString::fromUtf8(
      "This location does not exist, please choose a valid location." ) );
    this->private_->warning_message_->show();
    return false;
  }
    
  try
  {
    boost::filesystem::create_directory( boost::filesystem::path( filename.toStdString() )
      / "delete_me" );
  }
  catch( ... ) // if the create fails then we are not in a writable directory
  {
    this->private_->warning_message_->setText( QString::fromUtf8(
      "This location is not writable, please choose a valid location." ) );
      this->private_->warning_message_->show();
      return false;
    }
    // if we've made it here then we need to remove the folder we created
    boost::filesystem::remove( boost::filesystem::path( filename.toStdString() )
      / "delete_me" );
    
    if( !boost::filesystem::exists( boost::filesystem::path( filename.toStdString() ).parent_path() ) )
    {
      return false;
    }
    
    if( this->private_->single_file_radio_button_->isChecked() )
    {
      this->private_->file_name_ = filename.toStdString()
        + "/" + this->private_->filename_name_lineEdit_->text().toStdString();
    }
    else
    {
      this->private_->file_name_ = filename.toStdString();
    }
    
    return true;
}

void SegmentationSelectionPage::set_export_path()
{
  QString filename;
  boost::filesystem::path current_folder = ProjectManager::Instance()->get_current_file_folder();
    
  QFileDialog *dialog = new QFileDialog( this, tr( "Choose Directory for Export..." ) );
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setFileMode(QFileDialog::DirectoryOnly);
  dialog->setOption(QFileDialog::ShowDirsOnly);
  dialog->setOption(QFileDialog::DontResolveSymlinks);
    
  dialog->open( this, SLOT( set_filename( const QString& ) ) );
}
    
void SegmentationSelectionPage::set_filename( const QString& name )
{
  if(!name.isEmpty())
  {
    QDir export_directory_ = QDir(name);
        
    if( export_directory_.exists() )
    {
      this->private_->filename_path_lineEdit_
        ->setText( export_directory_.canonicalPath() );
    }
  }
}

void SegmentationSelectionPage::radio_button_change_path()
{
  auto file_name = this->private_->filename_name_lineEdit_->text();
  auto file_type = this->private_->export_selector_->currentText();
    
  QFileInfo info(file_name);
  if ( this->private_->single_file_radio_button_->isChecked() )
  {
      this->private_->filename_name_lineEdit_->setReadOnly(false);
      
      QString style_sheet = QString("QLineEdit[readOnly=\"false\"] {"
                                    " text-align: left;"
                                    " color: rgb(" "255,255,255" ") ;"
                                    " background-color: rgb( " "43, 43, 46" ") ;"
                                    " selection-color: rgb( " "0, 0, 0" ") ; "
                                    " border-radius: 3px;"
                                    " padding: 0px;"
                                    " margin: 0px;}");
      
      this->setStyleSheet(style_sheet);
      
      QString name = info.baseName();
      this->private_->filename_name_lineEdit_->setText(name + file_type);
  }
  else
  {
    this->private_->filename_name_lineEdit_->setReadOnly(true);

    QString style_sheet = QString("QLineEdit[readOnly=\"true\"] {"
                                  " text-align: left;"
                                  " color: rgb(" "128,128,128" ") ;"
                                  " background-color: rgb( " "43, 43, 46" ") ;"
                                  " selection-color: rgb( " "0, 0, 0" ") ; "
                                  " border-radius: 3px;"
                                  " padding: 0px;"
                                  " margin: 0px;}");
      
    this->setStyleSheet(style_sheet);
  }
}
    
SegmentationSummaryPage::SegmentationSummaryPage( SegmentationPrivateHandle private_handle, QWidget *parent )
    : QWizardPage( parent )
{
  this->private_ = private_handle;
    this->setTitle( "Export to Segmentation Summary" );

  this->setFinalPage( true );
  this->private_->summary_main_layout_ = new QVBoxLayout( this );
  this->private_->summary_main_layout_->setContentsMargins( 6, 6, 26, 6 );
  this->private_->summary_main_layout_->setSpacing( 6 );
  this->private_->summary_main_layout_->setObjectName( QString::fromUtf8( "summary_main_layout_" ) );

  this->private_->description_ = new QLabel( this );
  this->private_->description_->setWordWrap( true );
  this->private_->summary_main_layout_->addWidget( this->private_->description_ );

  this->private_->mask_scroll_area_ = new QScrollArea( this );
  this->private_->mask_scroll_area_->setObjectName( QString::fromUtf8( "mask_scroll_area_" ) );
  this->private_->mask_scroll_area_->setWidgetResizable( true );
  this->private_->mask_scroll_area_->setAlignment( Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop );
  this->private_->mask_scroll_area_->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  this->private_->mask_scroll_area_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  
  this->private_->layers_ = new QWidget( this );
  this->private_->layers_->setObjectName( QString::fromUtf8( "layers_" ) );
  this->private_->mask_scroll_area_->setStyleSheet( StyleSheet::SEGMENTATION_EXPORT_C );
  this->private_->masks_layout_ = new QVBoxLayout( this->private_->layers_ );
  this->private_->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->private_->mask_scroll_area_->setWidget( this->private_->layers_ );

  this->private_->summary_main_layout_->addWidget( this->private_->mask_scroll_area_ );

  this->setLayout( this->private_->summary_main_layout_ );
}

void SegmentationSummaryPage::initializePage()
{
  QString finishText = wizard()->buttonText(QWizard::FinishButton);
  finishText.remove('&');

  this->private_->layers_->deleteLater();

  this->private_->layers_ = new QWidget( this );
  this->private_->layers_->setObjectName( QString::fromUtf8( "layers_" ) );
  this->private_->mask_scroll_area_->setStyleSheet( StyleSheet::SEGMENTATION_EXPORT_C );

  this->private_->masks_layout_ = new QVBoxLayout( this->private_->layers_ );
  this->private_->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->private_->masks_layout_->setContentsMargins( 0, 0, 0, 0 );
  this->private_->masks_layout_->setSpacing( 0 );
  this->private_->masks_layout_->setAlignment( Qt::AlignTop );
  this->private_->mask_scroll_area_->setWidget( this->private_->layers_ );

  bool save_as_single_file = this->private_->single_file_radio_button_->isChecked();
  
  // TODO: creates new mask layer widget for background -> where should underlying mask layer be created?
  // Alternatively, don't put new mask layer widget in vector passed to action (is this possible???)
  
  // insert the background layer settings
  if ( save_as_single_file )
  {
    QtLayerListWidget* new_mask = new QtLayerListWidget( this );
    new_mask->set_mask_name( "Background" );
    this->private_->masks_layout_->addWidget( new_mask );
    this->private_->masks_.push_front( new_mask );
    connect( new_mask, SIGNAL( index_changed_signal() ), this, SIGNAL( completeChanged() ) ); 
  }
  
  for ( int i = 0; i < static_cast< int >( this->private_->masks_.size() ); ++i )
  {
    this->private_->masks_[ i ]->set_mask_index( i );
    this->private_->masks_[ i ]->hide_counter( ! save_as_single_file );
    this->private_->masks_layout_->addWidget( this->private_->masks_[ i ] );
    connect( this->private_->masks_[ i ], SIGNAL( index_changed_signal() ), this, SIGNAL( completeChanged() ) ); 
  }

  this->private_->description_->setText(  QString::fromUtf8( "Please verify that you want to export the "
    "following mask layers as a segmentation " ) );
    
}

bool SegmentationSummaryPage::validatePage()
{
  // TODO: selected_masks never gets used???
  QString selected_masks = "";
  for( int i = 0; i < this->private_->masks_.size(); ++i )
  {
    if ( selected_masks.isEmpty() )
    {
      selected_masks = this->private_->masks_[ i ]->get_label() + QString::fromUtf8( "," ) + 
        QString::number( this->private_->masks_[ i ]->get_value() );
    }
    else
    {
      selected_masks = selected_masks + QString::fromUtf8( "|" ) + 
        this->private_->masks_[ i ]->get_label() + QString::fromUtf8( "," ) + 
        QString::number( this->private_->masks_[ i ]->get_value() );
    }
  }
  
  std::vector< LayerHandle > layers;
  std::vector< double > values;
  for ( int i = 0; i < this->private_->masks_.size(); ++i )
  {
    layers.push_back( LayerManager::Instance()->find_layer_by_name( 
      this->private_->masks_[ i ]->get_label().toStdString() ) );
    values.push_back( this->private_->masks_[ i ]->get_value() );
  }
  
  LayerExporterHandle exporter;
  bool result = false;
  std::string extension = this->private_->export_selector_->currentText().toStdString();
    
  if ( extension == ".mat" )
  {
    result = LayerIO::Instance()->create_exporter( exporter, layers, "Matlab Exporter", extension );
  }
  else if ( extension == ".nrrd" )
  {
    result = LayerIO::Instance()->create_exporter( exporter, layers, "NRRD Exporter", extension );
  }
  else if ( extension == ".mrc" )
  {
    result = LayerIO::Instance()->create_exporter( exporter, layers, "MRC Exporter", extension );
  }
  else
  {
    result = LayerIO::Instance()->create_exporter( exporter, layers, "ITK Mask Exporter", extension );
  }

  if ( ! result )
  {
    std::string error_message = std::string("ERROR: No exporter is available for file '") + 
      this->private_->file_name_ + std::string("'.");

    QMessageBox message_box( this );
    message_box.setWindowTitle( "Export Layer..." );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( QString::fromStdString( error_message ) );
    message_box.exec();
    return false;
  }

  if ( this->private_->single_file_radio_button_->isChecked() )
  { 
    exporter->set_label_layer_values( values );
    ActionExportSegmentation::Dispatch( Core::Interface::GetWidgetActionContext(), exporter,
      LayerIO::LABEL_MASK_MODE_C, this->private_->file_name_, extension );
  }
  else
  {
    ActionExportSegmentation::Dispatch( Core::Interface::GetWidgetActionContext(), exporter,
      LayerIO::SINGLE_MASK_MODE_C, this->private_->file_name_, extension );
  }

  return true;
}

bool SegmentationSummaryPage::isComplete() const
{
  bool valid = true;
  for ( int i = 0; i < this->private_->masks_.size(); ++i )
  {
    bool found = false;
    for ( int j = 0; j < this->private_->masks_.size(); ++j )
    {
      if ( i == j ) continue;
      if ( this->private_->masks_[ i ]->get_value() == this->private_->masks_[ j ]->get_value() )
      {
        found = true;
        break;
      }
    }
    if ( found )
    {
      this->private_->masks_[ i ]->set_validity( false );
      valid = false;
    }
    else this->private_->masks_[ i ]->set_validity( true );
  }
  return valid;
}

} // end namespace Seg3D
