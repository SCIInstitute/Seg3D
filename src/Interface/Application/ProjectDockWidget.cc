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
#include <Core/State/Actions/ActionSet.h>

// Qt includes
#include <QtGui/QStandardItemModel>
#include <QtGui/QMessageBox>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>

//Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionLoadSession.h>
#include <Application/ProjectManager/Actions/ActionDeleteSession.h>
#include <Application/ProjectManager/Actions/ActionExportProject.h>

// Interface includes
#include <Interface/Application/ProjectDockWidget.h>
#include <Interface/Application/ProjectExportWizard.h>
#include "ui_ProjectDockWidget.h"

namespace Seg3D
{

class ProjectDockWidgetPrivate
{
public:

  Ui::ProjectDockWidget ui_;
  bool resetting_;
  QStandardItemModel* note_model_;

};

ProjectDockWidget::ProjectDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent ), 
  private_( new ProjectDockWidgetPrivate )
{
  if( this->private_ )
  {
    qpointer_type project_dock_widget( this );
    
    this->private_->ui_.setupUi( this );
    this->private_->resetting_ = false;
    
    // We dont want them to be able to save blank notes.
    this->private_->ui_.save_note_button_->setEnabled( false );

    int minutes = PreferencesManager::Instance()->auto_save_time_state_->get();
    this->private_->ui_.minutes_label_->setText( QString::number( minutes ) );
  
    this->private_->ui_.autosave_checkbox_->setChecked( 
      PreferencesManager::Instance()->auto_save_state_->get() );
    
    connect( this->private_->ui_.autosave_checkbox_, SIGNAL( clicked( bool ) ), this,
      SLOT( set_autosave_checked_state( bool ) ) );
    
    add_connection( PreferencesManager::Instance()->auto_save_state_->
       value_changed_signal_.connect( boost::bind( 
      &ProjectDockWidget::HandleAutosaveStateChanged, project_dock_widget, _1 ) ) );

    QtUtils::QtBridge::Connect( this->private_->ui_.project_name_, 
      ProjectManager::Instance()->current_project_->project_name_state_ );
    
    QtUtils::QtBridge::Connect( this->private_->ui_.custom_colors_checkbox_, 
      ProjectManager::Instance()->current_project_->save_custom_colors_state_ );

    QtUtils::QtBridge::Connect( this->private_->ui_.session_name_linedit_, 
      ProjectManager::Instance()->current_project_->current_session_name_state_ );

    add_connection( ProjectManager::Instance()->current_project_->sessions_state_->
      state_changed_signal_.connect( boost::bind( &ProjectDockWidget::HandleSessionsChanged, 
      project_dock_widget ) ) );

    add_connection( ProjectManager::Instance()->current_project_->project_notes_state_->
      state_changed_signal_.connect( boost::bind( &ProjectDockWidget::HandleNoteSaved, 
      project_dock_widget ) ) );

    add_connection( PreferencesManager::Instance()->auto_save_time_state_->
      value_changed_signal_.connect( boost::bind( 
      &ProjectDockWidget::HandleAutoSaveTimeChanged, project_dock_widget, _1 ) ) );

    add_connection( ProjectManager::Instance()->current_project_->project_file_size_state_->
      value_changed_signal_.connect( boost::bind(
      &ProjectDockWidget::HandleFileSizeChanged, project_dock_widget, _1 ) ) );
    
    connect( this->private_->ui_.save_session_button_, SIGNAL( clicked() ), 
      this, SLOT( save_session() ) );

    connect( this->private_->ui_.session_name_linedit_, SIGNAL( returnPressed() ),
      this, SLOT( save_session() ) );

    connect( this->private_->ui_.load_session_button_, SIGNAL( clicked() ), 
      this, SLOT( load_session() ) );
    
    connect( this->private_->ui_.delete_session_button_, SIGNAL( clicked() ),
      this, SLOT( delete_session() ) );

    connect( this->private_->ui_.sessions_list_, SIGNAL( cellDoubleClicked ( int, int ) ),
      this, SLOT( load_session() ) );

    connect( this->private_->ui_.save_note_button_, SIGNAL( clicked() ), 
      this, SLOT( save_note() ) );

    connect( this->private_->ui_.note_edit_, SIGNAL( cursorPositionChanged() ),
      this, SLOT( enable_save_notes_button() ) );

    connect( this->private_->ui_.export_project_button_, SIGNAL( clicked() ),
      this, SLOT( export_project() ) );

    connect( this->private_->ui_.sessions_list_, SIGNAL( cellClicked ( int, int ) ), 
      this, SLOT( enable_load_delete_and_export_buttons( int, int ) ) );
    
    QtUtils::QtBridge::Enable( this, ProjectManager::Instance()->project_saved_state_ );
    
    QStringList headers;
    headers << "Time:" << "Session Name:";
    this->private_->ui_.sessions_list_->setHorizontalHeaderLabels( headers );
    
    this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );

    this->disable_load_delete_and_export_buttons();

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
}

