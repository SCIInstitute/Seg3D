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

// Qt includes
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// Core indcludes
#include <Core/Utils/Log.h>

// Interface includes
#include <Interface/Application/SplashScreen.h>
#include <Interface/Application/LayerIOFunctions.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>
#include <Application/ProjectManager/Actions/ActionQuickOpen.h>

// Resources includes
#include <Resources/QtResources.h>

#include "ui_SplashScreen.h"

namespace Seg3D
{

class SplashScreenPrivate
{
public:
  Ui::SplashScreen ui_;

};

SplashScreen::SplashScreen( QWidget *parent ) :
  QtUtils::QtCustomDialog( parent ),
  private_( new SplashScreenPrivate )
{
  this->setAttribute( Qt::WA_DeleteOnClose, true );
  
  this->setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint );
  this->setModal( true );

  // Set up the private internals of the AppSplash class
  this->private_->ui_.setupUi( this );

  // Disable these since they arent being used yet.
  this->private_->ui_.load_recent_button_->setEnabled( false );
    
  this->populate_recent_projects();
    
  connect( this->private_->ui_.new_project_button_, SIGNAL( clicked() ), 
    this, SLOT( new_project() ) );
  
  connect( this->private_->ui_.existing_project_button_, SIGNAL( clicked() ), 
    this, SLOT( open_existing() ) );
  
  connect( this->private_->ui_.load_recent_button_, SIGNAL( clicked() ), 
    this, SLOT( open_recent() ) );

  connect( this->private_->ui_.quit_seg3d_button_, SIGNAL( clicked() ), 
    this, SLOT( quit() ) );
  
  connect( this->private_->ui_.quick_open_button_, SIGNAL( clicked() ), 
      this, SLOT( quick_open_file() ) );
  
  connect( this->private_->ui_.recent_project_listwidget_, SIGNAL( itemPressed( QListWidgetItem* ) ),
    this, SLOT( enable_load_recent_button( QListWidgetItem* ) ) );  

  connect( this->private_->ui_.recent_project_listwidget_, 
    SIGNAL( itemDoubleClicked ( QListWidgetItem* ) ),
    this, SLOT( open_recent() ) );  
  
  connect( this, SIGNAL( dialog_closed() ), this->parentWidget(), SLOT( close() ) );

  this->private_->ui_.new_project_button_->setFocus();
}

SplashScreen::~SplashScreen()
{
}
  
void SplashScreen::new_project()
{
  this->new_project_wizard_ = new ProjectWizard( this->parentWidget() );
  connect( this->new_project_wizard_, SIGNAL( canceled() ), this, SLOT( unhide() ) );
  this->new_project_wizard_->show();
  this->hide();
}

void SplashScreen::quit()
{
  reinterpret_cast<QWidget*>( this->parent() )->close();
}

void SplashScreen::unhide()
{
  this->show();
}
  
void SplashScreen::open_existing()
{
  boost::filesystem::path current_projects_path = complete( 
    boost::filesystem::path( ProjectManager::Instance()->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );

  boost::filesystem::path full_path = ( QFileDialog::getOpenFileName ( this, 
    tr( "Open Seg3D Project" ), QString::fromStdString( current_projects_path.string() ), 
    tr( "Seg3D Project File ( *.s3d )" ) ) ).toStdString(); 
  
  std::string path = full_path.parent_path().string();
  std::string file_name = full_path.filename();

  if( boost::filesystem::exists( full_path ) )
  {
    if ( ! ProjectManager::Instance()->check_if_file_is_valid_project( 
      ( full_path / ( full_path.leaf() + ".s3d" ) ) ) )
    {
      QMessageBox::critical( 0, 
        "Error reading project file",
        "Error reading project file:\n"
        "The project file was saved with newer version of Seg3D" );
      return;
    }   
  
    ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
    this->close();
  }
}
  
void SplashScreen::open_recent()
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

        if ( ! ProjectManager::Instance()->check_if_file_is_valid_project( 
          ( path / ( path.leaf() + ".s3d" ) ) ) )
        {
          QMessageBox::critical( 0, 
            "Error reading project file",
            "Error reading project file:\n"
            "The project file was saved with newer version of Seg3D" );
          return;
        }   
                
        if( boost::filesystem::exists( path ) )
        { 
          ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(),
            path.string() );
          break;
        }
      }
    }
  }
  this->close();
}

void SplashScreen::quick_open_file()
{
  ActionQuickOpen::Dispatch( Core::Interface::GetWidgetActionContext() );
  this->hide();
  LayerIOFunctions::ImportFiles( dynamic_cast< QMainWindow* >( this->parentWidget() ), "" );
  
}
  
void SplashScreen::populate_recent_projects()
{
  this->recent_project_list_ = ProjectManager::Instance()->
    recent_projects_state_->get();
  for( size_t i = 0; i < this->recent_project_list_.size(); ++i )
  {
    if( this->recent_project_list_[ i ] != "" )
    {
      QListWidgetItem *new_item;
      new_item = new QListWidgetItem( QString::fromStdString( ( Core::SplitString( 
        this->recent_project_list_[ i ], "|" ) )[ 1 ] ) );

      new_item->setToolTip( QString::fromUtf8( "This project was created on: " ) +
        QString::fromStdString( ( Core::SplitString( 
        this->recent_project_list_[ i ], "|" ) )[ 2 ] ) 
        + QString::fromUtf8( " and is located at: " ) 
        + QString::fromStdString( ( Core::SplitString( 
        this->recent_project_list_[ i ], "|" ) )[ 0 ] ) );

      this->private_->ui_.recent_project_listwidget_->addItem( new_item );

    }
  }
}
  
void SplashScreen::enable_load_recent_button( QListWidgetItem* not_used )
{
  this->private_->ui_.load_recent_button_->setEnabled( true );
}
  
} // end namespace Seg3D
