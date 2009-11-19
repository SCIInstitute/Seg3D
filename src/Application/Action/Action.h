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

#ifndef APPLICATION_ACTION_ACTION_H
#define APPLICATION_ACTION_ACTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <map>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Utils includes
#include <Utils/Core/Log.h>

// Application includes
#include <Application/Action/ActionContext.h>
#include <Application/Action/ActionParameter.h>

namespace Seg3D {

class Action;
typedef boost::shared_ptr<Action>        ActionHandle;

// CLASS ACTION:
// Main class that defines an action in the program
// An action is not copyable as that would invalidate 
// the ActionParameter pointers used internally.

class Action : public boost::noncopyable {

// -- Action types/properties --
  public:
    enum action_properties_type {
      // APPLICATION ACTION - For now all actions that are no layer actions are
      // classified as application actions.
      APPLICATION_E = 0x0001,
      
      // LAYER ACTION - Layer actions affect layers/layer manager and are currently
      // the only actions that can be undone.
      LAYER_E = 0x0002,
     
      // UNDOABLE ACTIONS - These actions will be forwarded to the undo/redo
      // stack. 
      UNDOABLE_E = 0x10000, 
      
      // ASYNCHRONOUS ACTIONS - These action do not complete on the main
      // application thread and run while new actions can be issued
      ASYNCHRONOUS_E =0x20000
    };

// -- Constructor/Destructor --
  public:    
    // Construct an action of a certain type and with certain properties
    Action(const std::string& type_name, action_properties_type properties);

    // Virtual destructor for memory management of derived classes  
    virtual ~Action(); // << NEEDS TO BE REIMPLEMENTED

    action_properties_type  properties() const { return properties_; }
    std::string             type_name() const { return type_name_; }

  private:
    action_properties_type  properties_;    
    std::string             type_name_;
    
// -- Run/Validate interface --

  public:

    // VALIDATE:
    // Each action needs to be validated just before it is posted. This way we
    // enforce that every action that hits the main post_action signal will be
    // a valid action to execute.
    // NOTE: If this function is *not* overloaded the function will return true.
    // NOTE: This function is *not* const and may alter the values of the parameters
    //       and correct faulty input. Run on the other hand is not allowed to
    //       change anything in the action, as it is posted to any observers
    //       after the action is validated    
    virtual bool validate(ActionContextHandle& context);  
    
    // RUN:
    // Each action needs to have this piece implemented. It spells out how the
    // action is run. It returns whether the action was successful or not.
    // NOTE: In case of an asynchronous action, the return value is ignored and the
    // program relies on report_done() from the context to be triggered when
    // the asynchronous part has finished. In any other case the ActionDispatcher
    // will issue the report_done() when run returns.
    // NOTE: This function is defined as *const* as parameters cannot be changed
    //       and the action needs to be executed as is, or fail.
    virtual bool run(ActionContextHandle& context) const = 0;

// -- Action parameters --

  public:
    
    // ADD_ARGUMENT:
    // A argument to the action needs to be registered with the base
    // class so we can import and export the arguments to a string.
    // This function links the arguments of the action to an internal
    // record of all the arguments
    template<class ARGUMENT>
    void add_argument(ARGUMENT& argument) { add_argument_ptr(&argument); }
    
    // ADD_PARAMETER:
    // A parameter needs to be registered with the base class
    // so we can import and export the parameters to a string.
    // This function links the parameters of the action to an internal
    // key value pair system to records all the parameters
    template<class PARAMETER>
    void add_parameter(const char *key,PARAMETER& param) {add_parameter_ptr(key,param); }

    // EXPORT_TO_STRING:
    // Export the action command into a string, so it can stored
    // The action factory can recreate the action from this string
    std::string export_to_string() const;

    // IMPORT_FROM_STRING:
    // Import an action command from astring. This function is used by the
    // ActionFactory.
    bool import_from_string(const std::string& action, std::string& error);

  private:

    // IMPLEMENTATION OF ADD_PARAMETER ADN ADD_ARGUMENT
    // These take pointers to the base class, the ones defined above work
    // with references of the parameters for more convenience.
    void add_argument_ptr(ActionParameterBase* param);
    void add_parameter_ptr(const char *key,ActionParameterBase* param);

    // Typedefs
    typedef std::vector<ActionParameterBase*>          argument_vector_type;
    typedef std::map<std::string,ActionParameterBase*> parameter_map_type;
    
    // Vector that stores the required arguments of the action.
    argument_vector_type arguments_;
    
    // Map that stores the location of the parameters in the action.
    parameter_map_type parameters_;
    
};

} // end namespace seg3D

#endif
