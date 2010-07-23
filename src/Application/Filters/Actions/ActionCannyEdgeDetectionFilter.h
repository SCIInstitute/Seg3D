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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONCANNYEDGEDETECTIONFILTER_H
#define APPLICATION_TOOL_ACTIONS_ACTIONCANNYEDGEDETECTIONFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{
  
class ActionCannyEdgeDetectionFilter : public Core::Action
{
CORE_ACTION_XML( 
  CORE_ACTION_TYPE( "CannyEdgeDetectionFilter", "ITK filter that detects where edges are in a data layer." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_KEY( "variance", "2.0", "The spatial variance." )
  CORE_ACTION_KEY( "max_error", "1.0", "The maximum error." )
  CORE_ACTION_KEY( "threshold", "1.0", "The threshold." )
  CORE_ACTION_KEY( "replace", "true", "Replace the old layer (true), or add an new layer (false)" )
)  
  
  // -- Constructor/Destructor --
public:
  ActionCannyEdgeDetectionFilter()
  {
    add_argument( this->layer_id_ );
    
    add_key( this->variance_ );
    add_key( this->max_error_ );
    add_key( this->threshold_ );
    add_key( this->replace_ );
  
    add_cachedhandle( this->layer_ ); 
  }
  
  virtual ~ActionCannyEdgeDetectionFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > layer_id_;
  Core::ActionParameter< double > variance_;
  Core::ActionParameter< double > max_error_;
  Core::ActionParameter< double > threshold_;
  Core::ActionParameter< bool > replace_;
  
  Core::ActionCachedHandle< LayerHandle > layer_;
  
  // -- Dispatch this action from the interface --
public:
    
  // CREATE:  
  // Create the action, but do not dispatch it
  static Core::ActionHandle Create( std::string layer_id, double variance, double max_error, 
    double threshold, bool replace );   
    
  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, 
    std::string layer_id, double variance, double max_error, 
    double threshold, bool replace );
  
};
  
} // end namespace Seg3D

#endif
