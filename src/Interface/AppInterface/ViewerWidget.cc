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


// Core includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionFlip.h>

// Application includes
#include <Application/Viewer/Actions/ActionAutoView.h>
#include <Application/ViewerManager/ViewerManager.h>

// QT includes
#include <QtGui>
#include <QtOpenGL>

// Qt Interface support classes
#include <QtInterface/Utils/QtApplication.h>
#include <QtInterface/Bridge/QtBridge.h>
#include <QtInterface/Utils/QtRenderResources.h>
#include <QtInterface/Utils/QtRenderWidget.h>

// Ui includes
#include "ui_ViewerWidget.h"

// Interface includes
#include <Interface/AppInterface/SingleShotToolButton.h>
#include <Interface/AppInterface/StyleSheet.h>
#include <Interface/AppInterface/ViewerWidget.h>

namespace Seg3D
{

class ViewerWidgetPrivate
{
public:
  Ui::ViewerWidget ui_;
  QtUtils::QtRenderWidget* render_widget_;
  
  // We need a special single shot button for the picking button
  QToolButton* picking_button_;
  
  // Handle to the underlying Viewer structure
  ViewerHandle viewer_;
  
  QIcon picking_icon_;
};

ViewerWidget::ViewerWidget( ViewerHandle viewer, QWidget *parent ) :
  QWidget( parent ),
  private_( new ViewerWidgetPrivate )
{
  
  // Setup the Icon
  this->private_->picking_icon_.addPixmap( QPixmap( ":/Images/Picking.png" ), QIcon::Normal, QIcon::On );
  this->private_->picking_icon_.addPixmap( QPixmap( ":/Images/PickingOff.png" ), QIcon::Normal, QIcon::Off );
  
  this->private_->viewer_ = viewer;
  this->private_->ui_.setupUi( this );
  
  // Setup the Custom Picking Button
  this->private_->picking_button_ =  new SingleShotToolButton( this->private_->ui_.buttonbar_ );
  this->private_->picking_button_->setCheckable( true );
  this->private_->picking_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->private_->picking_button_->setIcon( this->private_->picking_icon_ );
  this->private_->picking_button_->setText( QString( "Picking" ) );
  this->private_->picking_button_->setToolTip( QString( "Make the viewer a target for picking" ) );
  this->private_->picking_button_->setFixedHeight( 20 );
  this->private_->picking_button_->setFixedWidth( 20 );
  
  this->private_->ui_.button_layout_->insertWidget( 8, this->private_->picking_button_ );
  
  // --------------------------------------
  // Generate the OpenGL part of the widget
  this->private_->render_widget_ = QtUtils::QtApplication::Instance()->
    qt_renderresources_context()->create_qt_render_widget( this, this->private_->viewer_ );
  
  if( this->private_->render_widget_ == 0 )
  {
    CORE_THROW_LOGICERROR("OpenGL was not initialized correctly");
  }
  this->private_->render_widget_->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  this->private_->ui_.viewer_layout_->addWidget( this->private_->render_widget_ );
  

  // Update state of the widget to reflect current state
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    
    ViewerWidgetHandle viewer_widget( this );
    
    this->add_connection( this->private_->viewer_->view_mode_state_->value_changed_signal_.connect( 
      boost::bind( &ViewerWidget::HandleViewModeChanged, viewer_widget ) ) );

    this->connect( this->private_->ui_.viewer_states_,
      SIGNAL( currentIndexChanged( int ) ), SLOT( change_view_type( int ) ) );

    this->connect( this->private_->ui_.flip_horizontal_button_, SIGNAL( clicked() ),
      SLOT( flip_view_horiz() ) );

    this->connect( this->private_->ui_.flip_vertical_button_, SIGNAL( clicked() ),
      SLOT( flip_view_vert() ) );

    this->connect( this->private_->ui_.auto_view_button_, SIGNAL( clicked() ),
      SLOT( auto_view() ) );

  
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_states_, 
      this->private_->viewer_->view_mode_state_ );
    QtUtils::QtBridge::Connect( this->private_->picking_button_ , 
      this->private_->viewer_->is_picking_target_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.grid_button_, 
      this->private_->viewer_->slice_grid_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.lock_button_, 
      this->private_->viewer_->viewer_lock_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.slice_visible_button_,
      this->private_->viewer_->slice_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.slices_visible_button_,
      this->private_->viewer_->volume_slices_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.light_visible_button_,
      this->private_->viewer_->volume_light_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.isosurfaces_visible_button_,
      this->private_->viewer_->volume_isosurfaces_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.volume_rendering_visible_button_,
      this->private_->viewer_->volume_volume_rendering_visible_state_ );

