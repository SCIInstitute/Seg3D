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


#include <Application/LayerManager/LayerManager.h>
#include <Application/Filters/Actions/ActionCannyEdgeDetection.h>

namespace Seg3D
{
  
// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( CannyEdgeDetection );

bool ActionCannyEdgeDetection::validate( ActionContextHandle& context )
{
  if( !( StateEngine::Instance()->is_statealias( this->layer_alias_ ) ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_alias_ + "' is invalid" );
    return false;
  }
  if( this->variance_ < 0 )
  {
    return false;
  }
  if( this->max_error_ < 0 )
  {
    return false;
  }
  if( this->threshold_ < 0 )
  {
    return false;
  }
  return true;
}

bool ActionCannyEdgeDetection::run( ActionContextHandle& context, ActionResultHandle& result )
{
  if ( StateEngine::Instance()->is_statealias( this->layer_alias_ ) )
  {
    // TODO: run filter
    context->report_message( "The Canny Edge Detection Filter has been triggered "
      "successfully on layer: "  + this->layer_alias_ );
    return true;
  }
    
  return false;
}


void ActionCannyEdgeDetection::Dispatch( std::string layer_alias, double variance, 
  double max_error, double threshold, bool replace )
{
  ActionCannyEdgeDetection* action = new ActionCannyEdgeDetection;
  action->layer_alias_ = layer_alias;
  action->variance_ = variance;
  action->max_error_ = max_error;
  action->threshold_ = threshold;
  action->replace_ = replace;
  
  Interface::PostAction( ActionHandle( action ) );
}
  
} // end namespace Seg3D
