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
#include <Application/Tools/ThresholdSegmentationLSFilter.h>


namespace Seg3D {
  
  // Register the tool into the tool factory
  SCI_REGISTER_TOOL(ThresholdSegmentationLSFilter)
  
  
  ThresholdSegmentationLSFilter::ThresholdSegmentationLSFilter(const std::string& toolid) :
  Tool(toolid)
  {
    // add default values for the the states
    add_state("target_layer",target_layer_,"<none>","<none>");
    add_state("mask_layer",mask_layer_,"<none>","<none>");
    add_state("iterations",iterations_,1,100,1,2);
    add_state("upper_threshold",upper_threshold_,1,100,1,2);
    add_state("lower_threshold",lower_threshold_,1,100,1,2);
    add_state("curvature",curvature_,1,100,1,2);
    add_state("propagation",propagation_,1,100,1,2);
    add_state("edge",edge_,1,100,1,2);
    add_state("replace",replace_,false);
    
    // Add constaints, so that when the state changes the right ranges of 
    // parameters are selected
    target_layer_->value_changed_signal.connect(boost::bind(&ThresholdSegmentationLSFilter::target_constraint,this,_1));
    mask_layer_->value_changed_signal.connect(boost::bind(&ThresholdSegmentationLSFilter::target_constraint,this,_1));
    
    
  }
  
  void
  ThresholdSegmentationLSFilter::target_constraint(std::string layerid)
  {
  }
  
  ThresholdSegmentationLSFilter::~ThresholdSegmentationLSFilter()
  {
    disconnect_all();
  }
  
  void
  ThresholdSegmentationLSFilter::activate()
  {
  }
  
  void
  ThresholdSegmentationLSFilter::deactivate()
  {
  }
  
} // end namespace Seg3D