void ProjectDockWidget::save_note()
{
  std::string current_text = this->private_->ui_.note_edit_->toPlainText().toStdString();
  if( current_text == "Enter your note here." ) return;

  ProjectManager::Instance()->save_note( current_text );
  this->private_->resetting_ = true;
  this->private_->ui_.note_edit_->setStyleSheet( 
    QString::fromUtf8( "QTextEdit#note_edit_{ color: light gray; }" ) );
  this->private_->ui_.note_edit_->setPlainText( 
    QString::fromUtf8( "Enter your note here." ) );
  this->private_->ui_.save_note_button_->setEnabled( false );
}
  
void ProjectDockWidget::load_session()
{
  int row = this->private_->ui_.sessions_list_->currentRow();
  
  this->disable_load_delete_and_export_buttons();

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
    if ( ProjectManager::Instance()->current_project_ )
    {
      if ( ProjectManager::Instance()->current_project_->check_project_changed() )
      {

        // Check whether the users wants to save and whether the user wants to quit
        int ret = QMessageBox::warning( this, "Save Current Session ?",
          "Your current session has not been saved.\n"
          "Do you want to save your changes?",
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
        
        if ( ret == QMessageBox::Save )
        {
          Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
          ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false, 
            ProjectManager::Instance()->current_project_->
            current_session_name_state_->get() );   
        }

        if ( ret != QMessageBox::Cancel )
        {
          std::vector< std::string > sessions = 
            ProjectManager::Instance()->current_project_->sessions_state_->get();

          std::string session_name = sessions[ row ];
          ActionLoadSession::Dispatch( Core::Interface::GetWidgetActionContext(), 
            session_name );
        }
      }
      else
      {
        std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
          sessions_state_->get();

        std::string session_name = sessions[ row ];
        ActionLoadSession::Dispatch( Core::Interface::GetWidgetActionContext(), 
          session_name );   
      }
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
    message_box.setText( QString::fromUtf8( "WARNING: You cannot recover a deleted session.") );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( QMessageBox::Yes == message_box.exec() )
    {
      std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
        sessions_state_->get();

      std::string session_name = sessions[ row ];
      ActionDeleteSession::Dispatch( Core::Interface::GetWidgetActionContext(), 
        session_name );
    }
  }

  this->populate_session_list();
  this->disable_load_delete_and_export_buttons();
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

  // The session list has a list of session files that we need to list.  The session files are 
  // saved in the format timestamp - sessionname - username.xml
  for( int i = 0; i < static_cast< int >( sessions.size() ); ++i )
  {
    // in the saving and reloading using Splitstring we occasionally get empty spaces in our
    // vectors as well as extra ] characters, so we need to filter them out
    if( ( sessions[ i ] == "" ) || ( sessions[ i ] == "]" ) )
    {
      continue;
    }

    if( static_cast< int >( Core::SplitString( sessions[ i ], " - " ).size() ) < 3 )
    {
      continue;
    }

    std::string session_name = sessions[ i ];
    QTableWidgetItem *new_item_name;
    QTableWidgetItem *new_item_time;

    new_item_time = new QTableWidgetItem( QString::fromStdString( 
      ( Core::SplitString( session_name, " - " ) )[ 0 ] ) );
    new_item_name = new QTableWidgetItem( QString::fromStdString( 
      ( Core::SplitString( session_name, " - " ) )[ 1 ] ) );

    QFont font;
    //font.setPointSize( 11 );
    
    if( ( Core::SplitString( session_name, " - " ) )[ 1 ]== "AutoSave" )
    {
      font.setBold( true );
    }
    
    new_item_name->setFont( font );
    new_item_time->setFont( font );

    this->private_->ui_.sessions_list_->insertRow( i );

    std::vector< std::string > time_vector = Core::SplitString( 
      new_item_time->text().toStdString(), "-" );

    if( time_vector.size() == 6 )
    {

      QString date_stamp = QString::fromStdString( time_vector[ 0 ] + "|" + 
        time_vector[ 1 ] + "|" + time_vector[ 2 ] );

      QString time_stamp = QString::fromStdString( time_vector[ 3 ] + ":" + 
        time_vector[ 4 ] + ":" + time_vector[ 5 ] );

      if( date_stamp.toStdString() == this->get_date() )
      {
        new_item_time->setText( time_stamp );
      }
      else
      {
        new_item_time->setText( date_stamp );
      }

      // Add a tooltip to display the user who saved the session 
      QString tool_tip = QString::fromUtf8( "This session was saved by: " )
        + QString::fromStdString( ( Core::SplitString( session_name, " - " ) )[ 2 ] ) 
        + " on " + date_stamp + " at " + time_stamp;

      new_item_time->setToolTip( tool_tip );
      new_item_name->setToolTip( tool_tip );
    }

    std::string test = new_item_time->text().toStdString();
    this->private_->ui_.sessions_list_->setItem( i, 0, new_item_time );
    this->private_->ui_.sessions_list_->setItem( i, 1, new_item_name );
    this->private_->ui_.sessions_list_->verticalHeader()->resizeSection( i, 24 );
    
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

  this->private_->ui_.notes_tree_->expandItem( 
    this->private_->ui_.notes_tree_->topLevelItem( 0 ) );
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
  
  if( current_text == "Enter your note here." )
  {
    if( !this->private_->resetting_ )
    {
      this->private_->ui_.note_edit_->setPlainText( 
        QString::fromUtf8( "" ) );
      this->private_->ui_.note_edit_->setStyleSheet( 
        QString::fromUtf8( "QTextEdit#note_edit_{ color: black; }" ) );
    }
    else
    {
      this->private_->resetting_ = false;
    }
  }

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

void ProjectDockWidget::HandleFileSizeChanged( qpointer_type qpointer, long long file_size )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleFileSizeChanged, qpointer, file_size ) );
    return;
  }
  if( qpointer.data() ) qpointer->set_file_size_label( file_size );
}


