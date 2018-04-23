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

// Core includes
#include <Core/Interface/Interface.h>

//Qt includes
#include <QFileDialog>

// QtGui includes
#include "ui_PointSetFilterInterface.h"

//Application Includes
#include <Application/Tools/PointSetFilter.h>
#include <Application/LayerIO/Actions/ActionExportMatrix.h>
#include <Application/ProjectManager/ProjectManager.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/ToolInterface/PointSetFilterInterface.h>


SCI_REGISTER_TOOLINTERFACE( Seg3D, PointSetFilterInterface )

namespace Seg3D
{

class PointSetFilterInterfacePrivate
{
public:
  Ui::PointSetFilterInterface ui_;
  PointSetFilterInterface* interface_;

  void export_matrix_to_file() const;
};

void PointSetFilterInterfacePrivate::export_matrix_to_file() const
{
	Core::StateEngine::lock_type lock(Core::StateEngine::GetMutex());

	PointSetFilter* tool = dynamic_cast< PointSetFilter* > (this->interface_->tool().get());

	QString filename;
	boost::filesystem::path current_folder = ProjectManager::Instance()->get_current_file_folder();
	std::string file_selector = Core::StringToUpper("Text File ") + "(*.txt)";

	filename = QFileDialog::getSaveFileName(this->interface_,
		"Export Matrix As...",
		current_folder.string().c_str(),
		QString::fromStdString(file_selector));
	if (!filename.isNull() && !filename.isEmpty())
	{
		QFileInfo file(filename);
		if (file.suffix().isEmpty())
		{
			filename += ".txt";
		}

		//Get matrix
		ActionExportMatrix::Dispatch(Core::Interface::GetWidgetActionContext(),filename.toStdString(),
			tool->complete_transform_matrix_state_->get(),4,4);
	
	}
}

// constructor
PointSetFilterInterface::PointSetFilterInterface() :
  private_( new PointSetFilterInterfacePrivate )
{
	this->private_->interface_ = this;
}

// destructor
PointSetFilterInterface::~PointSetFilterInterface()
{
	this->disconnect_all();
}

// build the interface and connect it to the state manager
bool PointSetFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );

  //Step 2 - get a pointer to the tool
  PointSetFilter* tool = dynamic_cast< PointSetFilter* > ( this->tool().get() );
  
    //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_layer_, 
    tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, 
    tool->use_active_layer_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.mask_layer_, tool->mask_state_ );
  
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton,tool->valid_target_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.message_alert_, tool->valid_target_state_, true );
  
  QtUtils::QtBridge::Enable( this->private_->ui_.target_layer_, 
    tool->use_active_layer_state_, true );

  QtUtils::QtBridge::Connect( this->private_->ui_.iterations_, 
    tool->iterations_state_ );

  this->private_->ui_.iterations_->set_description( "Iterations" );

  QtUtils::QtBridge::Connect( this->private_->ui_.layer_list_, tool->target_layers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.runFilterButton, boost::bind(
    &PointSetFilter::registration, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.runFilterButton_2, boost::bind(
    &PointSetFilter::apply, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton_2, tool->registration_ready_state_ );

  QtUtils::QtBridge::Connect(this->private_->ui_.save_matrix_button, boost::bind(
	  &PointSetFilterInterfacePrivate::export_matrix_to_file, this->private_));

  QtUtils::QtBridge::Enable(this->private_->ui_.save_matrix_button, tool->registration_ready_state_);

  //this->private_->ui_.save_matrix_button->setEnabled(tool->complete_transform_matrix_state_->size() > 0);

  QtUtils::QtBridge::Connect( this->private_->ui_.label_rx_, tool->rotation_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.label_ry_, tool->rotation_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.label_rz_, tool->rotation_state_[ 2 ] );

  QtUtils::QtBridge::Enable( this->private_->ui_.label_rx_, tool->registration_ready_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.label_ry_, tool->registration_ready_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.label_rz_, tool->registration_ready_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.label_tx_, tool->translation_state_[ 0 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.label_ty_, tool->translation_state_[ 1 ] );
  QtUtils::QtBridge::Connect( this->private_->ui_.label_tz_, tool->translation_state_[ 2 ] );

  QtUtils::QtBridge::Enable( this->private_->ui_.label_tx_, tool->registration_ready_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.label_ty_, tool->registration_ready_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.label_tz_, tool->registration_ready_state_ );

  QtUtils::QtBridge::Connect(this->private_->ui_.transformationMatrix, tool->complete_transform_matrix_state_,4,4);

  return true;      
}
  
} // end namespace Seg3D
