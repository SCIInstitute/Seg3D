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
#include <sstream>
#include <iostream>

// Boost includes
#include <boost/lexical_cast.hpp>

// Utils includes
#include <Utils/Core/Log.h>

#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/LayerManagerWidget.h>

// Gt Gui Includes
#include "ui_LayerGroupWidget.h"
#include "ui_LayerWidget.h"

namespace Seg3D  {

typedef boost::shared_ptr <Ui::LayerGroupWidget> LayerGroupWidgetHandle_type;
typedef std::list<LayerGroupWidgetHandle_type> GroupList_type;

typedef boost::shared_ptr <Ui::LayerWidget> LayerWidgetHandle_type;
typedef std::list<LayerWidgetHandle_type> LayerList_type;


class LayerManagerWidgetPrivate {
  public:
    Ui::LayerGroupWidget group_ui_;
    Ui::LayerWidget layer_ui_;

    GroupList_type group_list_;
    LayerList_type layer_list_;
};


LayerManagerWidget::LayerManagerWidget( QWidget* parent ) :
  QScrollArea( parent )
{ 
  //initialize the number of groups
  number_of_groups_ = 0;

  { // initialize the icons
    active_close_icon_.addFile(QString::fromUtf8(":/Images/CloseWhite.png"), 
                                QSize(), QIcon::Normal, QIcon::Off);    
    inactive_close_icon_.addFile(QString::fromUtf8(":/Images/Close.png"), 
                                QSize(), QIcon::Normal, QIcon::Off);

    expand_close_group_icon_.addFile(QString::fromUtf8(":/Images/RightArrowWhite.png"), 
                                QSize(), QIcon::Normal, QIcon::Off);
    expand_close_group_icon_.addFile(QString::fromUtf8(":/Images/DownArrowWhite.png"), 
                                QSize(), QIcon::Normal, QIcon::On);


  } // end initialize the icons

  // make a new LayerManagerWidgetPrivateHandle_type object
  private_ = LayerManagerWidgetPrivateHandle( new LayerManagerWidgetPrivate );
  
  // set some values for the scrollarea widget
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  setContentsMargins( 1, 1, 1, 1 );
  setWidgetResizable( true );  

  main_ = new QWidget( parent );
  setWidget( main_ );
 
  main_layout_ = new QVBoxLayout( main_ );
  main_layout_->setContentsMargins( 1, 1, 1, 1 );
  main_layout_->setSpacing( 1 );

  group_layout_ = new QVBoxLayout;
  main_layout_->addLayout( group_layout_ );
  main_layout_->addStretch();

  main_->setLayout( main_layout_ );
  main_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );


  // add test groups
  new_group( QString::fromUtf8( "600x300x240" ));
  new_group( QString::fromUtf8( "600x300x240" ));
  new_group( QString::fromUtf8( "400x300x340" ));
  new_group( QString::fromUtf8( "300x200x400" ));
  new_group( QString::fromUtf8( "800x300x940" ));
  new_group( QString::fromUtf8( "900x200x300" ));
  new_group( QString::fromUtf8( "700x300x440" ));
  new_group( QString::fromUtf8( "100x200x200" ));

