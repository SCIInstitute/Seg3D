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
#include <QtCore/QVariant>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>

// Core includes
#include <Core/State/Actions/ActionSet.h>
#include <Core/Application/Application.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionExportProject.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// Interface includes
#include <Interface/Application/ProjectExportWizard.h>


namespace Seg3D
{

ProjectExportWizard::ProjectExportWizard( long long session_id, QWidget *parent ) :
    QWizard( parent )
{
  this->addPage( new ExportInfoPage );
  this->addPage( new ExportSummaryPage );
 
  this->setWizardStyle( QWizard::MacStyle );
  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );

  this->session_id_ = session_id;
  
  this->setWindowTitle( tr( "Export Project Wizard" ) );
}

ProjectExportWizard::~ProjectExportWizard()
{
}

void ProjectExportWizard::accept()
{
  ActionExportProject::Dispatch( Core::Interface::GetWidgetActionContext(), 
    field( "projectPath" ).toString().toStdString(),
    field( "projectName" ).toString().toStdString(),
    this->session_id_ );
    QDialog::accept();
}

ExportInfoPage::ExportInfoPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Export Project Information" );
    this->setSubTitle( "You are exporting the current project with the following settings: " );

    this->project_name_label_ = new QLabel( "Project name:" );

  this->project_name_lineedit_ = new QLineEdit();
  this->project_name_lineedit_->setText(  QString::fromStdString( ProjectManager::Instance()->
    get_current_project()->project_name_state_->get() ) );

    this->project_path_label_ = new QLabel( "Project Path:" );
    this->project_path_lineedit_ = new QLineEdit;
    
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
  
void ExportInfoPage::initializePage()
{
  this->project_path_lineedit_->setText( 
    QString::fromStdString( ProjectManager::Instance()->get_current_project_folder().string() ) );
  this->registerField( "projectPath", this->project_path_lineedit_ );
}
  
void ExportInfoPage::set_path()
{
  this->warning_message_->hide();
  
    QDir project_directory_ = QDir( QFileDialog::getExistingDirectory ( this, 
    tr( "Choose Directory for Export..." ), this->project_path_lineedit_->text(), 
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ) );
  
    if ( project_directory_.exists() )
    {
        this->project_path_lineedit_->setText( project_directory_.canonicalPath() );
    }
    else
    {
    this->project_path_lineedit_->setText( "" );
    }
    registerField( "projectPath", this->project_path_lineedit_ );
}

bool ExportInfoPage::validatePage()
{
  boost::filesystem::path project_path = 
    boost::filesystem::path( this->project_path_lineedit_->text().toStdString() ) / 
    boost::filesystem::path( this->project_name_lineedit_->text().toStdString() + 
      Project::GetDefaultProjectPathExtension() );

  if( boost::filesystem::exists( project_path ) )
  {
    QMessageBox::critical( this, 
      "A project with this name already exists!",
      "A project with this name already exists!\n"
      "You cannot export onto an existing project.\n"
      "Choose a different export location",
      QMessageBox::Ok );

    return false;
  }

  if( !boost::filesystem::exists( project_path.parent_path() ) )
  {
    this->warning_message_->setText( QString::fromUtf8( 
      "This location does not exist, please choose a valid location." ) );
    this->warning_message_->show();
    return false;
  }
  
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

ExportSummaryPage::ExportSummaryPage( QWidget *parent )
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
  
    QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget( this->description_ );
    layout->addWidget( this->project_name_ );
    layout->addWidget( this->project_path_ );
    this->setLayout( layout );
}

void ExportSummaryPage::initializePage()
{
    QString finishText = wizard()->buttonText( QWizard::FinishButton );
    finishText.remove('&');

    this->project_name_->setText( 
    QString::fromUtf8( "Project Name: " ) + field("projectName").toString() );
    this->project_path_->setText( 
    QString::fromUtf8( "Project Path: " ) + field("projectPath").toString() );
}

} // end namespace Seg3D
