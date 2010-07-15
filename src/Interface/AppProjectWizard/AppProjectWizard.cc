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


// Interface includes
#include <Interface/AppProjectWizard/AppProjectWizard.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionNewProject.h>

namespace Seg3D
{

AppProjectWizard::AppProjectWizard( QWidget *parent ) :
    QWizard( parent )
{
    //this->addPage( new IntroPage );
    this->addPage( new ProjectInfoPage );
    this->addPage( new SummaryPage );
  
  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );
  
  this->setWindowTitle(tr("New Project Wizard"));
}

AppProjectWizard::~AppProjectWizard()
{
}

void AppProjectWizard::accept()
{
  // TODO: call the create new project action dispatcher with the settings
  
  ActionNewProject::Dispatch( Core::Interface::GetWidgetActionContext(),
    field("projectPath").toString().toStdString(),
    field("projectName").toString().toStdString() );
//  ProjectManager::Instance()->new_project( field("projectName").toString().toStdString(),
//    field("projectPath").toString().toStdString() );
    QDialog::accept();
}

IntroPage::IntroPage( QWidget *parent )
    : QWizardPage( parent )
{
    setTitle( "Introduction" );
    this->description_ = new QLabel( "This wizard will help you create a new Seg3D2 project. "
                          "You simply need to specify the project name and set a few "
                          "options to get going." );
    this->description_->setWordWrap( true );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget( this->description_ );
    setLayout( layout );
}

ProjectInfoPage::ProjectInfoPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Project Information" );
    this->setSubTitle( "Specify basic information about the project for which you "
                   "want to create." );

    this->project_name_label_ = new QLabel( "Project name:" );

  QString default_name_count = QString::number( ProjectManager::Instance()->
    default_project_name_counter_state_->get() );

  if( default_name_count == "0" )
    this->project_name_lineedit_ = new QLineEdit( "New Project" );
  else
  {
    this->project_name_lineedit_ = new QLineEdit();
    this->project_name_lineedit_->setText(  "New Project " + default_name_count );
  }
  
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
  
void ProjectInfoPage::initializePage()
{
  this->project_path_lineedit_->setText( QString::fromStdString( ProjectManager::Instance()->current_project_path_state_->get() ) );
  registerField( "projectPath", this->project_path_lineedit_ );
  
}
  

void ProjectInfoPage::set_path()
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

SummaryPage::SummaryPage( QWidget *parent )
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
  
  this->consolidate_ = new QLabel;
  this->consolidate_->setWordWrap( true );

    QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget( this->description_ );
    layout->addWidget( this->project_name_ );
    layout->addWidget( this->project_path_ );
  layout->addWidget( this->consolidate_ );
    this->setLayout( layout );

}

void SummaryPage::initializePage()
{
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove('&');

    this->project_name_->setText( QString::fromUtf8( "Project Name: " ) + field("projectName").toString() );
    this->project_path_->setText( QString::fromUtf8( "Project Path: " ) + field("projectPath").toString() );
  if( field( "autoConsolidate" ).toBool() )
    this->consolidate_->setText( QString::fromUtf8( "Project Files Setting: Consolidate files into project directory" ) );
  else
    this->consolidate_->setText( QString::fromUtf8( "Project Files Setting: Leave files in place." ) );

  
}

} // end namespace Seg3D
