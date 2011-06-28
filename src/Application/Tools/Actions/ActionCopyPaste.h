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

#ifndef APPLICATION_TOOLS_ACTIONS_ACTIONCOPYPASTE_H
#define APPLICATION_TOOLS_ACTIONS_ACTIONCOPYPASTE_H

// Core includes
#include <Core/Action/Actions.h>

// Application includes
#include <Application/Layer/LayerAction.h>

namespace Seg3D
{

class ActionCopyPastePrivate;
typedef boost::shared_ptr< ActionCopyPastePrivate > ActionCopyPastePrivateHandle;

class ActionCopyPaste : public LayerAction
{

CORE_ACTION
( 
  CORE_ACTION_TYPE( "CopyPaste", "Copy one slice from the source mask layer and"
                    " paste onto the specified slice(s) of the destination mask layer." )
  CORE_ACTION_ARGUMENT( "source", "The ID of the source mask layer." )
  CORE_ACTION_ARGUMENT( "src_slice_type", "The source slicing direction." )
  CORE_ACTION_ARGUMENT( "src_slice_number", "The source slice number." )
  CORE_ACTION_ARGUMENT( "target", "The ID of the target mask layer." )
  CORE_ACTION_ARGUMENT( "dst_slice_type", "The slicing direction." )
  CORE_ACTION_ARGUMENT( "dst_min_slice", "The minimum slice number to paste onto." )
  CORE_ACTION_ARGUMENT( "dst_max_slice", "The maximum slice number to paste onto." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)

public:
  ActionCopyPaste();

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
  ActionCopyPastePrivateHandle private_;
};

} // end namespace Seg3D

#endif