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

#ifndef APPLICATION_FILTERS_ACTIONS_ActionPointSetTransformFilter_H
#define APPLICATION_FILTERS_ACTIONS_ActionPointSetTransformFilter_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>

#include <itkEuler3DTransform.h>

namespace Seg3D
{

class ActionPointSetTransformPrivate;
typedef boost::shared_ptr< ActionPointSetTransformPrivate > ActionPointSetTransformPrivateHandle;

class ActionPointSetTransformFilter : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "PointSetTransformFilter", "Transform Filter that does a Iterative Closest Points Registration on two masks." )
  CORE_ACTION_ARGUMENT( "targetlayerid", "Target layer id." )
  CORE_ACTION_ARGUMENT( "layerids", "The layerids on which this tool needs to be run." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionPointSetTransformFilter();

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context ) override;
  virtual bool run( Core::ActionContextHandle& context, 
    Core::ActionResultHandle& result ) override;

private:
  ActionPointSetTransformPrivateHandle private_;

  // -- Dispatch this action from the interface --
public:

  //static void Dispatch( Core::ActionContextHandle context, 
  //  std::string target_layer,
  //  const std::vector< std::string >& layer_ids,
  //  std::string toolid );

  static void Dispatch( Core::ActionContextHandle context, 
    std::string target_layer,
    const std::vector< std::string >& layer_ids,
    const std::vector< double >& matrix_params );
};
  
} // end namespace Seg3D

#endif
