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
#include <time.h>

// Boost includes
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// QT includes
#include <QtCore/QPointer>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/Utils/ConnectionHandler.h>

// Qt includes
#include <QtGui/QStandardItemModel>
#include <QMessageBox>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>

//Application includes
#include <Application/InterfaceManager/InterfaceManager.h>
#include <Application/Layer/Actions/ActionRecreateLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Project/Project.h>
#include <Application/Provenance/Provenance.h>

// Interface includes
#include <Interface/Application/ProvenanceDockWidget.h>
#include <Interface/Application/ProvenanceTreeModel.h>
#include <Interface/Application/StyleSheet.h>
#include "ui_ProvenanceDockWidget.h"

namespace Seg3D
{

class ProvenanceDockWidgetPrivate : public QObject, public Core::ConnectionHandler
{
public:
  ProvenanceDockWidgetPrivate( ProvenanceDockWidget* parent ) :
    QObject( parent ),
    parent_( parent )
  {
  }

  ~ProvenanceDockWidgetPrivate()
  {
    this->disconnect_all();
    this->active_layer_prov_connection_.disconnect();
    this->project_provenance_connection_.disconnect();
  }

  // POPULATE_PROVENANCE_LIST:
  // this handles the actual updating of the ui
  void populate_provenance_list( ProvenanceTrailHandle provenance_trail );

  // CONNECT_PROJECT:
  // this function does the actual reconnecting
  void connect_project();

  // CLEAR_PROVENANCE_LIST:
  // Clear the provenance tree widget.
  void clear_provenance_list();

  // SET_PROVENANCE_DIRTY:
  // Set the current provenance trail to be out-of-date.
  void set_provenance_dirty( bool dirty );

  // -- Internal variables --
public:
  Ui::ProvenanceDockWidget ui_;
  ProvenanceDockWidget* parent_;
  ProvenanceTreeModel* prov_tree_model_;
  boost::signals2::connection active_layer_prov_connection_;
  boost::signals2::connection project_provenance_connection_;

  // Cached provenance trail
  ProvenanceTrailHandle prov_trail_;
  // Whether the sandbox for provenance replay is available
  bool sandbox_available_;
  // Whether the provenance trail needs update
  bool provenance_dirty_;
  
  // -- Static signal handling functions --
public:
  typedef QPointer< ProvenanceDockWidgetPrivate > qpointer_type;

  // HANDLEPROVENANCERESULT:
  // A function that handles the signal that contains the provenance of a particular layer.
  static void HandleProvenanceResult( qpointer_type qpointer, ProvenanceTrailHandle provenance_trail );

  // HANDLEPROJECTCHANGED:
  // A function that handles reconnecting the provenance dock widget to the current project when it has changed
  static void HandleProjectChanged( qpointer_type qpointer );

  // HANDLEACTIVELAYERCHANGED:
  // Called when the active layer has changed.
  static void HandleActiveLayerChanged( qpointer_type qpointer, LayerHandle active_layer );

  // HANDLEPROJECTDATACHANGED:
  // Called when the data of the current project has changed.
  static void HandleActiveLayerProvenanceChanged( qpointer_type qpointer );

  // HANDLELAYERSDELETED:
  // Called when layers have been deleted.
  static void HandleLayersDeleted( qpointer_type qpointer );

  // HANDLESANDBOXCREATED:
  // Called when a sandbox has been created.
  static void HandleSandboxCreated( qpointer_type qpointer, SandboxID sandbox );

  // HANDLESANDBOXDELETED:
  // Called when a sandbox has been deleted.
  static void HandleSandboxDeleted( qpointer_type qpointer, SandboxID sandbox );
};

void ProvenanceDockWidgetPrivate::populate_provenance_list( ProvenanceTrailHandle provenance_trail )
{
  this->prov_trail_ = provenance_trail;
  this->prov_tree_model_->set_provenance_trail( provenance_trail );
  this->set_provenance_dirty( false );

  this->parent_->handle_current_step_changed( QModelIndex() );
}

void ProvenanceDockWidgetPrivate::connect_project()
{
  // Disconnect the connection to previous project and active layer
  this->active_layer_prov_connection_.disconnect();
  this->project_provenance_connection_.disconnect();
  this->clear_provenance_list();

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
  if( !current_project ) return;

  // NOTE: No need to store the connection as there is only one project at any time
  this->project_provenance_connection_ = current_project->provenance_trail_signal_.connect( 
    boost::bind( &ProvenanceDockWidgetPrivate::HandleProvenanceResult, qpointer_type( this ), _1 ) );

  // Connect to the active layer, and request the provenance trail from the new project
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( active_layer )
  {
    this->active_layer_prov_connection_ = active_layer->provenance_id_state_->state_changed_signal_.
      connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleActiveLayerProvenanceChanged,
      qpointer_type( this ) ) );
    this->set_provenance_dirty( true );
  }
} 

