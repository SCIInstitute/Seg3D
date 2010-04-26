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
#include <Application/Filters/Actions/ActionPaint.h>

namespace Seg3D
{
  
// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Paint );

bool ActionPaint::validate( ActionContextHandle& context )
{
  if( !( StateEngine::Instance()->is_statealias( this->layer_alias_ ) ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_alias_ + "' is invalid" );
    return false;
  }
  if( !( StateEngine::Instance()->is_statealias( this->mask_alias_ ) ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->mask_alias_ + "' is invalid" );
    return false;
  }
  if( this->brush_radius_ < 0 )
  {
    return false;
  }
  if( this->upper_threshold_ < 0 )
  {
    return false;
  }
  if( this->lower_threshold_ < 0 )
  {
    return false;
  }
  return true;
}

bool ActionPaint::run( ActionContextHandle& context, ActionResultHandle& result )
{
  if( StateEngine::Instance()->is_statealias( this->layer_alias_ ) )
  {
    // TODO: run filter
    context->report_message( "The Paint Tool has been triggered "
                 "successfully on: "  + this->layer_alias_ );
    return true;
  }
    
  return false;
}


void ActionPaint::Dispatch( std::string layer_alias, std::string mask_alias, 
  int brush_radius, double upper_threshold, double lower_threshold, bool erase )
{
  ActionPaint* action = new ActionPaint;
  action->layer_alias_ = layer_alias;
  action->mask_alias_ = mask_alias;
  action->brush_radius_ = brush_radius;
  action->upper_threshold_ = upper_threshold;
  action->lower_threshold_ = lower_threshold;
  action->erase_ = erase;
  
  Interface::PostAction( ActionHandle( action ) );
}
  
} // end namespace Seg3D
