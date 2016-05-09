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

#ifndef INTERFACE_APPCLICATION_CONTROLLERCONTEXT_H
#define INTERFACE_APPCLICATION_CONTROLLERCONTEXT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#ifndef Q_MOC_RUN

// Include all action related classes
#include <Core/Action/Actions.h>

// Include interface code
#include <Interface/Application/ControllerInterface.h>

#endif

namespace Seg3D
{

class ControllerContext;
typedef boost::shared_ptr< ControllerContext > ControllerContextHandle;

class ControllerContext : public Core::ActionContext
{

  // -- Constructor/destructor --
public:
  ControllerContext( ControllerInterface* controller );
  virtual ~ControllerContext();

  // -- Reporting functions --
public:
  virtual void report_error( const std::string& error );
  virtual void report_warning( const std::string& warning );
  virtual void report_message( const std::string& message );
  virtual void report_need_resource( Core::NotifierHandle resource );

  // -- Report that action was done --
public:
  virtual void report_done();

  // -- Source/Status information --
public:
  virtual Core::ActionSource source() const;

private:
  // To which controller does the action information need to be relayed
  ControllerInterface::qpointer_type controller_;
};

} //end namespace Seg3D

#endif
