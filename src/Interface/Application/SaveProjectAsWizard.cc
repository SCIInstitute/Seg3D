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

// Boost includes
#include <boost/filesystem.hpp>

// Qt includes
#include <QVariant>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>

// Core includes
#include <Core/State/Actions/ActionSet.h>
#include <Core/Application/Application.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionSaveProjectAs.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>

// Interface includes
#include <Interface/Application/SaveProjectAsWizard.h>

namespace Seg3D
{

SaveProjectAsWizard::SaveProjectAsWizard( QWidget *parent ) :
    QWizard( parent ),
    path_to_delete_( "" )
{
  this->addPage( new SaveAsInfoPage );
  this->addPage( new SaveAsSummaryPage );
    
  connect( this->page( 0 ), SIGNAL( just_a_save() ), this, SLOT( finish_early() ) );
  connect( this->page( 0 ), SIGNAL( need_to_set_delete_path( QString ) ), this, 
    SLOT( set_delete_path( QString ) ) );
  
  this->setWizardStyle( QWizard::MacStyle );
  this->setPixmap( QWizard::BackgroundPixmap, 
    QPixmap( QString::fromUtf8( ":/Images/Symbol.png" ) ) );
  
  this->setWindowTitle( tr( "Save Project As Wizard" ) );
}

SaveProjectAsWizard::~SaveProjectAsWizard()
{
}

void SaveProjectAsWizard::accept()
{
  if( this->path_to_delete_ != "" )
  {
    try
    {
      boost::filesystem::remove_all( boost::filesystem::path( this->path_to_delete_ ) );
    }
    catch ( ... )
    {
      std::string error = std::string( "Could not remove all files from directory '" ) +
        this->path_to_delete_ + "'.";
      CORE_LOG_ERROR( error );
    }
  }
  
  bool anonymize = field( "anonymize" ).toBool();

  ActionSaveProjectAs::Dispatch( Core::Interface::GetWidgetActionContext(), 
    field( "projectPath" ).toString().toStdString(),
    field( "projectName" ).toString().toStdString(), anonymize );
  
  // Switch on autosave if needed
  if( field( "autosave" ).toBool() )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
      PreferencesManager::Instance()->auto_save_state_, true );
  }

    QDialog::accept();
}

void SaveProjectAsWizard::finish_early()
{
  this->close();
}

void SaveProjectAsWizard::set_delete_path( QString path )
{
  this->path_to_delete_ = path.toStdString();
}


SaveAsInfoPage::SaveAsInfoPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Save Project As..." );
    this->setSubTitle( "You are saving the current project with the following settings: " );

    this->project_name_label_ = new QLabel( "Project name:" );

  this->project_name_lineedit_ = new QLineEdit();
  std::string project_name = ProjectManager::Instance()->
    get_current_project()->project_name_state_->get();
    
  std::string current_project_dir = ProjectManager::Instance()->
    get_current_project_folder().string();
    
  this->project_name_lineedit_->setText(  QString::fromStdString( project_name ) );

    this->project_path_label_ = new QLabel( "Project Path:" );
    this->project_path_lineedit_ = new QLineEdit;
    

  this->project_path_lineedit_->setText(  QString::fromStdString( current_project_dir ) );

    this->project_path_change_button_ = new QPushButton( "Choose Alternative Location" );
    connect( this->project_path_change_button_, SIGNAL( clicked() ), this, SLOT( set_path() ) );
  this->project_path_change_button_->setFocusPolicy( Qt::NoFocus );

    registerField( "projectName", this->project_name_lineedit_ );
    
  this->warning_message_ = new QLabel( QString::fromUtf8( "This location does not exist, please choose a valid location." ) );
  this->warning_message_->setObjectName( QString::fromUtf8( "warning_message_" ) );
  this->warning_message_->setWordWrap( true );
  this->warning_message_->setStyleSheet(QString::fromUtf8( "QLabel#warning_message_{ color: red; } " ) );
  this->warning_message_->hide();

    QGridLayout *layout = new QGridLayout;
    layout->addWidget( this->project_name_label_, 0, 0 );
    layout->addWidget( this->project_name_lineedit_, 0, 1 );
    layout->addWidget( this->project_path_label_, 1, 0 );
    layout->addWidget( this->project_path_lineedit_, 1, 1 );
    layout->addWidget( this->project_path_change_button_, 2, 1, 1, 2 );
    layout->addWidget( this->warning_message_, 3, 1, 1, 2 );
    
    setLayout( layout );
}
  
