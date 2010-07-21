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

#ifndef APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONDELETESESSION_H
#define APPLICATION_PROJECTMANAGER_ACTIONS_ACTIONDELETESESSION_H

#include <Core/Action/Action.h> 
#include <Core/Interface/Interface.h>


namespace Seg3D
{

// TODO: Need fix the session name <> index duality here

class ActionDeleteSession : public Core::Action
{
  CORE_ACTION( "DeleteSession|sessionname" );

  // -- Constructor/Destructor --
public:
  ActionDeleteSession()
  {
    this->add_argument( this->session_name_ );
    this->add_argument( this->session_index_ );
  }

  virtual ~ActionDeleteSession()
  {
  }

  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
private:


  // This parameter contains the name of the session to be loaded
  Core::ActionParameter< std::string > session_name_;

  // This parameter contains the index of the session to be loaded
  Core::ActionParameter< int > session_index_;
  
  // -- Dispatch this action from the interface --
public:
  
  // CREATE:
  // Create an action that loads a session
  static Core::ActionHandle Create( int session_index );
  
  // DISPATCH:
  // Dispatch an action loads a session
  static void Dispatch( Core::ActionContextHandle context, int session_index );
};

} // end namespace Seg3D

#endif  //ACTIONDELETESESSION_H
