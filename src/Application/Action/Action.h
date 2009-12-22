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
#include <vector>
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
typedef boost::shared_ptr<Action>   ActionHandle;
typedef std::vector<ActionHandle>   ActionHandleList;

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

      // INTERFACE ACTION - Interface actions only affect the appearance of the
      // interface. 
      INTERFACE_E = 0x0004,
           
      // UNDOABLE ACTIONS - These actions will be forwarded to the undo/redo
      // stack. 
      UNDOABLE_E = 0x10000, 
      
      // ASYNCHRONOUS ACTIONS - These actions do not complete on the main
      // application thread and run while new actions can be issued
      ASYNCHRONOUS_E = 0x20000,
      
      // COLLAPSEABLE ACTIONS - These actions can be inserted on top of each
      // other. Their undo/redo action consist of only one action in each
      // direction and they can be combined by taking the the last redo and
      // the first undo
      COLLAPSEABLE_E = 0x40000,
      
      // QUERY ACTIONS - These actions do not alter the state of the program
      // but query state. They do not need to be recorded in a playback script.
      QUERY_E = 0x1000000
    };

// -- Constructor/Destructor --
  public:    
    // Construct an action of a certain type and with certain properties
    Action();

    // Virtual destructor for memory management of derived classes  
    virtual ~Action(); // << NEEDS TO BE REIMPLEMENTED

    virtual std::string   type() const = 0;
    virtual std::string   usage() const = 0;
    virtual int           properties() const = 0;

// -- Test properties --

    // IS_UNDOABLE:
    // Check whether the action is undoable
    bool is_undoable() { return (UNDOABLE_E & properties()); }

    // IS_ASYNCHRONOUS:
    // Check whether this action is asynchronously
    bool is_asynchronous() { return (ASYNCHRONOUS_E & properties()); }

    // NEED_UNDO:
    // Test whether an undo needs to be generated
    bool need_undo(ActionContextHandle& context)
    {
      return (is_undoable() && !(context->from_undobuffer()));
    }
    
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
    //       after the action is validated.
    // NOTE: This function supplies a pointer to the handle to itself. As the
    //       handle is generated outside the action. Creating a new handle will
    //       lead to premature deletion of the object, the right handle is
    //       supplied, so the action can archive itself for provenance or
    //       undo buffers.
    virtual bool validate(ActionHandle& self, ActionContextHandle& context);  
    
    // RUN:
    // Each action needs to have this piece implemented. It spells out how the
    // action is run. It returns whether the action was successful or not.
    // NOTE: In case of an asynchronous action, the return value is ignored and the
    // program relies on report_done() from the context to be triggered when
    // the asynchronous part has finished. In any other case the ActionDispatcher
    // will issue the report_done() when run returns.

    virtual bool run(ActionHandle& self, ActionContextHandle& context) = 0;

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
    void add_parameter(const std::string& key,PARAMETER& param) {add_parameter_ptr(key,&param); }

    // ADD_RESULT:
    // Describe where the result is stored
    template<class PARAMETER>
    void add_result(PARAMETER& param) { add_result_ptr(&param); }

    // EXPORT_ACTION_TO_STRING:
    // Export the action command into a string, so it can stored
    // The action factory can recreate the action from this string
    std::string export_action_to_string() const;

    // EXPORT_RESULT_TO_STRING:
    // Export the result into a string, so it can be send back to the user
    std::string export_result_to_string() const;

  protected:
    friend class ActionFactory;

    // IMPORT_ACTION_FROM_STRING:
    // Import an action command from astring. This function is used by the
    // ActionFactory.
    bool import_action_from_string(const std::string& action, std::string& error);

  private:

    // IMPLEMENTATION OF ADD_PARAMETER, ADD_ARGUMENT AND ADD_RESULT
    // These take pointers to the base class, the ones defined above work
    // with references of the parameters for more convenience.
    void add_argument_ptr(ActionParameterBase* param);
    void add_parameter_ptr(const std::string& key,ActionParameterBase* param);
    void add_result_ptr(ActionParameterBase* param);

    // Typedefs
    typedef std::vector<ActionParameterBase*>          argument_vector_type;
    typedef std::map<std::string,ActionParameterBase*> parameter_map_type;
    typedef ActionParameterBase*                       result_type;
    
    // Vector that stores the required arguments of the action.
    argument_vector_type arguments_;
    
    // Map that stores the location of the parameters in the action.
    parameter_map_type parameters_;
    
    // Pointer for where the result is stored
    result_type result_;
    
};

// SCI_ACTION_TYPE:
// Action type should be defined at the top of each action. It renders code that
// allows both the class as well as the Action object to determine what its 
// properties are. By defining class specific static functions the class 
// properties can be queried without instantiating the action. On the other
// hand you want to query these properties from the object as well, even when
// we only have a pointer to the base object. Hence we need virtual functions
// as well. 

// Note: one would expect to use virtual static functions, but those are not
// allowed in C++

#define SCI_ACTION_TYPE(type_string,usage_string,properties_mask) \
  public: \
    static std::string action_type()  { return type_string; }\
    static std::string action_usage() { return usage_string; }\
    static int         action_properties() { return properties_mask; }\
    \
    virtual std::string   type() const  { return action_type(); } \
    virtual std::string   usage() const { return action_usage(); } \
    virtual int           properties() const { return action_properties(); }

} // end namespace seg3D

#endif
