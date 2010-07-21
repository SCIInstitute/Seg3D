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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONBINARYDILATEERODEFILTER_H
#define APPLICATION_TOOL_ACTIONS_ACTIONBINARYDILATEERODEFILTER_H

#include <Core/Action/Actions.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{
  
class ActionBinaryDilateErodeFilter : public Core::Action
{
CORE_ACTION( "BinaryDialateErodeFilter|layerid|dilate=0|erode=0|replace=true" ); 

  // -- Constructor/Destructor --
public:
  ActionBinaryDilateErodeFilter()
  {
    add_argument( layer_id_ );
    
    add_parameter( "dilate", dilate_, 0 );
    add_parameter( "erode", erode_, 0 );
    add_parameter( "replace", replace_, true );
    
    add_cachedhandle( layer_ );
  }
  
  virtual ~ActionBinaryDilateErodeFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:
  Core::ActionParameter< std::string > layer_id_;
  Core::ActionParameter< int > dilate_;
  Core::ActionParameter< int > erode_;
  Core::ActionParameter< bool > replace_;
  
  Core::ActionCachedHandle< LayerHandle > layer_;

  // -- Dispatch this action from the interface --
public:

  // CREATE:
  // Create the action, but do not post it.
  static Core::ActionHandle Create( std::string layer_id, int dilate, int erode, bool replace );
      
  // DISPATCH:
  // Create and dispatch action.
  static void Dispatch( Core::ActionContextHandle context, std::string layer_id, 
    int dilate, int erode, bool replace );
  
};
  
} // end namespace Seg3D

#endif
