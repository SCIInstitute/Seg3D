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
#include <Core/State/Actions/ActionFlip.h>

// Application includes
#include <Application/Viewer/Actions/ActionAutoView.h>
#include <Application/ViewerManager/ViewerManager.h>

// QT includes
#include <QtGui>
#include <QtOpenGL>

// Qt Interface support classes
#include <Interface/QtInterface/QtApplication.h>
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/QtInterface/QtRenderResources.h>
#include <Interface/QtInterface/QtRenderWidget.h>


// Interface includes
#include <Interface/AppInterface/SingleShotToolButton.h>
#include <Interface/AppInterface/StyleSheet.h>
#include <Interface/AppInterface/ViewerWidget.h>

namespace Seg3D
{

class ViewerWidgetPrivate
{
public:
  ViewerWidgetPrivate( QWidget *parent );

public:
  QVBoxLayout* layout_;
  QtRenderWidget* viewer_;
  QFrame* buttonbar_;

  QHBoxLayout* buttonbar_layout_;

  QToolButton* viewer_type_button_;
  QToolButton* auto_view_button_;
  QToolButton* grid_button_;
  QToolButton* lock_button_;

  QToolButton* flip_horiz_button_;
  QToolButton* flip_vert_button_;

  //QToolButton* next_slice_button_;
  //QToolButton* previous_slice_button_;

  QToolButton* picking_button_;

  QToolButton* slice_visible_button_;

  // Viewer actions
  // Which viewer to use
  QAction* axial_viewer_;
  QAction* sagittal_viewer_;
  QAction* coronal_viewer_;
  QAction* volume_viewer_;

  QActionGroup* viewer_selection_;
  QMenu* viewer_menu_;

  // Autoview a specific viewer
  QAction* auto_view_;
  // Switch on grid in 2D viewer
  QAction* grid_;

  // Lock zoom/pan/slice advance etc with viewers of the same type
  QAction* lock_;

  // Flip the viewer horizontally / vertically
  QAction* flip_horiz_;
  QAction* flip_vert_;

  // visibility buttons for 2D viewer
  QAction* slice_visible_;

  // visibility buttons for 3D viewer
  QAction* slices_visible_;
  QAction* isosurfaces_visible_;
  QAction* volume_visible_;

