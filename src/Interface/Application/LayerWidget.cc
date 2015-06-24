/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

//Boost Includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Qt includes
#include <QPointer>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <QBitmap>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

//Core Includes - for logging
#include <Core/Utils/Log.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/Isosurface/Isosurface.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtColorBarWidget.h>
#include <QtUtils/Widgets/QtSliderDoubleCombo.h>

//Application Includes
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionActivateLayer.h>
#include <Application/Layer/Actions/ActionComputeIsosurface.h>
#include <Application/Layer/Actions/ActionDeleteIsosurface.h>
#include <Application/Layer/Actions/ActionDeleteLayers.h>
#include <Application/Layer/Actions/ActionMoveLayer.h>
#include <Application/Layer/Actions/ActionCalculateMaskVolume.h>
#include <Application/Layer/Actions/ActionDuplicateLayer.h>
#include <Application/LayerIO/Actions/ActionExportLayer.h>
#include <Application/LayerIO/Actions/ActionExportSegmentation.h>
#include <Application/LayerIO/Actions/ActionExportIsosurface.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Filters/LayerResampler.h>
#include <Application/ProjectManager/ProjectManager.h>

//Interface Includes
#include <Interface/Application/LayerWidget.h>
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/PushDragButton.h>
#include <Interface/Application/DropSpaceWidget.h>
#include <Interface/Application/OverlayWidget.h>
#include <Interface/Application/LayerResamplerDialog.h>

//UI Includes
#include "ui_LayerWidget.h"

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerWidgetPrivate
//////////////////////////////////////////////////////////////////////////
  
class LayerWidgetPrivate : public QObject, public Core::ConnectionHandler
{
  // -- Constructor / Destructor --
public:
  LayerWidgetPrivate( LayerWidget* parent ) :
    QObject( parent ),
    parent_( parent )
  {}

  virtual ~LayerWidgetPrivate();

  // -- Internal helper functions --
public:
  // UPDATE_APPEARANCE:
  // Update the appearance of the widget to reflect its state
  void update_appearance( bool locked, bool active, bool in_use, bool initialize = false);

  // UPDATE_WIDGET_STATE:
  // Update the button state, the open menus and color of the widget
  void update_widget_state( bool initialize = false );

  // UPDATE_PROGRESS_BAR:
  // Update the progress in the widget
  void update_progress_bar( double progress );

  // ENABLE_DROP_SPACE:
  // this function is called when a drag enter or leave event is triggered and it calls hide
  // or show on the dropspace to make it look like there is a space for the user to drop the
  // dragged layer
  void enable_drop_space( bool drop );

  // GET_VOLUME_TYPE:
  // this function returns the type volume that the layerwidget represents
  int get_volume_type();

  // EXPORT_LAYER:
  // function that checks the type of the layer and calls the appropriate function to export the layer  
  void export_layer( const std::string& type_extension );

  // EXPORT_ISOSURFACE
  // function that exports an isosurface
  void export_isosurface();

  // SET_DROP_TARGET:
  // this function is for keeping track of which layer the drop is going to happen on
  void set_drop_target( LayerWidget* target_layer );

  // SET_PICKED_UP:
  // this function is called to set or unset the state of picked_up_
  void set_picked_up( bool up );

public:
  LayerWidget* parent_;
  // Layer
  LayerHandle layer_;
  
  // Icons
  QIcon large_data_layer_icon_;
  QIcon data_layer_icon_;
  QIcon label_layer_icon_;
  QIcon mask_layer_icon_; 
  
  // UI Designed by the QtDesginer
  Ui::LayerWidget ui_;
  
  // Custom widgets that cannot be added by the Qt Designer
  QtUtils::QtColorBarWidget* color_widget_;

  // Drag and drop stuff
  PushDragButton* activate_button_;
  DropSpaceWidget* drop_space_;
  OverlayWidget* overlay_;
  
  LayerWidget* drop_layer_;
  bool drop_layer_set_;

  GroupButtonMenu* drop_group_;
  bool drop_group_set_;

  // Local copy of state information 
  bool active_;
  bool locked_;
  bool in_use_;
  bool picked_up_;
  int picked_up_layer_size_;

  // -- Static functions --
public:
  typedef QPointer< LayerWidgetPrivate > qpointer_type;

  // UPDATESTATE:
  // Entry point for the state engine to notify state has changed
  static void UpdateState( qpointer_type qpointer );

  // UPDATEACTIVESTATE:
  // Entry point for the state engine to notify active layer has changed
  static void UpdateActiveState( qpointer_type qpointer, LayerHandle layer );

  // UPDATEVIEWERBUTTONS:
  // update the layout of the viewer buttons
  static void UpdateViewerButtons( qpointer_type qpointer, std::string layout );

  // UPDATEPROGRESS:
  // Update the progress bar 
  static void UpdateProgress( qpointer_type qpointer, double progress );
};

LayerWidgetPrivate::~LayerWidgetPrivate()
{
  this->disconnect_all();
}

void LayerWidgetPrivate::update_appearance( bool locked, bool active, bool in_use, bool initialize )
{
  if ( locked == this->locked_ && 
    active == this->active_ &&
    in_use == this->in_use_ && initialize == false ) return;

  this->locked_ = locked;
  this->active_ = active;
  this->in_use_ = in_use;

  // Update the background color inside the icon
  switch( this->get_volume_type() )
  {

    case Core::VolumeType::LARGE_DATA_E:
    case Core::VolumeType::DATA_E:
    {
      if ( locked )
      {
        this->ui_.type_->setStyleSheet( 
          StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C );
      }
      else
      {
        this->ui_.type_->setStyleSheet( StyleSheet::DATA_VOLUME_COLOR_C );
      }
    }
    break;
    case Core::VolumeType::MASK_E:
    {
      int color_index =  dynamic_cast< MaskLayer* >( 
        this->layer_.get() )->color_state_->get();
      this->parent_->set_mask_background_color( color_index );
    }
    break;
  }

  if ( locked )
  {
    this->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_LOCKED_C );
    this->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_LOCKED_C ); 
  }
  else if( active && !in_use )
  {
    this->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
    this->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_ACTIVE_C );
    if( this->layer_->gui_state_group_->get_enabled() )
      this->ui_.header_->setStyleSheet( StyleSheet::LAYER_WIDGET_HEADER_ACTIVE_C );
  }
  else if( active && in_use )
  {
    this->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_IN_USE_C );  
    this->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_ACTIVE_IN_USE_C );
    if( this->layer_->gui_state_group_->get_enabled() )
      this->ui_.header_->setStyleSheet( StyleSheet::LAYER_WIDGET_HEADER_ACTIVE_IN_USE_C );
  }
  else if ( in_use )
  {
    this->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_IN_USE_C );  
    this->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_IN_USE_C );
    if( this->layer_->gui_state_group_->get_enabled() )
      this->ui_.header_->setStyleSheet( StyleSheet::LAYER_WIDGET_HEADER_IN_USE_C );
  }
  else
  {
    this->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
    this->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_INACTIVE_C );
    if( this->layer_->gui_state_group_->get_enabled() )
      this->ui_.header_->setStyleSheet( StyleSheet::LAYER_WIDGET_HEADER_INACTIVE_C );
  }
}

