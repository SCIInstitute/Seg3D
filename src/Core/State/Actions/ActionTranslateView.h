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

#ifndef CORE_STATE_ACTIONS_ACTIONTRANSLATEVIEW_H
#define CORE_STATE_ACTIONS_ACTIONTRANSLATEVIEW_H

// Core Includes
#include <Core/Action/Action.h>
#include <Core/Action/ActionDispatcher.h>
#include <Core/Interface/Interface.h>
#include <Core/State/StateViewBase.h>
#include <Core/Geometry/Vector.h>

namespace Core
{

class ActionTranslateView : public Action
{

CORE_ACTION(
CORE_ACTION_TYPE( "TranslateView", "This action translates the state in a view state variable." )
CORE_ACTION_ARGUMENT( "stateid", "The name of the state variable." )
CORE_ACTION_ARGUMENT( "offset", "The offset that needs to be added." )
)

public:
  ActionTranslateView()
  {
    this->add_parameter( this->stateid_ );
    this->add_parameter( this->offset_ );
  }

  // -- Functions that describe action --
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );

  // -- Function that describes whether the action changes the data of the program --
  virtual bool changes_project_data();

private:
  std::string stateid_;
  Core::Vector offset_;

  StateViewBaseWeakHandle state_weak_handle_;

public:

  // DISPATCH:
  // Dispatch the action from the interface
  template< class VIEWSTATEHANDLE >
  static void Dispatch( ActionContextHandle context, VIEWSTATEHANDLE& view_state, 
    const Core::Vector& offset )
  {
    ActionTranslateView* action = new ActionTranslateView;
    action->stateid_ = view_state->get_stateid();
    action->offset_ = offset;
    action->state_weak_handle_ = view_state;

    ActionDispatcher::PostAction( ActionHandle( action ), context );
  }
};

} // end namespace Core

#endif
