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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONGrowCut_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONGrowCut_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>

#include <Application/Tools/Algorithm/GrowCutter.h>

namespace Seg3D
{

class ActionGrowCut : public LayerAction
{

  CORE_ACTION(
    CORE_ACTION_TYPE( "GrowCut", "GrowCut image segmentation algorithm." )
    CORE_ACTION_ARGUMENT( "data", "The data layer on which this filter needs to be run." )
    CORE_ACTION_ARGUMENT( "foreground", "The layerid of the foreground mask." )
    CORE_ACTION_ARGUMENT( "background", "The layerid of the background mask." )
    CORE_ACTION_ARGUMENT( "output", "The layerid of the output mask." )
    CORE_ACTION_ARGUMENT( "smoothing", "Whether to run the smoothing process as well." )
    CORE_ACTION_CHANGES_PROJECT_DATA()
    CORE_ACTION_IS_UNDOABLE()
    )

  // -- Constructor/Destructor --
public:
  ActionGrowCut()
  {
    // Action arguments
    this->add_layer_id( this->data_layer_ );
    this->add_layer_id( this->foreground_layer_ );
    this->add_layer_id( this->background_layer_ );
    this->add_layer_id( this->output_layer_ );
    this->add_parameter( this->run_smoothing_ );
  }

  virtual ~ActionGrowCut()
  {}

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

  // -- Action parameters --
private:

  std::string data_layer_;
  std::string foreground_layer_;
  std::string background_layer_;
  std::string output_layer_;
  bool run_smoothing_;

  GrowCutterHandle grow_cutter_;

  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch( Core::ActionContextHandle context,
                        std::string data_layer,
                        std::string foreground_layer,
                        std::string background_layer,
                        std::string output_layer,
                        bool run_smoothing,
                        Seg3D::GrowCutterHandle grow_cutter = GrowCutterHandle() );

  static void DispatchAndWait( Core::ActionContextHandle context,
                               std::string data_layer,
                               std::string foreground_layer,
                               std::string background_layer,
                               std::string output_layer,
                               bool run_smoothing,
                               Seg3D::GrowCutterHandle grow_cutter = GrowCutterHandle() );
};
} // end namespace Seg3D

#endif // ifndef APPLICATION_FILTERS_ACTIONS_ACTIONGrowCut_H
