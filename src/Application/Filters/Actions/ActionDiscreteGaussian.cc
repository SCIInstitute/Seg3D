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
#include <Application/Filters/Actions/ActionDiscreteGaussian.h>

namespace Seg3D
{
  
// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( DiscreteGaussian );

bool ActionDiscreteGaussian::validate( ActionContextHandle& context )
{
  if( !( StateEngine::Instance()->is_stateid( this->layer_alias_ ) ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_alias_ + "' is invalid" );
    return false;
  }
  if( this->variance_ < 0 )
  {
    return false;
  }
  if( this->kernelwidth_ < 0 )
  {
    return false;
  }
  return true;
}

bool ActionDiscreteGaussian::run( ActionContextHandle& context, ActionResultHandle& result )
{
  if ( StateEngine::Instance()->is_stateid( this->layer_alias_ ) )
  {
    // TODO: run filter
    return true;
  }
    
  return false;
}


void ActionDiscreteGaussian::Dispatch( std::string layer_alias, double variance, double kernelwidth, bool replace )
{
  ActionDiscreteGaussian* action = new ActionDiscreteGaussian;
  action->layer_alias_ = layer_alias;
  action->variance_ = variance;
  action->kernelwidth_ = kernelwidth;
  action->replace_ = replace;
  
  Interface::PostAction( ActionHandle( action ) );
}
  
} // end namespace Seg3D
