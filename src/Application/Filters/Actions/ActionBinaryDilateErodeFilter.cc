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

#include <Core/Interface/Interface.h>

#include <Application/LayerManager/LayerManager.h>
#include <Application/Filters/Actions/ActionBinaryDilateErodeFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, BinaryDilateErodeFilter )

namespace Seg3D
{
  
bool ActionBinaryDilateErodeFilter::validate( Core::ActionContextHandle& context )
{
  this->layer_.handle() = LayerManager::Instance()->get_layer_by_id( this->layer_id_.value() );
  
  if ( ! this->layer_.handle() )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_id_.value() + 
      "' is invalid" );
    return false;
  }

  if( this->dilate_.value() < 0 )
  {
    context->report_error( "Dilate radius cannot be negative. " );
    return false;
  }
  if( this->erode_.value() < 0 )
  {
    context->report_error( "Erode radius cannot be negative. " );
    return false;
  }
  
  return true;
}

bool ActionBinaryDilateErodeFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  context->report_message( "The BinaryDilateErodeFilter has been triggered "
    "successfully on layer: "  + this->layer_id_.value() );   

  return true;
}


Core::ActionHandle ActionBinaryDilateErodeFilter::Create( std::string layer_id, 
  int dilate, int erode, bool replace )
{
  ActionBinaryDilateErodeFilter* action = new ActionBinaryDilateErodeFilter;
  action->layer_id_.value() = layer_id;
  action->dilate_.value() = dilate;
  action->erode_.value() = erode;
  action->replace_.value() = replace;
  
  return Core::ActionHandle( action );
}

void ActionBinaryDilateErodeFilter::Dispatch( Core::ActionContextHandle context, 
  std::string layer_id, int dilate, int erode, bool replace )
{ 
  Core::ActionDispatcher::PostAction( Create( layer_id, dilate, erode, replace ), context );
}
  
} // end namespace Seg3D
