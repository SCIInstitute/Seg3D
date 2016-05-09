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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIONPASTE_H
#define APPLICATION_TOOLS_ACTIONS_ACTIONPASTE_H

#include <Application/Layer/LayerAction.h>

namespace Seg3D
{

class ActionPastePrivate;
typedef boost::shared_ptr< ActionPastePrivate > ActionPastePrivateHandle;

class ActionPaste : public LayerAction
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "Paste", "Paste the content of the clipboard onto a mask slice.")
  CORE_ACTION_ARGUMENT( "target", "The ID of the target mask layer." )
  CORE_ACTION_ARGUMENT( "slice_type", "The slicing direction." )
  CORE_ACTION_ARGUMENT( "min_slice", "The minimum slice number to paste onto." )
  CORE_ACTION_ARGUMENT( "max_slice", "The maximum slice number to paste onto." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "Which sandbox to use." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)

public:
  ActionPaste();

  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

  // CLEAR_CACHE:
  // Clear any objects that were given as a short cut to improve performance.
  virtual void clear_cache();

private:
  ActionPastePrivateHandle private_;

public:
  // DISPATCH:
  // Dispatch the action.
  // This version should only be called from the menu. It will deduce the parameters
  // from the current active viewer and active layer.
  static void Dispatch( Core::ActionContextHandle context, bool punch_through = false );

  // DISPATCH:
  // Dispatch the action.
  static void Dispatch( Core::ActionContextHandle context, const std::string& layer_id,
    int slice_type, size_t min_slice, size_t max_slice );
};

} // end namespace Seg3D

#endif
