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

#ifndef APPLICATION_TOOL_ACTIONS_ACTIONACTIVATETOOL_H
#define APPLICATION_TOOL_ACTIONS_ACTIONACTIVATETOOL_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

namespace Seg3D
{

class ActionActivateTool : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ActivateTool", "Activate a tool so it becomes the active tool." )
  CORE_ACTION_ARGUMENT( "toolid", "Toolid of the tool that needs to be activated." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  // -- Constructor/Destructor --
public:
  ActionActivateTool()
  {
    this->add_parameter( this->toolid_ );
  }

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context ) override;
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result ) override;

  // -- Dispatch this action from the interface --
public:
  /// CREATE
  /// Create action that activates a tool
  static Core::ActionHandle Create( const std::string& toolid );

  /// DISPATCH
  /// Create and dispatch action that activates a tool
  static void Dispatch( Core::ActionContextHandle context, const std::string& toolid );

  // -- Action parameters --
private:
  std::string toolid_;
};

} // end namespace Seg3D

#endif
