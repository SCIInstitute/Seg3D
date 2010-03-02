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

#include <Application/LayerManager/Actions/ActionCrop.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{
  
  // REGISTER ACTION:
  // Define a function that registers the action. The action also needs to be
  // registered in the CMake file.
  SCI_REGISTER_ACTION(Crop);
  
  bool ActionCrop::validate( ActionContextHandle& context )
  {
    if ( !( StateEngine::Instance()->is_stateid( name_.value() ) ) )
    {
      context->report_error( std::string( "Layer: '" ) + name_.value() + "' is invalid" );
      return false;
    }
    
    return true; // validated
  }
  
  bool ActionCrop::run( ActionContextHandle& context, ActionResultHandle& result )
  {
    //LayerManager::Instance()->crop( layerid_.value() );
    return true; // success
  }
  
  ActionHandle ActionCrop::Create( const std::string& name )
  {
    // Create new action
    ActionCrop* action = new ActionCrop;
    
    // Set action parameters
    action->name_.value() = name;
    
    // Post the new action
    return ActionHandle( action );
  }
  
  void ActionCrop::Dispatch( const std::string& name )
  {
    Interface::PostAction( Create( name ) );
  }
  
} // end namespace Seg3D
