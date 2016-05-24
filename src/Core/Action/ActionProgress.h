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

#ifndef CORE_ACTION_ACTIONPROGRESS_H
#define CORE_ACTION_ACTIONPROGRESS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

// Core includes
#include <Core/Utils/Lockable.h>

namespace Core
{

// CLASS ACTIONPROGRESS
/// For slow actions the action progress class, keeps track of progress

// Forward declaration
class ActionProgress;
/// NOTE: The intrusive pointer will allow a handle to be generate inside the class
typedef boost::shared_ptr< ActionProgress > ActionProgressHandle;

// Class definition
class ActionProgress :  public Core::Lockable, public boost::enable_shared_from_this<ActionProgress>
{
  // -- Constructor / Destructor --
public:

  ActionProgress( const std::string& message,
    bool is_interuptable = false,
    bool has_progress_updates = false );
  virtual ~ActionProgress();

  // -- Progress metering --
public:

  // BEGIN_PROGRESS_REPORTING:
  /// Begin the progress reporting
  void begin_progress_reporting();

  // END_PROGRESS_REPORTING:
  /// End the progress reporting
  void end_progress_reporting();
  
  // SET_INTERRUPT:
  /// Set the interrupt flag to notify that action needs to canceled
  void set_interrupt( bool interrupt );
  
  // GET_INTERRUPT:
  // Check the status of the interrupt flag
  bool get_interrupt();
  
  // SET_PROGRESS:
  /// Set the progress of the action
  void set_progress( double progress );

  // GET_PROGRESS:
  /// Get the progress of the action
  double get_progress();
  
  // GET_MESSAGE:
  // Get the message that is posted
  std::string get_message() const;
  
  // IS_INTERRUPTABLE:
  // Whether the action is interruptable
  bool is_interruptable() const;
  
  // HAS_PROGRESS_UPDATES:
  /// Whether progress will be updated
  bool has_progress_updates() const;
  
  // -- Internals of the progress reporter --
private:

  std::string message_;
  bool    is_interruptable_;
  bool    has_progress_updates_;
  
  bool    interrupt_;
  double    progress_;
};

} // namespace Core

#endif