  QColor select_color_;
  QColor select_color_dark_;
  QColor deselect_color_;
  QColor deselect_color_dark_;

};

ViewerWidgetPrivate::ViewerWidgetPrivate( QWidget *parent )
{
  // ensure viewer selection is mutual exclusive
  this->viewer_selection_ = new QActionGroup( parent );

  QIcon sagittal_icon;
  sagittal_icon.addPixmap( QPixmap( ":/Images/Xview.png" ), QIcon::Normal, QIcon::Off );

  QIcon coronal_icon;
  coronal_icon.addPixmap( QPixmap( ":/Images/Yview.png" ), QIcon::Normal, QIcon::Off );

  QIcon axial_icon;
  axial_icon.addPixmap( QPixmap( ":/Images/Zview.png" ), QIcon::Normal, QIcon::Off );

  QIcon volume_icon;
  volume_icon.addPixmap( QPixmap( ":/Images/Vview.png" ), QIcon::Normal, QIcon::Off );

  QIcon auto_view_icon;
  auto_view_icon.addPixmap( QPixmap( ":/Images/AutoViewOff.png" ), QIcon::Normal, QIcon::Off );

  QIcon picking_icon;
  picking_icon.addPixmap( QPixmap( ":/Images/Picking.png" ), QIcon::Normal, QIcon::On );
  picking_icon.addPixmap( QPixmap( ":/Images/PickingOff.png" ), QIcon::Normal, QIcon::Off );

  QIcon flipvert_icon;
  flipvert_icon.addPixmap( QPixmap( ":/Images/FlipVert.png" ), QIcon::Normal, QIcon::On );
  flipvert_icon.addPixmap( QPixmap( ":/Images/FlipVertOff.png" ), QIcon::Normal, QIcon::Off );

  QIcon fliphoriz_icon;
  fliphoriz_icon.addPixmap( QPixmap( ":/Images/FlipHoriz.png" ), QIcon::Normal, QIcon::On );
  fliphoriz_icon.addPixmap( QPixmap( ":/Images/FlipHorizOff.png" ), QIcon::Normal, QIcon::Off );

  QIcon lock_icon;
  lock_icon.addPixmap( QPixmap( ":/Images/Lock.png" ), QIcon::Normal, QIcon::On );
  lock_icon.addPixmap( QPixmap( ":/Images/LockOff.png" ), QIcon::Normal, QIcon::Off );

  QIcon grid_icon;
  grid_icon.addPixmap( QPixmap( ":/Images/Grid.png" ), QIcon::Normal, QIcon::On );
  grid_icon.addPixmap( QPixmap( ":/Images/GridOff.png" ), QIcon::Normal, QIcon::Off );

  QIcon slice_visible_icon;
  slice_visible_icon.addPixmap( QPixmap( ":/Images/Visible.png" ), QIcon::Normal, QIcon::On );
  slice_visible_icon.addPixmap( QPixmap( ":/Images/VisibleOff.png" ), QIcon::Normal, QIcon::Off );

  this->sagittal_viewer_ = new QAction( parent );
  this->sagittal_viewer_->setIcon( sagittal_icon );
  this->sagittal_viewer_->setText( QString( "Sagittal" ) );
  this->sagittal_viewer_->setToolTip( QString( "Sagittal Slice Viewer" ) );
  this->sagittal_viewer_->setIconVisibleInMenu( true );
  this->sagittal_viewer_->setActionGroup( this->viewer_selection_ );
  this->sagittal_viewer_->setObjectName( QString( Viewer::SAGITTAL_C.c_str() ) );

  this->coronal_viewer_ = new QAction( parent );
  this->coronal_viewer_->setIcon( coronal_icon );
  this->coronal_viewer_->setText( QString( "Coronal" ) );
  this->coronal_viewer_->setToolTip( QString( "Coronal Slice Viewer" ) );
  this->coronal_viewer_->setIconVisibleInMenu( true );
  this->coronal_viewer_->setActionGroup( this->viewer_selection_ );
  this->coronal_viewer_->setObjectName( QString( Viewer::CORONAL_C.c_str() ) );

  this->axial_viewer_ = new QAction( parent );
  this->axial_viewer_->setIcon( axial_icon );
  this->axial_viewer_->setText( QString( "Axial" ) );
  this->axial_viewer_->setToolTip( QString( "Axial Slice Viewer" ) );
  this->axial_viewer_->setIconVisibleInMenu( true );
  this->axial_viewer_->setActionGroup( this->viewer_selection_ );
  this->axial_viewer_->setObjectName( QString( Viewer::AXIAL_C.c_str() ) );

  this->volume_viewer_ = new QAction( parent );
  this->volume_viewer_->setIcon( volume_icon );
  this->volume_viewer_->setText( QString( "Volume" ) );
  this->volume_viewer_->setToolTip( QString( "3D Volume Viewer" ) );
  this->volume_viewer_->setIconVisibleInMenu( true );
  this->volume_viewer_->setActionGroup( this->viewer_selection_ );
  this->volume_viewer_->setObjectName( QString( Viewer::VOLUME_C.c_str() ) );

  this->auto_view_ = new QAction( parent );
  this->auto_view_->setIcon( auto_view_icon );
  this->auto_view_->setText( QString( "AutoView" ) );
  this->auto_view_->setToolTip( QString( "Zoom and Translate to see the full dataset" ) );
  this->auto_view_->setIconVisibleInMenu( true );

  this->slice_visible_ = new QAction( parent );
  this->slice_visible_->setCheckable( true );
  this->slice_visible_->setIcon( slice_visible_icon );
  this->slice_visible_->setText( QString( "SliceVisible" ) );
  this->slice_visible_->setToolTip( QString( "Show this slice in the volume viewer" ) );
  this->slice_visible_->setIconVisibleInMenu( true );

  this->grid_ = new QAction( parent );
  this->grid_->setCheckable( true );
  this->grid_->setIcon( grid_icon );
  this->grid_->setText( QString( "Grid" ) );
  this->grid_->setToolTip( QString( "Enable measurement grid" ) );
  this->grid_->setIconVisibleInMenu( true );

  this->lock_ = new QAction( parent );
  this->lock_->setCheckable( true );
  this->lock_->setIcon( lock_icon );
  this->lock_->setText( QString( "Lock" ) );
  this->lock_->setToolTip( QString( "Lock changes to view to other viewers" ) );
  this->lock_->setIconVisibleInMenu( true );

  this->flip_horiz_ = new QAction( parent );
  this->flip_horiz_->setCheckable( true );
  this->flip_horiz_->setIcon( fliphoriz_icon );
  this->flip_horiz_->setText( QString( "Flip Horizontal" ) );
  this->flip_horiz_->setToolTip( QString( "Flip the slice horizontal" ) );
  this->flip_horiz_->setIconVisibleInMenu( true );

  this->flip_vert_ = new QAction( parent );
  this->flip_vert_->setCheckable( true );
  this->flip_vert_->setIcon( flipvert_icon );
  this->flip_vert_->setText( QString( "Flip Vertical" ) );
  this->flip_vert_->setToolTip( QString( "Flip the slice vertical" ) );
  this->flip_vert_->setIconVisibleInMenu( true );

  // BUILD VIEWER WIDGET  

  // --------------------------------------
  // Add frame around widget for selection
  this->layout_ = new QVBoxLayout;
  this->layout_->setContentsMargins( 0, 0, 0, 0 );
  this->layout_->setSpacing( 0 );

  // --------------------------------------
  // Generate the OpenGL part of the widget

  this->viewer_ = QtApplication::Instance()->qt_renderresources_context() ->create_qt_render_widget(
      parent );

  if( this->viewer_ == 0 )
  {
    SCI_THROW_LOGICERROR("OpenGL was not initialized correctly");
  }
  this->viewer_->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  // --------------------------------------
  // Generate button bar at the bottom of
  // the Viewer widget
  SCI_LOG_DEBUG("Create button bar for ViewWidget");

  this->buttonbar_ = new QFrame( parent );
  
  // Add the stylesheet to the Buttons
  this->buttonbar_->setStyleSheet( StyleSheet::VIEWERWIDGET_TOOLBAR_C );
  
  this->buttonbar_layout_ = new QHBoxLayout( buttonbar_ );
  this->buttonbar_layout_->setContentsMargins( 0, 0, 0, 0 );
  this->buttonbar_layout_->setSpacing( 0 );

  this->buttonbar_->setLayout( this->buttonbar_layout_ );

  this->viewer_type_button_ = new QToolButton( this->buttonbar_ );
  this->viewer_menu_ = new QMenu( this->viewer_type_button_ );
  this->viewer_menu_->addAction( this->sagittal_viewer_ );
  this->viewer_menu_->addAction( this->coronal_viewer_ );
  this->viewer_menu_->addAction( this->axial_viewer_ );
  this->viewer_menu_->addAction( this->volume_viewer_ );
  this->viewer_type_button_->setPopupMode( QToolButton::InstantPopup );
  this->viewer_type_button_->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  this->viewer_type_button_->setDefaultAction( this->axial_viewer_ );
  this->viewer_type_button_->setFixedHeight( 20 );
  this->viewer_type_button_->setFixedWidth( 80 );
  this->viewer_type_button_->setMenu( this->viewer_menu_ );
  this->viewer_type_button_->setArrowType( Qt::NoArrow );

  this->lock_button_ = new QToolButton( this->buttonbar_ );
  this->lock_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->lock_button_->setDefaultAction( this->lock_ );
  this->lock_button_->setFixedHeight( 20 );
  this->lock_button_->setFixedWidth( 20 );

  this->auto_view_button_ = new QToolButton( this->buttonbar_ );
  this->auto_view_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->auto_view_button_->setDefaultAction( this->auto_view_ );
  this->auto_view_button_->setFixedHeight( 20 );
  this->auto_view_button_->setFixedWidth( 20 );

  this->flip_horiz_button_ = new QToolButton( this->buttonbar_ );
  this->flip_horiz_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->flip_horiz_button_->setDefaultAction( flip_horiz_ );
  this->flip_horiz_button_->setFixedHeight( 20 );
  this->flip_horiz_button_->setFixedWidth( 20 );

  this->flip_vert_button_ = new QToolButton( this->buttonbar_ );
  this->flip_vert_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->flip_vert_button_->setDefaultAction( this->flip_vert_ );
  this->flip_vert_button_->setFixedHeight( 20 );
  this->flip_vert_button_->setFixedWidth( 20 );

  this->grid_button_ = new QToolButton( this->buttonbar_ );
  this->grid_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->grid_button_->setDefaultAction( this->grid_ );
  this->grid_button_->setFixedHeight( 20 );
  this->grid_button_->setFixedWidth( 20 );
  this->grid_button_->setCheckable( true );

  this->slice_visible_button_ = new QToolButton( this->buttonbar_ );
  this->slice_visible_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->slice_visible_button_->setDefaultAction( this->slice_visible_ );
  this->slice_visible_button_->setFixedHeight( 20 );
  this->slice_visible_button_->setFixedWidth( 20 );

  this->picking_button_ = new SingleShotToolButton( this->buttonbar_ );
  this->picking_button_->setCheckable( true );
  this->picking_button_->setToolButtonStyle( Qt::ToolButtonIconOnly );
  this->picking_button_->setIcon( picking_icon );
  this->picking_button_->setText( QString( "Picking" ) );
  this->picking_button_->setToolTip( QString( "Make the viewer a target for picking" ) );
  this->picking_button_->setFixedHeight( 20 );
  this->picking_button_->setFixedWidth( 20 );

  this->buttonbar_layout_->addWidget( this->viewer_type_button_ );
  this->buttonbar_layout_->addWidget( this->auto_view_button_ );
  this->buttonbar_layout_->addWidget( this->lock_button_ );
  this->buttonbar_layout_->addWidget( this->grid_button_ );
  this->buttonbar_layout_->addWidget( this->slice_visible_button_ );
  this->buttonbar_layout_->addWidget( this->flip_horiz_button_ );
  this->buttonbar_layout_->addWidget( this->flip_vert_button_ );
  this->buttonbar_layout_->addWidget( this->picking_button_ );
  this->buttonbar_layout_->addStretch();

  this->layout_->addWidget( this->viewer_ );
  this->layout_->addWidget( this->buttonbar_ );

  this->deselect_color_ = QColor( 85, 85, 85 );
  this->deselect_color_dark_ = QColor( 35, 35, 35 );
  this->select_color_ = QColor( 225, 125, 0 );
  this->select_color_dark_ = QColor( 180, 90, 0 );
}

ViewerWidget::ViewerWidget( int viewer_id, QWidget *parent ) :
  QFrame( parent ), 
  viewer_id_( viewer_id )
{
  this->private_ = ViewerWidgetPrivateHandle( new ViewerWidgetPrivate( this ) );
  setLayout( private_->layout_ );
  setLineWidth( 3 );
  setFrameShape( QFrame::Panel );
  setFrameShadow( QFrame::Raised );

  this->private_->viewer_->set_viewer_id( viewer_id_ );

  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_  );
  this->private_->grid_->setChecked( viewer->slice_grid_state_->get() );

