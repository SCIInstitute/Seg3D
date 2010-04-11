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
#include <Application/LayerManager/Actions/ActionInsertLayerAbove.h>

namespace Seg3D
{
  
  // REGISTER ACTION:
  // Define a function that registers the action. The action also needs to be
  // registered in the CMake file.
  CORE_REGISTER_ACTION( InsertLayerAbove );
  
  bool ActionInsertLayerAbove::validate( ActionContextHandle& context )
  {

    if ( !( StateEngine::Instance()->is_stateid( layer_to_insert_id_ ) ) )
    {
      context->report_error( std::string( "LayerID '" ) + layer_to_insert_id_ + "' is invalid" );
      return false;
    }
    
    if ( !( StateEngine::Instance()->is_stateid( layer_below_id_ ) ) )
    {
      context->report_error( std::string( "LayerID '" ) + layer_below_id_ + "' is invalid" );
      return false;
    }

    return true;
  }
  
  bool ActionInsertLayerAbove::run( ActionContextHandle& context, ActionResultHandle& result )
  {
    if ( ( StateEngine::Instance()->is_stateid( layer_to_insert_id_ ) ) && 
      ( StateEngine::Instance()->is_stateid( layer_below_id_ ) ) )
    {
      LayerManager::Instance()->move_layer_above( this->layer_to_insert_id_, this->layer_below_id_ );
      return true;
    }

    return false;
  }
  

  void ActionInsertLayerAbove::Dispatch( std::string layer_to_insert_id, std::string layer_below_id )
  {
    ActionInsertLayerAbove* action = new ActionInsertLayerAbove;
    action->layer_below_id_ = layer_below_id;
    action->layer_to_insert_id_ = layer_to_insert_id;
  
    Interface::PostAction( ActionHandle( action ) );
  }
  
} // end namespace Seg3D