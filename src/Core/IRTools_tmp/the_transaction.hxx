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


// File         : the_transaction.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Feb 16 09:52:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : A thread transaction class.

#ifndef THE_TRANSACTION_HXX_
#define THE_TRANSACTION_HXX_

// system includes:
#include <iostream>
#include <string>

// the includes:
#include <Core/IRTools_tmp/the_text.hxx>
#include <Core/IRTools_tmp/the_log.hxx>

// forward declarations:
class the_thread_interface_t;
class the_transaction_handler_t;


//----------------------------------------------------------------
// the_transaction_t
// 
// NOTE: the transaction will not take ownership of the mutex.
// 
class the_transaction_t
{
public:
  the_transaction_t();
  virtual ~the_transaction_t();
  
  // execute the transaction:
  virtual void execute(the_thread_interface_t * thread) = 0;
  
  // transaction execution state:
  typedef enum {
    PENDING_E,
    SKIPPED_E,
    STARTED_E,
    ABORTED_E,
    DONE_E
  } state_t;
  
  inline state_t state() const
  { return state_; }
  
  inline void set_state(const state_t & s)
  { state_ = s; }
  
  inline bool done() const
  { return state_ == DONE_E; }
  
  //----------------------------------------------------------------
  // notify_cb_t
  // 
  typedef void(*notify_cb_t)(void *, the_transaction_t *, state_t s);
  
  inline notify_cb_t notify_cb() const
  { return notify_cb_; }
  
  inline void set_notify_cb(notify_cb_t cb, void * cb_data)
  {
    notify_cb_ = cb;
    notify_cb_data_ = cb_data;
  }
  
  //----------------------------------------------------------------
  // status_cb_t
  // 
  // NOTE: if status is NULL, it means the transaction is requesting
  // a callback from the main thread.  This could be used by the
  // transaction for some GUI user interaction (such as finding replacement
  // tiles for a mosaic, etc...)
  // 
  typedef void(*status_cb_t)(void *, the_transaction_t *, const char * status);
  
  inline status_cb_t status_cb() const
  { return status_cb_; }
  
  inline void set_status_cb(status_cb_t cb, void * cb_data)
  {
    status_cb_ = cb;
    status_cb_data_ = cb_data;
  }
  
  // notify the transaction about a change in it's state:
  virtual void notify(the_transaction_handler_t * handler,
		      state_t s,
		      const char * message = NULL);
  
  // helper:
  virtual void blab(the_transaction_handler_t * handler,
		    const char * message);
  
  // FIXME: this is a relic:
  inline static const the_text_t tr(const char * text)
  { return the_text_t(text); }
  
  inline static const the_text_t & tr(const the_text_t & text)
  { return text; }
  
  // if a transaction needs to have something executed in the main thread
  // context it should call callback_request. This requires that a valid
  // status callback is set for this transaction, and that the status
  // callback will acknowledge the main thread callback request.  This
  // call will block until the callback is executed in the main thread,
  // and the request is removed:
  bool callback_request();
  
  // if a transaction needs to have something executed in the main thread
  // context it should override this callback. This method will be executed
  // in the main thread context.  The default implementation will simply
  // remove the request:
  virtual void callback();
  
protected:
  // when requesting a callback from the main thread
  // the status of request will be set to WAITING_E:
  typedef enum {
    NOTHING_E,
    WAITING_E
  } request_t;
  
  // synchronization control:
  the_mutex_interface_t * mutex_;
  
  // status of callback request:
  request_t request_;
  
  // current state of the transaction:
  state_t state_;
  
public:
  // the callbacks:
  notify_cb_t notify_cb_;
  void * notify_cb_data_;
  
  status_cb_t status_cb_;
  void * status_cb_data_;
};

//----------------------------------------------------------------
// operator << 
// 
extern std::ostream &
operator << (std::ostream & so, const the_transaction_t::state_t & state);


//----------------------------------------------------------------
// the_transaction_handler_t
// 
class the_transaction_handler_t
{
public:
  virtual ~the_transaction_handler_t() {}
  
  virtual void handle(the_transaction_t * transaction,
		      the_transaction_t::state_t s) = 0;
  virtual void blab(const char * message) const = 0;
};


//----------------------------------------------------------------
// the_transaction_log_t
// 
class the_transaction_log_t : public the_log_t
{
public:
  the_transaction_log_t(the_transaction_handler_t * handler):
    handler_(handler)
  {}
  
  // virtual:
  the_log_t & operator << (std::ostream & (*f)(std::ostream &))
  {
    the_log_t::operator << (f);
    std::string text(the_log_t::line_.str());
    the_log_t::line_.str("");
    handler_->blab(text.c_str());
    return *this;
  }
  
  template <typename data_t>
  the_log_t & operator << (const data_t & data)
  { return the_log_t::operator << (data); }
  
private:
  the_transaction_handler_t * handler_;
};


#endif // THE_TRANSACTION_HXX_
