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

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionCalculateMaskVolume.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, CalculateMaskVolume )

namespace Seg3D
{

bool ActionCalculateMaskVolume::validate( Core::ActionContextHandle& context )
{
  if ( LayerManager::Instance()->get_mask_layer_by_id( this->mask_name_.value() ) ) return true;
  
  if( LayerManager::Instance()->get_layer_by_name( 
    this->mask_name_.value() )->get_type() == Core::VolumeType::MASK_E )
  {
    // If they passed the name instead, then we'll take the opportunity to get the id instead.
    this->mask_name_.value() = LayerManager::Instance()->get_layer_by_name( 
      this->mask_name_.value() )->get_layer_id();
    return true;
  }
  return false; // validated
}

bool ActionCalculateMaskVolume::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  std::string message = std::string( "Please wait, while your mask's volume is calculated..." );

  Core::ActionProgressHandle progress = 
    Core::ActionProgressHandle( new Core::ActionProgress( message ) );

  progress->begin_progress_reporting();

  LayerManager::Instance()->get_mask_layer_by_id( this->mask_name_.value() )->calculate_volume();
  
  progress->end_progress_reporting();
  
  return true;
}

Core::ActionHandle ActionCalculateMaskVolume::Create( const std::string& mask_name )
{
  ActionCalculateMaskVolume* action = new ActionCalculateMaskVolume;
  
  action->mask_name_.value() = mask_name;

  return Core::ActionHandle( action );
}


void ActionCalculateMaskVolume::Dispatch( Core::ActionContextHandle context, 
  const std::string& mask_name )
{
  Core::ActionDispatcher::PostAction( Create( mask_name ), context );
}

} // end namespace Seg3D