void ProvenanceDockWidgetPrivate::clear_provenance_list()
{
  this->prov_tree_model_->set_provenance_trail( ProvenanceTrailHandle() );
  this->parent_->handle_current_step_changed( QModelIndex() );
  this->set_provenance_dirty( false );
}

void ProvenanceDockWidgetPrivate::HandleProvenanceResult( qpointer_type qpointer, 
                          ProvenanceTrailHandle provenance_trail )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &ProvenanceDockWidgetPrivate::populate_provenance_list, qpointer.data(), provenance_trail ) ) );
}

void ProvenanceDockWidgetPrivate::HandleProjectChanged( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &ProvenanceDockWidgetPrivate::connect_project, qpointer.data() ) ) ); 
}

void ProvenanceDockWidgetPrivate::HandleActiveLayerChanged( qpointer_type qpointer, 
                               LayerHandle active_layer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
      &ProvenanceDockWidgetPrivate::HandleActiveLayerChanged, qpointer, active_layer ) ) );
    return;
  }

  // Disconnect the connection to previous active layer
  qpointer->active_layer_prov_connection_.disconnect();

  // If the new active layer exists, connect to it
  if ( active_layer )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    qpointer->active_layer_prov_connection_ = active_layer->provenance_id_state_->state_changed_signal_.
      connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleActiveLayerProvenanceChanged, qpointer ) );
    // Mark the provenance as dirty
    qpointer->set_provenance_dirty( true );
  }
  else
  {
    qpointer->clear_provenance_list();
  }
}

void ProvenanceDockWidgetPrivate::HandleActiveLayerProvenanceChanged( qpointer_type qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
      &ProvenanceDockWidgetPrivate::HandleActiveLayerProvenanceChanged, qpointer ) ) );
    return;
  }
  
  // Grey out the provenance list and enable the refresh button
  qpointer->set_provenance_dirty( true );
}

void ProvenanceDockWidgetPrivate::set_provenance_dirty( bool dirty )
{
  this->provenance_dirty_ = dirty;
  this->ui_.provenance_list_->setEnabled( !dirty );
  this->ui_.step_detail_groupbox_->setEnabled( !dirty );
  this->ui_.refresh_button_->setEnabled( dirty );

  if ( dirty )
  {
    this->ui_.replay_button_->setEnabled( false );
    this->prov_trail_.reset();
  }
}

void ProvenanceDockWidgetPrivate::HandleLayersDeleted( qpointer_type qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
      &ProvenanceDockWidgetPrivate::HandleLayersDeleted, qpointer ) ) );
    return;
  }

  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( !active_layer )
  {
    qpointer->clear_provenance_list();
  }
}

void ProvenanceDockWidgetPrivate::HandleSandboxCreated( qpointer_type qpointer, SandboxID sandbox )
{
  // Ignore any sandbox other than 0
  if ( sandbox != 0 ) return;

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
      &ProvenanceDockWidgetPrivate::HandleSandboxCreated, qpointer, sandbox ) ) );
    return;
  }
  
  qpointer->sandbox_available_ = false;
  qpointer->parent_->handle_current_step_changed( qpointer->ui_.provenance_list_->currentIndex() );
}

void ProvenanceDockWidgetPrivate::HandleSandboxDeleted( qpointer_type qpointer, SandboxID sandbox )
{
  // Ignore any sandbox other than 0
  if ( sandbox != 0 ) return;

  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
      &ProvenanceDockWidgetPrivate::HandleSandboxDeleted, qpointer, sandbox ) ) );
    return;
  }

  qpointer->sandbox_available_ = true;
  qpointer->parent_->handle_current_step_changed( qpointer->ui_.provenance_list_->currentIndex() );
}

//////////////////////////////////////////////////////////////////////////
// Class ProvenanceDockWidget
//////////////////////////////////////////////////////////////////////////

