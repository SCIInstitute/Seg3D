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
#include <Application/LayerManager/Actions/ActionInsertLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, InsertLayer )

namespace Seg3D
{
  
bool ActionInsertLayer::validate( Core::ActionContextHandle& context )
{
  if ( !this->layer_handle_ )
    return false;
  
  if ( !( Core::StateEngine::Instance()->is_stateid( layer_handle_->get_layer_id() ) ) )
    {
      context->report_error( std::string( "LayerID '" ) + layer_handle_->get_layer_id() + "' is invalid" );
      return false;
    }
  
  return true;

}

bool ActionInsertLayer::run( Core::ActionContextHandle& context, 
              Core::ActionResultHandle& result )
{
  if ( this->layer_handle_ )
  {
    LayerManager::Instance()->insert_layer( layer_handle_ );
    return true;
  }
    
  return false;
}


void ActionInsertLayer::Dispatch( LayerHandle layer )
{
  ActionInsertLayer* action = new ActionInsertLayer;
  action->layer_handle_ = layer;
  
  Core::Interface::PostAction( Core::ActionHandle( action ) );
}
  
} // end namespace Seg3D
