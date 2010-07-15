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

#ifndef APPLICATION_VIEWER_ACTIONS_ACTIONAUTO_VIEW_H
#define APPLICATION_VIEWER_ACTIONS_ACTIONAUTO_VIEW_H

#include <Core/Action/Action.h>
#include <Application/Viewer/Viewer.h>

namespace Seg3D
{

class ActionAutoView : public Core::Action
{
  CORE_ACTION( "AutoView", "AutoView <viewer_id>" );

public:
  ActionAutoView();
  virtual ~ActionAutoView() {}

  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  Core::ActionParameter< std::string > viewer_name_;

  ViewerWeakHandle viewer_weak_handle_;

public:
  static Core::ActionHandle Create( ViewerHandle& viewer );

  static void Dispatch( Core::ActionContextHandle context, ViewerHandle& viewer );
};

} // end namespace Seg3D

#endif