  QtBridge::Connect( this->private_->viewer_selection_, viewer->view_mode_state_ );

  // NOTE: Connect StateBool to QAction instead of QToolButton, because calling 
  // setChecked on QToolButton won't change the underlying QAction.
  
  QtBridge::Connect( this->private_->picking_button_, viewer->is_picking_target_state_ );
  QtBridge::Connect( this->private_->grid_, viewer->slice_grid_state_ );
  QtBridge::Connect( this->private_->lock_, viewer->viewer_lock_state_ );

  this->connect( this->private_->viewer_selection_, SIGNAL( triggered( QAction* ) ),
    this->private_->viewer_type_button_, SLOT( setDefaultAction( QAction* ) ) );

  this->connect( this->private_->viewer_selection_,
    SIGNAL( triggered( QAction* ) ), SLOT( change_view_type( QAction* ) ) );

  this->connect( this->private_->flip_horiz_, SIGNAL( triggered( bool ) ),
    SLOT( flip_view_horiz( bool ) ) );
  this->connect( this->private_->flip_vert_, SIGNAL( triggered( bool ) ),
    SLOT( flip_view_vert( bool ) ) );
  this->connect( this->private_->auto_view_, SIGNAL( triggered( bool ) ),
    SLOT( auto_view( bool ) ) );

}

