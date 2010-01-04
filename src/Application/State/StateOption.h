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

#ifndef APPLICATION_STATE_STATEOPTION_H
#define APPLICATION_STATE_STATEOPTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Application/State/State.h>

namespace Seg3D {

// STATE:
// Class that records the state of a variable

class StateOption;

class StateOption : public State<std::string> {
  public:
    // One cannot define a templated typedef of StateHandle<>,
    // Hence we settle for StateValue<T>::Handle
    typedef boost::shared_ptr<StateOption> Handle;


// -- constructor/destructor --
  public:

    StateOption(const std::vector<std::string>& option_list, 
                const std::string& default_option);
                
    virtual ~StateOption();
    
// -- set/get value --
  public:

    // SET:
    // Set the value in this State variable
    virtual void set(const std::string& option);

    // SET_OPTION_LIST:
    // Set the list of options from which one can choose
    void set_option_list(const std::vector<std::string>& option_list);

    // IS_OPTION:
    // Check whether a string is a valid option
    bool is_option(const std::string& option);

    // IMPORT_FROM_STRING:
    // Set the State from a string
    // NOTE: this does not trigger the value_changed_signal.
    // This function returns whether the conversion was successful
    // As the user may have altered the input the return value of
    // this function needs to be checked.
    virtual bool import_from_string(const std::string& str);
    
// -- action handling --
  public:    
    // VALIDATE_AND_COMPARE_VARIANT:
    // Validate that the data contained in the variant parameter can actually
    // be used and check whether it changed from the current value.
    virtual bool validate_and_compare_variant(ActionParameterVariant& variant, 
                                              bool& changed,
                                              std::string& error) const;

    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.  
    virtual bool import_from_variant(ActionParameterVariant& variant, 
                                     bool trigger_signal = true);

// -- option list --
  protected:
    // List with all the allowed options in lower case
    std::vector<std::string> option_list_;
    
    // Default option which will be inserted if the current option is invalidated
    std::string default_option_;
                                    
};

} // end namespace Seg3D

#endif
