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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/State/Actions/ActionAdd.h>

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
#include <Interface/Application/SaveProjectAsWizard.h>
#include "ui_ProjectDockWidget.h"

namespace Seg3D
{

class ProjectDockWidgetPrivate
{
public:
  ProjectDockWidgetPrivate() :
    resetting_( false )
  {
  }
  
public:
  // The UI that was created with QtCreator
  Ui::ProjectDockWidget ui_;

  // Handle to the current project
  ProjectHandle current_project_;

  bool resetting_;
  std::vector< SessionInfo > sessions_;
};

ProjectDockWidget::ProjectDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent ), 
  private_( new ProjectDockWidgetPrivate )
{

  // Setup the User Interface 
  this->private_->ui_.setupUi( this );
  
  // Update some settings in the design from QtDesigner that we cannot set directly
  QStringList headers; headers << "Time" << "Session Name";
  this->private_->ui_.sessions_list_->setHorizontalHeaderLabels( headers );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );

  connect( this->private_->ui_.save_session_button_, SIGNAL( clicked() ), 
    this, SLOT( save_session() ) );

//  connect( this->private_->ui_.session_name_linedit_, SIGNAL( returnPressed() ),
//    this, SLOT( save_session() ) );

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


    
  // This will make sure that the starting active tab is always the sessions tab
  this->private_->ui_.tabWidget->setCurrentIndex( this->private_->ui_.tabWidget->indexOf( 
    this->private_->ui_.sessions_tab_ ) );

  this->update_widget();
}

ProjectDockWidget::~ProjectDockWidget()
{
  this->disconnect_all();
}

void ProjectDockWidget::update_widget()
{
  // Get the current project
  // ProjectHandle current_project = ProjectManager::Instance()->get_current_project();

  // If the project is the same, do not do anything
  // if ( this->private_->current_project_ == current_project ) return;

  // -- First disconnect all open connections --

  // Disconnect all connections managed by the QtUtils and boost
  this->disconnect_all();

  // -------------------------------------------

  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    // Grab this one, once again, but now within the lock
    ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
    
    // Swap out the project handle
    this->private_->current_project_ = current_project;

    // We dont want them to be able to save blank notes.
    this->private_->ui_.save_note_button_->setEnabled( false );

    int minutes = PreferencesManager::Instance()->auto_save_time_state_->get();
    this->private_->ui_.minutes_label_->setText( QString::number( minutes ) );

    // This connection will trigger this function to be run, and forces all connections to be
    // updated and the new project to be used.
    this->add_connection( ProjectManager::Instance()->current_project_changed_signal_.
      connect( boost::bind( &ProjectDockWidget::HandleUpdateWidget, qpointer_type( this ) ) ) );

    // TODO: Need to reroute thsi through the QtBridge
    this->add_connection( PreferencesManager::Instance()->auto_save_time_state_->
      value_changed_signal_.connect( boost::bind( 
      &ProjectDockWidget::HandleAutoSaveTimeChanged, qpointer_type( this ), _1 ) ) );

    // This connection will ensure that the session list is updated when a new session is available
    this->add_connection( current_project->sessions_changed_signal_.
      connect( boost::bind( &ProjectDockWidget::HandleSessionsChanged, qpointer_type( this ) ) ) );

    // This connection will update the notes in the window
    this->add_connection( current_project->project_notes_state_->state_changed_signal_.
      connect( boost::bind( &ProjectDockWidget::HandleNoteSaved, qpointer_type( this ) ) ) );

    // TODO: Need to generate QtLabel conenctor with a formatting function, so this can run
    // through the Qt bridge.
    this->add_connection( current_project->project_size_state_->value_changed_signal_.
      connect( boost::bind( &ProjectDockWidget::HandleFileSizeChanged, qpointer_type( this ), _1 ) ) );
      
    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.autosave_checkbox_,
      PreferencesManager::Instance()->auto_save_state_ ) );

    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.project_name_, 
      current_project->project_name_state_ ) );
    
    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.custom_colors_checkbox_, 
      current_project->save_custom_colors_state_ ) );

    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.session_name_linedit_, 
      current_project->current_session_name_state_, true ) );

    this->add_connection( QtUtils::QtBridge::Enable( this,
      ProjectManager::Instance()->get_current_project()->project_files_generated_state_ ) );
  
    // Update the session list
    this->populate_session_list();
    
    // Update notes list
    this->populate_notes_list();
    
    // Update file size
    this->set_file_size_label( current_project->project_size_state_->get() );
    
    // Update buttons
    this->disable_load_delete_and_export_buttons();
  }
}

  
void ProjectDockWidget::save_session()
{
  ProjectHandle current_project = ProjectManager::Instance()->get_current_project();

  // Need to lock state engine as we need query state properties of the current project.
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  if ( current_project->project_files_generated_state_->get() == false ||
    current_project->project_files_accessible_state_->get() == false )
  {
    SaveProjectAsWizard* save_project_as_wizard_ = new SaveProjectAsWizard( qobject_cast< QWidget* >( this->parent() ) );
    save_project_as_wizard_->exec();
  }

  ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" );
}

