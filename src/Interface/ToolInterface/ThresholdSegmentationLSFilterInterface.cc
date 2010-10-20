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

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Interface Includes
#include <Interface/ToolInterface/ThresholdSegmentationLSFilterInterface.h>
#include "ui_ThresholdSegmentationLSFilterInterface.h"

//Application Includes
#include <Application/Tools/ThresholdSegmentationLSFilter.h>

SCI_REGISTER_TOOLINTERFACE( Seg3D, ThresholdSegmentationLSFilterInterface )

namespace Seg3D
{

class ThresholdSegmentationLSFilterInterfacePrivate
{
public:
  Ui::ThresholdSegmentationLSFilterInterface ui_; 
};

// constructor
ThresholdSegmentationLSFilterInterface::ThresholdSegmentationLSFilterInterface() :
  private_( new ThresholdSegmentationLSFilterInterfacePrivate )
{
}

// destructor
ThresholdSegmentationLSFilterInterface::~ThresholdSegmentationLSFilterInterface()
{
}

// build the interface and connect it to the state manager
bool ThresholdSegmentationLSFilterInterface::build_widget( QFrame* frame )
{
  //Step 1 - build the Qt GUI Widget
  this->private_->ui_.setupUi( frame );
  this->private_->ui_.horizontalLayout_5->setAlignment( Qt::AlignHCenter );

  //Step 2 - get a pointer to the tool
  ToolHandle base_tool_ = tool();
  ThresholdSegmentationLSFilter* tool =
      dynamic_cast< ThresholdSegmentationLSFilter* > ( base_tool_.get() );
      
  //Step 3 - connect the gui to the tool through the QtBridge
  QtUtils::QtBridge::Connect( this->private_->ui_.iterations_, tool->iterations_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.upper_threshold_, tool->upper_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.lower_threshold_, tool->lower_threshold_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.curvature_, tool->curvature_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.edge_, tool->propagation_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.propagation_, tool->edge_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.replaceCheckBox, tool->replace_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.runFilterButton, 
    boost::bind( &Tool::execute, tool, Core::Interface::GetWidgetActionContext() ) );
  
  //Send a message to the log that we have finished with building the Segmentation Level Set Filter Interface
  CORE_LOG_DEBUG("Finished building a Segmentation Level Set Filter Interface");
  return ( true );

} // end build_widget
  
} // end namespace Seg3D
