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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIOINFLOODFILL_H
#define APPLICATION_TOOLS_ACTIONS_ACTIOINFLOODFILL_H

#include <Application/LayerManager/Actions/ActionLayer.h>

namespace Seg3D
{

class ActionFloodFillPrivate;
typedef boost::shared_ptr< ActionFloodFillPrivate > ActionFloodFillPrivateHandle;

class ActionFloodFill : public ActionLayer
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "Floodfill", "Flood fill the content of a mask slice "
    "starting from seed points at the four corners.")
  CORE_ACTION_ARGUMENT( "target", "The ID of the target mask layer." )
  CORE_ACTION_ARGUMENT( "slice_type", "The slicing direction." )
  CORE_ACTION_ARGUMENT( "slice_number", "The slice number to be filled." )
  CORE_ACTION_KEY( "erase", "false", "Whether to erase instead of fill." )
)

public:
  ActionFloodFill();
  virtual ~ActionFloodFill();

  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  ActionFloodFillPrivateHandle private_;

public:
  // DISPATCH:
  // Dispatch the action.
  static void Dispatch( Core::ActionContextHandle context, const std::string& layer_id, 
    int slice_type, size_t slice_number, bool erase );
};

} // end namespace Seg3D

#endif