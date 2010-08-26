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


// Interface includes
#include <Interface/AppSegmentationExportWizard/AppSegmentationExportWizard.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/LayerManager/LayerManager.h>

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
//  ActionNewProject::Dispatch( Core::Interface::GetWidgetActionContext(),
//    field("projectPath").toString().toStdString(),
//    field("segmentationName").toString().toStdString() );
    QDialog::accept();
}

SegmentationSelectionPage::SegmentationSelectionPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setSubTitle( "Choose the layers that you would like to export as a segmentation." );

  QString default_name = QString::fromStdString( ProjectManager::Instance()->
    current_project_->project_name_state_->get() );

  this->mask_list_ = new QLineEdit( this );
  this->mask_list_->hide();

  this->main_layout_ = new QVBoxLayout( this );
  this->main_layout_->setContentsMargins( 6, 6, 6, 6 );
  this->main_layout_->setObjectName( QString::fromUtf8( "main_layout_" ) );
  this->segmentation_title_widget_ = new QWidget( this );
  this->segmentation_title_widget_->setObjectName( 
    QString::fromUtf8( "segmentation_title_widget_" ) );
  this->segmentation_title_widget_->setMinimumSize(QSize(0, 32));
  this->segmentation_title_widget_->setMaximumSize(QSize(16777215, 32));
  this->segmentation_name_layout_ = new QHBoxLayout( this->segmentation_title_widget_ );
#ifndef Q_OS_MAC
  this->segmentation_name_layout_->setSpacing( 6 );
#endif
  this->segmentation_name_layout_->setContentsMargins( 6, 6, 6, 6 );
  this->segmentation_name_layout_->setObjectName( 
    QString::fromUtf8( "segmentation_name_layout_" ) );
  this->segmentation_name_label_ = new QLabel( this->segmentation_title_widget_);
  this->segmentation_name_label_->setObjectName( 
    QString::fromUtf8( "segmentation_name_label_" ) );
  this->segmentation_name_label_->setText( QString::fromUtf8( "Segmentation Name:" ) );

  this->segmentation_name_layout_->addWidget( this->segmentation_name_label_);

  this->segmentation_name_lineedit_ = new QLineEdit( this->segmentation_title_widget_ );
  this->segmentation_name_lineedit_->setObjectName( 
    QString::fromUtf8( "segmentation_name_lineedit_" ) );
  this->segmentation_name_lineedit_->setText( default_name );
  
  this->segmentation_name_layout_->addWidget( this->segmentation_name_lineedit_ );
  this->segmentation_title_widget_->setStyleSheet( QString::fromUtf8( "margin-right: 14px;" ) );


  this->main_layout_->addWidget( this->segmentation_title_widget_ );

  this->group_with_masks_tree_ = new QTreeWidget( this );
  this->group_with_masks_tree_->setObjectName( QString::fromUtf8( "group_with_masks_tree_" ) );
  this->group_with_masks_tree_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->group_with_masks_tree_->setProperty( "showDropIndicator", QVariant( false ) );
  this->group_with_masks_tree_->setAlternatingRowColors( true );
  this->group_with_masks_tree_->setIndentation( 15 );
  this->group_with_masks_tree_->setItemsExpandable( true );
  this->group_with_masks_tree_->setHeaderHidden( true );

  this->group_with_masks_tree_->setStyleSheet( QString::fromUtf8( "margin-right: 20px;" ) );
  this->setStyleSheet( QString::fromUtf8( "background-color: white;" ) );

  main_layout_->addWidget( group_with_masks_tree_ );

  registerField( "maskList", this->mask_list_ );
  registerField( "segmentationName", this->segmentation_name_lineedit_ );

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
          selected_masks = selected_masks + QString::fromUtf8("|") + mask->text( 0 );
        }
      }
    }
  }
  setField( "maskList", selected_masks );
  setField( "segmentationName", this->segmentation_name_lineedit_->text() );
  return true;
}


SegmentationSummaryPage::SegmentationSummaryPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Export to Segmentation Summary" );

  this->main_layout_ = new QVBoxLayout( this );
  this->main_layout_->setContentsMargins( 6, 6, 6, 6 );
  this->main_layout_->setObjectName( QString::fromUtf8("main_layout_") );

  this->mask_scroll_area_ = new QScrollArea( this );
  this->mask_scroll_area_->setObjectName( QString::fromUtf8( "mask_scroll_area_" ) );
  this->mask_scroll_area_->setWidgetResizable( true );
  this->mask_scroll_area_->setAlignment( Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop );
  this->mask_scroll_area_->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  this->mask_scroll_area_->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->mask_scroll_area_->setStyleSheet( QString::fromUtf8( "margin-right: 20px;" ) );
  this->setStyleSheet( QString::fromUtf8( "background-color: white;" ) );

  this->scroll_contents_ = new QWidget();
  this->scroll_contents_->setObjectName(QString::fromUtf8( "scroll_contents_" ) );
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
  this->scroll_contents_->setObjectName(QString::fromUtf8( "scroll_contents_" ) );
  this->masks_layout_ = new QVBoxLayout( this->scroll_contents_ );
  this->masks_layout_->setObjectName( QString::fromUtf8( "masks_layout_" ) );
  this->masks_layout_->setAlignment( Qt::AlignTop );
  this->mask_scroll_area_->setWidget( scroll_contents_ );



  for( int i = 0; i < static_cast< int >( selected_masks_vector.size() ); ++i )
  {
    QLabel* mask_label = new QLabel( this->scroll_contents_ );
    mask_label->setText( QString::fromStdString( selected_masks_vector[ i ] ) );
    mask_label->setMinimumSize(QSize(0, 16));
    mask_label->setMaximumSize(QSize(16777215, 16));
    this->masks_layout_->addWidget( mask_label );
  }

  this->setSubTitle(  QString::fromUtf8( "Please verify that you want to export the following "
    "mask layers as a segmentation with the name: '" ) + field( "segmentationName" ).toString() 
    + QString::fromUtf8( "'" ) );

    //this->segmentation_name_->setText( QString::fromUtf8( "Project Name: " ) + field("projectName").toString() );
   
  
}

} // end namespace Seg3D
