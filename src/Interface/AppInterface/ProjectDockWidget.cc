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

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Application includes
#include <Application/ProjectManager/ProjectManager.h>
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
    
    
    connect( this->private_->ui_.save_session_button_, SIGNAL( clicked() ), 
      this, SLOT( save_project() ) );
    
    connect( this->private_->ui_.load_session_button_, SIGNAL( clicked() ), 
      this, SLOT( load_session() ) );
    
    connect( this->private_->ui_.delete_session_button_, SIGNAL( clicked() ),
      this, SLOT( delete_session() ) );

    connect( this->private_->ui_.sessions_list_, SIGNAL( itemDoubleClicked ( QListWidgetItem* ) ),
      this, SLOT( call_load_session( QListWidgetItem* ) ) );

    connect( this->private_->ui_.save_note_button_, SIGNAL( clicked() ), 
      this, SLOT( save_note() ) );

    connect( this->private_->ui_.note_edit_, SIGNAL( cursorPositionChanged() ),
      this, SLOT( enable_save_notes_button() ) );


  }
}

ProjectDockWidget::~ProjectDockWidget()
{
  this->disconnect_all();
}
  
  
void ProjectDockWidget::save_project()
{
  ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false );
}

void ProjectDockWidget::save_note()
{
  ProjectManager::Instance()->save_note( this->private_->ui_.note_edit_->toPlainText().toStdString() );
  this->private_->ui_.note_edit_->setPlainText( QString::fromUtf8( "" ) );

}
  
void ProjectDockWidget::load_session()
{
  if( !this->private_->ui_.sessions_list_->currentItem() )
    return;

  if( this->private_->ui_.sessions_list_->currentItem()->text() != "" )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "WARNING: By loading a saved session you will loose"
      " any unsaved changes.") );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( message_box.exec() )
    {
      std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
        sessions_state_->get();
      
      for( int i = 0; i < static_cast< int >( sessions.size() ); ++i )
      {
        if( QString::fromStdString( ( Core::SplitString( sessions[ i ], "|" ) )[ 1 ] ) == 
           this->private_->ui_.sessions_list_->currentItem()->text() )
        {
          ActionLoadSession::Dispatch( Core::Interface::GetWidgetActionContext(), i );
          break;
        }
      }     
    }
  }
}

void ProjectDockWidget::delete_session()
{ 
  if( !this->private_->ui_.sessions_list_->currentItem() )
    return;

  if( this->private_->ui_.sessions_list_->currentItem()->text() != "" )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "WARNING: You are going to regret this.") );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( message_box.exec() )
    {
      std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
        sessions_state_->get();

      for( int i = 0; i < static_cast< int >( sessions.size() ); ++i )
      {
        if( QString::fromStdString( ( Core::SplitString( sessions[ i ], "|" ) )[ 1 ] ) == 
          this->private_->ui_.sessions_list_->currentItem()->text() )
        {
          ActionDeleteSession::Dispatch( Core::Interface::GetWidgetActionContext(), i );  
          break;
        }
      }
    }
  }

  this->populate_session_list();
}

void ProjectDockWidget::populate_session_list()
{
  std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
    sessions_state_->get();
  
  this->private_->ui_.sessions_list_->clear();
  
  for( int i = 0; i < static_cast< int >( sessions.size() ); ++i )
  {
    if( ( sessions[ i ] != "" ) )// && ( sessions[ i ] != "]" ) )
    {
      this->private_->ui_.sessions_list_->addItem( QString::fromStdString( 
        ( Core::SplitString( sessions[ i ], "|" ) )[ 1 ] ) );
    }
  }
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

void ProjectDockWidget::call_load_session( QListWidgetItem* item )
{
  this->load_session();
}




} // end namespace Seg3D
