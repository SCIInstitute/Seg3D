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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONCONFIDENCECONNECTED_H
#define APPLICATION_TOOL_ACTIONS_ACTIONCONFIDENCECONNECTED_H

#include <Application/Action/Actions.h>
#include <Application/Interface/Interface.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{
  
class ActionConfidenceConnected : public Action
{
CORE_ACTION( "ConfidenceConnected", "Run Confidence Connected Filter on: <name>" );
  
  // -- Constructor/Destructor --
public:
  ActionConfidenceConnected()
  {
  }
  
  virtual ~ActionConfidenceConnected()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );
  
  // -- Action parameters --
private:
  // Layer_handle that is requested
  std::string layer_alias_;
  int iterations_;
  int multiplier_;
  
  // -- Dispatch this action from the interface --
public:
    
  // DISPATCH
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( std::string layer_alias, int iterations, int multiplier );
  
};
  
} // end namespace Seg3D

#endif