void LayerWidgetPrivate::update_widget_state( bool initialize )
{
  // Step 1 : retrieve the state of the layer
  std::string data_state;
  bool visual_lock = false;
  bool active_layer = false;

  {
    // NOTE:
    // Since state is changed on the application thread
    // We need to lock the state engine to prevent any change
    // to be made into the state manager.
    Layer::lock_type lock( Layer::GetMutex() );
    data_state = this->layer_->data_state_->get();
    visual_lock = this->layer_->locked_state_->get();
    active_layer = ( LayerManager::Instance()->get_active_layer() == this->layer_ );
  }

  if ( data_state == Layer::AVAILABLE_C )
  {
    // Change the color of the widget
    this->update_appearance( visual_lock,  active_layer, false, initialize );
    this->ui_.selection_checkbox_->setVisible( true );
  }
  else if ( data_state == Layer::CREATING_C )
  {
    // Change the color of the widget
    this->update_appearance( true,  false, false, initialize ); 
    this->ui_.selection_checkbox_->setVisible( false );
  }
  else if ( data_state == Layer::PROCESSING_C )
  {
    // Change the color of the widget
    this->update_appearance( visual_lock,  active_layer, true, initialize );  
    this->ui_.selection_checkbox_->setVisible( false );
  }
  else if ( data_state == Layer::IN_USE_C )
  {
    // Change the color of the widget
    this->update_appearance( visual_lock,  active_layer, true, initialize );    
    this->ui_.selection_checkbox_->setVisible( true );
  }
}

int LayerWidgetPrivate::get_volume_type()
{ 
  return this->layer_->get_type(); 
}

void LayerWidgetPrivate::set_picked_up( bool picked_up )
{ 
  this->picked_up_ = picked_up; 
  if ( picked_up )
  {
    this->overlay_->set_transparent( false );
    this->overlay_->show();
  }
  else
  {
    this->overlay_->hide();
  }
}

void LayerWidgetPrivate::set_drop_target( LayerWidget* target_layer )
{
  this->drop_layer_ = target_layer;
  this->drop_layer_set_ = true;
}

void LayerWidgetPrivate::enable_drop_space( bool drop )
{
  // First we check to see if it is picked up if so, we set change the way it looks
  this->drop_space_->set_height( this->picked_up_layer_size_ + 4 );

  // Ignore if the widget is currently being dragged
  if ( this->picked_up_ ) return;
  
  if( drop )
  {
    this->overlay_->set_transparent( true );
    this->overlay_->show();
    this->drop_space_->show();
  }
  else
  {
    this->drop_space_->hide();
    this->overlay_->hide();
  }
} 

void LayerWidgetPrivate::update_progress_bar( double progress )
{
  this->ui_.progress_bar_->setValue( static_cast<int>( progress * 100.0 ) );
}

void LayerWidgetPrivate::export_layer( const std::string& type_extension )
{
  boost::filesystem::path current_folder = ProjectManager::Instance()->get_current_file_folder();
  QString export_path = QFileDialog::getExistingDirectory( this->parent_, tr( "Choose Directory for Export..." ),
    current_folder.string().c_str(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  if( export_path == "" ) return;

  if( !QFileInfo( export_path ).exists() )
  {
    CORE_LOG_ERROR( "The layer could not be exported to that location. Please try another location." );
    return;
  }
  try
  {
    boost::filesystem::create_directory( boost::filesystem::path( export_path.toStdString() ) / "delete_me" );
  }
  catch( ... ) // if the create fails then we are not in a writable directory
  {
    CORE_LOG_ERROR( "The layer could not be exported to that location. Please try another location." );
    return;
  }

  // if we've made it here then we've created a folder and we need to delete it.
  boost::filesystem::remove(  boost::filesystem::path( export_path.toStdString() ) / "delete_me" );

  if( this->get_volume_type() == Core::VolumeType::MASK_E )
  {
    ActionExportSegmentation::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->layer_->get_layer_id(), "single_mask", 
      export_path.toStdString(), type_extension );
  }
  else if( this->get_volume_type() == Core::VolumeType::DATA_E )
  { 
    std::string file_name = ( boost::filesystem::path( export_path.toStdString() ) / 
      this->layer_->get_layer_name() ).string();

    ActionExportLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->layer_->get_layer_id(), file_name, type_extension );
  }
}


void LayerWidgetPrivate::export_isosurface()
{

  MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( this->layer_.get() );
  if ( ! mask_layer->iso_generated_state_->get() )
  {
    QMessageBox message_box;
    message_box.setWindowTitle( "Export Isosurface Error" );
    message_box.addButton( QMessageBox::Ok );
    message_box.setIcon( QMessageBox::Critical );
    message_box.setText( "Isosurface must be created before it can be exported." );
    message_box.exec();
    return;
  }

  QString filename;
  boost::filesystem::path current_folder = ProjectManager::Instance()->get_current_file_folder();  

  filename = QFileDialog::getSaveFileName( this->parent_,
    tr("Export Isosurface As... "), QString::fromStdString( current_folder.string() ),
    QString::fromStdString( Core::Isosurface::EXPORT_FORMATS_C ) );

  if ( filename.isEmpty() ) return;

  ActionExportIsosurface::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->layer_->get_layer_id(), filename.toStdString(), this->layer_->name_state_->get() );

}


void LayerWidgetPrivate::UpdateViewerButtons( qpointer_type qpointer, std::string layout )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &LayerWidgetPrivate::UpdateViewerButtons,
      qpointer, layout ) );
    return; 
  }

  LayerWidgetPrivate* widget_private = qpointer.data();

  if ( !widget_private || QCoreApplication::closingDown() ) return;

  if( layout == "single" )
  {
    widget_private->ui_.left_viewers_widget_->setMinimumWidth( 150 );
    widget_private->ui_.left_viewers_widget_->setMaximumWidth( 150 );
  }
  else if( layout == "1and2" )
  {
    widget_private->ui_.left_viewers_widget_->setMinimumWidth( 90 );
    widget_private->ui_.left_viewers_widget_->setMaximumWidth( 90 );
  }
  else if( layout == "1and3" )
  {
    widget_private->ui_.left_viewers_widget_->setMinimumWidth( 105 );
    widget_private->ui_.left_viewers_widget_->setMaximumWidth( 105 );
  }
  else
  {
    widget_private->ui_.left_viewers_widget_->setMinimumWidth( 75 );
    widget_private->ui_.left_viewers_widget_->setMaximumWidth( 75 );
  }
}

