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

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>
#include <Core/State/Actions/ActionAdd.h>

// Qt includes
#include <QtGui/QStandardItemModel>
#include <QMessageBox>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>

//Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/Actions/ActionAddNote.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionLoadSession.h>
#include <Application/ProjectManager/Actions/ActionDeleteSession.h>
#include <Application/ProjectManager/Actions/ActionExportProject.h>

// Interface includes
#include <Interface/Application/ProjectDockWidget.h>
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/ProjectExportWizard.h>
#include <Interface/Application/SaveProjectAsWizard.h>
#include "ui_ProjectDockWidget.h"

namespace Seg3D
{

class ProjectDockWidgetPrivate : public QObject
{
public:
  ProjectDockWidgetPrivate( QObject* parent ) :
    QObject( parent ),
    resetting_( false )
  {
  }

  // POPULATE_SESSION_LIST:
  // This function clears the current session list and reloads it from the state variables
  void populate_session_list( SessionInfoListHandle session_list );

  // POPULATE_NOTES_LIST:
  // This function clears the current notes list and reloads it from the state variables
  void populate_notes_list( ProjectNoteListHandle note_list );

  // UPDATE_PROJECT_NAME_TOOLTIP:
  // Set the tooltip for the project name label in case the name is too long and gets truncated.
  void update_project_name_tooltip( std::string project_name );
  
public:
  // The UI that was created with QtCreator
  Ui::ProjectDockWidget ui_;

  // Handle to the current project
  ProjectHandle current_project_;

  bool resetting_;
  SessionInfoList sessions_;
};

typedef QPointer< ProjectDockWidgetPrivate > ProjectDockWidgetPrivateQWeakHandle;


void ProjectDockWidgetPrivate::populate_session_list( SessionInfoListHandle session_list )
{
  this->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( false );

  // Clean out the old table
  for( int j = this->ui_.sessions_list_->rowCount() -1; j >= 0; j-- )
  {
    this->ui_.sessions_list_->removeRow( j );
  }
  this->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( true );
  this->ui_.sessions_list_->repaint();
  this->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( false );

  this->sessions_ = *session_list;

  typedef boost::date_time::c_local_adjustor< SessionInfo::timestamp_type > local_time_adjustor;
  SessionInfo::timestamp_type::date_type today = boost::posix_time::second_clock::local_time().date();

  for( size_t i = 0; i < session_list->size(); ++i )
  {
    const SessionInfo& session_info = session_list->at( i );
    QTableWidgetItem *new_item_time;
    QTableWidgetItem *new_item_name;

    // Convert the session timestamp from UTC to local time
    SessionInfo::timestamp_type session_local_time = 
      local_time_adjustor::utc_to_local( session_info.timestamp() );

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
      session_info.session_name() ) );

    QFont font;
    if( session_info.session_name() == "AutoSave" ||
      session_info.session_name() == "Auto Save" )
    {
      font.setBold( true );
    }
    new_item_name->setFont( font );
    new_item_time->setFont( font );

    QString tool_tip = QString::fromUtf8( "This session was saved by: " )
      + QString::fromStdString( session_info.user_id() ) +  QString::fromUtf8( " at " ) +
      QString::fromStdString( boost::posix_time::to_simple_string( session_local_time ) );

    new_item_time->setToolTip( tool_tip );
    new_item_name->setToolTip( tool_tip );

    this->ui_.sessions_list_->insertRow( static_cast< int >( i ) );
    this->ui_.sessions_list_->setItem( static_cast< int >( i ), 0, new_item_time );
    this->ui_.sessions_list_->setItem( static_cast< int >( i ), 1, new_item_name );
    this->ui_.sessions_list_->verticalHeader()->resizeSection( static_cast< int >( i ), 24 );
  }

  this->ui_.sessions_list_->verticalHeader()->setUpdatesEnabled( true );
  this->ui_.sessions_list_->repaint();
}

