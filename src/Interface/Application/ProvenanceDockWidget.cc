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
#include <QtGui/QMessageBox>

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

  // -- Internal variables --
public:
  Ui::ProvenanceDockWidget ui_;
  bool resetting_;
  std::vector< std::string > sessions_;
  ProvenanceDockWidget* parent_;  
  boost::signals2::connection active_layer_prov_connection_;
  boost::signals2::connection project_provenance_connection_;
  
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
};

void ProvenanceDockWidgetPrivate::populate_provenance_list( ProvenanceTrailHandle provenance_trail )
{
  // Clean out the old view
  this->ui_.provenance_list_->clear();

  typedef boost::date_time::c_local_adjustor< SessionInfo::timestamp_type > local_time_adjustor;

  size_t num_steps = provenance_trail->size();
  for( size_t i = 0; i < num_steps; ++i )
  {
    ProvenanceStepHandle prov_step = provenance_trail->at( i );
    std::string action_name = prov_step->get_action_name();
    std::string action_params = prov_step->get_action_params();
    ProvenanceIDList poi = prov_step->get_provenance_ids_of_interest();
    std::string user_id = prov_step->get_username();
    // Convert the timestamp from UTC to local time
    ProvenanceStep::timestamp_type prov_local_time = 
      local_time_adjustor::utc_to_local( prov_step->get_timestamp() );

    QStringList columns;
    columns << QString::fromStdString( boost::posix_time::to_simple_string( prov_local_time ) )
      << QString::fromStdString( user_id )
      << QString::fromStdString( action_name );
    QTreeWidgetItem* top_item = new QTreeWidgetItem( columns );
    top_item->setData( 0, Qt::UserRole, QVariant( QString::fromStdString( Core::ExportToString( poi ) ) ) );
    top_item->setData( 0, Qt::UserRole + 1, QVariant( QString::fromStdString( action_params ) ) );

    if ( action_name == "Paint" )
    {
      size_t start_step = i;
      while ( i < num_steps )
      {
        prov_step = provenance_trail->at( i );
        if ( prov_step->get_action_name() != "Paint" || 
          prov_step->get_username() != user_id )
        {
          break;
        }

        action_params = prov_step->get_action_params();
        poi = prov_step->get_provenance_ids_of_interest();
        prov_local_time = local_time_adjustor::utc_to_local( prov_step->get_timestamp() );

        QTreeWidgetItem* item = new QTreeWidgetItem( top_item );
        item->setText( 0, QString::fromStdString( boost::posix_time::to_simple_string( prov_local_time ) ) );
        item->setText( 1, QString::fromStdString( user_id ) );
        item->setText( 2, "Paint" );
        item->setData( 0, Qt::UserRole, QVariant( QString::fromStdString( Core::ExportToString( poi ) ) ) );
        item->setData( 0, Qt::UserRole + 1, QVariant( QString::fromStdString( action_params ) ) );

        ++i;
      }

      --i;
      if ( start_step == i )
      {
        this->ui_.provenance_list_->addTopLevelItem( top_item->takeChild( 0 ) );
        delete top_item;
      }
      else
      {
        this->ui_.provenance_list_->addTopLevelItem( top_item );
        QFont font = top_item->font( 0 );
        font.setBold( true );
        top_item->setFont( 0, font );
        top_item->setFont( 1, font );
        top_item->setFont( 2, font );
        top_item->setText( 0, QString::fromStdString( boost::posix_time::to_simple_string( prov_local_time ) ) );
        top_item->setData( 0, Qt::UserRole, QVariant( QString::fromStdString( Core::ExportToString( poi ) ) ) );
        top_item->setData( 0, Qt::UserRole + 1, QVariant( QString( "Various..." ) ) );
      }
    }
    else
    {
      this->ui_.provenance_list_->addTopLevelItem( top_item );
    }
  }

  this->ui_.provenance_list_->setEnabled( true );
  this->ui_.step_detail_groupbox_->setEnabled( true );
  this->ui_.refresh_button_->setEnabled( false );
  this->parent_->update_current_provenance_step();
  this->ui_.provenance_list_->repaint();
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
    current_project->request_provenance_trail( active_layer->provenance_id_state_->get() );
  }
} 

