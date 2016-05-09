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

// File         : the_thread_pool.hxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Feb 21 08:30:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thread pool class.

#ifndef THE_THREAD_POOL_HXX_
#define THE_THREAD_POOL_HXX_

// system includes:
#include <list>
#include <assert.h>

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_thread_interface.hxx>
#include <Core/ITKCommon/ThreadUtils/the_transaction.hxx>

// forward declarations:
class the_mutex_interface_t;
class the_thread_pool_t;


//----------------------------------------------------------------
// transaction_wrapper_t
// 
class the_transaction_wrapper_t
{
public:
  the_transaction_wrapper_t(const unsigned int & num_parts,
          the_transaction_t * transaction);
  ~the_transaction_wrapper_t();
  
  static void notify_cb(void * data,
      the_transaction_t * t,
      the_transaction_t::state_t s);
  
  bool notify(the_transaction_t * t,
        the_transaction_t::state_t s);
  
private:
  the_transaction_wrapper_t();
  the_transaction_wrapper_t & operator = (const the_transaction_wrapper_t &);
  
  the_mutex_interface_t * mutex_;
  
  the_transaction_t::notify_cb_t cb_;
  void * cb_data_;
  
  const unsigned int num_parts_;
  unsigned int notified_[the_transaction_t::DONE_E + 1];
};


//----------------------------------------------------------------
// the_thread_pool_data_t
// 
struct the_thread_pool_data_t
{
  the_thread_pool_data_t():
    parent_(NULL),
    thread_(NULL),
    id_(~0u)
  {}
  
  ~the_thread_pool_data_t()
  { thread_->delete_this(); }
  
  the_thread_pool_t * parent_;
  the_thread_interface_t * thread_;
  unsigned int id_;
};


//----------------------------------------------------------------
// the_thread_pool_t
// 
class the_thread_pool_t : public the_transaction_handler_t
{
  friend class the_thread_interface_t;
  
public:
  the_thread_pool_t(unsigned int num_threads);
  virtual ~the_thread_pool_t();
  
  // start the threads:
  void start();
  
  // this controls whether the thread will voluntarily terminate
  // once it runs out of transactions:
  void set_idle_sleep_duration(bool enable, unsigned int microseconds = 10000);
  
  // schedule a transaction:
  // NOTE: when multithreaded is true, the transaction will be scheduled
  // N times, where N is the number of threads in the pool.
  // This means the transaction will be executed by N threads, so
  // the transaction has to support concurrent execution internally.
  virtual void push_front(the_transaction_t * transaction,
        bool multithreaded = false);
  
  virtual void push_back(the_transaction_t * transaction,
       bool multithreaded = false);
  
  virtual void push_back(std::list<the_transaction_t *> & schedule,
       bool multithreaded = false);
  
  // split the work among the threads:
  void pre_distribute_work();
  
  // check whether the thread pool has any work left:
  bool has_work() const;
  
  // schedule a transaction and start the thread:
  virtual void start(the_transaction_t * transaction,
         bool multithreaded = false);
  
  // abort the current transaction and clear pending transactions;
  // transactionFinished will be emitted for the aborted transaction
  // and the discarded pending transactions:
  void stop();
  
  // wait for all threads to finish:
  void wait();
  
  // clear all pending transactions, do not abort the current transaction:
  void flush();
  
  // this will call terminate_all for the terminators in this thread,
  // but it will not stop the thread, so that new transactions may
  // be scheduled while the old transactions are being terminated:
  void terminate_transactions();
  
  // terminate the current transactions and schedule a new transaction:
  void stop_and_go(the_transaction_t * transaction,
       bool multithreaded = false);
  void stop_and_go(std::list<the_transaction_t *> & schedule,
       bool multithreaded = false);
  
  // flush the current transactions and schedule a new transaction:
  void flush_and_go(the_transaction_t * transaction,
       bool multithreaded = false);
  void flush_and_go(std::list<the_transaction_t *> & schedule,
       bool multithreaded = false);
  
  // virtual: default transaction communication handlers:
  void handle(the_transaction_t * transaction, the_transaction_t::state_t s);
  void blab(const char * message) const;
  
  // access control:
  inline the_mutex_interface_t * mutex()
  { return mutex_; }
  
  inline const unsigned int & pool_size() const
  { return pool_size_; }
  
private:
  // intentionally disabled:
  the_thread_pool_t(const the_thread_pool_t &);
  the_thread_pool_t & operator = (const the_thread_pool_t &);
  
protected:
  // thread callback handler:
  virtual void handle_thread(the_thread_pool_data_t * data);
  
  // helpers:
  inline the_thread_interface_t * thread(unsigned int id) const
  { 
    assert(id < pool_size_);
    return pool_[id].thread_;
  }
  
  void no_lock_flush();
  void no_lock_terminate_transactions();
  void no_lock_push_front(the_transaction_t * transaction, bool multithreaded);
  void no_lock_push_back(the_transaction_t * transaction, bool multithreaded);
  void no_lock_push_back(std::list<the_transaction_t *> & schedule, bool mt);
  
  // thread synchronization control:
  the_mutex_interface_t * mutex_;
  
  // the thread pool:
  the_thread_pool_data_t * pool_;
  unsigned int pool_size_;
  
  // the working threads:
  std::list<unsigned int> busy_;
  
  // the waiting threads:
  std::list<unsigned int> idle_;
  
  // scheduled transactions:
  std::list<the_transaction_t *> transactions_;
};


#endif // THE_THREAD_POOL_HXX_
