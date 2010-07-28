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

#ifndef APPLICATION_SESSION_SESSION_H
#define APPLICATION_SESSION_SESSION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/State/State.h>
#include <Core/Utils/EnumClass.h>

namespace Seg3D
{

CORE_ENUM_CLASS
(
  SessionPriority,
  DEFAULT_PRIORITY_E = -1,
  LAYER_MANAGER_PRIORITY_E = 300,
  VIEWER_MANAGER_PRIORITY_E = 200,
  TOOL_MANAGER_PRIORITY_E = 100
)

// CLASS Session
// This is the main class for collecting state information on a Session
class Session;
  
typedef boost::shared_ptr< Session > SessionHandle;

// Class definition
class Session : public Core::StateHandler
{
  // -- constructor/destructor --
public:
  Session( const std::string& session_name );
  virtual ~Session();
  
public:
  // general session data
  Core::StateStringHandle session_name_state_;

public:
  // INITIALIZE_FROM_FILE:
  // this file initializes the state values for Session from the file at the path specified
  bool load( boost::filesystem::path path, const std::string& session_name );
  
  // SAVE_CURRENT_STATE:
  // this function will take a snapshot of the current state of the project and save it
  bool save( boost::filesystem::path path, const std::string& session_name );
  
private:
  const static size_t VERSION_NUMBER_C;

};

} // end namespace Seg3D

#endif // SESSION_H