void LayerWidgetPrivate::UpdateState( qpointer_type qpointer )
{
  // Hand it off to the right thread
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( 
      boost::bind( &LayerWidgetPrivate::UpdateState, qpointer) );
    return; 
  }

  // When we are finally on the interface thread run this code:
  if ( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->parent_->setUpdatesEnabled( false );
    qpointer->update_widget_state();
    qpointer->parent_->setUpdatesEnabled( true );
  }
}

void LayerWidgetPrivate::UpdateActiveState( qpointer_type qpointer, LayerHandle layer )
{
  // Hand it off to the right thread
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( 
      boost::bind( &LayerWidgetPrivate::UpdateActiveState, qpointer, layer ) );
    return; 
  }

  // When we are finally on the interface thread run this code:
  if ( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->parent_->setUpdatesEnabled( false );
    qpointer->update_widget_state();
    qpointer->parent_->setUpdatesEnabled( true );
  }
}

void LayerWidgetPrivate::UpdateProgress( qpointer_type qpointer, double progress )
{
  // Hand it off to the right thread
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &LayerWidgetPrivate::UpdateProgress, 
      qpointer, progress) );
    return; 
  }

  // When we are finally on the interface thread run this code:
  if ( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->update_progress_bar( progress );
    qpointer->parent_->update();
  }
}

//////////////////////////////////////////////////////////////////////////
// Class LayerWidget
//////////////////////////////////////////////////////////////////////////

