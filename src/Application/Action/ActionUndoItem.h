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

#ifndef APPLICATION_ACTION_ACTIONUNDOITEM_H
#define APPLICATION_ACTION_ACTIONUNDOITEM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Application includes
#include <Application/Action/Action.h>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>


namespace Seg3D {

// CLASS ACTIONUNDOITEM:
// This class describes an item in the undo buffer and buffers both an undo as
// well as a redo and thereby follows the Command design pattern.

class ActionUndoItem;  
typedef boost::shared_ptr<ActionUndoItem> ActionUndoItemHandle;

class ActionUndoItem {

// -- Constructor --
  public:
                   
    ActionUndoItem(std::string& tag,
                   ActionHandleList& undo_actions,
                   ActionHandle& redo_action);


// -- Item contents --
  public:
    // TAG:
    // Get the undo tag for this action
    std::string& tag() { return (tag_); }
    
    // UNDO_ACTIONS:
    // Get the undo actions to get to the previous state
    std::vector<ActionHandle>& undo_actions() { return undo_actions_; }

    // REDO_ACTION:
    ActionHandle& redo_action() { return redo_action_; }
    
  private:
    // The tag that shows up in the menu
    std::string tag_;
  
    // Note: to undo an action, multiple actions may be grouped together to
    // form a complex undo action. Hence undo is created as a series of actions
    // instead of a single action. 
  
    // Actions needed to undo an action
    ActionHandleList undo_actions_;
    // Actions needed to redo an action
    ActionHandle redo_action_;

};


} // end namespace seg3D

#endif
