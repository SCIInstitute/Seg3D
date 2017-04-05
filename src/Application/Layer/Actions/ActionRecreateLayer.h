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

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONRECREATELAYER_H
#define APPLICATION_LAYER_ACTIONS_ACTIONRECREATELAYER_H

// Core includes
#include <Core/Action/Actions.h>

// Application includes
#include <Application/Provenance/ProvenanceStep.h>

namespace Seg3D
{

class ActionRecreateLayerPrivate;
typedef boost::shared_ptr< ActionRecreateLayerPrivate > ActionRecreateLayerPrivateHandle;

class ActionRecreateLayer : public Core::Action
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "RecreateLayer", "Recreate a layer with the given provenance ID." )
  CORE_ACTION_ARGUMENT( "prov_ids", "The provenance IDs to be recreated." )
)
  
  // -- Constructor/Destructor --
public:
  ActionRecreateLayer();
  virtual ~ActionRecreateLayer();

// -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context ) override;
  virtual bool run( Core::ActionContextHandle& context, 
    Core::ActionResultHandle& result ) override;
  virtual void clear_cache() override;
  
private:
  ActionRecreateLayerPrivateHandle private_;

public:
  /// DISPATCH:
  /// Dispatch an action to recreate the given provenance ID.
  static void Dispatch( Core::ActionContextHandle context, 
    const std::vector< ProvenanceID >& prov_ids, 
    ProvenanceTrailHandle prov_trail = ProvenanceTrailHandle() );
};
  
} // end namespace Seg3D

#endif
