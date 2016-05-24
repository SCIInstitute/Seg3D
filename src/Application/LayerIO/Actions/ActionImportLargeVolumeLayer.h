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

#ifndef APPLICATION_LAYERIO_ACTIONS_ACTIONIMPORTLARGEVOLUMELAYER_H
#define APPLICATION_LAYERIO_ACTIONS_ACTIONIMPORTLARGEVOLUMELAYER_H

// Core includes
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/LayerIO/LayerImporter.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionImportLargeVolumeLayer : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ImportLargeVolumeLayer", "This action imports a large volume layer into the layer manager.")
  CORE_ACTION_ARGUMENT( "dir", "The name of the directory to load." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)

  // -- Constructor/Destructor --
public:
  ActionImportLargeVolumeLayer()
  {
    this->add_parameter( this->dir_ );
    this->add_parameter( this->sandbox_ );
  }
  
  // -- Functions that describe action --
public:
  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
    
  // -- Action parameters --
private:

  // The filename of the file to load
  std::string dir_;

  // The sandbox in which to run the action
  SandboxID sandbox_;

  // -- Dispatch this action from the interface --
public:
  // DISPATCH:
  // Create and dispatch action that moves the layer above 
  static void Dispatch( Core::ActionContextHandle context, const std::string& filename );

};
  
} // end namespace Seg3D

#endif
