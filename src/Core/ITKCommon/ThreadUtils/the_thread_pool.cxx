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

// File         : the_thread_pool.cxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Feb 21 09:01:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thread pool class.

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_thread_pool.hxx>
#include <Core/ITKCommon/ThreadUtils/the_transaction.hxx>
#include <Core/ITKCommon/ThreadUtils/the_mutex_interface.hxx>
#include <Core/ITKCommon/the_utils.hxx>

// system includes:
#include <iostream>

// namespace access:
using std::cout;
using std::cerr;
using std::endl;

//----------------------------------------------------------------
// DEBUG_THREAD
// 
// #define DEBUG_THREAD


//----------------------------------------------------------------
// the_transaction_wrapper_t::the_transaction_wrapper_t
// 
the_transaction_wrapper_t::
the_transaction_wrapper_t(const unsigned int & num_parts,
        the_transaction_t * transaction):
  mutex_(the_mutex_interface_t::create()),
  cb_(transaction->notify_cb_),
  cb_data_(transaction->notify_cb_data_),
  num_parts_(num_parts)
{
  assert(mutex_ != NULL);
  
  for (unsigned int i = 0; i <= the_transaction_t::DONE_E; i++)
  {
    notified_[i] = 0;
  }
  transaction->set_notify_cb(notify_cb, this);
}

//----------------------------------------------------------------
// the_transaction_wrapper_t::~the_transaction_wrapper_t
// 
the_transaction_wrapper_t::~the_transaction_wrapper_t()
{
  mutex_->delete_this();
  mutex_ = NULL;
}

//----------------------------------------------------------------
// the_transaction_wrapper_t::notify_cb
// 
void
the_transaction_wrapper_t::notify_cb(void * data,
             the_transaction_t * t,
             the_transaction_t::state_t s)
{
  the_transaction_wrapper_t * wrapper = (the_transaction_wrapper_t *)(data);
  bool done = wrapper->notify(t, s);
  if (done)
  {
    delete wrapper;
  }
}

//----------------------------------------------------------------
// the_transaction_wrapper_t::notify
// 
bool
the_transaction_wrapper_t::notify(the_transaction_t * t,
          the_transaction_t::state_t s)
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  notified_[s]++;
  
  unsigned int num_terminal_notifications =
    notified_[the_transaction_t::SKIPPED_E] +
    notified_[the_transaction_t::ABORTED_E] +
    notified_[the_transaction_t::DONE_E];
  
  switch (s)
  {
    case the_transaction_t::PENDING_E:
    case the_transaction_t::STARTED_E:
      if (notified_[s] == 1 && num_terminal_notifications == 0 && cb_ != NULL)
      {
  cb_(cb_data_, t, s);
      }
      return false;
      
    case the_transaction_t::SKIPPED_E:
    case the_transaction_t::ABORTED_E:
    case the_transaction_t::DONE_E:
      {
  if (num_terminal_notifications == num_parts_)
  {
    // restore the transaction callback:
    t->set_notify_cb(cb_, cb_data_);
    
    // if necessary, consolidate the transaction state into one:
    if (num_terminal_notifications !=
        notified_[the_transaction_t::DONE_E])
    {
      the_transaction_t::state_t state =
        notified_[the_transaction_t::ABORTED_E] > 0 ?
        the_transaction_t::ABORTED_E :
        the_transaction_t::SKIPPED_E;
      t->set_state(state);
    }
    
    if (cb_ != NULL)
    {
      cb_(cb_data_, t, t->state());
    }
    else
    {
      delete t;
    }
    
    return true;
  }
  return false;
      }
      
    default:
      assert(0);
  }
  
  return false;
}


