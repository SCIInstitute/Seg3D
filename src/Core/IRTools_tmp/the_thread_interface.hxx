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


// File         : the_thread_interface.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Feb 16 09:20:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : An abstract thread class interface.

#ifndef THE_THREAD_INTERFACE_HXX_
#define THE_THREAD_INTERFACE_HXX_

// system includes:
#include <list>

// local includes:
#include <Core/IRTools_tmp/the_transaction.hxx>

// forward declarations:
class the_mutex_interface_t;
class the_terminators_t;
class the_thread_pool_t;
struct the_thread_pool_data_t;


//----------------------------------------------------------------
// the_thread_interface_t
// 
// 1. the thread will not take ownership of the transactions.
// 2. the thread will take ownership of the mutex.
// 
class the_thread_interface_t : public the_transaction_handler_t
{
  friend class the_thread_pool_t;

protected:
  // the destructor is protected on purpose,
  // see delete_this for details:
  virtual ~the_thread_interface_t();
  
public:
  // In order to avoid memory management problems with shared libraries,
  // whoever provides this interface instance (via it's creator), has to
  // provide a way to delete the instance as well.  This will avoid
  // issues with multiple-instances of C runtime libraries being
  // used by the app and whatever libraries it links against that
  // either use or provide this interface:
  virtual void delete_this() = 0;
  
  // the thread will own the mutex passed down to it,
  // it will delete the mutex when the thread is deleted:
  the_thread_interface_t(the_mutex_interface_t * mutex = NULL);
  
  //----------------------------------------------------------------
  // creator_t
  // 
  typedef the_thread_interface_t *(*creator_t)();
  
  // specify a thread creation method:
  static void set_creator(creator_t creator);
  
  // create a new instance of a thread:
  static the_thread_interface_t * create();
  
  inline const unsigned int & id() const
  { return id_; }
  
  // start the thread:
  virtual void start() = 0;
  
  // wait for the thread to finish:
  virtual void wait() = 0;
  
  // put the thread to sleep:
  virtual void take_a_nap(const unsigned long & microseconds) = 0;
  
  // accessor to the transaction terminators:
  virtual the_terminators_t & terminators() = 0;
  
  // give this thread an execution synchronization control:
  void set_mutex(the_mutex_interface_t * mutex);
  
  // this controls whether the thread will voluntarily terminate
  // once it runs out of transactions:
  void set_idle_sleep_duration(bool enable, unsigned int microseconds = 10000);
  
  // schedule a transaction:
  inline void add_transaction(the_transaction_t * transaction)
  { push_back(transaction); }
  
  void push_back(the_transaction_t * transaction);
  void push_front(the_transaction_t * transaction);
  
  // add transactions to the list:
  void push_back(std::list<the_transaction_t *> & schedule);
  
  // NOTE: it is the responsibility of the caller to secure a mutex lock
  // on this thread prior to checking whether the thread has any work left:
  bool has_work() const;
  
  // schedule a transaction and start the thread:
  void start(the_transaction_t * transaction);
  
  // abort the current transaction and clear pending transactions;
  // transactionFinished will be emitted for the aborted transaction
  // and the discarded pending transactions:
  void stop();
  
  // accessors to the thread "stopped" flag:
  inline const bool & stopped() const
  { return stopped_; }
  
  inline void set_stopped(bool stopped)
  { stopped_ = stopped; }
  
  // clear all pending transactions, do not abort the current transaction:
  void flush();
  
  // this will call terminate_all for the terminators in this thread,
  // but it will not stop the thread, so that new transactions may
  // be scheduled while the old transactions are being terminated:
  void terminate_transactions();
  
  // terminate the current transactions and schedule a new transaction:
  void stop_and_go(the_transaction_t * transaction);
  void stop_and_go(std::list<the_transaction_t *> & schedule);
  
  // flush the current transactions and schedule a new transaction:
  void flush_and_go(the_transaction_t * transaction);
  void flush_and_go(std::list<the_transaction_t *> & schedule);
  
  // execute the scheduled transactions, return true if all
  // transactions had been executed successfully:
  virtual bool work();
  
  // virtual: default transaction communication handlers:
  void handle(the_transaction_t * transaction, the_transaction_t::state_t s);
  void blab(const char * message) const;
  
  // transaction callback accessor:
  inline void
  set_thread_pool_cb(the_thread_pool_t * pool,
		     the_thread_pool_data_t * cb_data)
  {
    thread_pool_ = pool; 
    thread_pool_cb_data_ = cb_data;
  }
  
  // mutex accessor:
  inline the_mutex_interface_t * mutex()
  { return mutex_; }
  
protected:
  // an abstract creator of threads:
  static creator_t creator_;
  
  // FIXME: this may have been a bad idea:
  unsigned int id_;
  
  // thread synchronization control:
  the_mutex_interface_t * mutex_;
  
  // execution control flag:
  bool stopped_;
  
  // these attributes control the thread behavior once all transactions
  // have been processed. If sleep_when_idle_ flag is set to true, the
  // thread will put itself to sleep instead of terminating:
  bool sleep_when_idle_;
  unsigned int sleep_microsec_;
  
  // currently executing transaction:
  the_transaction_t * active_transaction_;
  
  // scheduled transactions:
  std::list<the_transaction_t *> transactions_;
  
  // the transaction callback:
  the_thread_pool_t * thread_pool_;
  the_thread_pool_data_t * thread_pool_cb_data_;
};


#endif // THE_THREAD_INTERFACE_HXX_
