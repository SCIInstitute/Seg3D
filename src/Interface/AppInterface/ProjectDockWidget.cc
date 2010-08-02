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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionLoadSession.h>
#include <Application/ProjectManager/Actions/ActionDeleteSession.h>

// Interface includes
#include <Interface/AppInterface/ProjectDockWidget.h>
#include "ui_ProjectDockWidget.h"

namespace Seg3D
{

class ProjectDockWidgetPrivate
{
public:

  Ui::ProjectDockWidget ui_;
  std::vector< std::string > notes;

  QStandardItemModel* note_model_;

};

ProjectDockWidget::ProjectDockWidget( QWidget *parent ) :
  QDockWidget( parent ), 
  private_( new ProjectDockWidgetPrivate )
{
  if( this->private_ )
  {
    qpointer_type project_dock_widget( this );
    
    this->private_->ui_.setupUi( this );

    // We dont want them to be able to save blank notes.
    this->private_->ui_.save_note_button_->setEnabled( false );

    int minutes = PreferencesManager::Instance()->auto_save_time_state_->get();
    this->private_->ui_.minutes_label_->setText( QString::number( minutes ) );

    QtUtils::QtBridge::Connect( this->private_->ui_.autosave_checkbox_,
      PreferencesManager::Instance()->auto_save_state_ );

    QtUtils::QtBridge::Connect( this->private_->ui_.project_name_edit_, 
      ProjectManager::Instance()->current_project_->project_name_state_ );
    
    QtUtils::QtBridge::Connect( this->private_->ui_.custom_colors_checkbox_, 
      ProjectManager::Instance()->current_project_->save_custom_colors_state_ );
    
    add_connection( ProjectManager::Instance()->current_project_->sessions_state_->
      state_changed_signal_.connect( boost::bind( &ProjectDockWidget::HandleSessionsChanged, 
      project_dock_widget ) ) );

    add_connection( ProjectManager::Instance()->current_project_->project_notes_state_->
      state_changed_signal_.connect( boost::bind( &ProjectDockWidget::HandleNoteSaved, 
      project_dock_widget ) ) );

    add_connection( PreferencesManager::Instance()->auto_save_time_state_->
      value_changed_signal_.connect( boost::bind( 
      &ProjectDockWidget::HandleAutoSaveTimeChanged, project_dock_widget, _1 ) ) );

    add_connection( PreferencesManager::Instance()->smart_save_state_->
      state_changed_signal_.connect( boost::bind( 
      &ProjectDockWidget::HandleSmartAutoSaveToggled, project_dock_widget ) ) );
    
    connect( this->private_->ui_.save_session_button_, SIGNAL( clicked() ), 
      this, SLOT( save_session() ) );
    
    connect( this->private_->ui_.load_session_button_, SIGNAL( clicked() ), 
      this, SLOT( load_session() ) );
    
    connect( this->private_->ui_.delete_session_button_, SIGNAL( clicked() ),
      this, SLOT( delete_session() ) );

    connect( this->private_->ui_.sessions_list_, SIGNAL( cellDoubleClicked ( int, int ) ),
      this, SLOT( call_load_session( int, int ) ) );

    connect( this->private_->ui_.save_note_button_, SIGNAL( clicked() ), 
      this, SLOT( save_note() ) );

    connect( this->private_->ui_.note_edit_, SIGNAL( cursorPositionChanged() ),
      this, SLOT( enable_save_notes_button() ) );

    QStringList headers;
    headers << "Time:" << "Session Name:";
    this->private_->ui_.sessions_list_->setHorizontalHeaderLabels( headers );
  }
}

ProjectDockWidget::~ProjectDockWidget()
{
  this->disconnect_all();
}
  
  
void ProjectDockWidget::save_session()
{
  std::string session_name = this->private_->ui_.session_name_linedit_->text().toStdString();
  ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false, session_name );
  this->private_->ui_.session_name_linedit_->setText( "" );
}

void ProjectDockWidget::save_note()
{
  ProjectManager::Instance()->save_note( this->private_->ui_.note_edit_->toPlainText().toStdString() );
  this->private_->ui_.note_edit_->setPlainText( QString::fromUtf8( "" ) );

}
  