ViewerWidget::~ViewerWidget()
{
}

void ViewerWidget::select()
{
  QPalette pal( palette() );
  pal.setColor( QPalette::Light, private_->select_color_ );
  pal.setColor( QPalette::Dark, private_->select_color_dark_ );
  setPalette( pal );
}

void ViewerWidget::deselect()
{
  QPalette pal( palette() );
  pal.setColor( QPalette::Light, private_->deselect_color_ );
  pal.setColor( QPalette::Dark, private_->deselect_color_dark_ );
  setPalette( pal );
}

void ViewerWidget::change_view_type( QAction* viewer_type )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  bool is_volume_view = viewer->is_volume_view();
  this->private_->flip_horiz_button_->setVisible( !is_volume_view );
  this->private_->flip_vert_button_->setVisible( !is_volume_view );
  this->private_->grid_button_->setVisible( !is_volume_view );
  this->private_->picking_button_->setVisible( !is_volume_view );
  this->private_->slice_visible_button_->setVisible( !is_volume_view );

  if( !is_volume_view )
  {
    Core::StateView2DHandle view2d_state = 
      boost::dynamic_pointer_cast<Core::StateView2D>( viewer->get_active_view_state() );
    this->private_->flip_horiz_->setChecked( view2d_state->x_flipped() );
    this->private_->flip_vert_->setChecked( view2d_state->y_flipped() );
  }
}

void ViewerWidget::flip_view_horiz( bool flip )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  if( !viewer->is_volume_view() )
  {
    Core::StateView2DHandle view2d_state = 
      boost::dynamic_pointer_cast<Core::StateView2D>( viewer->get_active_view_state() );
    Core::ActionFlip::Dispatch( view2d_state, Core::FlipDirectionType::HORIZONTAL_E );
  }
}

void ViewerWidget::flip_view_vert( bool flip )
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  if( !viewer->is_volume_view() )
  {
    Core::StateView2DHandle view2d_state = 
      boost::dynamic_pointer_cast<Core::StateView2D>( viewer->get_active_view_state() );
    Core::ActionFlip::Dispatch( view2d_state, Core::FlipDirectionType::VERTICAL_E );
  }
}

void ViewerWidget::auto_view( bool /* checked*/)
{
  ViewerHandle viewer = ViewerManager::Instance()->get_viewer( this->viewer_id_ );
  ActionAutoView::Dispatch( viewer );
}

} // end namespace Seg3D
