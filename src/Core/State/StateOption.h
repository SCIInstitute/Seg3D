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

#ifndef CORE_STATE_STATEOPTION_H
#define CORE_STATE_STATEOPTION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <Core/State/StateBase.h>

namespace Core
{

// STATEOPTION:
/// This class is a specification of State that is used to hold an option out of
/// a list of possible options.

class StateOption;

typedef boost::shared_ptr< StateOption > StateOptionHandle;

class StateOption : public StateBase
{
  // -- type definitions --
public:
  typedef std::string value_type;
  typedef StateOptionHandle handle_type;

  // -- constructor/destructor --
public:

  // CONSTRUCTOR
  StateOption( const std::string& stateid, const std::string& default_value, 
    const std::string& option_list );

  StateOption( const std::string& stateid, const std::string& default_value, 
    const std::vector< std::string >& option_list );
  
  // DESTRUCTOR
  virtual ~StateOption();

  // -- functions for accessing data --

public:
  // EXPORT_TO_STRING:
  /// Convert the contents of the State into a string
  virtual std::string export_to_string() const;

  // EXPORT_LIST_TO_STRING:
  /// Convert the contents of the State into a string
  virtual std::string export_list_to_string() const;

  // IMPORT_FROM_STRING:
  /// Set the State from a string
  virtual bool import_from_string( const std::string& str, ActionSource source =
      ActionSource::NONE_E );

protected:
  // EXPORT_TO_VARIANT
  /// Export the state data to a variant parameter
  virtual void export_to_variant( Variant& variant ) const;

  // IMPORT_FROM_VARIANT:
  /// Import the state data from a variant parameter.
  virtual bool import_from_variant( Variant& variant, 
    ActionSource source = ActionSource::NONE_E );

  // VALIDATE_VARIANT:
  /// Validate a variant parameter
  /// This function returns false if the parameter is invalid or cannot be
  /// converted and in that case error will describe the error.
  virtual bool validate_variant( Variant& variant, std::string& error );

  // -- signals describing the state --
public:
  // VALUE_CHANGED_SIGNAL:
  /// Signal when the data in the state is changed, the second parameter
  /// indicates the source of the change

  typedef boost::signals2::signal< void( std::string, Core::ActionSource ) > value_changed_signal_type;
  value_changed_signal_type value_changed_signal_;

  // OPTIONLIST_CHANGED_SIGNAL:
  /// Signal when the option list is changed
  typedef boost::signals2::signal< void() > optionlist_changed_signal_type;
  optionlist_changed_signal_type optionlist_changed_signal_;

  // -- Functions specific to this type of state --
public:

  // SET_OPTION_LIST:
  /// Set the list of options from which one can choose
  void set_option_list( const std::string& option_list );

  // SET_OPTION_LIST:
  /// Set the list of options from which one can choose
  void set_option_list( const std::string& option_list, const std::string& option );

  // SET_OPTION_LIST:
  /// Set the list of options from which one can choose
  void set_option_list( const std::vector< std::string >& option_list );

  // SET_OPTION_LIST:
  /// Set the list of options from which one can choose
  void set_option_list( const std::vector< std::string >& option_list, const std::string& option );

  // OPTION_LIST:
  /// Get the option list
  std::vector< std::string > option_list() const
  {
      return this->option_list_;
  }

  // IS_OPTION:
  /// Check whether a string is a valid option
  bool is_option( const std::string& option );

  // -- access value --
public:
  // GET:
  /// Get the value of the state variable
  const std::string& get() const
  {
    return this->value_;
  }

  // INDEX:
  /// Get the index of the value
  inline int index() const
  {
    return this->index_;
  }

  // SET:
  /// Set the value of the state variable
  
  /// NOTE: this function by passes the action mechanism and should only be used
  /// to enforce a constraint from another action.
  bool set( const std::string& value, 
    ActionSource source = ActionSource::NONE_E );

  // -- option list --
protected:

  /// Storage for the actual value
  std::string value_;

  /// Index of the current value in the option list
  int index_;

  /// List with all the allowed options in lower case
  std::vector< std::string > option_list_;

  typedef std::vector< std::string >::iterator option_list_iterator_type;

};

} // end namespace Core

#endif
