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

#include <Application/Application/Appliction.h>
#include <Application/Resource/ResourceLock.h>

namespace Seg3D {

ResourceLock::ResourceLock(std::string name) :
  name_(name),
  resource_lock_count_(0)
{
}

ResourceLock::~ResourceLock()
{
}
    
bool
ResourceLock::lock()
{
  guard_type guard(resource_lock_);
  if (resource_lock_count_ > 0) 
  {
    return (false);
  }
  resource_lock_count_++;
  resource_locked_signal();
}

void 
ResourceLock::unlock()
{
  guard_type guard(resource_lock_);
  if (resource_lock_count_ > 0)
  {
    resource_lock_count_--;
  }
  else
  {
    // it is already unlocked
    return;
  }

  // Check whether it was finally unlocked.
  if (resource_lock_count_ == 0)
  {
    resource_available_.notify_all();
    resource_unlocked_signal();
  }
}

void 
ResourceLock::Lock(ResourceLockHandle& resource_lock)
{
  // NOTE:
  // Lock *needs* to be called from the application thread to ensure that
  // resource management is done properly. The action that issues the lock
  // is run on the application thread, hence when it is locked inside the 
  // action or in a function called from Action::run this condition should
  // be automatically be met. However to ensure that the code is consistent
  // we test here that this is actually true
  if (!(Application::IsApplicationThread()))
  {
    SCI_THROW_LOGICERROR(
      "Lock is called from a thread other than the application thread");
  }
  
  resource_lock->lock();
}

void 
ResourceLock::Unlock(ResourceLockHandle& resource_lock)
{
  // NOTE:
  // Threads that do asynchronous processing unlock resource automatically using
  // the guard when they are done. However they do this on their respective
  // thread, hence to ensure that the application is fully synchronized we
  // forward the unlocking code to the application. This will ensure as well 
  // that the unlock signal is run on the application thread.
  if (!(Application::IsApplicationThread()))
  {
    Application::PostEvent(boost::bind(&ResourceLock::Unlock,resource_lock_));
    return;
  }

  resource_lock->lock();
}

void 
ResourceLock::wait()
{
  guard_type guard(resource_lock_);

  // wait until the resource has been unlocked
  while (resource_lock_count_ != 0) 
  {
    // sleep until a resource availability signal is triggered
    resource_available_.wait(guard);
  }
  // resource count so one can try again to lock the resource
  return;
}

bool
ResourceLock::is_locked()
{
  guard_type guard(resource_lock_);
  return (resource_lock_count_ != 0);
}


ResourceGuard::ResourceGuard(ResourceLockHandle& resource_lock)  :
  resource_lock_(resource_lock)
{
  resource_lock_.lock();
}

ResourceGuard::~ResourceGuard()
{
  resource_lock_.unlock();
}

} // end namespace Seg3D

#endif
