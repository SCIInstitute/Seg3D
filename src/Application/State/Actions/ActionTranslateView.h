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

#ifndef APPLICATION_STATE_ACTIONS_ACTIONTRANSLATEVIEW_H
#define APPLICATION_STATE_ACTIONS_ACTIONTRANSLATEVIEW_H

#include <Application/Action/Action.h>
#include <Application/Interface/Interface.h>
#include <Application/State/StateViewBase.h>

#include <Utils/Geometry/Vector.h>

namespace Seg3D
{

class ActionTranslateView : public Action
{
  CORE_ACTION( "Translate", "Translate <key> <offset>" );

public:
  ActionTranslateView();
  virtual ~ActionTranslateView()
  {
  }

  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

private:
  ActionParameter< std::string > stateid_;
  ActionParameter< Utils::Vector > offset_;

  StateViewBaseWeakHandle state_weak_handle_;

public:
  template< class VIEWSTATEHANDLE >
  static void Dispatch( VIEWSTATEHANDLE& view_state, const Utils::Vector& offset );

};

template< class VIEWSTATEHANDLE >
void ActionTranslateView::Dispatch( VIEWSTATEHANDLE& view_state, const Utils::Vector& offset )
{
  ActionTranslateView* action = new ActionTranslateView;
  action->stateid_ = view_state->stateid();
  action->offset_ = offset;
  action->state_weak_handle_ = view_state;

  Interface::PostAction( ActionHandle( action ) );
}

} // end namespace Seg3D

#endif
