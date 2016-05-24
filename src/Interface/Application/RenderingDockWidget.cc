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
 
// STL includes
#include <sstream>
#include <iostream>

// boost includes
#include <boost/foreach.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionToggle.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Utils/QtApplication.h>
#include <QtUtils/Widgets/QtTransferFunctionCurve.h>

// Application includes
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/ViewerManager/Actions/ActionNewFeature.h>
#include <Application/ViewerManager/Actions/ActionDeleteFeature.h>
#include <Application/Layer/LayerManager.h>

// Interface includes
#include <Interface/Application/TransferFunctionFeatureWidget.h>
#include <Interface/Application/RenderingDockWidget.h>
#include <Interface/Application/StyleSheet.h>

// Automatically generated UI file
#include "ui_RenderingDockWidget.h"

namespace Seg3D
{

typedef std::map< std::string, TransferFunctionFeatureWidget* > tf_widget_map_type;

class RenderingDockWidgetPrivate
{
public:
  Ui::RenderingDockWidget ui_;
  tf_widget_map_type tf_widget_map_;
};

RenderingDockWidget::RenderingDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent ),
  private_( new RenderingDockWidgetPrivate )
{
  // Set up the private internals of the LayerManagerInterface class
  this->private_->ui_.setupUi( this );
  this->private_->ui_.dockWidgetContents->setStyleSheet( StyleSheet::RENDERINGWIDGET_C );
  this->private_->ui_.dock_widget_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.scroll_area_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.feature_control_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.tf_preview_layout_->addWidget( QtUtils::QtApplication::Instance()->
    qt_renderresources_context()->create_qt_transfer_function_widget( 
    this, ViewerManager::Instance()->get_transfer_function() ) );

  qpointer_type qpointer( this );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Fog widgets
  QtUtils::QtBridge::Show( this->private_->ui_.fog_content_,
    ViewerManager::Instance()->show_fog_control_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.fog_density_, 
    ViewerManager::Instance()->fog_density_state_ );

  // Clipping widgets
  if ( Core::Application::Instance()->is_osx_10_5_or_less() )
  {
    // NOTE: No clipping on this platform as drivers are inconsistent
    this->private_->ui_.clipping_widget_->hide();
  }
  
  QtUtils::QtBridge::Show( this->private_->ui_.clipping_content_,
    ViewerManager::Instance()->show_clipping_control_state_ );

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

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    for ( int i = 0; i < 6; ++i )
    {
      this->add_connection( ViewerManager::Instance()->enable_clip_plane_state_[ i ]->
        value_changed_signal_.connect( boost::bind( 
        &RenderingDockWidget::HandleClippingPlanesStateChanged, qpointer, _1, i ) ) );
      update_tab_appearance( ViewerManager::Instance()->enable_clip_plane_state_[ i ]->get(), i );  
    }
  }
  // Volume rendering widgets
  Core::TransferFunctionHandle tf = ViewerManager::Instance()->get_transfer_function();
  QtUtils::QtBridge::Show( this->private_->ui_.vr_content_,
    ViewerManager::Instance()->show_volume_rendering_control_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.vr_target_, 
    ViewerManager::Instance()->volume_rendering_target_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.vr_sample_rate_,
    ViewerManager::Instance()->volume_sample_rate_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.renderer_combobox_,
    ViewerManager::Instance()->volume_renderer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.occlusion_angle_,
    ViewerManager::Instance()->vr_occlusion_angle_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.grid_resolution_,
    ViewerManager::Instance()->vr_occlusion_grid_resolution_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.occlusion_widget_,
    ViewerManager::Instance()->volume_renderer_state_,
    boost::lambda::bind( &Core::StateLabeledOption::index, 
    ViewerManager::Instance()->volume_renderer_state_.get() ) == 2 );
  QtUtils::QtBridge::Connect( this->private_->ui_.tf_view_->get_scene(), tf );
  QtUtils::QtBridge::Connect( this->private_->ui_.add_feature_button_, boost::bind(
    ActionNewFeature::Dispatch, Core::Interface::GetWidgetActionContext() ) );
  this->connect( this->private_->ui_.delete_feature_button_, SIGNAL( clicked() ),
    SLOT( delete_active_curve() ) );
  this->connect( this->private_->ui_.histogram_scale_combobox_, 
    SIGNAL( currentIndexChanged( int ) ), SLOT( set_histogram_mode( int ) ) );
  this->set_histogram_mode( 1 );

  this->add_connection( tf->feature_added_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleFeatureAdded, qpointer, _1 ) ) );
  this->add_connection( tf->feature_deleted_signal_.connect( boost::bind(
    &RenderingDockWidget::HandleFeatureDeleted, qpointer, _1 ) ) );
  this->add_connection( ViewerManager::Instance()->volume_rendering_target_state_->
    value_changed_signal_.connect( boost::bind( 
    &RenderingDockWidget::HandleVolumeRenderingTargetChanged, qpointer, _2 ) ) );
  this->add_connection( Core::Application::Instance()->reset_signal_.connect( boost::bind(
    &RenderingDockWidget::HandleReset, qpointer ) ) );
    
    
  QtUtils::QtBridge::Connect( this->private_->ui_.clipping_open_button_, 
    boost::bind( &Core::ActionToggle::Dispatch, Core::Interface::GetWidgetActionContext(), 
    ViewerManager::Instance()->show_clipping_control_state_ ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.clipping_spacer_, 
    boost::bind( &Core::ActionToggle::Dispatch, Core::Interface::GetWidgetActionContext(), 
    ViewerManager::Instance()->show_clipping_control_state_ ) );
    
  QtUtils::QtBridge::Connect( this->private_->ui_.fog_open_button_, 
    boost::bind( &Core::ActionToggle::Dispatch, Core::Interface::GetWidgetActionContext(), 
    ViewerManager::Instance()->show_fog_control_state_ ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.fog_spacer_, 
    boost::bind( &Core::ActionToggle::Dispatch, Core::Interface::GetWidgetActionContext(), 
    ViewerManager::Instance()->show_fog_control_state_ ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.vr_open_button_, 
    boost::bind( &Core::ActionToggle::Dispatch, Core::Interface::GetWidgetActionContext(), 
    ViewerManager::Instance()->show_volume_rendering_control_state_ ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.vr_spacer_, 
    boost::bind( &Core::ActionToggle::Dispatch, Core::Interface::GetWidgetActionContext(), 
    ViewerManager::Instance()->show_volume_rendering_control_state_ ) );

    
  this->add_connection( ViewerManager::Instance()->show_clipping_control_state_->
    state_changed_signal_.connect( boost::bind( 
      &RenderingDockWidget::HandleUpdateToolAppearance,qpointer ) ) ); 

  this->add_connection( ViewerManager::Instance()->show_volume_rendering_control_state_->
    state_changed_signal_.connect( boost::bind( 
      &RenderingDockWidget::HandleUpdateToolAppearance,qpointer ) ) ); 

  this->add_connection( ViewerManager::Instance()->show_fog_control_state_->
    state_changed_signal_.connect( boost::bind( 
      &RenderingDockWidget::HandleUpdateToolAppearance,qpointer ) ) ); 
  
  this->private_->ui_.cp1_d_->set_description( "Distance" );
  this->private_->ui_.cp2_d_->set_description( "Distance" );
  this->private_->ui_.cp3_d_->set_description( "Distance" );
  this->private_->ui_.cp4_d_->set_description( "Distance" );
  this->private_->ui_.cp5_d_->set_description( "Distance" );
  this->private_->ui_.cp6_d_->set_description( "Distance" );
  
  this->private_->ui_.cp1_x_->set_description( "X" );
  this->private_->ui_.cp2_x_->set_description( "X" );
  this->private_->ui_.cp3_x_->set_description( "X" );
  this->private_->ui_.cp4_x_->set_description( "X" );
  this->private_->ui_.cp5_x_->set_description( "X" );
  this->private_->ui_.cp6_x_->set_description( "X" );
  
  this->private_->ui_.cp1_y_->set_description( "Y" );
  this->private_->ui_.cp2_y_->set_description( "Y" );
  this->private_->ui_.cp3_y_->set_description( "Y" );
  this->private_->ui_.cp4_y_->set_description( "Y" );
  this->private_->ui_.cp5_y_->set_description( "Y" );
  this->private_->ui_.cp6_y_->set_description( "Y" );
  
  this->private_->ui_.cp1_z_->set_description( "Z" );
  this->private_->ui_.cp2_z_->set_description( "Z" );
  this->private_->ui_.cp3_z_->set_description( "Z" );
  this->private_->ui_.cp4_z_->set_description( "Z" );
  this->private_->ui_.cp5_z_->set_description( "Z" );
  this->private_->ui_.cp6_z_->set_description( "Z" );
  
  this->private_->ui_.fog_density_->set_description( "Density" );
  
  this->private_->ui_.vr_sample_rate_->set_description( "Sampling Rate" );
  this->private_->ui_.occlusion_angle_->set_description( "Occlusion Angle" );
  this->private_->ui_.grid_resolution_->set_description( "Sampling Resolution" );
  
      
  update_tool_appearance();
}

RenderingDockWidget::~RenderingDockWidget()
{
  this->disconnect_all();
}

void RenderingDockWidget::update_tab_appearance( bool enabled, int index )
{
  if( enabled )
  {
    this->private_->ui_.clipping_tabwidget_->
      setTabText( index, "+" );
  }
  else
  {
    this->private_->ui_.clipping_tabwidget_->
      setTabText( index, "-" );
  }
}

void RenderingDockWidget::update_tool_appearance()
{
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  if ( ViewerManager::Instance()->show_clipping_control_state_->get() )
  {
    this->private_->ui_.clipping_widget_->setStyleSheet( 
      StyleSheet::RENDERING_CLIPPING_ACTIVE_C );
  }
  else 
  {
    this->private_->ui_.clipping_widget_->setStyleSheet( 
      StyleSheet::RENDERING_CLIPPING_INACTIVE_C );    
  }

  if ( ViewerManager::Instance()->show_volume_rendering_control_state_->get() )
  {
    this->private_->ui_.vr_widget_->setStyleSheet( 
      StyleSheet::RENDERING_VR_ACTIVE_C );
  }
  else 
  {
    this->private_->ui_.vr_widget_->setStyleSheet( 
      StyleSheet::RENDERING_VR_INACTIVE_C );    
  }

  if ( ViewerManager::Instance()->show_fog_control_state_->get() )
  {
    this->private_->ui_.fog_widget_->setStyleSheet( 
      StyleSheet::RENDERING_FOG_ACTIVE_C );
  }
  else 
  {
    this->private_->ui_.fog_widget_->setStyleSheet( 
      StyleSheet::RENDERING_FOG_INACTIVE_C );   
  }

}

void RenderingDockWidget::handle_feature_added( Core::TransferFunctionFeatureHandle feature )
{
  TransferFunctionFeatureWidget* tf_feature_widget = 
    new TransferFunctionFeatureWidget( this, feature );
  QtUtils::QtTransferFunctionCurve* tf_curve = this->private_->ui_.tf_view_->get_scene()->
    get_curve( feature->get_feature_id() );
  this->private_->ui_.dummy_widget_->hide();
  this->private_->ui_.delete_feature_button_->setEnabled( true );
  tf_feature_widget->setVisible( tf_curve->is_active() );
  this->private_->ui_.feature_control_layout_->addWidget( tf_feature_widget );
  this->private_->tf_widget_map_[ feature->get_feature_id() ] = tf_feature_widget;
  QObject::connect( tf_curve, SIGNAL( activated( bool ) ), tf_feature_widget, SLOT( setVisible( bool ) ) );
}

void RenderingDockWidget::handle_feature_deleted( Core::TransferFunctionFeatureHandle feature )
{
  tf_widget_map_type::iterator it = this->private_->tf_widget_map_.find( feature->get_feature_id() );
  assert( it != this->private_->tf_widget_map_.end() );
  TransferFunctionFeatureWidget* tf_feature_widget = ( *it ).second;
  this->private_->ui_.feature_control_layout_->removeWidget( tf_feature_widget );
  this->private_->tf_widget_map_.erase( it );
  tf_feature_widget->deleteLater();
  if ( this->private_->tf_widget_map_.empty() )
  {
    this->private_->ui_.dummy_widget_->show();
    this->private_->ui_.delete_feature_button_->setEnabled( false );
}
}

void RenderingDockWidget::handle_volume_rendering_target_changed( std::string target_id )
{
  if ( target_id != "<none>" && target_id != Core::StateLabeledOption::EMPTY_OPTION_C )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    DataLayerHandle target_layer = LayerManager::Instance()->find_data_layer_by_id( target_id );
    if ( target_layer && target_layer->has_valid_data() )
    {
      this->private_->ui_.histogram_->set_histogram( target_layer->get_data_volume()->
        get_data_block()->get_histogram() );
      return;
    }
  }

  this->private_->ui_.histogram_->reset_histogram();
}

void RenderingDockWidget::delete_active_curve()
{
  QtUtils::QtTransferFunctionCurve* curve = this->private_->ui_.tf_view_->get_scene()->get_active_curve();
  if ( curve != 0 )
  {
    ActionDeleteFeature::Dispatch( Core::Interface::GetWidgetActionContext(), curve->get_feature_id() );
  }
}

void RenderingDockWidget::set_histogram_mode( int mode )
{
  this->private_->ui_.histogram_->set_logarithmic( mode == 1 );
}

void RenderingDockWidget::handle_reset()
{
  BOOST_FOREACH( tf_widget_map_type::value_type entry, this->private_->tf_widget_map_ )
  {
    TransferFunctionFeatureWidget* tf_feature_widget = entry.second;
    this->private_->ui_.feature_control_layout_->removeWidget( tf_feature_widget );
    delete tf_feature_widget;
  }
  this->private_->tf_widget_map_.clear();
  this->private_->ui_.dummy_widget_->show();
  this->private_->ui_.delete_feature_button_->setEnabled( false );
  this->private_->ui_.histogram_->reset_histogram();
}

void RenderingDockWidget::HandleClippingPlanesStateChanged( 
  qpointer_type qpointer, bool state, int index )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &RenderingDockWidget::update_tab_appearance, qpointer.data(), state, index ) ) );
}

void RenderingDockWidget::HandleFeatureAdded( qpointer_type qpointer, 
                       Core::TransferFunctionFeatureHandle feature )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &RenderingDockWidget::handle_feature_added, qpointer.data(), feature ) ) );
}

void RenderingDockWidget::HandleFeatureDeleted( qpointer_type qpointer, 
                         Core::TransferFunctionFeatureHandle feature )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &RenderingDockWidget::handle_feature_deleted, qpointer.data(), feature ) ) );
}

void RenderingDockWidget::HandleVolumeRenderingTargetChanged( qpointer_type qpointer, 
                               std::string target_id )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &RenderingDockWidget::handle_volume_rendering_target_changed, qpointer.data(), target_id ) ) );
}

void RenderingDockWidget::HandleReset( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &RenderingDockWidget::handle_reset, qpointer.data() ) ) );
}

void RenderingDockWidget::HandleUpdateToolAppearance( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &RenderingDockWidget::update_tool_appearance, qpointer.data() ) ) );
}

} // end namespace Seg3D
