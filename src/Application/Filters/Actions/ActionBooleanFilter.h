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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONBOOLEANFILTER_H
#define APPLICATION_TOOL_ACTIONS_ACTIONBOOLEANFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{
  
class ActionBooleanFilter : public Core::Action
{
CORE_ACTION( "BooleanFilter",
  "BooleanFilter <layerid> <expression> [b=layerid] [c=layerid] [d=layerid] [replace={true}]" );
  
  // -- Constructor/Destructor --
public:
  ActionBooleanFilter()
  {
    add_argument( layer_a_id_ );
    add_argument( expression_ );
    
    add_parameter( "b", layer_b_id_ );
    add_parameter( "c", layer_c_id_ );
    add_parameter( "d", layer_d_id_ );
    add_parameter( "replace", replace_, true );
    
    add_cachedhandle( layer_a_ );
    add_cachedhandle( layer_b_ );
    add_cachedhandle( layer_c_ );
    add_cachedhandle( layer_d_ ); 
  }
  
  virtual ~ActionBooleanFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:
  Core::ActionParameter< std::string > layer_a_id_;
  Core::ActionParameter< std::string > layer_b_id_;
  Core::ActionParameter< std::string > layer_c_id_;
  Core::ActionParameter< std::string > layer_d_id_;
  Core::ActionParameter< std::string > expression_;
  Core::ActionParameter< bool > replace_;
  
  Core::ActionCachedHandle< LayerHandle > layer_a_;
  Core::ActionCachedHandle< LayerHandle > layer_b_;
  Core::ActionCachedHandle< LayerHandle > layer_c_;
  Core::ActionCachedHandle< LayerHandle > layer_d_; 
  
  // -- Dispatch this action from the interface --
public:
    
  // CREATE:
  // Create the action, but do not post it.
  static Core::ActionHandle Create( std::string mask_a_id, std::string mask_b_id, 
    std::string mask_c_id, std::string mask_d_id,
    std::string expression, bool replace );
    
  // DISPATCH:
  // Create and dispatch action.
  static void Dispatch( std::string mask_a_id, std::string mask_b_id, 
    std::string mask_c_id, std::string mask_d_id,
    std::string expression, bool replace );
  
};
  
} // end namespace Seg3D

#endif