void ProjectDockWidget::load_session()
{
  int row = this->private_->ui_.sessions_list_->currentRow();

  if( ( row < 0 ) || ( row > this->private_->ui_.sessions_list_->rowCount() ) )
  {
    return;
  }

  if( !this->private_->ui_.sessions_list_->item( row, 1 ) )
  {
    return;
  }

  std::string row_time = this->private_->ui_.sessions_list_->item( row, 0 )->text().toStdString();

  if( row_time != "" )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "WARNING: By loading a saved session you will loose"
      " any unsaved changes.") );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( QMessageBox::Yes == message_box.exec() )
    {
      std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
        sessions_state_->get();

      std::string session_name = sessions[ row ];
      ActionLoadSession::Dispatch( Core::Interface::GetWidgetActionContext(), session_name );
    }
  }
}

void ProjectDockWidget::delete_session()
{ 
  int row = this->private_->ui_.sessions_list_->currentRow();

  if( ( row < 0 ) || ( row > this->private_->ui_.sessions_list_->rowCount() ) )
  {
    return;
  }

  if( !this->private_->ui_.sessions_list_->item( row , 1 ) )
  {
    return;
  }

  std::string row_time= this->private_->ui_.sessions_list_->item( row, 0 )->text().toStdString();

  if( row_time != "" )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "WARNING: You are going to regret this.") );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( QMessageBox::Yes == message_box.exec() )
    {
      std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
        sessions_state_->get();

      std::string session_name = sessions[ row ];
      ActionDeleteSession::Dispatch( Core::Interface::GetWidgetActionContext(), session_name );
    }
  }

  this->populate_session_list();
}

void ProjectDockWidget::populate_session_list()
{
  std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
    sessions_state_->get();

  this->private_->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( false );

  // Clean out the old table
  for( int j = this->private_->ui_.sessions_list_->rowCount() -1; j >= 0; j-- )
  {
    this->private_->ui_.sessions_list_->removeRow( j );
  }

  for( int i = 0; i < static_cast< int >( sessions.size() ); ++i )
  {
    if( ( sessions[ i ] != "" ) && ( sessions[ i ] != "]" ) )
    {
      std::string session_name = sessions[ i ];
      QTableWidgetItem *new_item_name;
      QTableWidgetItem *new_item_time;

      new_item_time = new QTableWidgetItem( QString::fromStdString( 
        ( Core::SplitString( session_name, " - " ) )[ 0 ] ) );
      new_item_name = new QTableWidgetItem( QString::fromStdString( 
        ( Core::SplitString( session_name, " - " ) )[ 1 ] ) );

      if( ( Core::SplitString( session_name, " - " ) )[ 1 ]== "AutoSave" )
      {
        QFont font;
        font.setBold( true );
        new_item_name->setFont( font );
        new_item_time->setFont( font );
      }

      this->private_->ui_.sessions_list_->insertRow( i );

      std::vector< std::string > time_vector = Core::SplitString( 
        new_item_time->text().toStdString(), "-" );
      if( time_vector.size() == 6 )
      {
        std::string date = time_vector[ 0 ] + "-" + time_vector[ 1 ] + "-" + time_vector[ 2 ];
        if( date == this->get_date() )
        {
          new_item_time->setText( QString::fromStdString( time_vector[ 3 ] + ":" + 
            time_vector[ 4 ] + ":" + time_vector[ 5 ] ) );
        }
        else
        {
          new_item_time->setText( QString::fromStdString( date ) );
        }
      }

      // Add a tooltip to display the user who saved the session 
      new_item_time->setToolTip( QString::fromUtf8( "This session was saved by: " )
        + QString::fromStdString( ( Core::SplitString( session_name, " - " ) )[ 2 ] ) );
      new_item_name->setToolTip( QString::fromUtf8( "This session was saved by: " )
        + QString::fromStdString( ( Core::SplitString( session_name, " - " ) )[ 2 ] ) );
    
      std::string test = new_item_time->text().toStdString();
      this->private_->ui_.sessions_list_->setItem( i, 0, new_item_time );
      this->private_->ui_.sessions_list_->setItem( i, 1, new_item_name );
      this->private_->ui_.sessions_list_->verticalHeader()->resizeSection( i, 24 );
    }
  }

  this->private_->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( true );
}
    

