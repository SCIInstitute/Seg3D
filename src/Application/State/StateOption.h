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

#include <boost/smart_ptr.hpp>

#include <Application/State/StateBase.h>

namespace Seg3D {

// STATEOPTION:
// This class is a specification of State that is used to hold an option out of
// a list of possible options.

class StateOption;

typedef boost::shared_ptr<StateOption> StateOptionHandle;

class StateOption : public StateBase {

// -- constructor/destructor --
  public:

    // CONSTRUCTOR
    StateOption(const std::string& default_value,
                const std::string& option_list);

    StateOption(const std::string& default_value,
                const std::vector<std::string>& option_list);                
    // DESTRUCTOR
    virtual ~StateOption();

// -- functions for accessing data --

  public:
    // EXPORT_TO_STRING:
    // Convert the contents of the State into a string
    virtual std::string export_to_string() const;
    
    // IMPORT_FROM_STRING:
    // Set the State from a string
    virtual bool import_from_string(const std::string& str,
                                    bool from_interface = false);
                                    
  protected:    
    // EXPORT_TO_VARIANT
    // Export the state data to a variant parameter
    virtual void export_to_variant(ActionParameterVariant& variant) const;
    
    // IMPORT_FROM_VARIANT:
    // Import the state data from a variant parameter.
    virtual bool import_from_variant(ActionParameterVariant& variant,
                                     bool from_interface = false);
          
    // VALIDATE_VARIANT:
    // Validate a variant parameter
    // This function returns false if the parameter is invalid or cannot be 
    // converted and in that case error will describe the error.
    virtual bool validate_variant(ActionParameterVariant& variant, 
                                  std::string& error);
    
    // COMPARE_VARIANT:
    // Compare with variant parameter
    virtual bool compare_variant(ActionParameterVariant& variant);                                    
                                    
// -- signals describing the state --

  public:
    // VALUE_CHANGED_SIGNAL:
    // Signal when the data in the state is changed, the second bool indicates
    // whether the signal was triggered from the interface, in which case it may
    // not need to update the interface.

    typedef boost::signals2::signal<void (std::string, bool)> value_changed_signal_type;
    value_changed_signal_type value_changed_signal;

    // OPTIONLIST_CHANGED_SIGNAL:
    // Signal when the option list is changed
    typedef boost::signals2::signal<void ()> optionlist_changed_signal_type;
    optionlist_changed_signal_type optionlist_changed_signal;    

// -- Functions specific to this type of state --
  public:

    // SET_OPTION_LIST:
    // Set the list of options from which one can choose
    void set_option_list(const std::string& option_list);

    // SET_OPTION_LIST:
    // Set the list of options from which one can choose
    void set_option_list(const std::string& option_list,
                         const std::string& option);
   

    // SET_OPTION_LIST:
    // Set the list of options from which one can choose
    void set_option_list(const std::vector<std::string>& option_list);

    // SET_OPTION_LIST:
    // Set the list of options from which one can choose
    void set_option_list(const std::vector<std::string>& option_list,
                         const std::string& option);    
      
    // OPTION_LIST:
    // Get the option list
    std::vector<std::string> option_list();
    
    // IS_OPTION:
    // Check whether a string is a valid option
    bool is_option(const std::string& option);
    
// -- option list --
  protected:
  
    // Storage for the actual value
    std::string value_;
  
    // List with all the allowed options in lower case
    std::vector<std::string> option_list_;
                                    
};

} // end namespace Seg3D

#endif