void SaveAsInfoPage::initializePage()
{
  QString finishText = wizard()->buttonText( QWizard::FinishButton );
  finishText.remove( '&' );

  this->project_path_lineedit_->setText( QString::fromStdString( 
    ProjectManager::Instance()->get_current_project_folder().string() ) );
  this->registerField( "projectPath", this->project_path_lineedit_ );
}
  
void SaveAsInfoPage::set_path()
{
  //QDir converts the empty QString "" to the current directory
  this->warning_message_->hide();

    
    QString path_name = QFileDialog::getExistingDirectory ( this, 
    tr( "Choose Save Directory..." ), this->project_path_lineedit_->text(), 
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

    // getExistingString generates an empty string if canceled or closed.
    //  If either happens, this will exit without changing anything.
    if(path_name == "")
    {
  return;
    }
    QDir project_directory_ = QDir(path_name);
    
  if( project_directory_.exists() )
    {
       this->project_path_lineedit_->setText( project_directory_.canonicalPath() );
    }

        registerField( "projectPath", this->project_path_lineedit_ );
}

bool SaveAsInfoPage::validatePage()
{
  // before we do anything we clear the delete path variable
  Q_EMIT need_to_set_delete_path( "" );
  
  boost::filesystem::path project_path = 
    boost::filesystem::path( this->project_path_lineedit_->text().toStdString() ) / 
    boost::filesystem::path( this->project_name_lineedit_->text().toStdString() + 
      Project::GetDefaultProjectPathExtension() );
    
  // We check to see if the path they are choosing already exists
  if( boost::filesystem::exists( project_path ) )
  {
    // If we save on top of our current project
    if( ( ProjectManager::Instance()->get_current_project()->project_name_state_->get() == 
      this->project_name_lineedit_->text().toStdString() ) && 
      ( ProjectManager::Instance()->get_current_project()->project_path_state_->get() == 
      project_path.string() ) )
    {
      // Just save a session
      ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" );
      Q_EMIT just_a_save();
      return true;    
    }
  
    int ret = QMessageBox::warning( this, 
      "A project with this name already exists!",
      "A project with this name already exists!\n"
      "If you proceed the old project will be deleted and replaced.\n"
      "Are you sure you would like to continue?",
      QMessageBox::Yes | QMessageBox::No );

    if( ret != QMessageBox::Yes )
    {
      return false;
    }
    
    Q_EMIT need_to_set_delete_path( QString::fromStdString( project_path.string() ) );
    
    return true;
  }
  
  // Check to see if the directory that we are trying to save in exists
  if( !boost::filesystem::exists( project_path.parent_path() ) )
  {
    this->warning_message_->setText( QString::fromUtf8( 
      "This location does not exist, please choose a valid location." ) );
    this->warning_message_->show();
    return false;
  }

  // Finally we check to see if we can write to that directory
  try // to create a project sessions folder
  {
    boost::filesystem::create_directory( project_path );
  }
  catch ( ... ) // any errors that we might get thrown would indicate that we cant write here
  {
    this->warning_message_->setText( QString::fromUtf8( 
      "This location is not writable, please choose a valid location." ) );
    this->warning_message_->show();
    return false;
  }

  // if we have made it to here we have created a new directory lets remove it.
  boost::filesystem::remove( project_path );
  
  this->warning_message_->hide();

  return true;
}

SaveAsSummaryPage::SaveAsSummaryPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Summary" );

  this->description_ = new QLabel( "Please verify these settings before you continue.  A new "
    "project will be created with the following settings.\n" );
  this->description_->setWordWrap( true );
  
    this->project_name_ = new QLabel;
    this->project_name_->setWordWrap( true );

    this->project_path_ = new QLabel;
    this->project_path_->setWordWrap( true );
  
  this->anonymize_checkbox_ = new QCheckBox();
  this->anonymize_checkbox_->setObjectName(QString::fromUtf8("anonymize_checkbox_"));
  this->anonymize_checkbox_->setChecked( false );
  this->anonymize_checkbox_->setText( QString::fromUtf8( "Anonymize (Remove provenance, embedded files, and meta data)" ) );
  
    QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget( this->description_ );
    layout->addWidget( this->project_name_ );
    layout->addWidget( this->project_path_ );
  layout->addWidget( this->anonymize_checkbox_ );
    this->setLayout( layout );
  
  registerField( "anonymize", this->anonymize_checkbox_ );
}

void SaveAsSummaryPage::initializePage()
{
    QString finishText = wizard()->buttonText( QWizard::FinishButton );
    finishText.remove('&');

    this->project_name_->setText( 
    QString::fromUtf8( "Project Name: " ) + field("projectName").toString() );
    this->project_path_->setText( 
    QString::fromUtf8( "Project Path: " ) + field("projectPath").toString() );  
}

} // end namespace Seg3D
