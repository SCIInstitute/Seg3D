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

// Core includes
#include <Core/Utils/Log.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>

// Application includes
#include <Application/ViewerManager/ViewerManager.h>

// Interface includes
#include <Interface/Application/RenderingDockWidget.h>

// Automatically generated UI file
#include "ui_RenderingDockWidget.h"

namespace Seg3D
{

class RenderingDockWidgetPrivate
{
public:
  Ui::RenderingDockWidget ui_;
  QButtonGroup* enable_clipping_planes_checkboxes_;
};

RenderingDockWidget::RenderingDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent ),
  private_( new RenderingDockWidgetPrivate )
{
  // Set up the private internals of the LayerManagerInterface class
  this->private_->ui_.setupUi( this );
  this->private_->ui_.dock_widget_layout_->setAlignment( Qt::AlignTop );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.fog_open_button_,
    ViewerManager::Instance()->show_fog_control_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.fog_content_,
    ViewerManager::Instance()->show_fog_control_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.clipping_open_button_,
    ViewerManager::Instance()->show_clipping_control_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.clipping_content_,
    ViewerManager::Instance()->show_clipping_control_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.fog_density_, 
    ViewerManager::Instance()->fog_density_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.vr_open_button_,
    ViewerManager::Instance()->show_volume_rendering_control_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.vr_content_,
    ViewerManager::Instance()->show_volume_rendering_control_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.vr_sample_rate_,
    ViewerManager::Instance()->volume_sample_rate_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_cp1_, 
    ViewerManager::Instance()->enable_clip_plane_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp1_x_,
    ViewerManager::Instance()->clip_plane_x_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp1_y_,
    ViewerManager::Instance()->clip_plane_y_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp1_z_,
    ViewerManager::Instance()->clip_plane_z_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp1_d_,
    ViewerManager::Instance()->clip_plane_distance_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp1_reverse_norm_,
    ViewerManager::Instance()->clip_plane_reverse_norm_state_[ 0 ] );
  QtUtils::QtBridge::Enable( this->private_->ui_.cp1_params_widget_,
    ViewerManager::Instance()->enable_clip_plane_state_[ 0 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_cp2_, 
    ViewerManager::Instance()->enable_clip_plane_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp2_x_,
    ViewerManager::Instance()->clip_plane_x_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp2_y_,
    ViewerManager::Instance()->clip_plane_y_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp2_z_,
    ViewerManager::Instance()->clip_plane_z_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp2_d_,
    ViewerManager::Instance()->clip_plane_distance_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp2_reverse_norm_,
    ViewerManager::Instance()->clip_plane_reverse_norm_state_[ 1 ] );
  QtUtils::QtBridge::Enable( this->private_->ui_.cp2_params_widget_,
    ViewerManager::Instance()->enable_clip_plane_state_[ 1 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_cp3_, 
    ViewerManager::Instance()->enable_clip_plane_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp3_x_,
    ViewerManager::Instance()->clip_plane_x_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp3_y_,
    ViewerManager::Instance()->clip_plane_y_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp3_z_,
    ViewerManager::Instance()->clip_plane_z_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp3_d_,
    ViewerManager::Instance()->clip_plane_distance_state_[ 2 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp3_reverse_norm_,
    ViewerManager::Instance()->clip_plane_reverse_norm_state_[ 2 ] );
  QtUtils::QtBridge::Enable( this->private_->ui_.cp3_params_widget_,
    ViewerManager::Instance()->enable_clip_plane_state_[ 2 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_cp4_, 
    ViewerManager::Instance()->enable_clip_plane_state_[ 3 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp4_x_,
    ViewerManager::Instance()->clip_plane_x_state_[ 3 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp4_y_,
    ViewerManager::Instance()->clip_plane_y_state_[ 3 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp4_z_,
    ViewerManager::Instance()->clip_plane_z_state_[ 3 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp4_d_,
    ViewerManager::Instance()->clip_plane_distance_state_[ 3 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp4_reverse_norm_,
    ViewerManager::Instance()->clip_plane_reverse_norm_state_[ 3 ] );
  QtUtils::QtBridge::Enable( this->private_->ui_.cp4_params_widget_,
    ViewerManager::Instance()->enable_clip_plane_state_[ 3 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_cp5_, 
    ViewerManager::Instance()->enable_clip_plane_state_[ 4 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp5_x_,
    ViewerManager::Instance()->clip_plane_x_state_[ 4 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp5_y_,
    ViewerManager::Instance()->clip_plane_y_state_[ 4 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp5_z_,
    ViewerManager::Instance()->clip_plane_z_state_[ 4 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp5_d_,
    ViewerManager::Instance()->clip_plane_distance_state_[ 4 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp5_reverse_norm_,
    ViewerManager::Instance()->clip_plane_reverse_norm_state_[ 4 ] );
  QtUtils::QtBridge::Enable( this->private_->ui_.cp5_params_widget_,
    ViewerManager::Instance()->enable_clip_plane_state_[ 4 ] );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_cp6_, 
    ViewerManager::Instance()->enable_clip_plane_state_[ 5 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp6_x_,
    ViewerManager::Instance()->clip_plane_x_state_[ 5 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp6_y_,
    ViewerManager::Instance()->clip_plane_y_state_[ 5 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp6_z_,
    ViewerManager::Instance()->clip_plane_z_state_[ 5 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp6_d_,
    ViewerManager::Instance()->clip_plane_distance_state_[ 5 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.cp6_reverse_norm_,
    ViewerManager::Instance()->clip_plane_reverse_norm_state_[ 5 ] );
  QtUtils::QtBridge::Enable( this->private_->ui_.cp6_params_widget_,
    ViewerManager::Instance()->enable_clip_plane_state_[ 5 ] );
  
  add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ 0 ]->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer_type( this ), _1, 0 ) ) );
  add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ 1 ]->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer_type( this ), _1, 1 ) ) );
  add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ 2 ]->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer_type( this ), _1, 2 ) ) );
  add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ 3 ]->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer_type( this ), _1, 3 ) ) );
  add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ 4 ]->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer_type( this ), _1, 4 ) ) );
  add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ 5 ]->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer_type( this ), _1, 5 ) ) ); 
    
}

RenderingDockWidget::~RenderingDockWidget()
{

}

void RenderingDockWidget::set_enabled_tab_appearance( bool enabled, int index )
{
  if( enabled )
  {
    this->private_->ui_.clipping_tabwidget_->
      setTabText( index, ( "=" + QString::number( index + 1 ) + "=" ) );
  }
  else
  {
    this->private_->ui_.clipping_tabwidget_->
      setTabText( index, QString::number( index + 1 ) );
  }
}

void RenderingDockWidget::HandleClippingPlanesStateChanged( qpointer_type qpointer, bool state, int index )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &RenderingDockWidget::set_enabled_tab_appearance, qpointer.data(), state, index ) ) );
}



} // end namespace Seg3D