    this->add_icons_to_combobox();
  }
}
  
void ViewerWidget::add_icons_to_combobox()
{ 
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/Images/Xview.png"), QSize(), QIcon::Normal, QIcon::On);
  this->private_->ui_.viewer_states_->setItemIcon( 0, icon );
  
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/Images/Yview.png"), QSize(), QIcon::Normal, QIcon::On);
  this->private_->ui_.viewer_states_->setItemIcon( 1, icon1 );
  
  QIcon icon2;
  icon2.addFile(QString::fromUtf8(":/Images/Zview.png"), QSize(), QIcon::Normal, QIcon::On);
  this->private_->ui_.viewer_states_->setItemIcon( 2, icon2 );
  
  QIcon icon3;
  icon3.addFile(QString::fromUtf8(":/Images/Vview.png"), QSize(), QIcon::Normal, QIcon::On);
  this->private_->ui_.viewer_states_->setItemIcon( 3, icon3 );  
}

ViewerWidget::~ViewerWidget()
{
  this->disconnect_all();
}

void ViewerWidget::select()
{
  this->private_->ui_.border_->setStyleSheet( StyleSheet::VIEWERSELECTED_C );
}

void ViewerWidget::deselect()
{
  this->private_->ui_.border_->setStyleSheet( StyleSheet::VIEWERNOTSELECTED_C );
}

void ViewerWidget::change_view_type( int index )
{
  bool is_volume_view = ( index == 3 );

  // 2D viewer specific buttons
  this->private_->ui_.flip_horizontal_button_->setVisible( !is_volume_view );
  this->private_->ui_.flip_vertical_button_->setVisible( !is_volume_view );
  this->private_->ui_.grid_button_->setVisible( !is_volume_view );
  this->private_->picking_button_->setVisible( !is_volume_view );
  this->private_->ui_.slice_visible_button_->setVisible( !is_volume_view );

  // 3D viewer specific buttons
  this->private_->ui_.slices_visible_button_->setVisible( is_volume_view );
  this->private_->ui_.light_visible_button_->setVisible( is_volume_view );
  this->private_->ui_.isosurfaces_visible_button_->setVisible( is_volume_view );
  this->private_->ui_.volume_rendering_visible_button_->setVisible( is_volume_view); 
}
  
void ViewerWidget::HandleViewModeChanged( ViewerWidgetHandle viewer_widget )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &ViewerWidget::HandleViewModeChanged,
      viewer_widget ) );
    return;
  }
  
  if( viewer_widget.data() )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    bool is_volume_view = viewer_widget->private_->viewer_->is_volume_view();

    if( !is_volume_view )
    {
      Core::StateView2D* view2d_state = static_cast< Core::StateView2D* >( 
        viewer_widget->private_->viewer_->get_active_view_state().get() );
      viewer_widget->private_->ui_.flip_horizontal_button_->setChecked( view2d_state->x_flipped() );
      viewer_widget->private_->ui_.flip_vertical_button_->setChecked( view2d_state->y_flipped() );
    }
  }
}
  
void ViewerWidget::flip_view_horiz()
{
  if( ! this->private_->viewer_->is_volume_view() )
  {
    Core::StateView2DHandle view2d_state = boost::dynamic_pointer_cast<Core::StateView2D>( 
      this->private_->viewer_->get_active_view_state() );
    Core::ActionFlip::Dispatch( view2d_state, Core::FlipDirectionType::HORIZONTAL_E );
  }
}

void ViewerWidget::flip_view_vert()
{
  if( ! this->private_->viewer_->is_volume_view() )
  {
    Core::StateView2DHandle view2d_state = boost::dynamic_pointer_cast<Core::StateView2D>( 
      this->private_->viewer_->get_active_view_state() );
    Core::ActionFlip::Dispatch( view2d_state, Core::FlipDirectionType::VERTICAL_E );
  }
}

void ViewerWidget::auto_view()
{
  ActionAutoView::Dispatch( this->private_->viewer_ );
}

} // end namespace Seg3D