  //add test layers
  new_layer(DATA_LAYER_E, QString::fromUtf8( "Green Footed Lemur Fetus" ), QString::fromUtf8( "300x200x400" ));
  //new_layer(DATA_LAYER_E, QString::fromUtf8("Mouse Specimen #1"), QString::fromUtf8("300x200x400") );
  //new_layer(DATA_LAYER_E, QString::fromUtf8("3rd Trimester Monkey Brain"), QString::fromUtf8("300x200x400") );
  //new_layer(DATA_LAYER_E, QString::fromUtf8("Green Footed Lemur Fetus"), QString::fromUtf8("300x200x400") );

}
// destructor
LayerManagerWidget::~LayerManagerWidget()
{ }


  // create a new group and add it to the group_layout_
  // add it also to the group_list_
  void
  LayerManagerWidget::new_group( const QString &dimensions )
  {
    if ( !validate_new_layer(dimensions) )
    {
      return;
    }

    // create a new widget to add to the group_layout_
    QWidget* new_group_ = new QWidget();

    private_->group_ui_.setupUi( new_group_ );
    
    // create a handle to the group ui object
    LayerGroupWidgetHandle_type group_handle_ ( new Ui::LayerGroupWidget( private_->group_ui_ ));
    
    // set some values of the GUI
    group_handle_->activate_button_->setText( dimensions );
    group_handle_->close_button_->setIcon( active_close_icon_ );
    group_handle_->close_button_->setIconSize( QSize( 16, 16 ));
    group_handle_->open_button_->setIcon( expand_close_group_icon_ );
    group_handle_->open_button_->setIconSize( QSize( 16, 16 ));
    group_handle_->open_button_->setChecked( false );
    group_handle_->group_frame_->hide();

    // hide the tool bars
    group_handle_->roi_->hide();
    group_handle_->resample_->hide();

    // connect the signals and slots
    connect(group_handle_->open_button_, SIGNAL( clicked( bool )), this, SLOT(hide_show_layers( bool )));
    connect(group_handle_->resample_button_group_, SIGNAL( clicked( bool ) ), this, SLOT(hide_show_resample( bool )));
    connect(group_handle_->roi_button_group_, SIGNAL( clicked( bool )), this, SLOT(hide_show_roi( bool )));
    
    // add the new widget to the group_layout_
    group_layout_->addWidget( new_group_ );

    // add the new group handle to the group_list_
    private_->group_list_.push_back(group_handle_);

    // increment the number of groups
    number_of_groups_++;
  } // end new_group

  
  // iterate through all the current layers and check to see if a layer with similar dimensions already exists
  bool
  LayerManagerWidget::validate_new_layer( const QString &dimensions )
  {
    for ( GroupList_type::const_iterator i = private_->group_list_.begin(); i != private_->group_list_.end(); i++ )
    {
      if ( (*i)->activate_button_->text() == dimensions )
      { 
        SCI_LOG_ERROR(std::string( "A group with dimensions: '") +  dimensions.toStdString() + "' already exists, not creating another." );
        return false; 
      }
    }
    return true;
  }


  // create a new layer and add it to the proper group
  // add it also to the group_list_
  void
  LayerManagerWidget::new_layer( int type, const QString &label, const QString &dimensions )
  {
    // create a new widget to add to the group layout
    QWidget* new_layer_ = new QWidget();
    private_->layer_ui_.setupUi( new_layer_ );

    // create a handle to the layer ui object
    LayerWidgetHandle_type layer_ ( new Ui::LayerWidget( private_->layer_ui_ ));

    // set some values in the GUI
    layer_->label_->setText(label);

    // hide the tool bars and the selection checkbox
    layer_->color_bar_->hide();
    layer_->bright_contrast_bar_->hide();
    layer_->selection_checkbox_->hide();

    // connect the signals and slots
    connect( layer_->colorChooseButton_, SIGNAL(clicked( bool )), this, SLOT( hide_show_color_choose_bar( bool )));
    connect( layer_->brightContrastButton_, SIGNAL(clicked( bool )), this, SLOT( hide_show_brightness_contrast_bar( bool )));
   
    // add the new widget to the appropriate group's group_frame_layout_
    for ( GroupList_type::const_iterator i = private_->group_list_.begin(); i != private_->group_list_.end(); i++ )
    {
      if ( (*i)->activate_button_->text() == dimensions )
      {
        (*i)->group_frame_layout_->addWidget( new_layer_ );
        (*i)->group_frame_->show();
        (*i)->open_button_->setChecked( true );
      }
    }

    // add the new layer handle to the layer_list_
    private_->layer_list_.push_back(layer_);

  } // end new_layer
    

  // --- Functions for Hiding and showing Layer and Group options --- //

  // hide or show the Resample tool box
  void
  LayerManagerWidget::hide_show_resample( bool down )
  {
    QPushButton *hide_show_button = ::qobject_cast<QPushButton*>( sender() );
    
    for( GroupList_type::const_iterator i = private_->group_list_.begin(); i != private_->group_list_.end(); i++ )
    {
      if( (*i)->resample_button_group_ == hide_show_button )
      {
        if( down ) 
        {
          (*i)->resample_->show();
          (*i)->roi_->hide();
          (*i)->roi_button_group_->setChecked(false);
        }
        else 
        {
          (*i)->resample_->hide();
        }
      } 
    }

    for (LayerList_type::const_iterator i = private_->layer_list_.begin(); i != private_->layer_list_.end(); i++)
    {
      if( down ) (*i)->selection_checkbox_->show();
      else (*i)->selection_checkbox_->hide();
    }
  } // end hide_show_resample


  // hide or show the Region Of Interest tool box
  void
  LayerManagerWidget::hide_show_roi( bool down )
  {
    QPushButton *hide_show_button = ::qobject_cast<QPushButton*>( sender() );
    QString dimensions_;
    
    for ( GroupList_type::const_iterator i = private_->group_list_.begin(); i != private_->group_list_.end(); i++ )
    {
      if ( (*i)->roi_button_group_ == hide_show_button )
      {
        dimensions_ = (*i)->activate_button_->text();
        if ( down ) 
        {
          (*i)->roi_->show();
          (*i)->resample_->hide();
          (*i)->resample_button_group_->setChecked( false );
        }
        else
        {
          (*i)->roi_->hide();
        }
      } 
    } 
    
    // hide or show the checkboxes
    // TODO - figure out how to link the layers to the groups
    for ( LayerList_type::const_iterator i = private_->layer_list_.begin(); i != private_->layer_list_.end(); i++ )
    {
      if( down ) (*i)->selection_checkbox_->show();
      else (*i)->selection_checkbox_->hide();
    }
  } // end hide_show_roi


  
  void
  LayerManagerWidget::hide_show_layers( bool hideshow )
  {
    QToolButton *hide_show_button = ::qobject_cast<QToolButton*>( sender() );
    
    for( GroupList_type::const_iterator i = private_->group_list_.begin(); i != private_->group_list_.end(); i++)
    {
      if( (*i)->open_button_ == hide_show_button )
      {
        if( hideshow ) 
        {
          (*i)->group_frame_->show();
        }
        else 
        {
          (*i)->group_frame_->hide();
        }
      } 
    }
  } // end hide_show_layers



  // hide or show the color choose bar
  void
  LayerManagerWidget::hide_show_color_choose_bar( bool hideshow )
  {
    QToolButton *hide_show_button = ::qobject_cast<QToolButton*>( sender() );
    
    for( LayerList_type::const_iterator i = private_->layer_list_.begin(); i != private_->layer_list_.end(); i++ )
    {
      if( (*i)->colorChooseButton_ == hide_show_button )
      {
        if( hideshow ) 
        {
          (*i)->color_bar_->show();
          (*i)->bright_contrast_bar_->hide();
        }
        else
        {
          (*i)->brightContrastButton_->setChecked( false );
          (*i)->color_bar_->hide();
          (*i)->bright_contrast_bar_->hide();
        }
      } 
    }
  } // end hide_show_color_choose_bar


  // hide or show the brightness/contrast bar
  void LayerManagerWidget::hide_show_brightness_contrast_bar( bool hideshow )
  {
    QToolButton *hide_show_button = ::qobject_cast<QToolButton*>( sender() );
  
    for( LayerList_type::const_iterator i = private_->layer_list_.begin(); i != private_->layer_list_.end(); i++ )
    {
      if( (*i)->brightContrastButton_ == hide_show_button )
      {
        if( hideshow ) 
        {
          (*i)->bright_contrast_bar_->show();
          (*i)->color_bar_->hide();
        }
        else
        {
          (*i)->colorChooseButton_->setChecked( false );
          (*i)->color_bar_->show();
          (*i)->bright_contrast_bar_->hide();
        }
      } 
    }
  } // end hide_show_brightness_contrast_bar

  void LayerManagerWidget::color_button_clicked()
  {
    QToolButton *color_button = ::qobject_cast<QToolButton*>( sender() );
  
    for ( LayerList_type::const_iterator i = private_->layer_list_.begin(); i != private_->layer_list_.end(); i++ )
    {
      if ( (*i)->color_button_01_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(251,255,74);}" ));
      }
      if ( (*i)->color_button_02_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(248,188,37);}" ));
      }
      if ( (*i)->color_button_03_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(249,152,28);}" ));
      }
      if ( (*i)->color_button_04_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(251,78,15);}" ));
      }
      if ( (*i)->color_button_05_ == color_button ) 
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(252,21,17);}" ));
      }
      if ( (*i)->color_button_06_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(166,12,73);}" ));
      }
      if ( (*i)->color_button_07_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(135,0,172);}" ));
      }
      if ( (*i)->color_button_08_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(66,0,161);}" ));
      }
      if ( (*i)->color_button_09_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(45,66,250);}" ));
      }
      if ( (*i)->color_button_10_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(32,146,204);}" ));
      }
      if ( (*i)->color_button_11_ == color_button ) 
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(100,177,61);}" ));
      }
      if ( (*i)->color_button_12_ == color_button )
      {
        (*i)->typeBackground_->setStyleSheet( QString::fromUtf8( "QWidget#typeBackground_{background-color: rgb(205,235,66);}" ));
      }
    }
  } // end color_button_clicked


  } //end Seg3D namespace

