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

/*
 *****************************************************************************
 *
 *   ViewManager.h
 *
 *   ViewManager provides the interface to manage the views in the system.
 *
 *   To perform an action to update a view or views, for instance add a view,
 *   delete a view, change a view's state, use the dispatch_* methods
 *
 *   Also see: View
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      10/26/2009
 *
 *    
 *****************************************************************************
 */

#ifndef APPLICATION_VIEW_VIEWMANAGER_H
#define APPLICATION_VIEW_VIEWMANAGER_H 1

//#ifdef (_MSC_VER) && (_MSC_VER >= 1020)
//# pragma once
//#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

// Application includes
#include <Utils/Singleton/Singleton.h>
#include <Application/View/View.h>
#include <Application/State/StateHandler.h>


namespace Seg3D {

// Forward declarations

// typedefs

class ViewManager : public StateHandler {
public:
  ViewManager();
  ~ViewManager();
  

  // -- Signals for the User Interface --
  typedef boost::signals2::signal<void (ViewHandle)> view_changed_signal_type;

  // CONNECT_VIEW_CHANGED:
  // Connect to the signal that indicates a view has changed
  boost::signals2::connection
  connect_view_changed(view_changed_signal_type::slot_type slot)
  {
    return view_changed_signal_.connect(slot);
  }

  // -- Dispatcher functions for User Interface --
  void dispatch_changeprimaryview(const std::string& view_name) const;
  void dispatch_changeviewtype(const std::string& view_name, View::view_type type) const;
  void dispatch_changeactivelayer(const std::string& view_name, const std::string& layer_name) const;
  void dispatch_changeopacity(const std::string& view_name, const std::string& layer_name, unsigned int opacity) const;
  void dispatch_addlayertoview(const std::string& view_name, const std::string& layer_name) const;
  //void dispatch_newview() const;
  void dispatch_removeview(const std::string& view_name) const;

  const ViewHandle get_view(const std::string& view_name) const;

protected:
  friend class ActionChangePrimaryView;

  // VIEW_CHANGED_SIGNAL:
  // This signal is triggered after a view has been modified
  view_changed_signal_type view_changed_signal_;

// -- Singleton interface --
  public:
    
    // INSTANCE:
    // Get the singleton interface
    static ViewManager* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<ViewManager> instance_;

    typedef boost::unordered_map<std::string, ViewHandle> view_map_type; 
    view_map_type views_;
    boost::mutex view_map_lock_;


}; // class ViewManager

} // end namespace Seg3D

#endif