LayerWidget::LayerWidget( QFrame* parent, LayerHandle layer ) :
  QWidget( parent )
{
  this->setAttribute( Qt::WA_TranslucentBackground, true ); 
  
  this->private_ = new LayerWidgetPrivate( this );
  this->private_->layer_ = layer;
  
  this->private_->active_ = false;
  this->private_->locked_ = false;
  this->private_->picked_up_ = false;
  this->private_->drop_layer_set_ = false;
  this->private_->drop_group_set_ = false;
  
  // Store the icons in the private class, so they only need to be generated once
  this->private_->label_layer_icon_.addFile( QString::fromUtf8( ":/Images/LabelMapWhite.png" ),
    QSize(), QIcon::Normal, QIcon::Off );
  this->private_->data_layer_icon_.addFile( QString::fromUtf8( ":/Images/DataWhite.png" ), 
    QSize(), QIcon::Normal, QIcon::Off );
  this->private_->mask_layer_icon_.addFile( QString::fromUtf8( ":/Images/MaskWhite_shadow.png" ), 
    QSize(), QIcon::Normal, QIcon::Off );
  this->private_->large_data_layer_icon_.addFile( QString::fromUtf8( ":/Images/LargeWhite.png" ), 
    QSize(), QIcon::Normal, QIcon::Off );

  // Update the style sheet of this widget
  this->setObjectName( QString::fromUtf8( "LayerWidget" ) );
  this->setAutoFillBackground( true );

  // Add the Ui children onto the QWidget
  this->private_->ui_.setupUi( this );
  
#ifdef __APPLE__
  QList<QLabel*> children = findChildren< QLabel* >();
  QList<QLabel*>::iterator it = children.begin();
  QList<QLabel*>::iterator it_end = children.end();
  
  while( it != it_end )
  {
    QFont font = ( *it )->font();
    font.setPointSize( 11 );
    ( *it )->setFont( font );
    ++it;
  }
#endif  
  
  
  this->setUpdatesEnabled( false );
  
  // set some Drag and Drop stuff
  this->setAcceptDrops( true );
  
  // Set the defaults
  // this is a default setting until we can get the name of the layer from the file or by some other means
  this->private_->ui_.label_->setText( QString::fromStdString( layer->name_state_->get() ) );
  this->private_->ui_.label_->setAcceptDrops( false );
  this->private_->ui_.label_->setValidator( new QRegExpValidator( 
    QRegExp( QString::fromStdString( Core::StateName::REGEX_VALIDATOR_C ) ), this ) );
  QSizePolicy size_policy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  this->private_->ui_. label_->setSizePolicy( size_policy );

  // Hide the tool bars and the selection checkbox
  this->private_->ui_.checkbox_widget_->hide();
  this->private_->ui_.progress_bar_bar_->hide();
  this->private_->ui_.appearance_bar_->hide();
  this->private_->ui_.info_bar_->hide();
  this->private_->ui_.abort_bar_->hide();
  this->private_->ui_.opacity_bar_->hide();
  this->private_->ui_.volume_rendered_button_->hide();

  // Connect the thresholds so that they keep in sync
  this->private_->ui_.display_max_adjuster_->connect_max( this->private_->ui_.display_min_adjuster_ );
  this->private_->ui_.display_min_adjuster_->connect_min( this->private_->ui_.display_max_adjuster_ );

  
  QtUtils::QtBridge::Show( this->private_->ui_.advanced_visibility_button_, 
    PreferencesManager::Instance()->advanced_visibility_settings_state_ );
  
  // add the PushDragButtongroup|group_0
  this->private_->activate_button_ = new PushDragButton( this->private_->ui_.typeGradient_ );
  this->private_->activate_button_->setObjectName( QString::fromUtf8( "activate_button_" ) );
  this->private_->activate_button_->setStyleSheet( StyleSheet::LAYER_PUSHDRAGBUTTON_C );
  this->private_->activate_button_->setMinimumHeight( 37 );
  this->private_->activate_button_->setMinimumWidth( 29 );
  this->private_->activate_button_->setIconSize( QSize( 25, 25 ) );
  this->private_->ui_.horizontalLayout_9->setContentsMargins( 1, 0, 1, 0 );
  this->private_->ui_.horizontalLayout_9->addWidget( this->private_->activate_button_ );
  this->private_->activate_button_->setAcceptDrops( false );
  
  // add the DropSpaceWidget
  this->private_->drop_space_ = new DropSpaceWidget( this );
  this->private_->ui_.verticalLayout_10->insertWidget( 0, this->private_->drop_space_ );
  this->private_->drop_space_->hide();
  
  this->private_->color_widget_ = new QtUtils::QtColorBarWidget( this );
  this->private_->ui_.horizontalLayout_14->addWidget( this->private_->color_widget_ );
  this->private_->color_widget_->setObjectName( QString::fromUtf8( "color_widget_" ) );
        
  this->connect( this->private_->ui_.abort_button_,
    SIGNAL ( pressed() ), this, SLOT( trigger_abort() ) );

  this->connect( this->private_->ui_.stop_button_,
    SIGNAL ( pressed() ), this, SLOT( trigger_stop() ) );

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
    LayerWidgetPrivate::qpointer_type qpointer( this->private_ );
    
    // Connect a slot to the signal that the visual lock state has changed
    this->private_->add_connection( layer->locked_state_->state_changed_signal_.connect(
        boost::bind( &LayerWidgetPrivate::UpdateState, qpointer ) ) );

    // Connect a slot to the signal that the data integrity state has changed
    this->private_->add_connection( layer->data_state_->state_changed_signal_.connect(
        boost::bind( &LayerWidgetPrivate::UpdateState, qpointer ) ) );

    // Connect a slot to the signal that posts updates to the current progress of a filter
    this->private_->add_connection( layer->update_progress_signal_.connect(
        boost::bind( &LayerWidgetPrivate::UpdateProgress, qpointer, _1 ) ) );
      
    // Connect a slot to the signal that the active layer has changed
    this->private_->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect(
        boost::bind( &LayerWidgetPrivate::UpdateActiveState, qpointer, _1 ) ) );  
    
    // Connect a slot to the signal that indicates that the layout of the viewer has
    // changed  
    this->private_->add_connection( ViewerManager::Instance()->layout_state_->value_changed_signal_.connect(
        boost::bind( &LayerWidgetPrivate::UpdateViewerButtons, qpointer, _1 ) ) );
      
    // Reflect the current state in the widget      
    LayerWidgetPrivate::UpdateViewerButtons( qpointer, ViewerManager::Instance()->layout_state_->get() );
  
    // Make the default connections, for any layer type, to the state engine
  
    // Connect activate button to dispatching an action activating the current layer
    QtUtils::QtBridge::Connect( this->private_->activate_button_, boost::bind( 
      &ActionActivateLayer::Dispatch, Core::Interface::GetWidgetActionContext(), 
      layer->get_layer_id() ) );
      
    connect( this->private_->ui_.label_, SIGNAL( activate_layer_signal() ), this, 
      SLOT( activate_from_lineedit_focus() ) );
      
    // now connect it to a signal that 
    connect( this->private_->ui_.selection_checkbox_, SIGNAL( stateChanged( int ) ), this, 
      SIGNAL( selection_box_changed() ) );
      
    // Connect the label to the layer name state variable 
    QtUtils::QtBridge::Connect( this->private_->ui_.label_, layer->name_state_ );

    // Connect the visibility button to its underlying state variable
    QtUtils::QtBridge::Connect( this->private_->ui_.visibility_button_, 
      layer->master_visible_state_ );
      
    // Connect the visibility buttons in the advanced viewer menu to their respective state 
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_0_button_, 
      layer->visible_state_[ 0 ] );
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_1_button_,
      layer->visible_state_[ 1 ] );
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_2_button_,
      layer->visible_state_[ 2 ] );
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_3_button_,
      layer->visible_state_[ 3 ] );
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_4_button_, 
      layer->visible_state_[ 4 ] );
    QtUtils::QtBridge::Connect( this->private_->ui_.viewer_5_button_,
      layer->visible_state_[ 5 ] );

    // Connect the opacity state to its state variable
    QtUtils::QtBridge::Connect( this->private_->ui_.opacity_adjuster_, layer->opacity_state_ );
    
    // Connect the visual lock button to the lock state variable
    QtUtils::QtBridge::Connect( this->private_->ui_.lock_button_, layer->locked_state_ );
    
    LayerGroupHandle layer_group = layer->get_layer_group();
    QtUtils::QtBridge::Connect( this->private_->ui_.dimensions_,
      layer_group->dimensions_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.origin_label_,
      layer_group->origin_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.spacing_label_,
      layer_group->spacing_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.centering_label_,
      layer->centering_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.provenance_id_,
      layer->provenance_id_state_ );
      
    QtUtils::QtBridge::Connect( this->private_->ui_.info_button_, 
      layer->show_information_state_ );
    QtUtils::QtBridge::Show( this->private_->ui_.info_bar_, 
      layer->show_information_state_ );

    QtUtils::QtBridge::Show( this->private_->ui_.progress_bar_bar_, 
      layer->show_progress_bar_state_ );
    QtUtils::QtBridge::Show( this->private_->ui_.abort_bar_, 
      layer->show_abort_message_state_ );

    QtUtils::QtBridge::Show( this->private_->ui_.stop_button_, 
      layer->show_stop_button_state_ );

    // Connect all the buttons to the viewers
    QtUtils::QtBridge::Show( this->private_->ui_.viewer_0_button_, 
      ViewerManager::Instance()->get_viewer( 0 )->viewer_visible_state_ );
      
    QtUtils::QtBridge::Show( this->private_->ui_.viewer_1_button_, 
      ViewerManager::Instance()->get_viewer( 1 )->viewer_visible_state_ );
      
    QtUtils::QtBridge::Show( this->private_->ui_.viewer_2_button_, 
      ViewerManager::Instance()->get_viewer( 2 )->viewer_visible_state_ );
      
    QtUtils::QtBridge::Show( this->private_->ui_.viewer_3_button_, 
      ViewerManager::Instance()->get_viewer( 3 )->viewer_visible_state_ );
      
    QtUtils::QtBridge::Show( this->private_->ui_.viewer_4_button_, 
      ViewerManager::Instance()->get_viewer( 4 )->viewer_visible_state_ );
      
    QtUtils::QtBridge::Show( this->private_->ui_.viewer_5_button_, 
      ViewerManager::Instance()->get_viewer( 5 )->viewer_visible_state_ );
    
    std::vector< Core::StateBaseHandle > enable_states( 2 );
    enable_states[ 0 ] = layer->locked_state_;
    enable_states[ 1 ] = layer->data_state_;
    
    boost::function< bool () > condition = !boost::lambda::bind( &Core::StateBool::get, 
      layer->locked_state_.get() ) && boost::lambda::bind( &Core::StateOption::get,
      layer->data_state_.get() ) != Layer::CREATING_C;

    // The following buttons are enabled when the layer is not locked and not being created
    QtUtils::QtBridge::Enable( this->private_->activate_button_, enable_states, condition );
    QtUtils::QtBridge::Enable( this->private_->ui_.appearance_button_, enable_states, 
      condition );
    QtUtils::QtBridge::Enable( this->private_->ui_.volume_rendered_button_, enable_states, 
      condition );
    QtUtils::QtBridge::Enable( this->private_->ui_.label_, enable_states, condition );
    
    // The following buttons are enabled when the layer is not being created
    condition = boost::lambda::bind( &Core::StateOption::get, layer->data_state_.get() ) != 
      Layer::CREATING_C;
    QtUtils::QtBridge::Enable( this->private_->ui_.opacity_button_, layer->data_state_, 
      condition );
    QtUtils::QtBridge::Enable( this->private_->ui_.info_button_, layer->data_state_, 
      condition );
    QtUtils::QtBridge::Enable( this->private_->ui_.lock_button_, layer->data_state_, 
      condition );
      
    QtUtils::QtBridge::Connect( this->private_->ui_.appearance_button_,
      layer->show_appearance_state_ );
    QtUtils::QtBridge::Show( this->private_->ui_.appearance_bar_,
      layer->show_appearance_state_ );
      
    QtUtils::QtBridge::Connect( this->private_->ui_.advanced_visibility_button_,
      layer->show_advanced_visibility_state_ );
    QtUtils::QtBridge::Show( this->private_->ui_.advanced_visibility_bar_,
      layer->show_advanced_visibility_state_ );
    
    QtUtils::QtBridge::Connect( this->private_->ui_.opacity_button_,
      layer->show_opacity_state_ );
    QtUtils::QtBridge::Show( this->private_->ui_.opacity_bar_,
      layer->show_opacity_state_ );

    // Compute isosurface button is enabled when the layer is not locked and is available
    QtUtils::QtBridge::Enable( this->private_->ui_.compute_iso_surface_button_, enable_states,
      !boost::lambda::bind( &Core::StateBool::get, layer->locked_state_.get() ) && 
      boost::lambda::bind( &Core::StateOption::get, layer->data_state_.get() ) == 
      Layer::AVAILABLE_C );

    // The selection check box is visible when the group delete layers menu is open
    // and the layer is not locked
    enable_states[ 1 ] = layer_group->show_delete_menu_state_;
    QtUtils::QtBridge::Show( this->private_->ui_.checkbox_widget_, enable_states,
      !boost::lambda::bind( &Core::StateBool::get, layer->locked_state_.get() ) &&
      boost::lambda::bind( &Core::StateBool::get, 
      layer_group->show_delete_menu_state_.get() ) );
    
    
    enable_states[ 1 ] = layer_group->show_duplicate_menu_state_;
    QtUtils::QtBridge::Show( this->private_->ui_.checkbox_widget_, enable_states,
      !boost::lambda::bind( &Core::StateBool::get, layer->locked_state_.get() ) &&
      boost::lambda::bind( &Core::StateBool::get, 
      layer_group->show_duplicate_menu_state_.get() ) );
    
    // Control the enable/disable state of the visibility_button_
    enable_states.clear();
    size_t num_of_viewers = ViewerManager::Instance()->number_of_viewers();
    std::vector< Core::StateBoolHandle > viewer_visible_states( num_of_viewers );
    for ( size_t i = 0; i < num_of_viewers; ++i )
    {
      ViewerHandle viewer = ViewerManager::Instance()->get_viewer( i );
      viewer_visible_states[ i ] = viewer->viewer_visible_state_;
      enable_states.push_back( viewer->viewer_visible_state_ );
      enable_states.push_back( layer->visible_state_[ i ] );
    }
    condition = ( boost::lambda::bind( &Core::StateBool::get, viewer_visible_states[ 0 ].get() ) &&
      boost::lambda::bind( &Core::StateBool::get, layer->visible_state_[ 0 ].get() ) ) ||
      ( boost::lambda::bind( &Core::StateBool::get, viewer_visible_states[ 1 ].get() ) &&
      boost::lambda::bind( &Core::StateBool::get, layer->visible_state_[ 1 ].get() ) ) || 
      ( boost::lambda::bind( &Core::StateBool::get, viewer_visible_states[ 2 ].get() ) &&
      boost::lambda::bind( &Core::StateBool::get, layer->visible_state_[ 2 ].get() ) ) ||
      ( boost::lambda::bind( &Core::StateBool::get, viewer_visible_states[ 3 ].get() ) &&
      boost::lambda::bind( &Core::StateBool::get, layer->visible_state_[ 3 ].get() ) ) ||
      ( boost::lambda::bind( &Core::StateBool::get, viewer_visible_states[ 4 ].get() ) &&
      boost::lambda::bind( &Core::StateBool::get, layer->visible_state_[ 4 ].get() ) ) ||
      ( boost::lambda::bind( &Core::StateBool::get, viewer_visible_states[ 5 ].get() ) &&
      boost::lambda::bind( &Core::StateBool::get, layer->visible_state_[ 5 ].get() ) );
    QtUtils::QtBridge::Enable( this->private_->ui_.visibility_button_, enable_states, condition );
    
    this->private_->ui_.brightness_adjuster_->set_description( "Brightness" );
    this->private_->ui_.contrast_adjuster_->set_description( "Contrast" );
    this->private_->ui_.display_max_adjuster_->set_description( "Max" );
    this->private_->ui_.display_min_adjuster_->set_description( "Min" );
    this->private_->ui_.opacity_adjuster_->set_description( "Opacity" );
    
    
    switch( this->private_->get_volume_type() )
    {
      // This if for the Data Layers
      case Core::VolumeType::DATA_E:
      {
        // Update the icons
        this->private_->activate_button_->setIcon(this->private_->data_layer_icon_);
      
        // Hide the buttons that are not needed for this widget
        this->private_->ui_.compute_iso_surface_button_->hide();
        this->private_->ui_.delete_iso_surface_button_->hide();
        this->private_->ui_.show_iso_surface_button_->hide();
        this->private_->ui_.iso_control_separator_line_->hide();
        this->private_->ui_.border_->hide();
        this->private_->ui_.fill_->hide();
        this->private_->ui_.mask_volume_widget_->hide();
        this->private_->ui_.colors_->hide();
        this->private_->ui_.isosurface_area_widget_->hide();
        
        // Add the layer specific connections
        DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
        if ( data_layer )
        {
          QtUtils::QtBridge::Connect( this->private_->ui_.brightness_adjuster_, 
            data_layer->brightness_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.contrast_adjuster_, 
            data_layer->contrast_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.datatype_label_,
            data_layer->data_type_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.min_label_,
            data_layer->min_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.max_label_,
            data_layer->max_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.display_max_adjuster_,
            data_layer->display_max_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.display_min_adjuster_,
            data_layer->display_min_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.adjust_minmax_checkbox_,
            data_layer->adjust_display_min_max_state_ );
          QtUtils::QtBridge::Show( this->private_->ui_.brightness_, 
            data_layer->adjust_display_min_max_state_, true );
          QtUtils::QtBridge::Show( this->private_->ui_.contrast_, 
            data_layer->adjust_display_min_max_state_, true );
          QtUtils::QtBridge::Show( this->private_->ui_.display_max_, 
            data_layer->adjust_display_min_max_state_ );
          QtUtils::QtBridge::Show( this->private_->ui_.display_min_, 
            data_layer->adjust_display_min_max_state_ );
      
          connect( this->private_->ui_.reset_brightness_contrast_button_, 
            SIGNAL( clicked() ), this, SLOT( set_brightness_contrast_to_default() ) );
        }
      }
      break;

      case Core::VolumeType::LARGE_DATA_E:
      {
        // Update the icons
        this->private_->activate_button_->setIcon(this->private_->large_data_layer_icon_);
      
        // Hide the buttons that are not needed for this widget
        this->private_->ui_.compute_iso_surface_button_->hide();
        this->private_->ui_.delete_iso_surface_button_->hide();
        this->private_->ui_.show_iso_surface_button_->hide();
        this->private_->ui_.iso_control_separator_line_->hide();
        this->private_->ui_.border_->hide();
        this->private_->ui_.fill_->hide();
        this->private_->ui_.mask_volume_widget_->hide();
        this->private_->ui_.colors_->hide();
        this->private_->ui_.isosurface_area_widget_->hide();
        
        // Add the layer specific connections
        LargeVolumeLayer* data_layer = dynamic_cast< LargeVolumeLayer* >( layer.get() );
        if ( data_layer )
        {
          QtUtils::QtBridge::Connect( this->private_->ui_.brightness_adjuster_, 
            data_layer->brightness_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.contrast_adjuster_, 
            data_layer->contrast_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.datatype_label_,
            data_layer->data_type_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.min_label_,
            data_layer->min_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.max_label_,
            data_layer->max_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.display_max_adjuster_,
            data_layer->display_max_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.display_min_adjuster_,
            data_layer->display_min_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.adjust_minmax_checkbox_,
            data_layer->adjust_display_min_max_state_ );
          QtUtils::QtBridge::Show( this->private_->ui_.brightness_, 
            data_layer->adjust_display_min_max_state_, true );
          QtUtils::QtBridge::Show( this->private_->ui_.contrast_, 
            data_layer->adjust_display_min_max_state_, true );
          QtUtils::QtBridge::Show( this->private_->ui_.display_max_, 
            data_layer->adjust_display_min_max_state_ );
          QtUtils::QtBridge::Show( this->private_->ui_.display_min_, 
            data_layer->adjust_display_min_max_state_ );
      
          connect( this->private_->ui_.reset_brightness_contrast_button_, 
            SIGNAL( clicked() ), this, SLOT( set_brightness_contrast_to_default() ) );
        }
      }
      break;

      // This is for the Mask Layers  
      case Core::VolumeType::MASK_E:
      {
        // Update the icons
        this->private_->activate_button_->setIcon( this->private_->mask_layer_icon_ );

        // Hide the buttons that are not needed for this widget
        this->private_->ui_.bright_contrast_->hide();
        this->private_->ui_.datainfo_widget_->hide();
        
        this->connect( this->private_->color_widget_, SIGNAL( color_index_changed( int ) ), 
          this, SLOT( set_mask_background_color( int ) ) );

        // Color changes only come from preferences and we only want to change the mask
        // background color if the color was changed for the current index 
        this->connect( this->private_->color_widget_, SIGNAL( color_changed( int ) ), 
          this, SLOT( set_mask_background_color_from_preference_change( int ) ) );
          
        MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );  
        if ( mask_layer )
        {
          // Connect isosurface buttons
          QtUtils::QtBridge::Connect( this->private_->ui_.show_iso_surface_button_, 
            mask_layer->show_isosurface_state_ );
          connect( this->private_->ui_.compute_iso_surface_button_, 
            SIGNAL( clicked() ), this, SLOT( compute_isosurface() ) );
          connect( this->private_->ui_.delete_iso_surface_button_, 
            SIGNAL( clicked() ), this, SLOT( delete_isosurface() ) );

          QtUtils::QtBridge::Connect( this->private_->ui_.border_selection_combo_, 
            mask_layer->border_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.fill_selection_combo_, 
            mask_layer->fill_state_ );
          QtUtils::QtBridge::Connect( this->private_->color_widget_, 
            mask_layer->color_state_,
            PreferencesManager::Instance()->color_states_ );
            
          QtUtils::QtBridge::Enable( this->private_->ui_.show_iso_surface_button_, 
            mask_layer->iso_generated_state_ );
          QtUtils::QtBridge::Enable( this->private_->ui_.delete_iso_surface_button_, 
            mask_layer->iso_generated_state_ );
            
          QtUtils::QtBridge::Connect( this->private_->ui_.mask_volume_label_, 
            mask_layer->calculated_volume_state_ );
            
          QtUtils::QtBridge::Connect( this->private_->ui_.mask_pixel_count_, 
            mask_layer->counted_pixels_state_ );
            
          QtUtils::QtBridge::Connect( this->private_->ui_.calculate_volume_button_,
            boost::bind( &ActionCalculateMaskVolume::Dispatch, 
            Core::Interface::GetWidgetActionContext(), this->get_layer_id() ) );

          QtUtils::QtBridge::Connect( this->private_->ui_.isosurface_area_,
            mask_layer->isosurface_area_state_ );
          QtUtils::QtBridge::Show( this->private_->ui_.isosurface_area_widget_,
            mask_layer->iso_generated_state_ );

          QtUtils::QtBridge::Connect( this->private_->ui_.min_label_,
                                     mask_layer->min_value_state_ );
          QtUtils::QtBridge::Connect( this->private_->ui_.max_label_,
                                     mask_layer->max_value_state_ );
          
          this->set_mask_background_color( mask_layer->color_state_->get() );
        }
      }
      break;
        
      // This is for the Label Layers
      case Core::VolumeType::LABEL_E:
      {
        // Update the icons
        this->private_->activate_button_->setIcon(this->private_->label_layer_icon_);
      }
      break;
        
      default:
      break;
    }   
  }
  
  // Set up the overlay widgets
  this->private_->overlay_ = new OverlayWidget( this );
  this->private_->overlay_->hide(); 
  this->private_->ui_.facade_widget_->hide();
  this->private_->ui_.verticalLayout_3->setAlignment( Qt::AlignBottom );
  
  this->private_->update_widget_state( true );
  this->setUpdatesEnabled( true );
}

