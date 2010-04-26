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
#include <Application/Filters/Actions/ActionBoolean.h>

namespace Seg3D
{
  
// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Boolean );

bool ActionBoolean::validate( ActionContextHandle& context )
{
  if( !( StateEngine::Instance()->is_statealias( this->mask_a_alias_ ) ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->mask_a_alias_ + "' is invalid" );
    return false;
  }
  
  return true;
}

bool ActionBoolean::run( ActionContextHandle& context, ActionResultHandle& result )
{
  if ( StateEngine::Instance()->is_statealias( this->mask_a_alias_ ) )
  {
    // TODO: run filter
    context->report_message( "The Arithmetic Filter has been triggered "
      "successfully on layers: "  + this->mask_a_alias_ + ", " + this->mask_b_alias_
      + ", " + this->mask_c_alias_ + ", and " + this->mask_d_alias_ );
    
    return true;
  }
    
  return false;
}


void ActionBoolean::Dispatch( std::string mask_a_alias, std::string mask_b_alias, 
  std::string mask_c_alias, std::string mask_d_alias, std::string expression, bool replace )
{
  ActionBoolean* action = new ActionBoolean;
  action->mask_a_alias_ = mask_a_alias;
  action->mask_b_alias_ = mask_b_alias;
  action->mask_c_alias_ = mask_c_alias;
  action->mask_d_alias_ = mask_d_alias;
  action->expression_ = expression;
  action->replace_ = replace;
  
  Interface::PostAction( ActionHandle( action ) );
}
  
} // end namespace Seg3D
