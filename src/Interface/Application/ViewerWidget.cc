/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionAdd.h>
#include <Core/State/Actions/ActionSet.h>

// Application includes
#include <Application/Viewer/Actions/ActionAutoView.h>
#include <Application/Viewer/Actions/ActionSnap.h>
#include <Application/ViewerManager/ViewerManager.h>

// QtUtils includes
#include <QtUtils/Utils/QtApplication.h>
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtRenderResources.h>
#include <QtUtils/Utils/QtRenderWidget.h>

// Interface includes
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/ViewerWidget.h>

// Automatically generated UI file
#include "ui_ViewerWidget.h"

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ViewerWidgetPrivate
//////////////////////////////////////////////////////////////////////////

class ViewerWidgetPrivate
{
public:
  void handle_widget_activated();

  Ui::ViewerWidget ui_;
  QtUtils::QtRenderWidget* render_widget_;

  int minimum_toolbar_width_;
  bool initialized_size_;
  QLabel *facade_widget_;

  // Handle to the underlying Viewer structure
  ViewerHandle viewer_;

  QVector< QToolButton* > buttons_;

public:
  static void HandleViewModeChanged( ViewerWidgetQWeakHandle viewer_widget );
};

void ViewerWidgetPrivate::handle_widget_activated()
{
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    ViewerManager::Instance()->active_viewer_state_, this->viewer_->get_viewer_id() );
}

void ViewerWidgetPrivate::HandleViewModeChanged( ViewerWidgetQWeakHandle viewer_widget )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &ViewerWidgetPrivate::HandleViewModeChanged,
      viewer_widget ) );
    return;
  }

  if ( viewer_widget.isNull() || QCoreApplication::closingDown() )
  {
    return;
  }

  QCoreApplication::postEvent( viewer_widget.data(), new QResizeEvent(
    viewer_widget->size(), viewer_widget->size() ) );
}

//////////////////////////////////////////////////////////////////////////
// Class ViewerWidget
//////////////////////////////////////////////////////////////////////////

