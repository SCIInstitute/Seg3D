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

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONMIGRATESANDBOXLAYER_H
#define APPLICATION_LAYER_ACTIONS_ACTIONMIGRATESANDBOXLAYER_H

// Core includes
#include <Core/Action/Action.h>

namespace Seg3D
{

class ActionMigrateSandboxLayerPrivate;
typedef boost::shared_ptr< ActionMigrateSandboxLayerPrivate > ActionMigrateSandboxLayerPrivateHandle;

class ActionMigrateSandboxLayer : public Core::Action
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "MigrateSandboxLayer", "Move a layer out of a sandbox." )
  CORE_ACTION_ARGUMENT( "layerid", "The ID of the sandbox layer." )
  CORE_ACTION_ARGUMENT( "sandbox", "The sandbox ID from which to migrate the layer." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "prov_id", "-1", "The provenance ID to be used for the layer." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  
  // -- Constructor/Destructor --
public:
  ActionMigrateSandboxLayer();
  virtual ~ActionMigrateSandboxLayer();

  // -- Functions that describe action --
public:
  /// VALIDATE:
  /// Each action needs to be validated just before it is posted. This way we
  /// enforce that every action that hits the main post_action signal will be
  /// a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  /// RUN:
  /// Each action needs to have this piece implemented. It spells out how the
  /// action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

  /// CLEAR_CACHE:
  /// Clear any objects that were given as a short cut to improve performance.
  virtual void clear_cache();
  
private:
  ActionMigrateSandboxLayerPrivateHandle private_;
};
  
} // end namespace Seg3D

#endif
