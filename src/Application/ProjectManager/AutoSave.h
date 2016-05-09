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

#ifndef APPLICATION_PROJECTMANAGER_AUTOSAVE_H
#define APPLICATION_PROJECTMANAGER_AUTOSAVE_H

#include <boost/thread.hpp>


// Core includes
#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>

namespace Seg3D
{

class AutoSave : public Core::Lockable, Core::ConnectionHandler
{

CORE_SINGLETON( AutoSave );

public:
  AutoSave();
  virtual ~AutoSave();

public:
  /// START:
  /// function that makes the connections to the ProjectManager and then starts the thread
  void start();

  /// RECOMPUTE_AUTO_SAVE:
  /// function that 
  void recompute_auto_save();

  /// AUTO_SAVE_DONE:
  /// function that notifies us that the autosave is done
  void auto_save_is_done();

private:
  /// RUN:
  /// function that actually runs the timer
  void run();

  /// NEEDS_AUTO_SAVE:
  /// checks to see if autosave is needed
  bool needs_auto_save();

  /// DO_AUTO_SAVE:
  /// function that actually dispatches the session save action
  void do_auto_save();

  /// COMPUTE_TIMEOUT:
  /// function that computes the timeout
  double compute_timeout();
  
  int get_smart_auto_save_timeout() const;


private:
  boost::thread auto_save_thread_;
  boost::condition_variable recompute_auto_save_;
  boost::condition_variable auto_save_done_;
  int auto_save_smart_save_timer_;

};







} // end namespace Seg3D
#endif