void ProjectDockWidget::save_note()
{
  std::string current_text = this->private_->ui_.note_edit_->toPlainText().toStdString();
  if( current_text == "Enter your note here." ) return;

  this->private_->resetting_ = true;
  this->private_->ui_.note_edit_->setStyleSheet( 
    QString::fromUtf8( "QPlainTextEdit#note_edit_{ color: rgb(150, 150, 150); }" ) );
  this->private_->ui_.note_edit_->setPlainText( 
    QString::fromUtf8( "Enter your note here." ) );
  this->private_->ui_.save_note_button_->setEnabled( false );

  std::string current_user;
  Core::Application::Instance()->get_user_name( current_user );

  std::string time_stamp = boost::posix_time::to_simple_string( 
    boost::posix_time::second_clock::local_time() );
    
  // TODO: This function is probably called from the wrong thread
  Core::ActionAdd::Dispatch( Core::Interface::GetWidgetActionContext(),
    ProjectManager::Instance()->get_current_project()->project_notes_state_, 
    time_stamp + " - " + current_user + "|" + current_text );
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
  
  if ( this->private_->current_project_ )
  {
    if ( this->private_->current_project_->check_project_changed() )
    {
      // Check whether the users wants to save and whether the user wants to quit
      int ret = QMessageBox::warning( this, "Save Current Session ?",
        "Your current session has not been saved.\n"
        "Do you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
      
      if ( ret == QMessageBox::Save )
      {
        Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" );   
      }

      if ( ret == QMessageBox::Cancel )
      {
        return;
      }
    }

    ActionLoadSession::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->sessions_[ row ].session_id() );
  }
}

void ProjectDockWidget::delete_session()
{ 
  if( static_cast<size_t>( this->private_->ui_.sessions_list_->rowCount() ) 
    != this->private_->sessions_.size() )
  {
    CORE_LOG_ERROR( "DANGER! The project dock's session list is out of sync with the actual sessions" );
    return;
  }

  std::vector< SessionID > sessions_to_delete;
    
  for( int i = 0; i < this->private_->ui_.sessions_list_->rowCount(); ++i ) 
  {
    if( this->private_->ui_.sessions_list_->item( i, 0 )->isSelected() )
    {
      sessions_to_delete.push_back( this->private_->sessions_[ i ].session_id() );
    }
  }

  if( sessions_to_delete.size() > 0 )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "WARNING: You cannot recover deleted sessions.") );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( QMessageBox::Yes == message_box.exec() )
    {
      for( size_t i = 0; i < sessions_to_delete.size(); ++i )
      {
        // Issue a command to delete the session on the application thread
        // This needs to be done synchronously with the main program and hence
        // it needs to be dispatched to the main application thread.
        ActionDeleteSession::Dispatch( Core::Interface::GetWidgetActionContext(), 
          sessions_to_delete[ i ] );
      } 
    }
  }
  this->disable_load_delete_and_export_buttons();
}

