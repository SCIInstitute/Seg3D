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

#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/MaskDataFilter.h>


namespace Seg3D {
  
  // Register the tool into the tool factory
  SCI_REGISTER_TOOL(MaskDataFilter)
  
  
  MaskDataFilter::MaskDataFilter(const std::string& toolid) :
  Tool(toolid)
  {
    // add default values for the the states
    add_state("target_layer",target_layer_state_,"<none>","<none>");
    add_state("mask_layer",mask_layer_state_,"<none>","<none>");
    add_state("replace_with",replace_with_state_,"<none>","<none>");
    add_state("replace",replace_state_,false);
    
    // Add constaints, so that when the state changes the right ranges of 
    // parameters are selected
    target_layer_state_->value_changed_signal_.connect(boost::bind(&MaskDataFilter::target_constraint,this,_1));
    mask_layer_state_->value_changed_signal_.connect(boost::bind(&MaskDataFilter::target_constraint,this,_1));
    replace_with_state_->value_changed_signal_.connect(boost::bind(&MaskDataFilter::target_constraint,this,_1));
    
  }
  
  void
  MaskDataFilter::target_constraint(std::string layerid)
  {
  }
  
  MaskDataFilter::~MaskDataFilter()
  {
    disconnect_all();
  }
  
  void
  MaskDataFilter::activate()
  {
  }
  
  void
  MaskDataFilter::deactivate()
  {
  }
  
} // end namespace Seg3D


