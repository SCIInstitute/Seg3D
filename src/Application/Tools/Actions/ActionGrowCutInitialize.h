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

#ifndef APPLICATION_TOOLS_ACTIONGROWCUTINITIALIZE_H
#define APPLICATION_TOOLS_ACTIONGROWCUTINITIALIZE_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/Layer/LayerAction.h>

namespace Seg3D
{

class ActionGrowCutInitializePrivate;
typedef boost::shared_ptr< ActionGrowCutInitializePrivate > ActionGrowCutInitializePrivateHandle;

class ActionGrowCutInitialize : public LayerAction
{

  CORE_ACTION(
    CORE_ACTION_TYPE( "GrowCutInitialize", "Add a new mask to the top of a group." )
    CORE_ACTION_ARGUMENT( "groupid", "The groupid of the group in which to insert the new mask." )
    CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
    CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
    CORE_ACTION_CHANGES_PROJECT_DATA()
    CORE_ACTION_IS_UNDOABLE()
    )

  // -- Constructor/Destructor --
public:
  ActionGrowCutInitialize();

// -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context,
                    Core::ActionResultHandle& result );
  virtual void clear_cache();

  typedef boost::function< void ( std::string, std::string, std::string ) > callback_type;

private:
  ActionGrowCutInitializePrivateHandle private_;

  // -- Dispatch this action from the interface --
public:
  // DISPATCH:
  // Dispatch action that creates a new mask layer
  static void Dispatch( Core::ActionContextHandle context, const std::string& group_id, callback_type callback = 0 );

  static void DispatchAndWait( Core::ActionContextHandle context, const std::string& group_id, callback_type callback = 0 );
};
} // end namespace Seg3D

#endif // ifndef APPLICATION_TOOLS_ACTIONGROWCUTINITIALIZE_H