void ProjectDockWidget::populate_session_list()
{
  // We are going to update our local copy of the session list so we'll clear it first
  this->private_->sessions_.clear();

  this->private_->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( false );

  // Clean out the old table
  for( int j = this->private_->ui_.sessions_list_->rowCount() -1; j >= 0; j-- )
  {
    this->private_->ui_.sessions_list_->removeRow( j );
  }
  this->private_->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( true );
  this->private_->ui_.sessions_list_->repaint();
  this->private_->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( false );

  std::vector< SessionInfo > sessions_info;
  if( !this->private_->current_project_->get_all_sessions( sessions_info ) )
  {
    CORE_LOG_DEBUG( "no previous sessions exist" );
    return;
  }
  this->private_->sessions_ = sessions_info;

  typedef boost::date_time::c_local_adjustor< SessionInfo::timestamp_type > local_time_adjustor;
  SessionInfo::timestamp_type::date_type today = boost::posix_time::second_clock::local_time().date();

  for( size_t i = 0; i < sessions_info.size(); ++i )
  {
    QTableWidgetItem *new_item_time;
    QTableWidgetItem *new_item_name;

    // Convert the session timestamp from UTC to local time
    SessionInfo::timestamp_type session_local_time = 
      local_time_adjustor::utc_to_local( sessions_info[ i ].timestamp() );
     
    if( today == session_local_time.date() )
    {
      new_item_time = new QTableWidgetItem( QString::fromStdString( 
        boost::posix_time::to_simple_string( session_local_time.time_of_day() ) ) );
    }
    else
    {
      new_item_time = new QTableWidgetItem( QString::fromStdString( 
        boost::gregorian::to_simple_string( session_local_time.date() ) ) );
    }
    
    new_item_name = new QTableWidgetItem( QString::fromStdString( 
      sessions_info[ i ].session_name() ) );
      
    QFont font;
    if( sessions_info[ i ].session_name() == "AutoSave" )
    {
      font.setBold( true );
    }
    new_item_name->setFont( font );
    new_item_time->setFont( font );
    
    QString tool_tip = QString::fromUtf8( "This session was saved by: " )
      + QString::fromStdString( sessions_info[ i ].user_id() ) +  QString::fromUtf8( " at " ) +
       QString::fromStdString( boost::posix_time::to_simple_string( session_local_time ) );
       
    new_item_time->setToolTip( tool_tip );
    new_item_name->setToolTip( tool_tip );
    
    this->private_->ui_.sessions_list_->insertRow( static_cast< int >( i ) );
    this->private_->ui_.sessions_list_->setItem( static_cast< int >( i ), 0, new_item_time );
    this->private_->ui_.sessions_list_->setItem( static_cast< int >( i ), 1, new_item_name );
    this->private_->ui_.sessions_list_->verticalHeader()->resizeSection( static_cast< int >( i ), 24 );
  }

  this->private_->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( true );
  this->private_->ui_.sessions_list_->repaint();
}
    

void ProjectDockWidget::populate_notes_list()
{
  std::vector< std::string > notes;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    notes = this->private_->current_project_->project_notes_state_->get();
  }
  
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
        QString::fromUtf8( "QPlainTextEdit#note_edit_{ color: black; }" ) );
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

  QPointer< ProjectExportWizard > project_export_wizard_ = 
    new ProjectExportWizard( this->private_->sessions_[ row ].session_id(), this );
  project_export_wizard_->show();

  this->disable_load_delete_and_export_buttons();
}

void ProjectDockWidget::enable_load_delete_and_export_buttons( int row, int column )
{
  if( row >= 0 )
  {
    if( this->private_->ui_.sessions_list_->selectedItems().size() <= 2 )
    {
      this->private_->ui_.load_session_button_->setEnabled( true );
    }
    else
    {
      this->private_->ui_.load_session_button_->setEnabled( false );
    }
    this->private_->ui_.export_project_button_->setEnabled( true );
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

void ProjectDockWidget::HandleUpdateWidget( qpointer_type qpointer )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.
  
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleUpdateWidget, qpointer ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() ) qpointer->update_widget();  
}

void ProjectDockWidget::HandleAutoSaveTimeChanged( qpointer_type qpointer, int duration )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleAutoSaveTimeChanged, qpointer, duration ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() ) qpointer->set_auto_save_label( duration );
}

void ProjectDockWidget::HandleFileSizeChanged( qpointer_type qpointer, long long file_size )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleFileSizeChanged, qpointer, file_size ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() ) qpointer->set_file_size_label( file_size );
}


void ProjectDockWidget::HandleSessionsChanged( qpointer_type qpointer )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleSessionsChanged,  qpointer ) );
    return;
  }
  
  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() ) qpointer->populate_session_list();
}


void ProjectDockWidget::HandleNoteSaved( qpointer_type qpointer )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &ProjectDockWidget::HandleNoteSaved, qpointer ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() ) qpointer->populate_notes_list();
}

} // end namespace Seg3D