void ProvenanceDockWidgetPrivate::clear_provenance_list()
{
  this->ui_.provenance_list_->clear();
  this->parent_->update_current_provenance_step();
  this->ui_.refresh_button_->setEnabled( false );
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

  // If the new active layer exists, request the provenance trail of it
  if ( active_layer )
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    qpointer->active_layer_prov_connection_ = active_layer->provenance_id_state_->state_changed_signal_.
      connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleActiveLayerProvenanceChanged, qpointer ) );
    // Request the provenance trail only if there is a project connected
    if ( qpointer->project_provenance_connection_.connected() )
    {
      ProjectManager::Instance()->get_current_project()->request_provenance_trail(
        active_layer->provenance_id_state_->get() );
    }
  }
  // Otherwise clear the tree widget
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
  qpointer->ui_.provenance_list_->setEnabled( false );
  qpointer->ui_.replay_button_->setEnabled( false );
  qpointer->ui_.step_detail_groupbox_->setEnabled( false );
  qpointer->ui_.refresh_button_->setEnabled( true );
}

//////////////////////////////////////////////////////////////////////////
// Class ProvenanceDockWidget
//////////////////////////////////////////////////////////////////////////

ProvenanceDockWidget::ProvenanceDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent )
{
  this->private_ = new ProvenanceDockWidgetPrivate( this );

  this->private_->ui_.setupUi( this );
  this->private_->ui_.provenance_list_->header()->resizeSection( 0, 140 );
  this->private_->ui_.provenance_list_->header()->resizeSection( 1, 45 );
  this->private_->ui_.header_bkg_->setStyleSheet( StyleSheet::PROVENANCE_LIST_HEADER_C );

  ProvenanceDockWidgetPrivate::qpointer_type qpointer( this->private_ );
  this->private_->add_connection( ProjectManager::Instance()->current_project_changed_signal_.
    connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleProjectChanged, qpointer ) ) );
  this->private_->add_connection( LayerManager::Instance()->active_layer_changed_signal_.
    connect( boost::bind( &ProvenanceDockWidgetPrivate::HandleActiveLayerChanged, qpointer, _1 ) ) );

  this->private_->connect_project();

  this->connect( this->private_->ui_.provenance_list_, SIGNAL( itemSelectionChanged () ), 
    SLOT( update_current_provenance_step() ) );
  this->connect( this->private_->ui_.replay_button_, SIGNAL( clicked ( bool ) ),
    SLOT( dispatch_recreate_provenance() ) );
  this->connect( this->private_->ui_.refresh_button_, SIGNAL( clicked( bool ) ),
    SLOT( refresh_provenance_trail() ) );
}

ProvenanceDockWidget::~ProvenanceDockWidget()
{
}

void ProvenanceDockWidget::update_current_provenance_step()
{
  QTreeWidgetItem* current_item = this->private_->ui_.provenance_list_->currentItem();
  QString timestamp_str, user_id, action_name, action_params, poi_text;
  if ( current_item != 0 )
  {
    timestamp_str = current_item->text( 0 );
    user_id = current_item->text( 1 );
    action_name = current_item->text( 2 );
    poi_text = current_item->data( 0, Qt::UserRole ).toString();
    action_params = current_item->data( 0, Qt::UserRole + 1 ).toString();
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
    this->private_->ui_.replay_button_->setEnabled( true );
  }
  else
  {
    this->private_->ui_.replay_button_->setEnabled( false );
  }
}

void ProvenanceDockWidget::dispatch_recreate_provenance()
{
  QTreeWidgetItem* current_item = this->private_->ui_.provenance_list_->currentItem();
  if ( current_item != 0 )
  {
    std::string poi_text = current_item->data( 0, Qt::UserRole ).toString().toStdString();
    ProvenanceIDList poi;
    if ( !poi_text.empty() &&
      Core::ImportFromString( poi_text, poi ) &&
      poi.size() > 0 )
    {
      ActionRecreateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), poi );
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