void ProjectDockWidget::populate_notes_list()
{
  std::vector< std::string > notes = ProjectManager::Instance()->current_project_->
    project_notes_state_->get();
  
  // Clear out the treewidget
  this->private_->ui_.notes_tree_->clear();

  int num_of_notes = static_cast< int >( notes.size() );

  if( num_of_notes < 1 )
  {
    return;
  }


  for( int i = num_of_notes - 1; i >= 0; i-- )
  {
    if( ( notes[ i ] == "]" ) || ( notes[ i ] == "[" ) || ( notes[ i ] == "" ) )
    {
      continue;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem( this->private_->ui_.notes_tree_ );
    item->setText( 0, QString::fromStdString( ( Core::SplitString( notes[ i ], "|" ) )[ 0 ] ) );
    QTreeWidgetItem* note = new QTreeWidgetItem();

    std::string test = ( Core::SplitString( notes[ i ], "|" ) )[ 1 ];
    QString note_body = QString::fromStdString( test );
    QLabel* note_text = new QLabel( note_body );

    note_text->setWordWrap( true );
    item->addChild( note );

    this->private_->ui_.notes_tree_->addTopLevelItem( item );
    this->private_->ui_.notes_tree_->setItemWidget( note, 0, note_text );
    
  }

  this->private_->ui_.notes_tree_->expandItem( this->private_->ui_.notes_tree_->topLevelItem( 0 ) );
}
  
void ProjectDockWidget::HandleSessionsChanged( qpointer_type qpointer )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleSessionsChanged,  qpointer ) );
    return;
  }
  
  CORE_LOG_DEBUG( "HandleSessionsChanged started" );
  if( qpointer.data() ) qpointer->populate_session_list();
  CORE_LOG_DEBUG( "HandleSessionsChanged done" );
}

void ProjectDockWidget::HandleNoteSaved( qpointer_type qpointer )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleNoteSaved, qpointer ) );
    return;
  }

  CORE_LOG_DEBUG( "HandleNoteSaved started" );
  if( qpointer.data() ) qpointer->populate_notes_list();
  CORE_LOG_DEBUG( "HandleNoteSaved done" );
}

void ProjectDockWidget::enable_save_notes_button()
{
std::string current_text = this->private_->ui_.note_edit_->toPlainText().toStdString();

  if( current_text.size() > 3 )
  {
    this->private_->ui_.save_note_button_->setEnabled( true );
  }
  else
  {
    this->private_->ui_.save_note_button_->setEnabled( false );
  }
}

void ProjectDockWidget::call_load_session( int row, int column )
{
  this->load_session();
}

void ProjectDockWidget::HandleAutoSaveTimeChanged( qpointer_type qpointer, int duration )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleAutoSaveTimeChanged, qpointer, duration ) );
    return;
  }

  if( qpointer.data() ) qpointer->set_auto_save_label( duration );

}

void ProjectDockWidget::HandleSmartAutoSaveToggled( qpointer_type qpointer )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleSmartAutoSaveToggled, qpointer ) );
    return;
  }
  if( qpointer.data() ) qpointer->set_smart_save_label();
}

void ProjectDockWidget::set_auto_save_label( int duration )
{
  this->private_->ui_.minutes_label_->setText( QString::number( duration ) );
}

void ProjectDockWidget::set_smart_save_label()
{
  if( PreferencesManager::Instance()->smart_save_state_->get() == true )
  {
    this->private_->ui_.smart_autosave_status_label_->setText( 
      QString::fromUtf8( "ACTIVE" ) );
  }
  else
  {
    this->private_->ui_.smart_autosave_status_label_->setText( 
      QString::fromUtf8( "INACTIVE" ) );
  }

}

std::string ProjectDockWidget::get_date()
{
  time_t rawtime;
  struct tm * timeinfo;
  char time_buffer [80];

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  strftime ( time_buffer, 80, "%d-%b-%Y", timeinfo );
  std::string date = time_buffer;
  return date;
}

} // end namespace Seg3D