LayerWidget::~LayerWidget()
{
  this->private_->disconnect_all();
}

void LayerWidget::compute_isosurface()
{
  // Get isosurface quality factor from LayerGroup that this layer belongs to
  LayerGroupHandle layer_group = this->private_->layer_->get_layer_group();

  double quality = 1.0;
  bool capping_enabled;
  {
    Core::StateEngine::lock_type state_engine_lock( Core::StateEngine::GetMutex() );  
    Core::ImportFromString( layer_group->isosurface_quality_state_->get(), quality ); 
    capping_enabled = layer_group->isosurface_capping_enabled_state_->get();
  }

  // Dispatch action to compute isosurface
  MaskLayerHandle mask_layer = boost::dynamic_pointer_cast< MaskLayer >( this->private_->layer_ );
  ActionComputeIsosurface::Dispatch( Core::Interface::GetWidgetActionContext(), mask_layer, 
    quality, capping_enabled, true );
}

void LayerWidget::delete_isosurface()
{
  // Dispatch action to delete isosurface
  MaskLayerHandle mask_layer = boost::dynamic_pointer_cast< MaskLayer >( this->private_->layer_ );
  ActionDeleteIsosurface::Dispatch( Core::Interface::GetWidgetActionContext(), mask_layer );
  this->private_->ui_.show_iso_surface_button_->setChecked( false );
}
  
