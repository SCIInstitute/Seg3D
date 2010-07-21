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

#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONACTIVATELAYER_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONACTIVATELAYER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/LayerFWD.h>
#include <Application/LayerManager/Actions/ActionLayer.h>

namespace Seg3D
{

class ActionActivateLayer : public ActionLayer
{
  CORE_ACTION( "ActivateLayer|layerid" );

  // -- Constructor/Destructor --
public:
  ActionActivateLayer()
  {
    this->add_argument( this->layer_id_ );
    this->add_cachedhandle( this->layer_ );
  }

  virtual ~ActionActivateLayer()
  {
  }

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
private:
  // This parameter contains the id of the layer
  Core::ActionParameter< std::string > layer_id_;
  
  // This cached handle contains a short cut to the layer that needs activating
  Core::ActionCachedHandle< LayerHandle > layer_;

  // -- Dispatch this action from the interface --
public:
  
  // CREATE:
  // Create an action that activates a layer
  static Core::ActionHandle Create( LayerHandle layer );

  // CREATE:
  // Create an action that activates a layer
  static Core::ActionHandle Create( std::string layer_id );
  
  // DISPATCH:
  // Dispatch an action that activates a layer
  static void Dispatch( Core::ActionContextHandle context, LayerHandle layer );

  // DISPATCH:
  // Dispatch an action that activates a layer
  static void Dispatch( Core::ActionContextHandle context, std::string layer_id );
};

} // end namespace Seg3D

#endif
