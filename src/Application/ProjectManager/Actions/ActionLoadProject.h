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

#ifndef APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONLOADPROJECT_H
#define APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONLOADPROJECT_H

// Core includes
#include <Core/Action/Action.h> 
#include <Core/Interface/Interface.h>

namespace Seg3D
{

class ActionLoadProject : public Core::Action
{
  
CORE_ACTION( 
  CORE_ACTION_TYPE( "LoadProject", "Load a new project and discard the current one." )
  CORE_ACTION_ARGUMENT( "project", "The project file that needs to be opened." )
)

  // -- Constructor/Destructor --
public:
  ActionLoadProject()
  {
    this->add_parameter( this->project_file_ );
  }

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
private:

  // This parameter contains the name of the s3d file of the project that needs to be loaded.
  std::string project_file_;
  
  // -- Dispatch this action from the interface --
public:
  /// DISPATCH:
  /// Dispatch an action loads a session
  static void Dispatch( Core::ActionContextHandle context, const std::string& project_file );
};

} // end namespace Seg3D

#endif