void ProjectDockWidget::set_file_size_label( long long file_size )
{
  // TODO:
  // Fix this!
  QString pretty_file_size = QString::number( file_size, 10 );
  int number_length = pretty_file_size.size();

  if( number_length > 12 )
  {
    if( number_length == 13 )
    {
      pretty_file_size.insert( 1, "." );
      pretty_file_size.resize( 4 );
    }

    if( number_length == 14 )
    {
      pretty_file_size.insert( 2, "." );
      pretty_file_size.resize( 5 );
    }

    if( number_length == 15 )
    {
      pretty_file_size.insert( 3, "." );
      pretty_file_size.resize( 6 );
    }
    pretty_file_size = pretty_file_size + " TB";
  }
  else if( number_length > 9 )
  {
    if( number_length == 10 )
    {
      pretty_file_size.insert( 1, "." );
      pretty_file_size.resize( 4 );
    }

    if( number_length == 11 )
    {
      pretty_file_size.insert( 2, "." );
      pretty_file_size.resize( 5 );
    }

    if( number_length == 12 )
    {
      pretty_file_size.insert( 3, "." );
      pretty_file_size.resize( 6 );
    }
    pretty_file_size = pretty_file_size + " GB";
  }
  else if( number_length > 6 )
  {
    if( number_length == 7 )
    {
      pretty_file_size.insert( 1, "." );
      pretty_file_size.resize( 4 );
    }

    if( number_length == 8 )
    {
      pretty_file_size.insert( 2, "." );
      pretty_file_size.resize( 5 );
    }

    if( number_length == 9 )
    {
      pretty_file_size.insert( 3, "." );
      pretty_file_size.resize( 6 );
    }
    pretty_file_size = pretty_file_size + " MB";
  }
  else if( number_length > 3 )
  {
    if( number_length == 4 )
    {
      pretty_file_size.insert( 1, "." );
      pretty_file_size.resize( 4 );
    }

    if( number_length == 5 )
    {
      pretty_file_size.insert( 2, "." );
      pretty_file_size.resize( 5 );
    }

    if( number_length == 6 )
    {
      pretty_file_size.insert( 3, "." );
      pretty_file_size.resize( 6 );
    }

    pretty_file_size = pretty_file_size + " KB";
  }
  else
  {
    pretty_file_size = pretty_file_size + " B";
  }


  if( file_size == 0 )
  {
    pretty_file_size = QString::fromUtf8( "No Data Files" );
  }


  this->private_->ui_.file_size_label_->setText( pretty_file_size );
}

