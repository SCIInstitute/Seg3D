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

//Boost Includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core includes
#include <Core/Interface/Interface.h>

// QtGui includes
#include "ui_ImplicitModelToolInterface.h"
#include <QButtonGroup>

//Application Includes
#include <Application/Tools/ImplicitModelTool.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

//Interface Includes
#include <Interface/ToolInterface/ImplicitModelToolInterface.h>


SCI_REGISTER_TOOLINTERFACE( Seg3D, ImplicitModelToolInterface )

namespace Seg3D
{

class ImplicitModelToolInterfacePrivate
{
public:
	Ui::ImplicitModelToolInterface ui_;	
};

// constructor
ImplicitModelToolInterface::ImplicitModelToolInterface() :
	private_( new ImplicitModelToolInterfacePrivate )
{
}

// destructor
ImplicitModelToolInterface::~ImplicitModelToolInterface()
{
}

// build the interface and connect it to the state manager
bool ImplicitModelToolInterface::build_widget( QFrame* frame )
{
	//Step 1 - build the Qt GUI Widget
	this->private_->ui_.setupUi( frame );

	//Step 2 - get a pointer to the tool
	ImplicitModelTool* tool = dynamic_cast< ImplicitModelTool* > ( this->tool().get() );
	    
	//Step 3 - connect the gui to the tool through the QtBridge
	QtUtils::QtBridge::Connect( this->private_->ui_.targetLayer_, tool->target_layer_state_ );
	QtUtils::QtBridge::Connect( this->private_->ui_.useActiveLayer_, tool->use_active_layer_state_ );
	
	QtUtils::QtBridge::Connect( this->private_->ui_.normalOffsetRange_, tool->normalOffset_state_ );
	QtUtils::QtBridge::Connect( this->private_->ui_.kernel_, tool->kernel_state_ );

  QButtonGroup* convex_hull_group = new QButtonGroup( this );
  convex_hull_group->addButton( this->private_->ui_.convex_hull_2D_ );
  convex_hull_group->addButton( this->private_->ui_.convex_hull_3D_ );

  QtUtils::QtBridge::Connect( convex_hull_group, tool->convex_hull_selection_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.invert_seed_order_, tool->invert_seed_order_state_ );

	QtUtils::QtBridge::Connect( this->private_->ui_.runFilterButton_, boost::bind(
		&Tool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
	QtUtils::QtBridge::Connect( this->private_->ui_.clearSeedsButton_, boost::bind(
   &SeedPointsTool::clear, tool, Core::Interface::GetWidgetActionContext() ) );

	QtUtils::QtBridge::Show( this->private_->ui_.messageAlert_, tool->valid_target_state_, true );

	QtUtils::QtBridge::Enable( this->private_->ui_.runFilterButton_, tool->valid_target_state_ );
	QtUtils::QtBridge::Enable( this->private_->ui_.targetLayer_, tool->use_active_layer_state_, true );

	this->private_->ui_.normalOffsetRange_->set_description( "Normal Offset" );
  this->private_->ui_.bbox_size_x_->set_description( "Bounding Box X" );
  this->private_->ui_.bbox_size_y_->set_description( "Bounding Box Y" );
  this->private_->ui_.bbox_size_z_->set_description( "Bounding Box Z" );

	boost::function< bool () > condition = boost::lambda::bind( &Core::StateLabeledOption::get,
    tool->target_layer_state_.get() ) != Tool::NONE_OPTION_C;

  QtUtils::QtBridge::Enable( this->private_->ui_.normalOffsetRange_, tool->target_layer_state_, condition );

  QtUtils::QtBridge::Enable( this->private_->ui_.bbox_size_x_, tool->disabled_widget_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.bbox_size_y_, tool->disabled_widget_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.bbox_size_z_, tool->disabled_widget_state_ );

	return true;
}
	
} // end namespace Seg3D
