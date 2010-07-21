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

#ifndef APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONNEWPROJECT_H
#define APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONNEWPROJECT_H

#include <Core/Action/Action.h> 
#include <Core/Interface/Interface.h>


namespace Seg3D
{

class ActionNewProject : public Core::Action
{
  CORE_ACTION( "NewProject|projectpoth|projectname" );

  // -- Constructor/Destructor --
public:
  ActionNewProject()
  {
    this->add_argument( this->project_path_ );
    this->add_argument( this->project_name_ );
  }

  virtual ~ActionNewProject()
  {
  }

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
private:

  // This parameter contains the name of the session to be loaded
  Core::ActionParameter< std::string > project_name_;

  // This parameter contains the index of the session to be loaded
  Core::ActionParameter< std::string > project_path_;
  
  // -- Dispatch this action from the interface --
public:
  
  // CREATE:
  // Create an action that loads a session
  static Core::ActionHandle Create( const std::string& project_path, 
    const std::string& project_name );
  
  // DISPATCH:
  // Dispatch an action loads a session
  static void Dispatch( Core::ActionContextHandle context, const std::string& project_path, 
    const std::string& project_name );
};

} // end namespace Seg3D

#endif  //ACTIONNEWPROJECT_H