ProvenanceDockWidget::ProvenanceDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent )
{
  this->private_ = new ProvenanceDockWidgetPrivate( this );
  this->private_->prov_tree_model_ = new ProvenanceTreeModel( this );
  this->private_->sandbox_available_ = true;
  this->private_->provenance_dirty_ = false;

  this->private_->ui_.setupUi( this );
  this->private_->ui_.provenance_list_->setModel( this->private_->prov_tree_model_ );
  this->private_->ui_.provenance_list_->header()->resizeSection( 0, 120 );
  this->private_->ui_.provenance_list_->header()->resizeSection( 1, 50 );
  this->private_->ui_.provenance_list_->header()->setDefaultAlignment( Qt::AlignHCenter );
  this->private_->ui_.active_layer_prov_widget_->setStyleSheet( StyleSheet::PROVENANCE_LIST_HEADER_C );

  ProvenanceDockWidgetPrivate::qpointer_type qpointer( this->private_ );
  this->private_->add_connection( ProjectManager::Instance()->current_project_changed_signal_.
    connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleProjectChanged, qpointer ) ) );
  this->private_->add_connection( LayerManager::Instance()->active_layer_changed_signal_.
    connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleActiveLayerChanged, qpointer, _1 ) ) );
  this->private_->add_connection( LayerManager::Instance()->layers_deleted_signal_.connect(
    boost::bind( &ProvenanceDockWidgetPrivate::HandleLayersDeleted, qpointer ) ) );
  this->private_->add_connection( LayerManager::Instance()->sandbox_created_signal_.connect(
    boost::bind( &ProvenanceDockWidgetPrivate::HandleSandboxCreated, qpointer, _1 ) ) );
  this->private_->add_connection( LayerManager::Instance()->sandbox_deleted_signal_.connect(
    boost::bind( &ProvenanceDockWidgetPrivate::HandleSandboxDeleted, qpointer, _1 ) ) );

  this->private_->connect_project();

  this->connect( this->private_->ui_.provenance_list_, 
    SIGNAL( current_item_changed( const QModelIndex& ) ), 
    SLOT( handle_current_step_changed( const QModelIndex& ) ) );
  this->connect( this->private_->ui_.replay_button_, SIGNAL( clicked ( bool ) ),
    SLOT( dispatch_recreate_provenance() ) );
  this->connect( this->private_->ui_.refresh_button_, SIGNAL( clicked( bool ) ),
    SLOT( refresh_provenance_trail() ) );
}

ProvenanceDockWidget::~ProvenanceDockWidget()
{
}

void ProvenanceDockWidget::handle_current_step_changed( const QModelIndex& index )
{
  QString timestamp_str, user_id, action_name, action_params, poi_text;
  if ( index.isValid() )
  {
    timestamp_str = this->private_->prov_tree_model_->data( 
      index, ProvenanceTreeModel::TIMESTAMP_E ).toString();
    user_id = this->private_->prov_tree_model_->data( 
      index, ProvenanceTreeModel::USER_ID_E ).toString();
    action_name = this->private_->prov_tree_model_->data( 
      index, ProvenanceTreeModel::ACTION_NAME_E ).toString();
    poi_text = this->private_->prov_tree_model_->data( 
      index, ProvenanceTreeModel::PID_OF_INTEREST_E ).toString();
    action_params = this->private_->prov_tree_model_->data( 
      index, ProvenanceTreeModel::ACTION_PARAMS_E ).toString();
  }

  this->private_->ui_.timestamp_label_->setText( timestamp_str );
  this->private_->ui_.user_label_->setText( user_id );
  this->private_->ui_.action_label_->setText( action_name );
  this->private_->ui_.prov_ids_label_->setText( poi_text );
  this->private_->ui_.action_params_->setText( action_params );

  ProvenanceIDList poi;
  if ( poi_text.size() > 0 &&
    Core::ImportFromString( poi_text.toStdString(), poi ) &&
    poi.size() > 0 )
  {
    this->private_->ui_.replay_button_->setEnabled( 
      this->private_->sandbox_available_ && !this->private_->provenance_dirty_ );
  }
  else
  {
    this->private_->ui_.replay_button_->setEnabled( false );
  }
}

void ProvenanceDockWidget::dispatch_recreate_provenance()
{
  QModelIndex index = this->private_->ui_.provenance_list_->currentIndex();
  if ( this->private_->prov_trail_ && index.isValid() )
  {
    std::string poi_text = this->private_->prov_tree_model_->data( 
      index, ProvenanceTreeModel::PID_OF_INTEREST_E ).toString().toStdString();
    size_t prov_step_index = this->private_->prov_tree_model_->data(
      index, ProvenanceTreeModel::PROV_STEP_INDEX_E ).value< size_t >();
    ProvenanceIDList poi;
    if ( !poi_text.empty() &&
      Core::ImportFromString( poi_text, poi ) &&
      poi.size() > 0 )
    {
      if ( prov_step_index < this->private_->prov_trail_->size() )
      {
        ProvenanceTrailHandle sub_trail( new ProvenanceTrail );
        sub_trail->insert( sub_trail->end(), this->private_->prov_trail_->begin(),
          this->private_->prov_trail_->begin() + prov_step_index + 1 );
        ActionRecreateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), poi, sub_trail );
      }
      else
      {
        ActionRecreateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), poi );
      }
    }
  }
}

void ProvenanceDockWidget::refresh_provenance_trail()
{
  this->private_->ui_.refresh_button_->setEnabled( false );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( active_layer )
  {
    ProjectManager::Instance()->get_current_project()->request_provenance_trail(
      active_layer->provenance_id_state_->get() );
  }
}

} // end namespace Seg3D
