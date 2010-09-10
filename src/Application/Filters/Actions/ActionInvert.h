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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONINVERT_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONINVERT_H

#include <Core/Action/Actions.h>

namespace Seg3D
{
  
class ActionInvert : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "Invert", "Invert the values of the data layer" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this tool needs to be run." )
  CORE_ACTION_KEY( "replace", "true", "Replace the old layer (true), or add an new layer (false)" )
)
  
  // -- Constructor/Destructor --
public:
  ActionInvert()
  {
    // Action arguments
    this->add_argument( this->layer_id_ );
    
    // Action options
    this->add_key( this->replace_ );
  }
  
  virtual ~ActionInvert() {}
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > layer_id_;
  Core::ActionParameter< bool > replace_;
  
public:
  static void Dispatch( Core::ActionContextHandle context, 
    std::string layer_id, bool replace );
};
  
} // end namespace Seg3D

#endif
