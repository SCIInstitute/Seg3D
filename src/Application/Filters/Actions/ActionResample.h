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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONRESAMPLE_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONRESAMPLE_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Geometry/GridTransform.h>

// Application includes
#include <Application/Layer/LayerAction.h>

namespace Seg3D
{
  
class ActionResamplePrivate;
typedef boost::shared_ptr< ActionResamplePrivate > ActionResamplePrivateHandle;

class ActionResample : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "Resample", "Resample the input layers to the specified size" )
  CORE_ACTION_ARGUMENT( "layerids", "The layerids on which this tool needs to be run." )
  CORE_ACTION_ARGUMENT( "x", "Number of samples along X-axis" )
  CORE_ACTION_ARGUMENT( "y",  "Number of samples along Y-axis" )
  CORE_ACTION_ARGUMENT( "z",  "Number of samples along Z-axis" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "crop", "false", "Whether to crop/pad the dataset." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "range_min", "[0,0,0]", "The minimum value of resample range in index space." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "range_max", "[0,0,0]", "The maximum value of resample range in index space." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "padding", "0", "The value used to pad data outside the existing boundary." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "kernel", "box", "Name of the interpolation kernel to use")
  CORE_ACTION_OPTIONAL_ARGUMENT( "param1", "1.0", "The first parameter for the kernel" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "param2", "1.0", "The second parameter for the kernel" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "replace", "false", "Whether to delete the input layers when done" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionResample();

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
private:
  ActionResamplePrivateHandle private_;
  
public:
  static void Dispatch( Core::ActionContextHandle context, 
    const std::vector< std::string >& layer_ids, int x, int y, int z, 
    const std::string& kernel, double param1, double param2, bool replace );

  static void Dispatch( Core::ActionContextHandle context,
    const std::vector< std::string >& layer_ids, 
    const Core::GridTransform& grid_trans, const std::string& padding, 
    const std::string& kernel, double param1, double param2, bool replace );

  static void Dispatch( Core::ActionContextHandle context, const std::string& src_layer,
    const std::string& dst_layer, const std::string& padding, const std::string& kernel, 
    double param1, double param2, bool replace );

public:
  static const std::string ZERO_C;
  static const std::string MIN_C;
  static const std::string MAX_C;

  static const std::string BOX_C;
  static const std::string TENT_C;
  static const std::string CUBIC_CR_C;
  static const std::string CUBIC_BS_C;
  static const std::string QUARTIC_C;
  static const std::string GAUSSIAN_C;
};
  
} //
#endif