void ProjectDockWidget::set_auto_save_label( int duration )
{
  this->private_->ui_.minutes_label_->setText( QString::number( duration ) );
}


std::string ProjectDockWidget::get_date()
{
  time_t rawtime;
  struct tm * timeinfo;
  char time_buffer [80];

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  strftime ( time_buffer, 80, "%d|%b|%Y", timeinfo );
  std::string date = time_buffer;
  return date;
}

void ProjectDockWidget::export_project()
{
  int row = this->private_->ui_.sessions_list_->currentRow();
  if( row < 0 )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "You cannot export a project without selecting a "
      " session to export.") );
    message_box.setInformativeText( QString::fromUtf8( 
      "Please choose a session and try again." ) );
    message_box.setStandardButtons( QMessageBox::Ok );
    message_box.setDefaultButton( QMessageBox::Ok );
    message_box.exec();
    return;
  }
  
  
  std::vector< std::string > sessions = ProjectManager::Instance()->current_project_->
    sessions_state_->get();
  std::string session_name = sessions[ row ];

  QPointer< ProjectExportWizard > project_export_wizard_ = 
    new ProjectExportWizard( session_name, this );
  project_export_wizard_->show();

  this->disable_load_delete_and_export_buttons();
}

void ProjectDockWidget::enable_load_delete_and_export_buttons( int row, int column )
{
  if( row >= 0 )
  {
    this->private_->ui_.export_project_button_->setEnabled( true );
    this->private_->ui_.load_session_button_->setEnabled( true );
    this->private_->ui_.delete_session_button_->setEnabled( true );
  }
}

void ProjectDockWidget::disable_load_delete_and_export_buttons()
{
  this->private_->ui_.export_project_button_->setEnabled( false );
  this->private_->ui_.load_session_button_->setEnabled( false );
  this->private_->ui_.delete_session_button_->setEnabled( false );
  
  for( int i = 0; i < this->private_->ui_.sessions_list_->columnCount(); ++i )
  {
    for( int j = 0; j < this->private_->ui_.sessions_list_->rowCount(); ++j )
    {
      this->private_->ui_.sessions_list_->itemAt( j, i )->setSelected( false );
    }
  }
}

void ProjectDockWidget::set_autosave_checked_state( bool state )
{
  Core::ActionSet::Dispatch( Core::Interface::GetMouseActionContext(),
    PreferencesManager::Instance()->auto_save_state_, state );

}
             
void ProjectDockWidget::set_autosave_checkbox( bool state )
{
   this->private_->ui_.autosave_checkbox_->blockSignals( true );
   this->private_->ui_.autosave_checkbox_->setChecked( state );
   this->private_->ui_.autosave_checkbox_->blockSignals( false );
}

void ProjectDockWidget::HandleAutosaveStateChanged( qpointer_type qpointer, bool state )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &ProjectDockWidget::set_autosave_checkbox, qpointer.data(), state ) ) );
   
}


} // end namespace Seg3D