ViewerWidget::ViewerWidget( ViewerHandle viewer, QWidget *parent ) :
  QWidget( parent ),
  private_( new ViewerWidgetPrivate )
{
  this->private_->viewer_ = viewer;
  this->private_->ui_.setupUi( this );
  this->setStyleSheet( StyleSheet::VIEWERWIDGET_C );

  // NOTE: IF YOU ADD ANOTHER BUTTON TO THE VIEWERWIDGET, PLEASE ADD IT TO THE buttons_ VECTOR.
  // We make a vector of all the buttons this way we can calculate the minimum size that the
  // viewer bar can be
  this->private_->buttons_.push_back( this->private_->ui_.auto_view_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.lock_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.slice_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.snap_to_axis_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.light_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.fog_button_ );
  if ( !( Core::Application::Instance()->is_osx_10_5_or_less() ) )
  {
    // NOTE: No clipping on this platform as driver are inconsistent
  this->private_->buttons_.push_back( this->private_->ui_.enable_clipping_button_ );
  }
  this->private_->buttons_.push_back( this->private_->ui_.grid_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.flip_horizontal_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.flip_vertical_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.isosurfaces_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.slices_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.show_invisible_slices_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.show_bounding_box_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.volume_rendering_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.overlay_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.picking_lines_visible_button_ );
  this->private_->buttons_.push_back( this->private_->ui_.picking_button_ );

//  // Setup the widget so for a small size it can be broken into two
//
//  this->private_->minimum_toolbar_width_ = 300;
  this->private_->initialized_size_ = false;
  this->private_->ui_.buttonbar_->setMinimumSize( QSize( 170, 0 ) );
  this->private_->ui_.button_layout_->setStretchFactor( this->private_->ui_.common_tools_, 0 );
  this->private_->ui_.button_layout_->setStretchFactor( this->private_->ui_.less_common_tools_, 1 );
  this->private_->ui_.button_layout_->setAlignment( Qt::AlignLeft );
  this->private_->ui_.toolbar_layout_->setAlignment( Qt::AlignCenter );
  this->private_->ui_.common_tools_layout_->setAlignment( Qt::AlignLeft );
  this->private_->ui_.less_common_tools_layout_->setAlignment( Qt::AlignLeft );
  this->private_->ui_.horizontalLayout->setAlignment( Qt::AlignLeft );

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

    QtUtils::QtBridge::Connect( this->private_->ui_.flip_horizontal_button_,
      this->private_->viewer_->flip_horizontal_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.flip_vertical_button_,
      this->private_->viewer_->flip_vertical_state_ );

    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_mode_,
      this->private_->viewer_->view_mode_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.picking_button_ ,
      this->private_->viewer_->is_picking_target_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.grid_button_,
      this->private_->viewer_->slice_grid_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.lock_button_,
      this->private_->viewer_->lock_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.slice_visible_button_,
      this->private_->viewer_->slice_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.slices_visible_button_,
      this->private_->viewer_->volume_slices_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.show_invisible_slices_button_,
      this->private_->viewer_->volume_show_invisible_slices_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.show_bounding_box_button_,
      this->private_->viewer_->volume_show_bounding_box_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.picking_lines_visible_button_,
      this->private_->viewer_->slice_picking_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.overlay_visible_button_,
      this->private_->viewer_->overlay_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.light_visible_button_,
      this->private_->viewer_->volume_light_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.fog_button_,
      this->private_->viewer_->volume_enable_fog_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.enable_clipping_button_,
      this->private_->viewer_->volume_enable_clipping_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.isosurfaces_visible_button_,
      this->private_->viewer_->volume_isosurfaces_visible_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.volume_rendering_visible_button_,
      this->private_->viewer_->volume_volume_rendering_visible_state_ );

    QtUtils::QtBridge::Connect( this->private_->ui_.snap_to_axis_button_, boost::bind(
      &ActionSnap::Dispatch, Core::Interface::GetWidgetActionContext(),
      this->private_->viewer_->get_viewer_id() ) );
    QtUtils::QtBridge::Connect( this->private_->ui_.auto_view_button_, boost::bind(
      &ActionAutoView::Dispatch, Core::Interface::GetWidgetActionContext(),
      this->private_->viewer_->get_viewer_id() ) );

    this->private_->render_widget_->activate_signal_.connect(
      boost::bind( &ViewerWidgetPrivate::handle_widget_activated, this->private_  ) );

    // Show the following buttons when it's not volume view
    boost::function< bool () > show_buttons_condition = boost::lambda::bind(
      &Core::StateLabeledOption::get, viewer->view_mode_state_.get() ) !=
      boost::lambda::constant( Viewer::VOLUME_C );
    QtUtils::QtBridge::Show( this->private_->ui_.flip_horizontal_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.flip_vertical_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.grid_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.slice_visible_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.picking_lines_visible_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.picking_button_,
      viewer->view_mode_state_, show_buttons_condition );

    // Show the following buttons when it's volume view
    show_buttons_condition = boost::lambda::bind(
      &Core::StateLabeledOption::get, viewer->view_mode_state_.get() ) ==
      boost::lambda::constant( Viewer::VOLUME_C );
    QtUtils::QtBridge::Show( this->private_->ui_.slices_visible_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.light_visible_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.fog_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.enable_clipping_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.isosurfaces_visible_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.snap_to_axis_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.show_invisible_slices_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.show_bounding_box_button_,
      viewer->view_mode_state_, show_buttons_condition );
    QtUtils::QtBridge::Show( this->private_->ui_.volume_rendering_visible_button_,
      viewer->view_mode_state_, show_buttons_condition );

    // When view mode changes, we need to rearrange the toolbar and update the flip buttons
    // NOTE: This must happen after the toolbar buttons have been shown/hidden properly,
    // and thus the connection should come after the previous QtBridge calls.
    ViewerWidgetQWeakHandle viewer_widget( this );
    this->add_connection( this->private_->viewer_->view_mode_state_->state_changed_signal_.
      connect( boost::bind( &ViewerWidgetPrivate::HandleViewModeChanged, viewer_widget ) ) );
  }

  this->private_->facade_widget_ = new QLabel( this->private_->ui_.border_ );
  this->private_->ui_.border_layout_->insertWidget( 0, this->private_->facade_widget_ );
  this->private_->facade_widget_->hide();

  if ( Core::Application::Instance()->is_osx_10_5_or_less() )
  {
    // NOTE: No clipping on this platform as driver are inconsistent
    this->private_->ui_.enable_clipping_button_->hide();
}
}

