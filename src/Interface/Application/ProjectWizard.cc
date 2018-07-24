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

// Qt includes
#include <QVariant>
#include <QGridLayout>
#include <QMessageBox>

// Application includs
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionNewProject.h>
#include <Application/PreferencesManager/PreferencesManager.h>

// Interface includes
#include <Interface/Application/ProjectWizard.h>

namespace Seg3D
{

ProjectWizard::ProjectWizard( QWidget *parent ) :
  QWizard( parent ),
  path_to_delete_( "" )
{
  this->addPage( new ProjectInfoPage );
  this->addPage( new SummaryPage );
  
  this->setWizardStyle (QWizard::MacStyle );
  this->setPixmap( QWizard::BackgroundPixmap, QPixmap( QString::fromUtf8( 
    ":/Images/Symbol.png" ) ) );
    
  connect( this->page( 0 ), SIGNAL( need_to_set_delete_path( QString ) ), this, 
    SLOT( set_delete_path( QString ) ) );
  
  this->setWindowTitle( tr( "New Project Wizard" ) );
}

ProjectWizard::~ProjectWizard()
{
}

void ProjectWizard::set_delete_path( QString path )
{
  this->path_to_delete_ = path.toStdString();
}

void ProjectWizard::accept()
{
  if( this->path_to_delete_ != "" )
  {
    try
    { 
      boost::filesystem::remove_all( boost::filesystem::path( this->path_to_delete_ ) );
    }
    catch( boost::filesystem::filesystem_error& )
    { 
      CORE_LOG_ERROR( "Couldn't remove directory '" + this->path_to_delete_ + "'." );
      this->reject();
      return;
    } 
  }

  ActionNewProject::Dispatch( Core::Interface::GetWidgetActionContext(),
    field( "projectPath" ).toString().toStdString(),
    field( "projectName" ).toString().toStdString() );
    QDialog::accept();

    Q_EMIT this->finished();
}

void ProjectWizard::reject()
{
  QDialog::reject();
  Q_EMIT this->canceled();
}

ProjectInfoPage::ProjectInfoPage( QWidget *parent )
    : QWizardPage( parent )
{
    this->setTitle( "Project Information" );
    this->setSubTitle( "Specify basic information about the project which you "
                   "want to create." );

    this->project_name_label_ = new QLabel( "Project Name:" );


  QString default_name_count;
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
    default_name_count = QString::number( ProjectManager::Instance()->
      default_project_name_counter_state_->get() );
  }
  
  if( default_name_count == "0" )
    this->project_name_lineedit_ = new QLineEdit( "New Project" );
  else
  {
    this->project_name_lineedit_ = new QLineEdit();
    this->project_name_lineedit_->setText( "New Project " + default_name_count );
  }
  
    this->project_path_label_ = new QLabel( "Project Path:" );
    this->project_path_lineedit_ = new QLineEdit();
    
    this->project_path_change_button_ = new QPushButton( "Choose Alternative Location" );
    connect( this->project_path_change_button_, SIGNAL( clicked() ), this, SLOT( set_path() ) );
    this->project_path_change_button_->setFocusPolicy( Qt::NoFocus );

    this->registerField( "projectName", this->project_name_lineedit_ );
    
  this->warning_message_ = new QLabel( QString::fromUtf8( 
    "This location does not exist, please choose a valid location." ) );
  this->warning_message_->setObjectName( QString::fromUtf8( "warning_message_" ) );
  this->warning_message_->setWordWrap( true );
  this->warning_message_->setStyleSheet(QString::fromUtf8( 
    "QLabel#warning_message_{ color: red; } " ) );
  this->warning_message_->hide();

    QGridLayout *layout = new QGridLayout;
    layout->addWidget( this->project_name_label_, 0, 0 );
    layout->addWidget( this->project_name_lineedit_, 0, 1 );
    layout->addWidget( this->project_path_label_, 1, 0 );
    layout->addWidget( this->project_path_lineedit_, 1, 1 );
    layout->addWidget( this->project_path_change_button_, 2, 1, 1, 2 );
    layout->addWidget( this->warning_message_, 3, 1, 1, 2 );
    this->setLayout( layout );

}
  
void ProjectInfoPage::initializePage()
{
  this->project_path_lineedit_->setText( QString::fromStdString( 
    ProjectManager::Instance()->get_current_project_folder().string() ) );
  this->registerField( "projectPath", this->project_path_lineedit_ );
}
  

void ProjectInfoPage::set_path()
{
    //QDir converts the empty QString "" to the current directory
    this->warning_message_->hide();
    
    QString path_name;
    QFileDialog *dialog = new QFileDialog( this, tr( "Choose Save Directory..." ));
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    dialog->setOption(QFileDialog::ShowDirsOnly);
    dialog->setOption(QFileDialog::DontResolveSymlinks);
    
    dialog->open( this, SLOT( set_name( const QString& ) ) );
}
    
void ProjectInfoPage::set_name(const QString& name)
{
  if(!name.isEmpty())
  {
    QDir project_directory_ = QDir(name);
        
    if( project_directory_.exists() )
    {
      this->project_path_lineedit_->setText( project_directory_.canonicalPath() );
      this->most_recent_path_ = &project_directory_;
    }
  }
}

bool ProjectInfoPage::validatePage()
{
  boost::filesystem::path project_path = 
    boost::filesystem::path( this->project_path_lineedit_->text().toStdString() ) / 
    boost::filesystem::path( this->project_name_lineedit_->text().toStdString() +
      Project::GetDefaultProjectPathExtension()  );
    
  if( boost::filesystem::exists( project_path ) )
  {
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
  
  // Check whether the location has a valid parent path
  if( !boost::filesystem::exists( project_path.parent_path() ) )
  {
    this->warning_message_->setText( QString::fromUtf8( 
      "This location does not exist, please choose a valid location." ) );
    this->warning_message_->show();
    return false;
  }

  // NOTE: Try to make the directory, so we know that we can write to this location.
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
  
  try 
  {
    // Remove the directory we just made
    boost::filesystem::remove( project_path );
  }
  catch( ... )
  {
    std::string error = "Could not remove file '" + project_path.string() + "'.";
    CORE_LOG_ERROR( error );
  }
  
  this->warning_message_->hide();

  return true;
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

    this->project_location_ = new QLabel;
    this->project_location_->setWordWrap( true );
  
    QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget( this->description_ );
    layout->addWidget( this->project_name_ );
    layout->addWidget( this->project_location_ );
    this->setLayout( layout );

}

void SummaryPage::initializePage()
{
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove('&');

    this->project_name_->setText( QString::fromUtf8( "Project Name: " ) +
    field( "projectName" ).toString() );
    this->project_location_->setText( QString::fromUtf8( "Project Path: " ) + 
    field( "projectPath" ).toString() );
}

} // end namespace Seg3D
