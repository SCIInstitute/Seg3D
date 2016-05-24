/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

// Core includes
#include <Core/Action/ActionFactory.h>

// Application includes
#include <Application/Layer/Actions/ActionSynchronize.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, Synchronize )

namespace Seg3D
{

ActionSynchronize::ActionSynchronize()
{
  this->add_parameter( this->layerids_ );
  this->add_parameter( this->sandbox_ );
}

bool ActionSynchronize::validate( Core::ActionContextHandle& context )
{
  // Make sure the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  for ( size_t i = 0; i < this->layerids_.size(); ++i )
  {
    LayerHandle layer = LayerManager::FindLayer( this->layerids_[ i ], this->sandbox_ );
    // Ignore any non-exist layers, so this action can be used in scripts to generically sync
    // any actions, not just actions that output layers
    if ( !layer ) continue;

    // CheckLayerAvailabilityForProcessing requires exclusive access to the layer,
    // and thus guarantees that all operations are done when it returns true.
    if ( !LayerManager::CheckLayerAvailabilityForProcessing( this->layerids_[ i ], context, this->sandbox_ ) )
      return false; 
  }
  
  return true;
}

bool ActionSynchronize::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  // The layers are all in sync now
  return true;
}

} // end namespace Seg3D