std::string LayerWidget::get_layer_id() const
{ 
  return this->private_->layer_->get_layer_id(); 
}

std::string LayerWidget::get_layer_name() const
{
  return this->private_->layer_->get_layer_name();
}

void LayerWidget::set_mask_background_color( int color_index )
{
  Core::Color color = PreferencesManager::Instance()->color_states_[ color_index ]->get();
    
  int r = static_cast< int >( color.r() );
  int g = static_cast< int >( color.g() );
  int b = static_cast< int >( color.b() );

  QString style_sheet = QString::fromUtf8( 
  "background-color: rgb(" ) + QString::number( r ) +
  QString::fromUtf8( ", " ) + QString::number( g ) +
  QString::fromUtf8( ", " ) + QString::number( b ) +
  QString::fromUtf8( ");" );

  this->private_->ui_.type_->setStyleSheet( style_sheet );
}

void LayerWidget::set_mask_background_color_from_preference_change( int color_index )
{
  // We only want to change the mask background color if the color was changed for the 
  // selected index 
  if( dynamic_cast< MaskLayer* >( this->private_->layer_.get()  )->color_state_->get() == 
    color_index )
  {
    this->set_mask_background_color( color_index );
  }
}

void LayerWidget::trigger_abort()
{
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    this->private_->layer_->show_progress_bar_state_, false );
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    this->private_->layer_->show_abort_message_state_, true );

  // Text for when the abort button has been pressed
  this->private_->ui_.abort_text_->setText( "Waiting for process to abort ..." );
  this->private_->layer_->abort_signal_();
}