//----------------------------------------------------------------
// the_thread_pool_t::the_thread_pool_t
// 
the_thread_pool_t::the_thread_pool_t(unsigned int num_threads):
  pool_size_(num_threads)
{
  mutex_ = the_mutex_interface_t::create();
  assert(mutex_ != NULL);
  
  pool_ = new the_thread_pool_data_t[num_threads];
  for (unsigned int i = 0; i < num_threads; i++)
  {
    pool_[i].id_ = i;
    pool_[i].parent_ = this;
    pool_[i].thread_ = the_thread_interface_t::create();
    assert(pool_[i].thread_ != NULL);
    pool_[i].thread_->set_thread_pool_cb(this, &pool_[i]);
    
    // mark the thread as idle, initially:
    idle_.push_back(i);
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::~the_thread_pool_t
// 
the_thread_pool_t::~the_thread_pool_t()
{
  // TODO: sketchy!
  mutex_->delete_this();
  mutex_ = NULL;
  
  delete [] pool_;
  pool_ = NULL;
  pool_size_ = 0;
}

//----------------------------------------------------------------
// the_thread_pool_t::start
// 
void
the_thread_pool_t::start()
{
  if (!transactions_.empty())
  {
    while (true)
    {
      the_lock_t<the_mutex_interface_t> locker(mutex_);
      if (transactions_.empty()) return;
      
      if (idle_.empty())
      {
#ifdef DEBUG_THREAD
  // sanity test:
  for (unsigned int i = 0; i < pool_size_; i++)
  {
    the_lock_t<the_mutex_interface_t> locker(pool_[i].thread_->mutex());
    if (!pool_[i].thread_->has_work())
    {
      cerr << "WARNING: thread " << i << ", " << pool_[i].thread_
     << " is actually idle" << endl;
    }
  }
#endif
  return;
      }
      
      // get the next worker thread:
      unsigned int id = remove_head(idle_);
      busy_.push_back(id);
      
      // get the next transaction:
      the_transaction_t * t = remove_head(transactions_);
      
      // start the thread:
      thread(id)->start(t);
      
#ifdef DEBUG_THREAD
      cerr << "starting thread " << id << ", " << thread(id) << ", " << t
     << endl;
      
      for (std::list<unsigned int>::const_iterator i = idle_.begin();
     i != idle_.end(); ++i)
      {
  cerr << "idle: thread " << *i << ", " << thread(*i) << endl;
      }
      
      for (std::list<unsigned int>::const_iterator i = busy_.begin();
     i != busy_.end(); ++i)
      {
  cerr << "busy: thread " << *i << ", " << thread(*i) << endl;
      }
#endif
    }
  }
  else
  {
    // Transaction list is empty, perhaps they've already
    // been distributed to the threads? Just start the threads
    // and let them figure it out for themselves.
    for (unsigned int i = 0; i < pool_size_; i++)
    {
      pool_[i].thread_->start();
    }
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::set_idle_sleep_duration
// 
void
the_thread_pool_t::set_idle_sleep_duration(bool enable, unsigned int microsec)
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  for (unsigned int i = 0; i < pool_size_; i++)
  {
    pool_[i].thread_->set_idle_sleep_duration(enable, microsec);
  }
}

//----------------------------------------------------------------
// notify_cb
// 
static void
notify_cb(void * data,
    the_transaction_t * transaction,
    the_transaction_t::state_t s)
{
  the_thread_pool_t * pool = (the_thread_pool_t *)(data);
  pool->handle(transaction, s);
}

//----------------------------------------------------------------
// status_cb
// 
static void
status_cb(void * data, the_transaction_t *, const char * text)
{
  the_thread_pool_t * pool = (the_thread_pool_t *)(data);
  pool->blab(text);
}

//----------------------------------------------------------------
// wrap
// 
static void
wrap(the_transaction_t * transaction,
     the_thread_pool_t * pool,
     bool multithreaded)
{
  if (transaction->notify_cb() == NULL)
  {
    // override the callback:
    transaction->set_notify_cb(::notify_cb, pool);
  }
  
  if (transaction->status_cb() == NULL)
  {
    // override the callback:
    transaction->set_status_cb(::status_cb, pool);
  }
  
  if (multithreaded)
  {
    // silence the compiler warning:
    // the_transaction_wrapper_t * wrapper =
    new the_transaction_wrapper_t(pool->pool_size(), transaction);
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::push_front
// 
void
the_thread_pool_t::push_front(the_transaction_t * transaction,
            bool multithreaded)
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  no_lock_push_front(transaction, multithreaded);
}

//----------------------------------------------------------------
// the_thread_pool_t::push_back
// 
void
the_thread_pool_t::push_back(the_transaction_t * transaction,
           bool multithreaded)
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  no_lock_push_back(transaction, multithreaded);
}

//----------------------------------------------------------------
// the_thread_pool_t::push_back
// 
void
the_thread_pool_t::push_back(std::list<the_transaction_t *> & schedule,
           bool multithreaded)
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  no_lock_push_back(schedule, multithreaded);
}

//----------------------------------------------------------------
// the_thread_pool_t::pre_distribute_work
// 
void
the_thread_pool_t::pre_distribute_work()
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  std::vector<std::list<the_transaction_t *> > split_schedule_(pool_size_);

  while (!transactions_.empty())
  {
    for (unsigned int i = 0; i < pool_size_ && !transactions_.empty(); i++)
    {
      the_transaction_t * job = remove_head(transactions_);
      split_schedule_[i].push_back(job);
    }
  }
  
  for (unsigned int i = 0; i < pool_size_; i++)
  {
    pool_[i].thread_->push_back(split_schedule_[i]);
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::has_work
// 
bool
the_thread_pool_t::has_work() const
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  if (!transactions_.empty()) return true;
  
  // make sure the busy threads have work:
  for (std::list<unsigned int>::const_iterator
   iter = busy_.begin(); iter != busy_.end(); ++iter)
  {
    unsigned int i = *iter;
    if (pool_[i].thread_->has_work()) return true;
  }
  
  return false;
}

//----------------------------------------------------------------
// the_thread_pool_t::start
// 
void
the_thread_pool_t::start(the_transaction_t * transaction, bool multithreaded)
{
  push_back(transaction, multithreaded);
  start();
}

//----------------------------------------------------------------
// the_thread_pool_t::stop
// 
void
the_thread_pool_t::stop()
{
  // remove any pending transactions:
  flush();
  
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  for (unsigned int i = 0; i < pool_size_; i++)
  {
    pool_[i].thread_->stop();
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::wait
// 
void
the_thread_pool_t::wait()
{
  // the_lock_t<the_mutex_interface_t> locker(mutex_);
  for (unsigned int i = 0; i < pool_size_; i++)
  {
    pool_[i].thread_->wait();
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::flush
// 
void
the_thread_pool_t::flush()
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  no_lock_flush();
}

//----------------------------------------------------------------
// the_thread_pool_t::terminate_transactions
// 
void
the_thread_pool_t::terminate_transactions()
{
  the_lock_t<the_mutex_interface_t> locker(mutex_);
  no_lock_terminate_transactions();
}

//----------------------------------------------------------------
// the_thread_pool_t::stop_and_go
// 
void
the_thread_pool_t::stop_and_go(the_transaction_t * transaction,
             bool multithreaded)
{
  // safely manipulate the transactions:
  {
    the_lock_t<the_mutex_interface_t> locker(mutex_);
    no_lock_terminate_transactions();
    no_lock_push_back(transaction, multithreaded);
  }
  
  start();
}

//----------------------------------------------------------------
// the_thread_pool_t::stop_and_go
// 
void
the_thread_pool_t::stop_and_go(std::list<the_transaction_t *> & schedule,
             bool multithreaded)
{
  // safely manipulate the transactions:
  {
    the_lock_t<the_mutex_interface_t> locker(mutex_);
    no_lock_terminate_transactions();
    no_lock_push_back(schedule, multithreaded);
  }
  
  start();
}

//----------------------------------------------------------------
// the_thread_pool_t::flush_and_go
// 
void
the_thread_pool_t::flush_and_go(the_transaction_t * transaction,
        bool multithreaded)
{
  // safely manipulate the transactions:
  {
    the_lock_t<the_mutex_interface_t> locker(mutex_);
    no_lock_flush();
    no_lock_push_back(transaction, multithreaded);
  }
  
  start();
}

//----------------------------------------------------------------
// the_thread_pool_t::flush_and_go
// 
void
the_thread_pool_t::flush_and_go(std::list<the_transaction_t *> & schedule,
             bool multithreaded)
{
  // safely manipulate the transactions:
  {
    the_lock_t<the_mutex_interface_t> locker(mutex_);
    no_lock_flush();
    no_lock_push_back(schedule, multithreaded);
  }
  
  start();
}

//----------------------------------------------------------------
// the_thread_pool_t::blab
// 
void
the_thread_pool_t::handle(the_transaction_t * transaction,
        the_transaction_t::state_t s)
{
  switch (s)
  {
    case the_transaction_t::SKIPPED_E:
    case the_transaction_t::ABORTED_E:
    case the_transaction_t::DONE_E:
      delete transaction;
      break;
      
    default:
      break;
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::handle
// 
void
the_thread_pool_t::blab(const char * message) const
{
  cerr << message << endl;
}

//----------------------------------------------------------------
// the_thread_pool_t::handle_thread
// 
// prior to calling this function the thread interface locks
// the pool mutex and it's own mutex -- don't try locking these
// again while inside this callback, and don't call any other
// pool or thread member functions which lock either of these
// because that would result in a deadlock
// 
void
the_thread_pool_t::handle_thread(the_thread_pool_data_t * data)
{
  const unsigned int & id = data->id_;
  the_thread_interface_t * t = thread(id);
  
  if (t->stopped_)
  {
#ifdef DEBUG_THREAD
    cerr << "thread has stopped: " << t << endl;
#endif
    
    if (!has(idle_, id))
    {
      idle_.push_back(id);
    }
    
    if (has(busy_, id))
    {
      busy_.remove(id);
    }
    
    return;
  }
  
  if (t->has_work())
  {
#ifdef DEBUG_THREAD
    cerr << "thread still has work: " << t << endl;
#endif
    return;
  }
  
  if (transactions_.empty())
  {
    // tell the thread to stop:
    t->stopped_ = true;
    
    if (!has(idle_, id))
    {
      idle_.push_back(id);
    }
    
    if (has(busy_, id))
    {
      busy_.remove(id);
    }
    
#ifdef DEBUG_THREAD
    cerr << "no more work for thread: " << t << endl;
    
    for (std::list<unsigned int>::const_iterator i = idle_.begin();
   i != idle_.end(); ++i)
    {
      cerr << "idle: thread " << *i << ", " << thread(*i) << endl;
    }
    
    for (std::list<unsigned int>::const_iterator i = busy_.begin();
   i != busy_.end(); ++i)
    {
      cerr << "busy: thread " << *i << ", " << thread(*i) << endl;
    }
#endif
    
    return;
  }
  
  // execute another transaction:
  the_transaction_t * transaction = remove_head(transactions_);
#ifdef DEBUG_THREAD
  cerr << "giving " << transaction << " to thread " << t << endl;
#endif
  
  t->transactions_.push_back(transaction);
}

//----------------------------------------------------------------
// the_thread_pool_t::no_lock_flush
// 
void
the_thread_pool_t::no_lock_flush()
{
  while (!transactions_.empty())
  {
    the_transaction_t * t = remove_head(transactions_);
    t->notify(this, the_transaction_t::SKIPPED_E);
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::no_lock_terminate_transactions
// 
void
the_thread_pool_t::no_lock_terminate_transactions()
{
  // remove any pending transactions:
  no_lock_flush();
  
  for (unsigned int i = 0; i < pool_size_; i++)
  {
    pool_[i].thread_->terminate_transactions();
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::no_lock_push_front
// 
void
the_thread_pool_t::no_lock_push_front(the_transaction_t * transaction,
              bool multithreaded)
{
  wrap(transaction, this, multithreaded);
  transactions_.push_front(transaction);
  for (unsigned int i = 1; multithreaded && i < pool_size_; i++)
  {
    transactions_.push_front(transaction);
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::no_lock_push_back
// 
void
the_thread_pool_t::no_lock_push_back(the_transaction_t * transaction,
             bool multithreaded)
{
  wrap(transaction, this, multithreaded);
  transactions_.push_back(transaction);
  for (unsigned int i = 1; multithreaded && i < pool_size_; i++)
  {
    transactions_.push_back(transaction);
  }
}

//----------------------------------------------------------------
// the_thread_pool_t::no_lock_push_back
// 
void
the_thread_pool_t::no_lock_push_back(std::list<the_transaction_t *> & schedule,
             bool multithreaded)
{
  // preprocess the transactions:
  for (std::list<the_transaction_t *>::iterator i = schedule.begin();
       i != schedule.end(); i++)
  {
    wrap(*i, this, multithreaded);
  }
  
  if (multithreaded)
  {
    while (!schedule.empty())
    {
      the_transaction_t * t = remove_head(schedule);
      for (unsigned int i = 0; i < pool_size_; i++)
      {
  transactions_.push_back(t);
      }
    }
  }
  else
  {
    transactions_.splice(transactions_.end(), schedule);
  }
}
