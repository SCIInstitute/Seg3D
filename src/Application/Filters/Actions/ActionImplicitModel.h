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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIONIMPLICITMODEL_H
#define APPLICATION_TOOLS_ACTIONS_ACTIONIMPLICITMODEL_H

#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace ImplicitModel
{

class NotifierRunnable;
typedef boost::shared_ptr<NotifierRunnable> NotifierRunnableHandle;

class ActionImplicitModelPrivate;
typedef boost::shared_ptr< ActionImplicitModelPrivate > ActionImplicitModelPrivateHandle;

typedef Core::Point VertexCoord;
typedef std::vector< VertexCoord > VertexList;

typedef std::vector< std::string > ViewModeList;

class ActionImplicitModel : public Seg3D::LayerAction
{
CORE_ACTION(
  CORE_ACTION_TYPE( "ImplicitModel", "Implicit model function action interface" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "vertices", "The 3D points needed to generate the implicit model function." )
  CORE_ACTION_ARGUMENT( "view_modes", "The 2D view where each point was picked (axial or sagittal or coronal)." )
  CORE_ACTION_ARGUMENT( "normal_offset", "Normal vector offset parameter." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "convex_hull_2D", "true", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "invert_seed_order", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "kernel", "thin_plate", "Implicit model function kernel (thin_plate or gaussian or multi_quadratic)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)

public:
  ActionImplicitModel();
  
  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context ) override;
  
  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result ) override;
  bool run_threshold( Core::ActionContextHandle& context );

private:
  ActionImplicitModelPrivateHandle private_;
  Seg3D::SandboxID sandbox_;

public:
  static void Dispatch(
                       Core::ActionContextHandle context,
                       const std::string& target,
                       const VertexList& vertices,
                       const ViewModeList& viewModes,
                       double normalOffset,
                       bool compute2DConvexHull,
                       bool invertSeedOrder,
                       const std::string& kernel
                      );
};

}

#endif
