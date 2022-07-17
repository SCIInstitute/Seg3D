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

#ifndef CORE_EVENTHANDLER_EVENT_H
#define CORE_EVENTHANDLER_EVENT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

// include forward declaration
#include <Core/EventHandler/EventFWD.h>

namespace Core
{

// CLASS EVENTSYNC:
/// Auxiliary class needed for doing synchronization

class EventSync
{

public:
  /// Thread communication model
  boost::mutex lock_;
  boost::condition_variable condition_;
};

/// Define an Application Event Sync type handle
typedef boost::shared_ptr< EventSync > EventSyncHandle;

// CLASS EVENT:
/// Base class for function callback into the event handler thread
/// This class is an empty base class we need it for launching the
/// functor object from a non-templated environment.

class Event
{
public:

  /// Constructor
  Event();

  /// Destructor needs to be virtual so we can use the base class to delete
  /// the object without leaking memory
  virtual ~Event();

  // HANDLE_EVENT:
  /// Handle the synchronization part and run the event code
  void handle_event();

  // RUN:
  /// Function for running the functor object.
  virtual void run() = 0;

  // SYNC_HANDLE:
  /// Get/set the synchronization handle
  EventSyncHandle& sync_handle()
  {
    return sync_handle_;
  }

private:
  EventSyncHandle sync_handle_;
};

// CLASS EVENTT <TEMPLATE>
/// This class is redefined for each functor and contains only the
/// pointer to the function object. It is generated on the fly wherever
/// the compiler needs it. 

template< class FUNCTOR >
class EventT : public Event
{
public:

  /// Constructor
  EventT( FUNCTOR functor ) :
    functor_( functor )
  {
  }

  /// Destructor (we need one for the base class destruction)
  virtual ~EventT()
  {
  }

  // RUN:
  /// Run the functor object. This function is accessed from the base class
  /// so we do not need to template the inner workings of the event handler.
  virtual void run()
  {
    functor_();
  }

private:
  FUNCTOR functor_;
};

} // end namespace Core

#endif