void LayerWidget::trigger_stop()
{
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    this->private_->layer_->show_progress_bar_state_, false );
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    this->private_->layer_->show_abort_message_state_, true );

  // Text for when the abort button has been pressed
  this->private_->ui_.abort_text_->setText( "Waiting for process to stop ..." );
  this->private_->layer_->stop_signal_();
}

void LayerWidget::set_brightness_contrast_to_default()
{
  // These are hard coded because we have no way of retrieving the default values
  // We can set the sliderspinners directly because they will emit the proper signals
  this->private_->ui_.brightness_adjuster_->setCurrentValue( 50.0 );
  this->private_->ui_.contrast_adjuster_->setCurrentValue( 0 );
}
  
void LayerWidget::mousePressEvent( QMouseEvent *event )
{
  // Exit immediately if they are no longer holding the button the press event isn't valid
  if( event->button() != Qt::LeftButton )
  { 
    event->setAccepted( true );
    return;
  }
  
  if( ( event->modifiers() != Qt::ControlModifier ) && ( event->modifiers() != Qt::ShiftModifier ) )
  {
    ActionActivateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), this->get_layer_id() );
    event->setAccepted( true );
    return;
  }
  
  QPoint hotSpot = event->pos();
  
  // Make up some mimedata containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;

  LayerHandle layer = this->private_->layer_;
  mimeData->setText( QString::fromStdString( layer->get_layer_name() ) );
  
  // Create a drag object and insert the hotspot
  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );
  
  // here we add basically a screenshot of the widget
  QImage temp_image( this->private_->ui_.base_->size(), QImage::Format_ARGB32 );
  temp_image.fill( 0 );
  this->private_->ui_.base_->render( &temp_image, QPoint(), this->private_->ui_.base_->rect(), QWidget::DrawChildren );
  drag->setPixmap( QPixmap::fromImage( temp_image ) );
  drag->setHotSpot( hotSpot );

  // Next we hide the LayerWidget that we are going to be dragging.
  this->parentWidget()->setMinimumHeight( this->parentWidget()->height() );
  this->private_->set_picked_up( true );
  
  Q_EMIT prep_for_drag_and_drop( true );
  Q_EMIT layer_size_signal_( this->height() - 2 );
  
  // Finally if our drag was aborted then we reset the layers styles to be visible
  if( ( ( drag->exec(Qt::MoveAction, Qt::MoveAction) ) == Qt::MoveAction ) 
    && ( this->private_->drop_layer_set_ || this->private_->drop_group_set_ ) )
  { 
    if ( this->private_->drop_layer_set_ )
    {
      LayerGroupHandle dst_group = this->private_->drop_layer_->
        private_->layer_->get_layer_group();
      if ( this->private_->layer_->get_layer_group() != dst_group )
      {
        LayerResamplerHandle layer_resampler( new LayerResampler(
          this->private_->layer_, this->private_->drop_layer_->private_->layer_ ) );
        LayerResamplerDialog* dialog = new LayerResamplerDialog( layer_resampler, this );
        if ( dialog->exec() == QDialog::Accepted )
        {
          layer_resampler->execute( Core::Interface::GetWidgetActionContext() );
        }
        dialog->deleteLater();
      }
      else
      {
        ActionMoveLayer::Dispatch( Core::Interface::GetWidgetActionContext(),
          this->get_layer_id(), this->private_->drop_layer_->get_layer_id() );
      }
    }
    else if ( this->private_->drop_group_set_ )
    {
      LayerGroupHandle dst_group = this->private_->drop_group_->get_group();
      if ( dst_group )
      {
        if ( this->private_->layer_->get_layer_group() != dst_group )
        {
          LayerHandle last_layer = dst_group->bottom_layer();

          if ( last_layer )
          {
            LayerResamplerHandle layer_resampler( new LayerResampler(
              this->private_->layer_, last_layer ) );
            LayerResamplerDialog* dialog = new LayerResamplerDialog( layer_resampler, this );
            if ( dialog->exec() == QDialog::Accepted )
            {
              layer_resampler->execute( Core::Interface::GetWidgetActionContext() );
            }

            dialog->deleteLater();
          }
        }
        else
        {
          ActionMoveLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
            this->get_layer_id() );
        }
      }
    }
  }

  this->private_->set_picked_up( false );
  this->private_->drop_layer_set_ = false;
  this->private_->drop_group_set_ = false;

  Q_EMIT prep_for_drag_and_drop( false );
  this->parentWidget()->setMinimumHeight( 0 );
  this->repaint();
  event->setAccepted( true );
}