void ProjectDockWidgetPrivate::populate_notes_list( ProjectNoteListHandle note_list )
{ 
  // Clear out the tree widget
  this->ui_.notes_tree_->clear();

  int num_of_notes = static_cast< int >( note_list->size() );

  if ( num_of_notes < 1 )
  {
    return;
  }

  typedef boost::date_time::c_local_adjustor< ProjectNote::timestamp_type > local_time_adjustor;

  for ( int i = num_of_notes - 1; i >= 0; i-- )
  {
    const ProjectNote& note = note_list->at( i );

    // Convert the note timestamp from UTC to local time
    ProjectNote::timestamp_type note_local_time = 
      local_time_adjustor::utc_to_local( note.timestamp() );

    QTreeWidgetItem* item = new QTreeWidgetItem( this->ui_.notes_tree_ );
    item->setText( 0, QString::fromStdString( boost::posix_time::to_simple_string( note_local_time ) +
      " - " + note.user_id() ) );

    QTreeWidgetItem* note_item = new QTreeWidgetItem();
    QLabel* note_text = new QLabel( QString::fromStdString( note.note() ) );
    note_text->setWordWrap( true );
    item->addChild( note_item );

    this->ui_.notes_tree_->addTopLevelItem( item );
    this->ui_.notes_tree_->setItemWidget( note_item, 0, note_text );
  }

  this->ui_.notes_tree_->expandItem( this->ui_.notes_tree_->topLevelItem( 0 ) );
}

void ProjectDockWidgetPrivate::update_project_name_tooltip( std::string project_name )
{
  ASSERT_IS_INTERFACE_THREAD();

  this->ui_.project_name_->setToolTip( QString::fromStdString( project_name ) );
}

// HANDLESESSIONSCHANGED:
// A function that verifies that we're operating on the proper thread and if not, it moves the 
// process to the correct one in order to reload the sessions displayed after they have been
// updated elsewhere.
static void UpdateSessionList( ProjectDockWidgetPrivateQWeakHandle qpointer, 
                SessionInfoListHandle session_list )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &UpdateSessionList, qpointer, session_list ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->populate_session_list( session_list );
  }
}

// HANDLENOTESSAVED:
// A function that verifies that we're operating on the proper thread and if not, it moves the 
// process to the correct one in order to save reload the notes displayed after they have been
// updated elsewhere.
static void UpdateNoteList( ProjectDockWidgetPrivateQWeakHandle qpointer, 
               ProjectNoteListHandle note_list )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &UpdateNoteList, qpointer, note_list ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() && !QCoreApplication::closingDown() ) 
  {
    qpointer->populate_notes_list( note_list );
  }
}

// UPDATEPROJECTNAMETOOLTIP:
// Update the tooltip for the project name label to contain the full project name in case the
// project name was too long to fit in the widget and got truncated.
static void UpdateProjectNameTooltip( ProjectDockWidgetPrivateQWeakHandle qpointer, 
  std::string project_name )
{
  // This function needs to be called on the Interface thread, hence if we are not on the
  // Interface thread we need send a message to the Interface thread to actually execute
  // this function, with the current parameters.

  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( 
      boost::bind( &UpdateProjectNameTooltip, qpointer, project_name ) );
    return;
  }

  // We need to check whether the object still exists, the use of the qpointer allows for
  // checking if the object still exists.
  if( qpointer.data() && !QCoreApplication::closingDown() ) 
  {
    qpointer->update_project_name_tooltip( project_name );
  }
}

//////////////////////////////////////////////////////////////////////////
// Class ProjectDockWidget
//////////////////////////////////////////////////////////////////////////

