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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONCONNECTEDCOMPONENTSIZEFILTER_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONCONNECTEDCOMPONENTSIZEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Geometry/Point.h>

namespace Seg3D
{

class ActionConnectedComponentSizeFilter : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ConnectedComponentSizeFilter", "Segment a mask into connected regions and order"
    " them by the size of the volume of the component.")
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_KEY( "log_scale", "false", "Whether the log of the data should be used." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionConnectedComponentSizeFilter()
  {
    // Action arguments
    this->add_argument( this->target_layer_ );
  }
  
  virtual ~ActionConnectedComponentSizeFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > target_layer_;
  Core::ActionParameter< bool > log_scale_;

  // -- Dispatch this action from the interface --
public:
  // DISPATCH:
  // Create and dispatch action that run the filter
  static void Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, bool log_scale );
};
  
} // end namespace Seg3D

#endif
