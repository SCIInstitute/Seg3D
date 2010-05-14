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
#include <Application/Filters/Actions/ActionBooleanFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, BooleanFilter )

namespace Seg3D
{
  
bool ActionBooleanFilter::validate( Core::ActionContextHandle& context )
{
  this->layer_a_.handle() = LayerManager::Instance()->get_layer_by_id( this->layer_a_id_.value() );
  
  if ( ! this->layer_a_.handle() )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_a_id_.value() + "' is invalid." );
    return false;
  }
  
  // NEED LOGIC TO CHECK EXPRESSION
  
  if ( this->layer_b_id_.value() != "" )
  {
    this->layer_b_.handle() = 
      LayerManager::Instance()->get_layer_by_id( this->layer_b_id_.value() );
    
    if ( ! this->layer_b_.handle() )
    {
      context->report_error( std::string( "LayerID '" ) + 
        this->layer_b_id_.value() + "' is invalid" );
      return false;
    }
  }

  if ( this->layer_c_id_.value() != "" )
  {
    this->layer_c_.handle() = 
      LayerManager::Instance()->get_layer_by_id( this->layer_c_id_.value() );
    
    if ( ! this->layer_c_.handle() )
    {
      context->report_error( std::string( "LayerID '" ) + 
        this->layer_c_id_.value() + "' is invalid" );
      return false;
    }
  }

  if ( this->layer_d_id_.value() != "" )
  {
    this->layer_d_.handle() = 
      LayerManager::Instance()->get_layer_by_id( this->layer_d_id_.value() );
    
    if ( ! this->layer_d_.handle() )
    {
      context->report_error( std::string( "LayerID '" ) + 
        this->layer_d_id_.value() + "' is invalid" );
      return false;
    }
  }
  
  return true;
}

bool ActionBooleanFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // TODO: run filter
  context->report_message( "The BooleanFilter has been triggered "
    "successfully on layer: "  + this->layer_a_.handle()->name_state_->get() );
  
  return true;
}


Core::ActionHandle ActionBooleanFilter::Create( std::string layer_a_id, std::string layer_b_id, 
  std::string layer_c_id, std::string layer_d_id, std::string expression, bool replace )
{
  ActionBooleanFilter* action = new ActionBooleanFilter;
  
  action->layer_a_id_.value() = layer_a_id;
  action->layer_b_id_.value() = layer_b_id;
  action->layer_c_id_.value() = layer_c_id;
  action->layer_d_id_.value() = layer_d_id;
  
  action->expression_.value() = expression;
  action->replace_.value() = replace;
  
  return Core::ActionHandle( action );
}

void ActionBooleanFilter::Dispatch( std::string layer_a_id, std::string layer_b_id, 
  std::string layer_c_id, std::string layer_d_id, std::string expression, bool replace )
{
  Core::Interface::PostAction( Create( layer_a_id, layer_b_id, layer_c_id, layer_d_id, 
    expression, replace ) );
}

  
} // end namespace Seg3D