ProjectDockWidget::ProjectDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent )
{
  this->private_ = new ProjectDockWidgetPrivate( this );
  // Setup the User Interface 
  this->private_->ui_.setupUi( this );

  this->private_->ui_.dockWidgetContents->setStyleSheet( StyleSheet::PROJECTDOCKWIDGET_C );
  // Update some settings in the design from QtDesigner that we cannot set directly
  QStringList headers; headers << "Time" << "Session Name";
  this->private_->ui_.sessions_list_->setHorizontalHeaderLabels( headers );
  this->private_->ui_.sessions_list_->horizontalHeader()->setDefaultAlignment( Qt::AlignLeft );
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );

  this->private_->ui_.session_name_linedit_->setValidator( new QRegExpValidator( 
    QRegExp( QString::fromStdString( Core::StateName::REGEX_VALIDATOR_C ) ), this ) );

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

    qpointer_type qpointer( this );

    // This connection will trigger this function to be run, and forces all connections to be
    // updated and the new project to be used.
    this->add_connection( ProjectManager::Instance()->current_project_changed_signal_.
      connect( boost::bind( &ProjectDockWidget::HandleUpdateWidget, qpointer ) ) );

    // TODO: Need to reroute this through the QtBridge
    this->add_connection( PreferencesManager::Instance()->auto_save_time_state_->
      value_changed_signal_.connect( boost::bind( 
      &ProjectDockWidget::HandleAutoSaveTimeChanged, qpointer, _1 ) ) );

    // This connection will ensure that the session list is updated when a new session is available
    this->add_connection( current_project->session_list_changed_signal_.
      connect( boost::bind( &UpdateSessionList, ProjectDockWidgetPrivateQWeakHandle( this->private_ ), _1 ) ) );

    // This connection will update the notes in the window
    this->add_connection( current_project->note_list_changed_signal_.
      connect( boost::bind( &UpdateNoteList, ProjectDockWidgetPrivateQWeakHandle( this->private_ ), _1 ) ) );

    // TODO: Need to generate QtLabel connector with a formatting function, so this can run
    // through the Qt bridge.
    this->add_connection( current_project->project_size_state_->value_changed_signal_.
      connect( boost::bind( &ProjectDockWidget::HandleFileSizeChanged, qpointer, _1 ) ) );
      
    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.autosave_checkbox_,
      PreferencesManager::Instance()->auto_save_state_ ) );

    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.project_name_, 
      current_project->project_name_state_ ) );
    
    // Set the tooltip for the project name label in case the name is too long and gets truncated
    std::string project_name = current_project->project_name_state_->get();
    this->private_->update_project_name_tooltip( project_name );
    
    // Add connection to update project name tooltip in case it changes (e.g. Save As)
    this->add_connection( current_project->project_name_state_->value_changed_signal_.
      connect( boost::bind( &UpdateProjectNameTooltip, 
      ProjectDockWidgetPrivateQWeakHandle( this->private_ ), _1 ) ) );

    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.custom_colors_checkbox_, 
      current_project->save_custom_colors_state_ ) );

    this->add_connection( QtUtils::QtBridge::Connect( this->private_->ui_.session_name_linedit_, 
      current_project->current_session_name_state_, true ) );

    this->add_connection( QtUtils::QtBridge::Enable( this, current_project->project_files_generated_state_ ) );
  
    // Update the session list
    current_project->request_session_list();
    
    // Update notes list
    current_project->request_note_list();
    
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
    SaveProjectAsWizard* save_project_as_wizard_ = new SaveProjectAsWizard( this->parentWidget() );
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

  ActionAddNote::Dispatch( Core::Interface::GetWidgetActionContext(), current_text );
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
      int ret = QMessageBox::warning( this->parentWidget(), "Save Current Session ?",
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
    new ProjectExportWizard( this->private_->sessions_[ row ].session_id(), this->parentWidget() );
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
      this->private_->ui_.sessions_list_->item( j, i )->setSelected( false );
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
  if( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->update_widget();  
  }
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
  if( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->set_auto_save_label( duration );
  }
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
  if( qpointer.data() && !QCoreApplication::closingDown() )
  {
    qpointer->set_file_size_label( file_size );
  }
}

} // end namespace Seg3D
