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

// Boost includes
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionExportProject.h>

// Interface includes
#include <Interface/AppProjectExportWizard/AppProjectExportWizard.h>


namespace Seg3D
{

AppProjectExportWizard::AppProjectExportWizard( const std::string& session_name, QWidget *parent ) :
    QWizard( parent )
{
    this->addPage( new ExportInfoPage );
    this->addPage( new ExportSummaryPage );

  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );

  this->session_name_ = QString::fromStdString( session_name );
  
  this->setWindowTitle( tr( "Export Project Wizard" ) );
}

AppProjectExportWizard::~AppProjectExportWizard()
{
}

void AppProjectExportWizard::accept()
{
  ActionExportProject::Dispatch( Core::Interface::GetWidgetActionContext(), 
    field( "projectPath" ).toString().toStdString(),
    field( "projectName" ).toString().toStdString(),
    this->session_name_.toStdString() );
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
    current_project_->project_name_state_->get() ) );

    this->project_path_label_ = new QLabel( "Project Path:" );
    this->project_path_lineedit_ = new QLineEdit;
    
    this->project_path_change_button_ = new QPushButton( "Choose Alternative Location" );
    connect( this->project_path_change_button_, SIGNAL( clicked() ), this, SLOT( set_path() ) );
  this->project_path_change_button_->setFocusPolicy( Qt::NoFocus );

    registerField( "projectName", this->project_name_lineedit_ );

    QGridLayout *layout = new QGridLayout;
    layout->addWidget( this->project_name_label_, 0, 0 );
    layout->addWidget( this->project_name_lineedit_, 0, 1 );
    layout->addWidget( this->project_path_label_, 1, 0 );
    layout->addWidget( this->project_path_lineedit_, 1, 1 );
    layout->addWidget( this->project_path_change_button_, 2, 1, 1, 2 );
    setLayout( layout );
}
  
void ExportInfoPage::initializePage()
{
  boost::filesystem::path desktop_path;
  Core::Application::Instance()->get_user_desktop_directory( desktop_path );
  this->project_path_lineedit_->setText( QString::fromStdString( desktop_path.string() ) );
  registerField( "projectPath", this->project_path_lineedit_ );
}
  
void ExportInfoPage::set_path()
{
    QDir project_directory_;
  QString path = QFileDialog::getExistingDirectory ( this, "Directory",
    this->project_path_lineedit_->text() );
  
    if ( path.isNull() == false )
    {
        project_directory_.setPath( path );
    }
    this->project_path_lineedit_->setText( project_directory_.canonicalPath() );
    registerField( "projectPath", this->project_path_lineedit_ );
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
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove('&');

    this->project_name_->setText( 
    QString::fromUtf8( "Project Name: " ) + field("projectName").toString() );
    this->project_path_->setText( 
    QString::fromUtf8( "Project Path: " ) + field("projectPath").toString() );
  
}

} // end namespace Seg3D
