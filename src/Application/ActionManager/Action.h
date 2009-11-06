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

#ifndef APPLICATION_ACTIONMANAGER_ACTION_H
#define APPLICATION_ACTIONMANAGER_ACTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>

#include <Core/Utils/Log.h>
#include <Core/State/State.h>


namespace Seg3D {

// Forward declaration of the Action class
class Action;

// Handle for memory management of the actions
typedef boost::shared_ptr<Action> ActionHandle;


class Action {

// -- Constructor/Destructor --
  public:
  
    // Construct an action of a certain type and with certain flags
    Action(unsigned int type, unsigned int flags);

    // Virtual destructor for memory management of derived classes

    // THIS FUNCTION NEEDS TO BE REIMPLEMENTED IN THE DERIVED CLASS  
    virtual ~Action();

    // The name of the action for the derived class for serialization of the
    // class and for automatic instantiation through the ActionFactory
    // NOTE: this functionality is similar to typeinfo, however typeinfo is
    // implemented differently acrosss compilers, and the goal is to make 
    // session/action information independent of the compiler implementation.
    
    // THIS FUNCTION NEEDS TO BE REIMPLEMENTED IN THE DERIVED CLASS  
    virtual std::string type_name() const = 0;

// -- Action Types --

  public:
    enum {
      // INTERFACE ACTION - Interface actions do not act on the data and do not
      // change the state of application
      INTERFACE_E = 0x0001,

      // APPLICATION ACTION - Fundamental actions like undo and redo inside the
      // application layer
      APPLICATION_E = 0x0002,
      
      // LAYER ACTION - An action that modifies any of the layer states. This
      // value can be used to filter for actions that impact the layer
      LAYER_E = 0x0004,
      
      // TOOL ACTION - An action that affects the state of the 

    };

    // GET_KIND: get the type of the action. The action type can only be set by
    // the constructor, as types do not change
    unsigned int get_type() const { return type_; }

  private:
    unsigned int type_;

// -- Action Properties --
  public:

    enum {
      // Default property
      NONE_E = 0x000,
    
      // UNDOABLE ACTIONS - These actions will be forwarded to the undo/redo
      // stack. Undoable actions need to have an build_undo() and undo() method
      // These functions gather the information for undo and do the actual undo
      UNDOABLE_E = 0x0001,
    
      // UPDATEABLE_E ACTIONS - These actions can be merged in the top slot of
      // the undo/redo stack. Updateable actions allow for the update to be 
      // merged into the previous action and then undoed together with the
      // previous action. This is intended for actions that need feedback and
      // that are undoable.
      UPDATEABLE_E = 0x0002,
      
      // QUEUEABLE_E ACTIONS - Actions that do not finish immediately may hinder
      // the execution of other actions, hence there may be a need to queue an
      // action. Generally action dispatched from the user interface are not
      // queueable and those from a script are queueable. Hence the same action
      // may have both flavors.
      QUEUEABLE_E = 0x0004
      
    };

    // test, set, and clear flags describe properties of the action
    void set_property(unsigned int prop)   { properties_ |= prop; }
    void clear_property(unsigned int prop) { properties_ &= ~prop; }
    bool test_property(unsigned int prop)  { return (properties_ & prop); }

  private:
    unsigned int properties_;    


// -- Do interface --

  public:
    
    // Each action needs to be validated just before it is posted. This way we
    // enforce that every action that hits the main post_action signal will be
    // a valid action to execute.
    // If this function is *not* overloaded the function will return true    
    virtual bool validate();

    // Apply the action. The action is actually performed and the state of the
    // program is altered.

    // THIS FUNCTION NEEDS TO BE REIMPLEMENTED IN THE DERIVED CLASS  
    virtual void do_action() = 0;

  private:
  
    // State variable for storing information for the forward action
    Core::State update_state_;

// -- Undo interface --

  public:
    
    // BUILD_UNDO:
    // Build the undo state information. This function is called once the 
    // the action has been validated and posted, but before the action is actually
    // executed, so it can query the current state and build the right undo
    // information
    virtual void build_undo();
    
    // UNDO_ACTION:
    // Undo an action that has fully been executed.
    virtual void undo_action();

  private:
  
    // State variable for storing information for the backward action
    Core::State rollback_state_;


// -- Update facility --

  public:
    
    // MERGE_ACTION:
    // This function determines and merges the current action with the previous
    // action. This function will handle undo information internally and hence
    // when this function returns true both build_undo() and do_action will not
    // be called. Instead when once two actions have been merged it will call
    // update_action on the original action.
    // If this function is *not* overloaded it will return false
    virtual bool merge_action(ActionHandle& previous);

    // UPDATE_ACTION:
    // In case the action was merged with the previous action, just execute the
    // difference. This function is actually called on the previous action.
    virtual void update_action();
    
// -- Status functions --

  public:
  
    
    
};


} // end namespace seg3D

#endif
