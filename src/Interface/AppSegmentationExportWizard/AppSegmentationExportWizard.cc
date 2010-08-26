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
#include <Application/Layer/LayerGroup.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionExportSegmentation.h>

namespace Seg3D
{

AppSegmentationExportWizard::AppSegmentationExportWizard( QWidget *parent ) :
    QWizard( parent )
{
    this->addPage( new SegmentationSelectionPage );
    this->addPage( new SegmentationSummaryPage );
  
  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );
  
  this->setWindowTitle( tr( "Segmentation Export Wizard" ) );
}

AppSegmentationExportWizard::~AppSegmentationExportWizard()
{
}

void AppSegmentationExportWizard::accept()
{
  ActionExportSegmentation::Dispatch( Core::Interface::GetWidgetActionContext(),
    field( "segmentationName" ).toString().toStdString(), 
    field( "maskList" ).toString().toStdString(),
    field( "segmentationPath" ).toString().toStdString() );
    QDialog::accept();
}

SegmentationSelectionPage::SegmentationSelectionPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setSubTitle( "Choose the layers that you would like to export as a segmentation." );

  QString default_name = QString::fromStdString( ProjectManager::Instance()->
    current_project_->project_name_state_->get() );

  boost::filesystem::path desktop_path;
  Core::Application::Instance()->get_user_desktop_directory( desktop_path );

  this->mask_list_ = new QLineEdit( this );
  this->mask_list_->hide();

  this->main_layout_ = new QVBoxLayout( this );
  this->main_layout_->setContentsMargins(6, 6, 26, 6);
  this->main_layout_->setObjectName( QString::fromUtf8( "main_layout_" ) );
  this->segmentation_top_widget_ = new QWidget( this );
  this->segmentation_top_widget_->setObjectName( QString::fromUtf8( 
    "segmentation_top_widget_" ) );
  this->segmentation_top_widget_->setMinimumSize( QSize( 0, 96 ) );

  this->segmentation_top_widget_->setStyleSheet( QString::fromUtf8( 
    "QWidget#segmentation_top_widget_{ "
    //" margin-right: 14px;"
    " background-color: white;"
    "}" ) ); 

  this->verticalLayout = new QVBoxLayout( this->segmentation_top_widget_ );
  this->verticalLayout->setContentsMargins(0, 0, 0, 0);
  this->verticalLayout->setObjectName( QString::fromUtf8( "verticalLayout" ) );
  this->verticalLayout->setSpacing( 0 );
  this->segmentation_name_widget_ = new QWidget( this->segmentation_top_widget_ );
  this->segmentation_name_widget_->setObjectName( QString::fromUtf8( "segmentation_name_widget_" ) );
  this->segmentation_name_widget_->setMinimumSize( QSize(0, 32 ) );
  this->horizontalLayout = new QHBoxLayout( this->segmentation_name_widget_ );
  this->horizontalLayout->setSpacing(0);
  this->horizontalLayout->setContentsMargins(0, 0, 0, 0);
  this->horizontalLayout->setObjectName( QString::fromUtf8( "horizontalLayout" ) );
  this->segmentation_name_label_ = new QLabel( this->segmentation_name_widget_ );
  this->segmentation_name_label_->setObjectName( QString::fromUtf8( "segmentation_name_label_" ) );
  this->segmentation_name_label_->setText( QString::fromUtf8( "Segmentation name:" ) );

  this->horizontalLayout->addWidget( this->segmentation_name_label_ );

  this->segmentation_name_lineedit_ = new QLineEdit( this->segmentation_name_widget_ );
  this->segmentation_name_lineedit_->setObjectName( QString::fromUtf8( "segmentation_name_lineedit_" ) );
  this->segmentation_name_lineedit_->setText( default_name );

  this->horizontalLayout->addWidget( this->segmentation_name_lineedit_ );

  this->horizontalLayout->setStretch( 0, 1 );
  this->horizontalLayout->setStretch( 1, 2 );

  this->verticalLayout->addWidget( this->segmentation_name_widget_ );

  this->widget_2 = new QWidget( this->segmentation_top_widget_ );
  this->widget_2->setObjectName( QString::fromUtf8( "widget_2" ) );
  this->widget_2->setMinimumSize( QSize( 0, 64 ) );
  this->verticalLayout_2 = new QVBoxLayout( this->widget_2 );
  this->verticalLayout_2->setSpacing(0);
  this->verticalLayout_2->setContentsMargins(0, 0, 0, 0);
  this->verticalLayout_2->setObjectName( QString::fromUtf8( "verticalLayout_2" ) );
  this->widget_3 = new QWidget( this->widget_2 );
  this->widget_3->setObjectName( QString::fromUtf8( "widget_3" ) );
  this->widget_3->setMinimumSize( QSize( 0, 32 ) );
  this->horizontalLayout_2 = new QHBoxLayout( this->widget_3 );
  this->horizontalLayout_2->setSpacing( 0 );
  this->horizontalLayout_2->setContentsMargins( 0, 0, 0, 0 );
  this->horizontalLayout_2->setObjectName( QString::fromUtf8( "horizontalLayout_2" ) );
  this->segmentation_path_label_ = new QLabel( this->widget_3 );
  this->segmentation_path_label_->setObjectName( QString::fromUtf8( "segmentation_path_label_" ) );
  this->segmentation_path_label_->setText( QString::fromUtf8( "Path:" ) );

  this->horizontalLayout_2->addWidget( this->segmentation_path_label_ );

  this->segmentation_path_lineedit_ = new QLineEdit( this->widget_3 );
  this->segmentation_path_lineedit_->setObjectName( QString::fromUtf8( "segmentation_path_lineedit_" ) );
  this->segmentation_path_lineedit_->setText( QString::fromStdString( desktop_path.string() ) );

  this->horizontalLayout_2->addWidget( this->segmentation_path_lineedit_ );

  this->horizontalLayout_2->setStretch( 0, 1 );
  this->horizontalLayout_2->setStretch( 1, 2 );

  this->verticalLayout_2->addWidget( widget_3 );

  this->widget_4 = new QWidget( this->widget_2 );
  this->widget_4->setObjectName( QString::fromUtf8( "widget_4" ) );
  this->widget_4->setMinimumSize( QSize( 0, 32 ) );
  this->horizontalLayout_3 = new QHBoxLayout( this->widget_4 );
  this->horizontalLayout_3->setSpacing( 0 );
  this->horizontalLayout_3->setContentsMargins( 0, 0, 0, 0 );
  this->horizontalLayout_3->setObjectName( QString::fromUtf8( "horizontalLayout_3" ) );
  this->horizontalSpacer = new QSpacerItem(277, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  this->horizontalLayout_3->addItem( this->horizontalSpacer );

  this->set_path_button_ = new QPushButton( this->widget_4 );
  this->set_path_button_->setObjectName( QString::fromUtf8( "set_path_button_" ) );
  this->set_path_button_->setMinimumSize( QSize( 100, 0 ) );
  this->set_path_button_->setMaximumSize( QSize( 100, 16777215 ) );
  this->set_path_button_->setText( QString::fromUtf8( "Change Path" ) );

  connect( this->set_path_button_, SIGNAL( clicked() ), this, SLOT( set_path() ) );

  this->horizontalLayout_3->addWidget( this->set_path_button_ );
  this->verticalLayout_2->addWidget( this->widget_4 );
  this->verticalLayout->addWidget( this->widget_2 );

  main_layout_->addWidget( this->segmentation_top_widget_ );

  this->group_with_masks_tree_ = new QTreeWidget( this );
  this->group_with_masks_tree_->setObjectName( QString::fromUtf8( "group_with_masks_tree_" ) );
  this->group_with_masks_tree_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->group_with_masks_tree_->setProperty( "showDropIndicator", QVariant( false ) );
  this->group_with_masks_tree_->setAlternatingRowColors( true );
  this->group_with_masks_tree_->setIndentation( 15 );
  this->group_with_masks_tree_->setItemsExpandable( true );
  this->group_with_masks_tree_->setHeaderHidden( true );

  main_layout_->addWidget( group_with_masks_tree_ );

  registerField( "maskList", this->mask_list_ );
  registerField( "segmentationName", this->segmentation_name_lineedit_ );
  registerField( "segmentationPath", this->segmentation_path_lineedit_ );

}
  
