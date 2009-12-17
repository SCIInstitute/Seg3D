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

#ifndef APPLICATION_ACTIONMANAGER_ACTIONMANAGER_H
#define APPLICATION_ACTIONMANAGER_ACTIONMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/StringUtil.h>
#include <Utils/Singleton/Singleton.h>

// Application includes
#include <Application/Action/Action.h>

namespace Seg3D {

class ActionManager;

class ActionManager : public boost::noncopyable {

// -- Constructor --

  public:
    ActionManager();

// -- dispatcher --
  public:

    void dispatch_undo() const;
    void dispatch_redo() const;

// -- Signal/Slots --

  public:
    
    // CONNECT_TAGS_CHANGED:
    // This signal tells when a new undo/redo action was inserted or removed from
    // the list.
    
    boost::signals2::connection 
      connect_undo_tags_changed(ActionUndoBuffer::tags_changed_type::slot_type slot)
    {
      return (ActionUndoBuffer::Instance()->tags_changed_.connect(slot));
    }

// -- Singleton interface --
  public:
    
    static ActionManager* Instance() { return instance_.instance(); }

  private:
    static Utils::Singleton<ActionManager> instance_;
};

} // end namespace seg3D

#endif
