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


//Interface Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <Interface/ToolInterface/CustomWidgets/TargetComboBox.h>

//Qt Gui Includes
#include <Interface/ToolInterface/PolylineToolInterface.h>
#include "ui_PolylineToolInterface.h"

//Application Includes
#include <Application/Tools/PolylineTool.h>
//#include <Application/Filters/Actions/ActionPolyline.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, PolylineToolInterface )

namespace Seg3D
{

class PolylineToolInterfacePrivate
{
public:
  Ui::PolylineToolInterface ui_;
};

// constructor
PolylineToolInterface::PolylineToolInterface() :
  private_( new PolylineToolInterfacePrivate )
{
}

// destructor 
PolylineToolInterface::~PolylineToolInterface()
{
}

// build the interface and connect it to the state manager
bool PolylineToolInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  
  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  PolylineTool* tool = dynamic_cast< PolylineTool* > ( base_tool_.get() );

  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.target_mask_, tool->target_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.use_active_layer_, tool->use_active_layer_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.fill_button_, boost::bind(
    &PolylineTool::fill, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.erase_button_, boost::bind(
    &PolylineTool::erase, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.clear_button_, boost::bind(
    &PolylineTool::reset, tool, Core::Interface::GetWidgetActionContext() ) );
  QtUtils::QtBridge::Enable( this->private_->ui_.fill_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.erase_button_, tool->valid_target_state_ );
  QtUtils::QtBridge::Enable( this->private_->ui_.target_mask_, 
    tool->use_active_layer_state_, true );

  //Send a message to the log that we have finised with building the Polyline Tool Interface
  CORE_LOG_MESSAGE("Finished building a Polyline Tool Interface");

  return ( true );
} // end build_widget

} // end namespace Seg3D

