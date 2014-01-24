// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: t -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// File         : the_thread_pool.hxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Feb 21 08:30:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thread pool class.

#ifndef THE_THREAD_POOL_HXX_
#define THE_THREAD_POOL_HXX_

// system includes:
#include <list>
#include <assert.h>

// TODO: temporary
#include <Core/IRTools_tmp/the_thread_interface.hxx>
#include <Core/IRTools_tmp/the_transaction.hxx>

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
