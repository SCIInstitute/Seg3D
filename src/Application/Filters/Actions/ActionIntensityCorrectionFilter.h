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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONINTENSITYCORRECTIONFILTER_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONINTENSITYCORRECTIONFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{

class ActionIntensityCorrectionFilter : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "IntensityCorrection", "Correct for small gradients in intensity, "
    "due for instance, an inhomogenous sensitivity of image recording systems, such as MRI." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_KEY( "preserve_data_format", "true", "ITK filters run in floating point percision,"
  " this option will convert the result back into the original format." )
  CORE_ACTION_KEY( "replace", "true", "Replace the old layer (true), or add an new layer (false)." )
  CORE_ACTION_KEY( "order", "2", "Polynomial order that approximates the sensitivity field." )
  CORE_ACTION_KEY( "edge", "0.1", "The sensitivity to edges.")
)
  
  // -- Constructor/Destructor --
public:
  ActionIntensityCorrectionFilter()
  {
    // Action arguments
    this->add_argument( this->target_layer_ );
    
    // Action options
    this->add_key( this->preserve_data_format_ );
    this->add_key( this->replace_ );  
      
    this->add_key( this->order_ );
    this->add_key( this->edge_ );
  }
  
  virtual ~ActionIntensityCorrectionFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > target_layer_;
  Core::ActionParameter< bool > preserve_data_format_;
  Core::ActionParameter< bool > replace_; 
  Core::ActionParameter< int > order_;
  Core::ActionParameter< double > edge_;
  
  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, bool preserve_data_format, bool replace,
    int order, double edge );         
};
  
} // end namespace Seg3D

#endif
