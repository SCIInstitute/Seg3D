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
#include <Interface/ToolInterface/ConnectedComponentFilterInterface.h>
#include "ui_ConnectedComponentFilterInterface.h"

//Application Includes
#include <Application/Tools/ConnectedComponentFilter.h>

namespace Seg3D {
  
  SCI_REGISTER_TOOLINTERFACE(ConnectedComponentFilterInterface)
  
class ConnectedComponentFilterInterfacePrivate {
public:
  Ui::ConnectedComponentFilterInterface ui_;
};

  // constructor
  ConnectedComponentFilterInterface::ConnectedComponentFilterInterface() :
  private_(new ConnectedComponentFilterInterfacePrivate)
  { }
  
  // destructor
  ConnectedComponentFilterInterface::~ConnectedComponentFilterInterface()
  { }
  
   // build the interface and connect it to the state manager
  bool
  ConnectedComponentFilterInterface::build_widget(QFrame* frame)
  {
    //Step 1 - build the Qt GUI Widget
    private_->ui_.setupUi(frame);

    //Step 2 - get a pointer to the tool
    ToolHandle base_tool_ = tool();
    ConnectedComponentFilter* tool = dynamic_cast<ConnectedComponentFilter*>(base_tool_.get());
    
    //Step 3 - connect the gui to the tool through the QtBridge
    QtBridge::connect(private_->ui_.targetComboBox, tool->target_layer_);
    
    //Send a message to the log that we have finised with building the Connected Component Filter Interface
    SCI_LOG_DEBUG("Finished building a Connected Component Filter Interface");
    return (true);
    
  } // end build_widget
  
} // namespace Seg3D