void LayerWidget::set_drop_group( GroupButtonMenu* target_group )
{
  this->private_->drop_group_ = target_group;
  this->private_->drop_group_set_ = true;
}

void LayerWidget::dropEvent( QDropEvent* event )
{
  this->private_->enable_drop_space( false );
  event->setAccepted( true );
  
  LayerWidget* layer_widget = dynamic_cast< LayerWidget* >( event->source() );
  if ( layer_widget != 0 && layer_widget != this )
  {
    layer_widget->private_->set_drop_target( this ); 
    event->setDropAction( Qt::MoveAction );
    return;
  }
  
  event->setDropAction( Qt::IgnoreAction );
}

void LayerWidget::dragEnterEvent( QDragEnterEvent* event )
{
  LayerWidget* layer_widget = dynamic_cast< LayerWidget* >( event->source() );
  if ( layer_widget != 0 && layer_widget != this )
  {
    this->private_->enable_drop_space( true );
  }
  event->setAccepted( true );
}

void LayerWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->private_->enable_drop_space( false );
  event->setAccepted( true );
}
  
void LayerWidget::show_selection_checkbox( bool show )
{
  if( show && (!this->private_->ui_.lock_button_->isChecked()) )
  { 
    this->private_->ui_.checkbox_widget_->show();
  }
  else 
  {
    this->private_->ui_.checkbox_widget_->hide();
  }
}

void LayerWidget::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->accept();
}

void LayerWidget::prep_for_animation( bool move_time )
{
  if( this->private_->picked_up_ )
    return;
  
  //this->setUpdatesEnabled( false );
  
  if( move_time )
  {
    this->private_->ui_.facade_widget_->setMinimumSize( this->private_->ui_.base_->size() );
    this->private_->ui_.facade_widget_->setPixmap( QPixmap::grabWidget( this ) );
    this->private_->ui_.base_->hide();
    this->private_->ui_.facade_widget_->show();
  }
  else
  {
    this->private_->ui_.facade_widget_->hide();
    this->private_->ui_.base_->show();
  }
  
  //this->setUpdatesEnabled( true );
}

void LayerWidget::instant_hide_drop_space()
{
  this->private_->drop_space_->instant_hide();
}

void LayerWidget::set_picked_up_layer_size( int size )
{
  this->private_->picked_up_layer_size_ = size;
}

void LayerWidget::set_selected( bool selected )
{
  this->private_->ui_.selection_checkbox_->setChecked( selected );
}

bool LayerWidget::is_selected() const
{
  return this->private_->ui_.selection_checkbox_->isChecked() &&
    this->private_->ui_.selection_checkbox_->isVisible();
}

void LayerWidget::activate_from_lineedit_focus()
{
  ActionActivateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
    this->private_->layer_->get_layer_id() ) ;
}

void LayerWidget::contextMenuEvent( QContextMenuEvent * event )
{
  std::string data_state;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    data_state = this->private_->layer_->data_state_->get();
  }

  if ( data_state == Layer::PROCESSING_C || data_state == Layer::CREATING_C )
  {
    event->accept();
    return;
  }

  QMenu menu( this );
  QAction* qaction;
  if ( this->private_->layer_->get_type() != Core::VolumeType::LARGE_DATA_E )
  {
    qaction = menu.addAction( tr( "Duplicate Layer" ) );
    QtUtils::QtBridge::Connect( qaction,
    boost::bind( &ActionDuplicateLayer::Dispatch, Core::Interface::GetWidgetActionContext(), 
    this->private_->layer_->get_layer_id() ) );
  }

  if ( data_state != Layer::IN_USE_C  )
  {
    qaction = menu.addAction( tr( "Delete Layer" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( delete_layer_from_context_menu() ) );
  }

  if ( this->private_->layer_->get_type() != Core::VolumeType::LARGE_DATA_E )
  {
    QMenu* export_menu;
    export_menu = new QMenu( this );
    
    export_menu->setTitle( tr( "Export Data As..." ) );
    qaction = export_menu->addAction( tr( "DICOM" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_dicom() ) );

    if ( this->private_->layer_->get_type() == Core::VolumeType::MASK_E )
    {
    export_menu->setTitle( tr( "Export Segmentation As..." ) );
    
    qaction = export_menu->addAction( tr( "BITMAP" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_bitmap() ) );
    }

    qaction = export_menu->addAction( tr( "NRRD" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_nrrd() ) );

    qaction = export_menu->addAction( tr( "MATLAB" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_matlab() ) );

    qaction = export_menu->addAction( tr( "PNG" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_png() ) );

    qaction = export_menu->addAction( tr( "TIFF" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_tiff() ) );

    qaction = export_menu->addAction( tr( "MRC" ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_mrc() ) );

    menu.addMenu( export_menu );
  }

  if ( this->private_->layer_->get_type() == Core::VolumeType::MASK_E )
  {
    QAction *qaction = menu.addAction( tr( "Export Isosurface..." ) );
    connect( qaction, SIGNAL( triggered() ), this, SLOT( export_isosurface() ) );
  }

  menu.exec( event->globalPos() );
}

void LayerWidget::delete_layer_from_context_menu()
{ 
  // Check whether the users wants to save and whether the user wants to quit
  int ret = QMessageBox::warning( this->parentWidget(), "Delete Warning",
    "Are you sure you want to delete this layer?",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No  );

  if( ret == QMessageBox::Yes )
  {
    std::vector< std::string > layer;
    std::string layer_id;
    Core::ImportFromString( this->get_layer_id(), layer_id );
    layer.push_back( layer_id );
    ActionDeleteLayers::Dispatch( Core::Interface::GetWidgetActionContext(), 
      layer );
  }
}

void LayerWidget::export_nrrd()
{
  this->private_->export_layer( ".nrrd" );
}

void LayerWidget::export_matlab()
{
  this->private_->export_layer( ".mat" );
}

void LayerWidget::export_mrc()
{
  this->private_->export_layer( ".mrc" );
}
  
void LayerWidget::export_dicom()
{
  this->private_->export_layer( ".dcm" );
}

void LayerWidget::export_tiff()
{
  this->private_->export_layer( ".tiff" );
}

void LayerWidget::export_bitmap()
{
  this->private_->export_layer( ".bmp" );
}

void LayerWidget::export_png()
{
  this->private_->export_layer( ".png" );
}

void LayerWidget::export_isosurface()
{
  this->private_->export_isosurface();
}

} //end namespace Seg3D
