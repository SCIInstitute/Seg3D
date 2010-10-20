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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONTHRESHOLD_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONTHRESHOLD_H

#include <Core/Action/Actions.h>

namespace Seg3D
{

class ActionThreshold : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ThresholdTool", "Build a mask layer by thresholding a data layer." )
  CORE_ACTION_ARGUMENT( "layerid", "The ID of the data layer on which to run the tool." )
  CORE_ACTION_ARGUMENT( "lower_threshold", "The minimum value of the threshold range." )
  CORE_ACTION_ARGUMENT( "upper_threshold", "The maximum value of the threshold range." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  
  // -- Constructor/Destructor --
public:
  ActionThreshold();
  
  virtual ~ActionThreshold() {}
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > target_layer_;
  
  Core::ActionParameter< double > upper_threshold_;
  Core::ActionParameter< double > lower_threshold_;
  
  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, double lower_threshold,
    double upper_threshold );
          
};
  
} // end namespace Seg3D

#endif
