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

#ifndef APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONNEWPROJECT_H
#define APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONNEWPROJECT_H

#include <Core/Action/Action.h> 
#include <Core/Interface/Interface.h>


namespace Seg3D
{

class ActionNewProject : public Core::Action
{
  
CORE_ACTION( 
  CORE_ACTION_TYPE( "NewProject", "Create a new project and close the current one." )
  CORE_ACTION_ARGUMENT( "path", "Where to create the new project." )
  CORE_ACTION_ARGUMENT( "name", "The name of the new project." )
)

  // -- Constructor/Destructor --
public:
  ActionNewProject()
  {
    this->add_parameter( this->project_location_ );
    this->add_parameter( this->project_name_ );
  }

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
private:

  /// The name of the new project
  std::string project_name_;
  
  /// The name of the directory in which the project directory will be created
  std::string project_location_;
    
  // -- Dispatch this action from the interface --
public:
  /// DISPATCH:
  /// Dispatch an action loads a session
  static void Dispatch( Core::ActionContextHandle context, const std::string& project_location, 
    const std::string& project_name );
};

} // end namespace Seg3D

#endif
