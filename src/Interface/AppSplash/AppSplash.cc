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

// Core indcludes
#include <Core/Utils/Log.h>

// Interface includes
#include <Interface/AppSplash/AppSplash.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>

// Resources includes
#include <Resources/QtResources.h>

#include "ui_AppSplash.h"

namespace Seg3D
{

class AppSplashPrivate
{
public:

  Ui::AppSplash ui_;
  bool ready_to_quit_;

};

AppSplash::AppSplash( QWidget *parent ) :
  QDialog( parent ),
  private_( new AppSplashPrivate )
{
  InitQtResources();
  setAttribute( Qt::WA_DeleteOnClose );
  // Set up the private internals of the AppSplash class
  this->private_->ui_.setupUi( this );
  
  // Disable these since they arent being used yet.
  this->private_->ui_.load_recent_button_->setEnabled( false );
  //this->private_->ui_.existing_project_button_->setEnabled( false );
  
  this->populate_recent_projects();
    
  connect( this->private_->ui_.new_project_button_, SIGNAL( clicked() ), 
    this, SLOT( new_project() ) );
  
  connect( this->private_->ui_.existing_project_button_, SIGNAL( clicked() ), 
    this, SLOT( open_existing() ) );
  
  connect( this->private_->ui_.load_recent_button_, SIGNAL( clicked() ), 
    this, SLOT( open_recent() ) );
  
  connect( this->private_->ui_.recent_project_listwidget_, SIGNAL( itemPressed( QListWidgetItem* ) ),
    this, SLOT( enable_load_recent_button( QListWidgetItem* ) ) );  

  connect( this->private_->ui_.recent_project_listwidget_, 
    SIGNAL( itemDoubleClicked ( QListWidgetItem* ) ),
    this, SLOT( call_open_recent( QListWidgetItem* ) ) ); 
  
  connect( this, SIGNAL( dialog_closed() ), this->parentWidget(), SLOT( close() ) );
  
  this->private_->ready_to_quit_ = true;

}

AppSplash::~AppSplash()
{
}
  
void AppSplash::new_project()
{
  this->new_project_wizard_ = new AppProjectWizard( this->parentWidget() );
  this->new_project_wizard_->show();
  // now we just close the splash window
  this->private_->ready_to_quit_ = false;
  this->close();
}
  
void AppSplash::open_existing()
{
  //QDir project_directory_;

  boost::filesystem::path current_projects_path = complete( 
    boost::filesystem::path( ProjectManager::Instance()->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );

  boost::filesystem::path full_path = ( QFileDialog::getOpenFileName ( this, 
    tr( "Open Seg3D Project" ), QString::fromStdString( current_projects_path.string() ), 
    tr( "Text files( *.s3d )" ) ) ).toStdString(); 
  
  std::string path = full_path.parent_path().string();
  std::string file_name = full_path.filename();

  ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );//, file_name );
  
  this->private_->ready_to_quit_ = false;
  this->close();
}
  
void AppSplash::open_recent()
{
  if( this->private_->ui_.recent_project_listwidget_->currentItem()->text() != "" )
  {
    for( size_t i = 0; i < this->recent_project_list_.size(); ++i )
    {
      std::string recent_project = ( Core::SplitString( 
        this->recent_project_list_[ i ], "|" ) )[ 1 ];
      std::string list_item = ( this->private_->ui_.recent_project_listwidget_->
        currentItem()->text() ).toStdString();

      list_item = Core::SplitString( list_item, "  -  " )[ 0 ];
      
      if( recent_project == list_item )
      {
        std::vector<std::string> project_entry = 
          Core::SplitString( this->recent_project_list_[ i ], "|" );
        boost::filesystem::path path = project_entry[ 0 ];
        path = path / project_entry[ 1 ];
        
        ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(),
          path.string() );//, project_entry[ 1 ] ); 
        break;
      }
    }
  }
  this->private_->ready_to_quit_ = false;
  this->close();
}

void AppSplash::call_open_recent( QListWidgetItem* item )
{
  this->open_recent();
}
  
void AppSplash::populate_recent_projects()
{
  this->recent_project_list_ = ProjectManager::Instance()->
    recent_projects_state_->get();
  for( size_t i = 0; i < this->recent_project_list_.size(); ++i )
  {
    if( this->recent_project_list_[ i ] != "" )
    {
      this->private_->ui_.recent_project_listwidget_->addItem( 
        QString::fromStdString( ( Core::SplitString( 
        this->recent_project_list_[ i ], "|" ) )[ 1 ] ) + "  -  " +
        QString::fromStdString( ( Core::SplitString( 
        this->recent_project_list_[ i ], "|" ) )[ 2 ] ) );
      
    }
  }
}
  
void AppSplash::enable_load_recent_button( QListWidgetItem* not_used )
{
  this->private_->ui_.load_recent_button_->setEnabled( true );
}
  
void AppSplash::closeEvent( QCloseEvent *event )
{
  if( this->private_->ready_to_quit_ )
  {
    Q_EMIT dialog_closed();
  }
  event->accept();
}

  
} // end namespace Seg3D