void SegmentationSelectionPage::initializePage()
{
  std::vector< LayerGroupHandle > groups;
  LayerManager::Instance()->get_groups( groups );

  QStringList masks;
  
  for( int i = 0; i < static_cast< int >( groups.size() ); ++i )
  {
    bool mask_found = false;

    QTreeWidgetItem *group = new QTreeWidgetItem( this->group_with_masks_tree_ );
    group->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate );
    group->setCheckState( 0, Qt::Checked );
      
    std::string group_name = Core::ExportToString( 
    groups[ i ]->get_grid_transform().get_nx() ) + " x " +
    Core::ExportToString( groups[ i ]->get_grid_transform().get_ny() ) + " x " +
    Core::ExportToString( groups[ i ]->get_grid_transform().get_nz() );
    group->setText( 0,  QString::fromUtf8( "Group - " ) + QString::fromStdString( group_name ) );
    group->setExpanded( true );

    layer_list_type layers = groups[ i ]->get_layer_list();
    layer_list_type::iterator it = layers.begin();
    while( it != layers.end() )
    {
      if( ( *it )->type() == Core::VolumeType::MASK_E )
      {
        QTreeWidgetItem *mask = new QTreeWidgetItem( group );
        mask->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        mask->setCheckState( 0, Qt::Checked );
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
  setField( "maskList", selected_masks );
  setField( "segmentationName", this->segmentation_name_lineedit_->text() );
  return true;
}

void SegmentationSelectionPage::set_path()
{
  QDir path_directory_;
  QString path = QFileDialog::getExistingDirectory ( this, "Directory",
    this->segmentation_path_lineedit_->text() );

  if ( path.isNull() == false )
  {
    path_directory_.setPath( path );
  }
  this->segmentation_path_lineedit_->setText( path_directory_.canonicalPath() );
  setField( "segmentationPath", this->segmentation_path_lineedit_->text() );
}

SegmentationSummaryPage::SegmentationSummaryPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Export to Segmentation Summary" );

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

  this->scroll_contents_ = new QWidget();
  this->scroll_contents_->setObjectName( QString::fromUtf8( "scroll_contents_" ) );
  this->masks_layout_ = new QVBoxLayout( this->scroll_contents_ );
  this->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->masks_layout_->setAlignment( Qt::AlignTop );
  this->mask_scroll_area_->setWidget( scroll_contents_ );

  this->main_layout_->addWidget( this->mask_scroll_area_ );
  this->setLayout( main_layout_ );

}

void SegmentationSummaryPage::initializePage()
{
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove('&');
  
  QString masks = field( "maskList" ).toString();
  std::vector< std::string > selected_masks_vector = 
    Core::SplitString( masks.toStdString(), "|" );

  this->scroll_contents_->deleteLater();

  this->scroll_contents_ = new QWidget();
  this->scroll_contents_->setObjectName( QString::fromUtf8( "scroll_contents_" ) );
  this->masks_layout_ = new QVBoxLayout( this->scroll_contents_ );
  this->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->masks_layout_->setAlignment( Qt::AlignTop );
  this->mask_scroll_area_->setWidget( scroll_contents_ );

  for( int i = 0; i < static_cast< int >( selected_masks_vector.size() ); ++i )
  {
    QLabel* mask_label = new QLabel( this->scroll_contents_ );
    mask_label->setText( QString::fromStdString( selected_masks_vector[ i ] ) );
    mask_label->setMinimumSize( QSize(0, 16 ) );
    mask_label->setMaximumSize( QSize(16777215, 16 ) );
    this->masks_layout_->addWidget( mask_label );
  }

  this->description_->setText(  QString::fromUtf8( "Please verify that you want to export the following "
    "mask layers as a segmentation with the name: '" ) + field( "segmentationName" ).toString() 
    + QString::fromUtf8( "'" ) );

    //this->segmentation_name_->setText( QString::fromUtf8( "Project Name: " ) + field( "projectName" ).toString() );
   
  
}

} // end namespace Seg3D
