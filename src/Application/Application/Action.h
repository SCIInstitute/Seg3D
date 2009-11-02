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

#ifndef APPLICATION_APPLICATION_ACTION_H
#define APPLICATION_APPLICATION_ACTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>

// Boost includes
#include <boost/shared_ptr.hpp>


namespace Seg3D {

class Action {
  
// -- name tag for persistent IO --
  public:
    static std::string get_classname() { return "Action"; }
    
// -- Constants/Types --
  public:
    typedef enum { INVALID_E = 0,
                   INTERFACE_E,
                   APPLICATION_E,
                   DATALAYER_E };

    typedef enum { UNDOABLE_E = 0x0001 };

    typedef unsigned int kind_type;
    typedef unsigned int flags_type;

// -- Constructor/Destructor --

  public:
    Action(kind_type kind, flags_type flags = 0);
    // Destructor needs to be virtual for derived classes
    virtual ~Action();

// -- Basic interface --
  
    // Each action needs to be validated just before it is posted. This way we
    // enforce that every action that hits the main post_action signal will be
    // a valid action to execute.
    // If this function is *not* overloaded the function will return true
  
    virtual bool validate(std::string& error);
    
    // Some actions work on layers and they may need to wait for completion of
    // a previous action that initiated a asynchronous process. Depending on 
    // the source initiating the action, one may invalidate the action. For 
    // instance a call from the interface is removed this way, as the user can
    // reissue the action when the process has finished. It is canceled as results
    // are not immediate and actions may pile up. In script/replay mode the action
    // is valid and the action needs to be marked at that time in the provenance
    // record, most of the action however will be delayed until the other action
    // finishes with the layers it depends on.
    // If this function is *not* overloaded the function will return true
      
    virtual bool needs_queue(std::string& error);
    
    // Once in the dispatcher the internal function that runs the action will
    // be called. This function always needs to be overloaded with the right
    // actions.
    
    virtual void run() = 0;
    
// -- Properties --   

    // kind describes the kind of action
    void set_kind(kind_type kind) { kind_ = kind; }
    kind_type get_kind() const { return kind_; }
    
    // flags describe properties of the action
    void set_flag(flags_type flag)   { flags_ |= flag; }
    void clear_flag(flags_type flag) { flags_ &= ~flag; }
    bool test_flag(flags_type flag)  { return (flags_ & flag); }

  private:

    // Actions come in different classes, kind tells which one this one is
    kind_type kind_;

    // Actions have different properties such as whether they are undoable 
    flags_type flags_;
};

typedef boost::shared_ptr<Action> ActionHandle;

} // end namespace seg3D

#endif
