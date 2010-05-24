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

// Core indcludes
#include <Core/Utils/Log.h>

// Interface includes
#include <Interface/AppSplash/AppSplash.h>


// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include "ui_AppSplash.h"

namespace Seg3D
{

class AppSplashPrivate
{
public:

  Ui::AppSplash ui_;

};

AppSplash::AppSplash( QWidget *parent ) :
  QDialog( parent ),
  private_( new AppSplashPrivate )
{
  
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

}

AppSplash::~AppSplash()
{
}
  
void AppSplash::new_project()
{
  this->new_project_wizard_ = new AppProjectWizard( this->parentWidget() );
  this->new_project_wizard_->show();
  // now we just close the splash window
  this->close();
}
  
void AppSplash::open_existing()
{
  //QDir project_directory_;
//  QString path = QFileDialog::getOpenFileName( this, "Open Seg3D2 Project",
//    QString::fromStdString( ProjectManager::Instance()->current_project_path_state_->get() ) ); 
  this->close();
}
  
void AppSplash::open_recent()
{
  
}
  
void AppSplash::populate_recent_projects()
{
  std::vector< std::string > temp_projects_vector = ProjectManager::Instance()->
    recent_projects_state_->get();
  for( size_t i = 0; i < temp_projects_vector.size(); ++i )
  {
    if (temp_projects_vector[ i ] != "")
    {
      this->private_->ui_.recent_project_list_->addItem( QString::fromStdString( 
        ( Core::SplitString( temp_projects_vector[ i ], "|" ) )[ 1 ] ) );
    }
  }
}

  
} // end namespace Seg3D
