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
#include <Interface/QtInterface/QtBridge.h>

//Qt Gui Includes
#include <Interface/ToolInterface/GradientMagnitudeFilterInterface.h>
#include "ui_GradientMagnitudeFilterInterface.h"

//Application Includes
#include <Application/Tools/GradientMagnitudeFilter.h>

namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(GradientMagnitudeFilterInterface)
  
class GradientMagnitudeFilterInterfacePrivate {
public:
  Ui::GradientMagnitudeFilterInterface ui_;
};
  
  // constructor
  GradientMagnitudeFilterInterface::GradientMagnitudeFilterInterface() :
  private_(new GradientMagnitudeFilterInterfacePrivate)
  { }
  
  // destructor
  GradientMagnitudeFilterInterface::~GradientMagnitudeFilterInterface()
  { }
  
  // build the interface and connect it to the state manager
  bool
  GradientMagnitudeFilterInterface::build_widget(QFrame* frame)
  {
    //Step 1 - build the Qt GUI Widget
    private_->ui_.setupUi(frame);
    
    //Step 2 - get a pointer to the tool
    ToolHandle base_tool_ = tool();
    GradientMagnitudeFilter* tool = dynamic_cast<GradientMagnitudeFilter*>(base_tool_.get());
    
    //Step 3 - connect the gui to the tool through the QtBridge
    QtBridge::connect(private_->ui_.targetComboBox, tool->target_layer_);
    QtBridge::connect(private_->ui_.replaceCheckBox,tool->replace_);
    
    //Send a message to the log that we have finised with building the Gradient Magnitude Filter Interface
    SCI_LOG_DEBUG("Finished building a Gradient Magnitude Filter Interface");
    return (true);
  } // end build_widget
  
} // end namespace Seg3D