/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

// File         : the_transaction.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Feb 16 09:52:00 MST 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A thread transaction class.

#ifndef THE_TRANSACTION_HXX_
#define THE_TRANSACTION_HXX_

// system includes:
#include <iostream>
#include <string>

// local includes:
#include <Core/ITKCommon/ThreadUtils/the_mutex_interface.hxx>

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
//  inline static const the_text_t tr(const char * text)
//  { return the_text_t(text); }
//  
//  inline static const the_text_t & tr(const the_text_t & text)
//  { return text; }
  
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


#endif // THE_TRANSACTION_HXX_
