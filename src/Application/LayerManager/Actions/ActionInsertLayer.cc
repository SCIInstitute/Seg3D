/*
 *  ActionInsertLayer.cc
 *  Seg3D
 *
 *  Created by Wes Christiansen on 3/9/10.
 *  Copyright 2010 Numira Biosciences. All rights reserved.
 *
 */

#include "ActionInsertLayer.h"

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

namespace Seg3D
{
  
  // REGISTER ACTION:
  // Define a function that registers the action. The action also needs to be
  // registered in the CMake file.
  SCI_REGISTER_ACTION(InsertLayer);
  
  bool ActionInsertLayer::validate( ActionContextHandle& context )
  {
    if ( this->layer_handle_ )
      return true;
    
    
    if ( this->layer_group_handle_.lock() )  {
      return true;
    }
    LayerGroupHandle temp_group_handle = LayerManager::Instance()->check_for_group(this->group_id_.value() );
    
    if ( temp_group_handle ) {
      this->layer_group_handle_ = temp_group_handle;
      return true;
    }
    
    return false;

  }
  
  bool ActionInsertLayer::run( ActionContextHandle& context, ActionResultHandle& result )
  {
    if ( this->layer_handle_ )
    {
      LayerManager::Instance()->insert_layer( layer_handle_ );
      return true;
    }
      
    LayerGroupHandle temp_group_handle = this->layer_group_handle_.lock();
    
    if ( temp_group_handle )  {
      LayerManager::Instance()->insert_layer( temp_group_handle );
      return true;
    }
    
    
    return false;
  }
  
  
  void ActionInsertLayer::Dispatch( LayerHandle layer )
  {
    ActionInsertLayer* action = new ActionInsertLayer;
    action->layer_handle_ = layer;
    
    Interface::PostAction( ActionHandle( action ) );
  }
  void ActionInsertLayer::Dispatch( LayerGroupHandle group )
  {
    ActionInsertLayer* action = new ActionInsertLayer;
    action->group_id_.value() = group->get_group_id();
    action->layer_group_handle_ = group;
    
    Interface::PostAction( ActionHandle( action ) );
  }
  
} // end namespace Seg3D