ViewerWidget::~ViewerWidget()
{
  this->disconnect_all();
}

void ViewerWidget::image_mode( bool picture )
{
  if( picture )
  {
    this->private_->facade_widget_->setMinimumSize( this->private_->render_widget_->size() );
    this->private_->facade_widget_->setPixmap( QPixmap::fromImage( this->private_->render_widget_->grabFrameBuffer() ) );
    this->private_->render_widget_->hide();
    this->private_->facade_widget_->show();
  }
  else
  {
    this->private_->facade_widget_->hide();
    this->private_->render_widget_->show();
  }
}

int ViewerWidget::get_minimum_size()
{
  // We start with padding the minimum width by 1 because of the 1px margin on the left-hand side
  int minimum_width = 0;
  if( !this->private_->ui_.line_->isHidden() ) minimum_width += 3;
//  if( !this->private_->ui_.sep_line_->isHidden() ) minimum_width += 3;


  // Next we get the width of the viewer mode holder and we pad it by 2 for the left and right
  // margins
  int viewer_mode_width = this->private_->ui_.viewer_mode_->width();

  // Now we combine them.
  minimum_width = minimum_width + viewer_mode_width;

  // Now we add the sizes of the visible buttons plus a 1px padding for the right hand margin
  for( int i = 0; i < this->private_->buttons_.size(); ++i )
  {
    if( !this->private_->buttons_[ i ]->isHidden() )
    {
      minimum_width = minimum_width + this->private_->buttons_[ i ]->minimumWidth();// + 1;
    }
  }

  // Now we return the final value
  return minimum_width;
}

void ViewerWidget::resizeEvent( QResizeEvent * event )
{
  // In the case that the widget hasn't been initialized we just use the arbitrary minimum size of
  // 300px
  if( !this->private_->initialized_size_ )
  {
    this->private_->minimum_toolbar_width_ = 300;
  }

  else
  {
    this->private_->minimum_toolbar_width_ = this->get_minimum_size();
  }

  int new_width = ( event->size().width() - 6 );

  if ( new_width <= this->private_->minimum_toolbar_width_ )
  {
//    this->private_->ui_.sep_line_->hide();
    this->private_->ui_.button_layout_->removeWidget( this->private_->ui_.less_common_tools_ );
    this->private_->ui_.toolbar_->setStyleSheet( StyleSheet::VIEWERWIDGET_DOUBLELINE_C );
    this->private_->ui_.toolbar_layout_->addWidget( this->private_->ui_.less_common_tools_, 0 );

    this->update();
  }
  else if ( new_width > this->private_->minimum_toolbar_width_ )
  {
//    this->private_->ui_.sep_line_->show();
    this->private_->ui_.toolbar_layout_->removeWidget( this->private_->ui_.less_common_tools_ );
    this->private_->ui_.toolbar_->setStyleSheet( StyleSheet::VIEWERWIDGET_SINGLELINE_C );
    this->private_->ui_.button_layout_->addWidget( this->private_->ui_.less_common_tools_, 1 );

    this->update();
  }

  this->private_->initialized_size_ = true;

  QWidget::resizeEvent( event );
}

void ViewerWidget::select()
{
  this->private_->ui_.border_->setStyleSheet( StyleSheet::VIEWERSELECTED_C );
}

void ViewerWidget::deselect()
{
  this->private_->ui_.border_->setStyleSheet( StyleSheet::VIEWERNOTSELECTED_C );
}

} // end namespace Seg3D
