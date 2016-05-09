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

#ifndef APPLICATION_VIEWERMANAGER_ACTIONS_ACTIONPICKPOINT_H
#define APPLICATION_VIEWERMANAGER_ACTIONS_ACTIONPICKPOINT_H

#include <Core/Action/Action.h>
#include <Core/Geometry/Point.h>

namespace Seg3D
{

// TODO: Source viewer needs to be dealt with properly

class ActionPickPoint : public Core::Action
{

CORE_ACTION(
  CORE_ACTION_TYPE( "PickPoint", "Pick a new point to align slices with." )
  CORE_ACTION_ARGUMENT( "point", "The new pick point." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

public:
  ActionPickPoint();

  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  // TODO: Need to replace this with viewerid instead of number
  int viewer_;
  Core::Point point_;

public:
  /// DISPATCH:
  /// Move slices for all viewers
  static void Dispatch( Core::ActionContextHandle context, const Core::Point& pt );

  /// DISPATCH:
  /// Move slices for all but src_viewer
  static void Dispatch( Core::ActionContextHandle context, 
    size_t src_viewer, const Core::Point& pt );
};

} // end namespace Seg3D

#endif
